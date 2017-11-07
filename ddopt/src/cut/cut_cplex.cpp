/**
 * DD cut generation for CPLEX
 */

#include <cassert>
#include <queue>
#include <stack>
#include <boost/unordered_set.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/any.hpp>
#include "cut_cplex.hpp"
#include "flow_decomp.hpp"
#include "../bdd/bdd.hpp"
#include "../util/graph.hpp"
#include "../util/stats.hpp"

using boost::any_cast;
using boost::unordered_set;


/**
 * Generates a cut for the set of points represented by the given BDD towards the direction of the given point.
 * All input must be in layer space. Store additional information in cut_info unless it is NULL (default).
 */
Inequality* generate_bdd_inequality(BDD* bdd, const vector<double>& x, const vector<double>& interior_point, Options* options,
                                    CutInfo* cut_info /* = NULL */)
{
	Inequality* facet;
	int nvars = bdd->layers.size() - 1;

	Stats stats;
	stats.register_name("cut-time");
	stats.start_timer(0);
	try {

		IloEnv env;
		IloModel model(env);

		IloNumVarArray u(env, nvars);
		IloArray<IloNumVarArray> v(env, nvars+1);

		// Create variables
		for (int i = 0; i < nvars; ++i) {
			u[i] = IloNumVar(env, -IloInfinity, IloInfinity, IloNumVar::Float, ("u" + to_string(i)).c_str());
		}

		for (int i = 0; i < nvars + 1; ++i) {
			int size = bdd->layers[i].size();
			v[i] = IloNumVarArray(env, size);
			for (int j = 0; j < size; ++j) {
				v[i][j] = IloNumVar(env, -IloInfinity, IloInfinity, IloNumVar::Float,
				                    ("v" + to_string(i) + "," + to_string(j)).c_str());
			}
		}

		// Define objective
		IloObjective obj = IloAdd(model, IloMaximize(env));
		for (int i = 0; i < nvars; ++i) {
			obj.setLinearCoef(u[i], x[i] - interior_point[i]);
		}

		// Initialize constraint arrays
		IloArray<IloRangeArray> zero_arc_constrs(env, nvars + 1);
		IloArray<IloRangeArray> one_arc_constrs(env, nvars + 1);
		for (int i = 0; i < nvars + 1; ++i) {
			int size = bdd->layers[i].size();
			zero_arc_constrs[i] = IloRangeArray(env, size);
			one_arc_constrs[i] = IloRangeArray(env, size);
		}

		// Add constraints
		for (int i = 0; i < nvars; ++i) {
			int size = bdd->layers[i].size();
			for (int j = 0; j < size; ++j) {
				Node* zero_node = bdd->layers[i][j]->zero_arc;
				Node* one_node = bdd->layers[i][j]->one_arc;

				if (zero_node != NULL) {
					// cout << i << "," << j << " / " << zero_node->layer << "," << zero_node->id << endl;
					// model.add( v[zero_node->layer][zero_node->id] <= v[i][j] );
					zero_arc_constrs[i][j] = IloRange(env, v[zero_node->layer][zero_node->id] - v[i][j], 0,
					                                  ("a0_" + to_string(i) + "," + to_string(j)).c_str());
					model.add(zero_arc_constrs[i][j]);
				}
				if (one_node != NULL) {
					// cout << i << "," << j << " / " << one_node->layer << "," << one_node->id << endl;
					// model.add( v[one_node->layer][one_node->id] <= v[i][j] - u[i] );
					one_arc_constrs[i][j] = IloRange(env, v[one_node->layer][one_node->id] - v[i][j] + u[i], 0,
					                                 ("a1_" + to_string(i) + "," + to_string(j)).c_str());
					model.add(one_arc_constrs[i][j]);
				}
			}
		}

		// v_s = 1 + u^T interior_point
		IloExpr vs(env);
		vs += 1;
		for (int i = 0; i < nvars; ++i) {
			vs += u[i] * interior_point[i];
		}
		assert(bdd->layers[0].size() == 1);
		model.add(v[0][0] == vs);

		// v_t = 0
		assert(bdd->layers[nvars].size() == 1);
		model.add(v[nvars][0] == 0);

		// Create CPLEX object
		IloCplex cplex(model);
		cplex.setParam(IloCplex::Threads, 1);
		cplex.setParam(IloCplex::AggInd, 100); // Important parameter for efficiency; there is often a lot to aggregate
		// cplex.setOut(env.getNullStream()); // Suppress output

		stats.end_timer(0);
		cout << "Time to construct LP model: " << stats.get_time(0) << endl;
		cout << cplex.getNrows() << " rows and " << cplex.getNcols() << " columns" << endl;
		// cplex.exportModel("cutlp.lp");

		// Solve cut LP
		stats.start_timer(0);
		cplex.solve();
		stats.end_timer(0);

		cout << "Time to solve LP: " << stats.get_time(0) << endl;

		double bound = cplex.getObjValue();
		cout << "Polar opt obj: " << bound << endl;

		// // Debugging info, if unbounded
		// cout << "Status: " << cplex.getStatus() << endl;
		// IloNumArray ray_vals(env);
		// IloNumVarArray ray_vars(env);
		// cplex.getRay(ray_vals, ray_vars);
		// cout << "Ray: ";
		// for (int i = 0; i < ray_vars.getSize(); ++i) {
		//   cout << ray_vars[i] << " " << ray_vals[i] << endl;
		// }
		// cout << endl;

		// Restrict to optimal face and perturb to obtain extreme point of the polar
		if (options->cut_perturbation_iterative) {
			perturb_bdd_cut_iterative(env, cplex, model, u, obj, nvars, x, interior_point);
		} else if (options->cut_perturbation_random) {
			perturb_bdd_cut_random(env, cplex, model, u, obj, nvars, x, interior_point);
		}

		vector<double> coeffs(nvars);
		double rhs;

		// Retrieve left-hand side coefficients: u
		for (int i = 0; i < nvars; ++i) {
			if (cplex.isExtracted(u[i])) {
				coeffs[i] = cplex.getValue(u[i]);
			} else {
				// If u[i] not extracted, this means it was not added to model: objective is zero and not added to constraints
				// Could be a bug, but it also may be the valid scenario of the variable being fixed to zero in the DD
				coeffs[i] = 0;
			}
		}

		// Retrieve right-hand side: 1 + u^T interior
		rhs = 1;
		for (int i = 0; i < nvars; ++i) {
			rhs += coeffs[i] * interior_point[i];
		}

		// Inequality u^T x <= 1 + u^T interior
		facet = new Inequality();
		facet->coeffs = coeffs;
		facet->rhs = rhs;

		// cout << "Relaxation facet generated (normalized), BDD order: ";
		// for( int i = 0; i < nvars; ++i ) {
		//   cout << facet->coeffs[i] / facet->rhs << " ";
		// }
		// cout << "<= 1" << endl;
		// cout << "Relaxation facet generated, BDD order: ";
		// for( int i = 0; i < nvars; ++i ) {
		//   cout << facet->coeffs[i] << " ";
		// }
		// cout << "<= " << facet->rhs << endl;

		double lhs = 0;
		for (int i = 0; i < nvars; ++i) {
			lhs += facet->coeffs[i] * x[i];
			// cout << "coeff = " << facet->coeffs[i] << ", x = " << x[i] << endl;
		}
		cout << "LHS = " << lhs << " / Violation: " << lhs - facet->rhs << endl;

		cout << "Distance = " << get_distance_hyperplane_point(facet, x) << endl;

		// // Debugging info: Assert complementary slackness
		// int n_tight = 0;
		// int n_nzdual = 0;
		// for (int i = 0; i < nvars; ++i) {
		//  	int size = bdd->layers[i].size();
		//  	// cout << "u[" << i << "] = " << cplex.getValue(u[i]) << endl;
		//  	for (int j = 0; j < size; ++j) {
		//  		Node* zero_node = bdd->layers[i][j]->zero_arc;
		//  		Node* one_node = bdd->layers[i][j]->one_arc;
		//  		if (zero_node != NULL) {
		//  			cout << "Dual (" << i << "," << j << ") -0-> (" << zero_node->layer << "," << zero_node->id << "): " << cplex.getDual(zero_arc_constrs[i][j]);
		//  			cout << " / slack: " << cplex.getValue(v[zero_node->layer][zero_node->id]) - cplex.getValue(v[i][j]) << endl;
		//  			assert(DBL_EQ(cplex.getDual(zero_arc_constrs[i][j]), 0) || DBL_EQ(cplex.getValue(v[zero_node->layer][zero_node->id]), cplex.getValue(v[i][j])));
		//  			if (DBL_EQ(cplex.getValue(v[zero_node->layer][zero_node->id]), cplex.getValue(v[i][j])))
		//  				n_tight++;
		//  			if (!DBL_EQ(cplex.getDual(zero_arc_constrs[i][j]), 0))
		//  				n_nzdual++;
		//  		}
		//  		if (one_node != NULL) {
		//  			cout << "Dual (" << i << "," << j << ") -1-> (" << one_node->layer << "," << one_node->id << "): " << cplex.getDual(one_arc_constrs[i][j]);
		//  			cout << " / slack: " << cplex.getValue(v[one_node->layer][one_node->id]) - (cplex.getValue(v[i][j]) - cplex.getValue(u[i])) << endl;
		//  			assert(DBL_EQ(cplex.getDual(one_arc_constrs[i][j]), 0) || DBL_EQ(cplex.getValue(v[one_node->layer][one_node->id]), cplex.getValue(v[i][j]) - cplex.getValue(u[i])));
		//  			if (DBL_EQ(cplex.getValue(v[one_node->layer][one_node->id]), cplex.getValue(v[i][j]) - cplex.getValue(u[i])))
		//  				n_tight++;
		//  			if (!DBL_EQ(cplex.getDual(one_arc_constrs[i][j]), 0))
		//  				n_nzdual++;
		//  		}
		//  		// cout << "v[" << i << "][" << j << "] = " << cplex.getValue(v[i][j]) << endl;
		//  	}
		// }
		// // cout << "Tight arcs = " << n_tight << " / Nonzero duals = " << n_nzdual << endl;

		// Store additional information in cut_info
		if (cut_info != NULL) {
			int bdd_size = bdd->layers.size();
			cut_info->zero_arc_flow.clear();
			cut_info->zero_arc_flow.resize(bdd_size);
			cut_info->one_arc_flow.clear();
			cut_info->one_arc_flow.resize(bdd_size);
			for (int layer = 0; layer < bdd_size; ++layer) {
				int size = bdd->layers[layer].size();
				for (int k = 0; k < size; ++k) {
					Node* node = bdd->layers[layer][k];
					double zero_flow = 0;
					if (node->zero_arc != NULL) {
						zero_flow = cplex.getDual(zero_arc_constrs[layer][k]);
					}
					double one_flow = 0;
					if (node->one_arc != NULL) {
						one_flow = cplex.getDual(one_arc_constrs[layer][k]);
					}
					cut_info->zero_arc_flow[layer].push_back(zero_flow);
					cut_info->one_arc_flow[layer].push_back(one_flow);
				}
			}

			// // Uncomment for debugging purposes
			// print_all_paths_in_flow(bdd, cut_info->zero_arc_flow, cut_info->one_arc_flow);
		}

		obj.end();
		zero_arc_constrs.end();
		one_arc_constrs.end();
		env.end();

	} catch (IloException& ex) {
		cout << "error: " << ex << endl;
		exit(1);
	}

	return facet;
}


void perturb_bdd_cut_iterative(IloEnv env, IloCplex cplex, IloModel model, const IloNumVarArray u, IloObjective obj, int nvars,
	const vector<double>& x, const vector<double>& interior_point)
{
	// cplex.setOut(env.getNullStream()); // Suppress output
	cout << "Before perturbation: ";
	for (int i = 0; i < nvars; ++i) {
		cout << cplex.getValue(u[i]) << " ";
	}
	cout << endl;

	IloExpr obj_expr(env);
	for (int i = 0; i < nvars; ++i) {
		obj_expr += u[i] * (x[i] - interior_point[i]);
	}
	double bound = cplex.getObjValue();
	// model.add( obj_expr == bound );
	model.add(obj_expr <= bound + 1e-5);
	model.add(obj_expr >= bound - 1e-5);
	obj_expr.end();

	for (int k = 0; k < nvars; ++k) {
		for (int i = 0; i < nvars; ++i) {
			obj.setLinearCoef(u[i], 0);
		}
		obj.setLinearCoef(u[k], 1);
		cplex.solve();
		if (k != nvars - 1) {
			cout << "u[" << k << "] = " << cplex.getObjValue() << endl;
			if (DBL_EQ(cplex.getObjValue(), 0)) {
				u[k].setBounds(0, 0);
			} else {
				// u[k].setBounds(cplex.getObjValue(), cplex.getObjValue());
				u[k].setBounds(cplex.getObjValue() - 1e-5, cplex.getObjValue() + 1e-5);
			}
		}
	}

	cout << "After perturbation: ";
	for (int i = 0; i < nvars; ++i) {
		cout << cplex.getValue(u[i]) << " ";
	}
	cout << endl;
}


void perturb_bdd_cut_random(IloEnv env, IloCplex cplex, IloModel model, const IloNumVarArray u, IloObjective obj, int nvars,
	const vector<double>& x, const vector<double>& interior_point)
{
	// Perturb the objective slightly
	cout << "Before perturbation: ";
	for (int i = 0; i < nvars; ++i) {
		cout << cplex.getValue(u[i]) << " ";
	}
	cout << endl;

	IloExpr obj_expr(env);
	for (int i = 0; i < nvars; ++i) {
		obj_expr += u[i] * (x[i] - interior_point[i]);
	}
	double bound = cplex.getObjValue();
	model.add(obj_expr == bound);
	// model.add( obj_expr <= bound + 1e-5 );
	// model.add( obj_expr >= bound - 1e-5 );
	obj_expr.end();

	for (int i = 0; i < nvars; ++i) {
		double pert = ((double) rand() / (double) RAND_MAX - 0.5) * 2 * 1e-4;
		double new_coeff = x[i] - interior_point[i] + pert;
		if (new_coeff < 0) {
			new_coeff = x[i] - interior_point[i] - pert;
		}
		obj.setLinearCoef(u[i], new_coeff);
	}

	cplex.solve();

	cout << "After perturbation: ";
	for (int i = 0; i < nvars; ++i) {
		cout << cplex.getValue(u[i]) << " ";
	}
	cout << endl;
}

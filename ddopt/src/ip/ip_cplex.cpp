/**
 * Main IP modeling and solving
 */

#include <list>
#include <limits>
#include "ip_cplex.hpp"
#include "ip_target_cplex.hpp"
#include "ip_lag_cplex.hpp"
#include "../cut/cut_cplex.hpp"
#include "../cut/flow_decomp.hpp"
#include "../util/stats.hpp"

using namespace std;


void solve_ip(Instance* inst, BDD* bdd, ModelCplex* model_builder, Options* options)
{
	Stats stats;
	stats.register_name("time");

	try {

		IloEnv env;
		IloCplex cplex(env);
		IloModel model(env);
		IloNumVarArray x(env);

		// Create IP model
		model_builder->create_ip_model(env, cplex, bdd, options, model, x);

		// Optional: Add objective cut from BDD
		if (options->bdd_bound_constraint) {
			add_bdd_bound_constraint(env, model, x, bdd, inst);
		}

		// Optional: Add fixed value objective cut from BDD
		if (options->bdd_bound_constraint_val >= 0) {
			IloExpr lhs(env);
			for (int i = 0; i < inst->nvars; ++i) {
				lhs += inst->weights[i] * x[i];
			}
			model.add(lhs <= options->bdd_bound_constraint_val);
		}

		// Optional: Add flow constraints from BDD
		if (options->bdd_flow_constraints) {
			add_bdd_flow_constraints(env, model, x, bdd);
		}

		// Optional: Fix variables that are fixed in the BDD
		if (options->add_trivial_bdd_var_fixing) {
			vector<int> layers_fixed_to_zero;
			vector<int> layers_fixed_to_one;
			bdd->identify_fixed_layers(layers_fixed_to_zero, layers_fixed_to_one);
			cout << "Fixing variables to zero:  ";
			for (int layer : layers_fixed_to_zero) {
				int var_idx = bdd->layer_to_var[layer];
				x[var_idx].setUB(0);
				cout << var_idx << " ";
			}
			cout << endl;
			cout << "Fixing variables to one:  ";
			for (int layer : layers_fixed_to_one) {
				int var_idx = bdd->layer_to_var[layer];
				x[var_idx].setLB(1);
				cout << var_idx << " ";
			}
			cout << endl;
		}

		stats.start_timer(0);

		cplex.extract(model);

		//cplex.exportModel("test.lp");

		// If flow constraints are used (optional), try to preprocess the problem as much as possible
		if (options->bdd_flow_constraints) {
			cplex.setParam(IloCplex::AggInd, 100);
		}

		// time limit & threads
		cplex.setParam(IloCplex::TiLim, 3600);
		cplex.setParam(IloCplex::Threads, 1);

		if (options->root_lp >= 0) {
			cplex.setParam(IloCplex::RootAlg, options->root_lp);
		}

		if (options->stop_after_root) {
			cplex.setParam(IloCplex::NodeLim, 0);
		}

		// Turn off heuristics
		cplex.setParam(IloCplex::FPHeur, -1);
		cplex.setParam(IloCplex::HeurFreq, -1);
		cplex.setParam(IloCplex::RINSHeur, -1);
		cplex.setParam(IloCplex::LBHeur, 0);

		// cplex.setParam(IloCplex::PreInd, 0);
		cplex.setParam(IloCplex::PreLinear, 0); // Always disable this for fair comparison
		cplex.setParam(IloCplex::MIPSearch, CPX_MIPSEARCH_TRADITIONAL); // For fair comparison

		cplex.setParam(IloCplex::Cliques, options->mip_cuts);
		cplex.setParam(IloCplex::Covers, options->mip_cuts);
		cplex.setParam(IloCplex::DisjCuts, options->mip_cuts);
		cplex.setParam(IloCplex::FlowCovers, options->mip_cuts);
		cplex.setParam(IloCplex::FlowPaths, options->mip_cuts);
		cplex.setParam(IloCplex::FracCuts, options->mip_cuts);
		cplex.setParam(IloCplex::GUBCovers, options->mip_cuts);
		cplex.setParam(IloCplex::ImplBd, options->mip_cuts);
		cplex.setParam(IloCplex::LiftProjCuts, options->mip_cuts);
		cplex.setParam(IloCplex::MCFCuts, options->mip_cuts);
		cplex.setParam(IloCplex::MIRCuts, options->mip_cuts);
		cplex.setParam(IloCplex::ZeroHalfCuts, options->mip_cuts);

		InteriorPointSelector* intpt_selector = NULL;

		if (options->generate_cuts && options->limit_ncuts != 0 && bdd != NULL) {
			if (options->cut_lagrangian || options->cut_lagrangian_cb) {
				// Lagrangian cuts
				vector<double> objective(inst->weights, inst->weights + inst->nvars);
				vector<double> obj_layer = bdd->convert_to_layer_space(objective);
				cplex.use(BddLagrangianCutCallback(env, x, bdd, obj_layer, options));
			} else {
				// Target cuts
				InteriorPointSelectorId intpt_id = static_cast<InteriorPointSelectorId>(options->cut_interior_point);
				intpt_selector = get_interior_point_selector_from_id(intpt_id, inst, bdd);
				cplex.use(BddTargetCutCallback(env, x, bdd, intpt_selector, inst, options, false));
			}
		}

		cplex.solve();
		stats.end_timer(0);

		double bound = -1;
		if (options->stop_after_root) {
			bound = cplex.getBestObjValue();
		} else {
			bound = cplex.getObjValue();
		}

		cout << "CPLEX obj: " << bound << endl;
		cout << "CPLEX best obj: " << cplex.getBestObjValue() << endl;
		cout << "Number of nodes: " << cplex.getNnodes() << endl;

		env.end();
		delete intpt_selector;

	} catch (IloException& e) {
		cerr << "Concert exception caught: " << e << endl;
	} catch (...) {
		cerr << "Unknown exception caught" << endl;
	}
}


/** Add BDD flow constraints to the model */
void add_bdd_flow_constraints(IloEnv env, IloModel model, const IloNumVarArray x, BDD* bdd)
{
	// Initialize variables
	int nvars = bdd->nvars();
	IloArray<IloNumVarArray> f_zero(env, nvars + 1); // flow variables for 0-arcs
	IloArray<IloNumVarArray> f_one(env, nvars + 1); // flow variables for 1-arcs
	for (int i = 0; i < nvars + 1; ++i) {
		int size = bdd->layers[i].size();
		f_zero[i] = IloNumVarArray(env, size);
		f_one[i] = IloNumVarArray(env, size);
	}

	// Create variables
	for (int i = 0; i < nvars; ++i) {
		int size = bdd->layers[i].size();
		for (int j = 0; j < size; ++j) {
			Node* zero_node = bdd->layers[i][j]->zero_arc;
			Node* one_node = bdd->layers[i][j]->one_arc;
			if (zero_node != NULL) {
				f_zero[i][j] = IloNumVar(env, 0, 1);
			}
			if (one_node != NULL) {
				f_one[i][j] = IloNumVar(env, 0, 1);
			}
		}
	}

	// Initialize flow constraint array
	IloArray<IloRangeArray> flow_constrs(env, nvars);
	for (int i = 0; i < nvars; ++i) {
		int size = bdd->layers[i].size();
		flow_constrs[i] = IloRangeArray(env, size);
	}

	// Add flow constraints
	for (int i = 0; i < nvars; ++i) {
		int size = bdd->layers[i].size();
		for (int j = 0; j < size; ++j) {
			IloExpr lhs(env);
			Node* node = bdd->layers[i][j];
			Node* zero_node = bdd->layers[i][j]->zero_arc;
			Node* one_node = bdd->layers[i][j]->one_arc;

			// Incoming arcs
			for (Node* zero_ancestor : node->zero_ancestors) {
				lhs += f_zero[zero_ancestor->layer][zero_ancestor->id];
			}
			for (Node* one_ancestor : node->one_ancestors) {
				lhs += f_one[one_ancestor->layer][one_ancestor->id];
			}

			// Outgoing arcs
			if (zero_node != NULL) {
				lhs += -f_zero[i][j];
			}
			if (one_node != NULL) {
				lhs += -f_one[i][j];
			}

			if (i == 0) {
				flow_constrs[i][j] = IloRange(env, -1, lhs, -1);
			} else {
				flow_constrs[i][j] = IloRange(env, 0, lhs, 0);
			}
		}
		model.add(flow_constrs[i]);
	}

	// Add 1-arc constraints: sum of 1-arc flow at level i should be x_i
	IloRangeArray one_arc_constrs(env, nvars);
	for (int i = 0; i < nvars; ++i) {
		int size = bdd->layers[i].size();
		IloExpr lhs(env);
		for (int j = 0; j < size; ++j) {
			if (bdd->layers[i][j]->one_arc != NULL) {
				lhs += f_one[i][j];
			}
		}
		one_arc_constrs[i] = IloRange(env, 0, lhs - x[bdd->layer_to_var[i]], 0);
	}
	model.add(one_arc_constrs);
}


void add_bdd_bound_constraint(IloEnv env, IloModel model, const IloNumVarArray x, BDD* bdd, Instance* inst)
{
	vector<double> weights(inst->weights, inst->weights + inst->nvars);
	vector<int> path_x;
	int nvars = bdd->nvars();
	double rhs = bdd->get_optimal_sol(weights, path_x, true);
	IloExpr lhs(env);
	for (int i = 0; i < nvars; ++i) {
		lhs += inst->weights[i] * x[i];
	}
	model.add(lhs <= rhs);

	cout << "Added objective cut:  ";
	for (int i = 0; i < nvars; ++i) {
		cout << " + " << inst->weights[i] << "<" << x[i].getName() << ">";
	}
	cout << " <= " << rhs;
	cout << endl;
}

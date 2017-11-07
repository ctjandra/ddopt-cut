/**
 * CPLEX target cut callback functions
 */

#include <list>
#include <limits>
#include "ip_target_cplex.hpp"
#include "../cut/cut_cplex.hpp"
#include "../cut/cut_info.hpp"
#include "../cut/flow_decomp.hpp"


IloCplex::Callback BddTargetCutCallback(IloEnv env, IloNumVarArray vars, BDD* bdd, InteriorPointSelector* intpt_selector,
        Instance* inst, Options* options, bool objective_cut)
{
	return (IloCplex::Callback(new(env) BddTargetCutCallbackI(env, vars, bdd, intpt_selector, inst,
	                           options, objective_cut)));
}

// Equivalent to:
// ILOUSERCUTCALLBACK6(BddTargetCutCallback, IloNumVarArray, vars, BDD*, bdd, InteriorPointSelector*, intpt_selector,
//   Instance*, inst, Options*, options, bool, objective_cut)
void BddTargetCutCallbackI::main()
{
	bool normalize_cut = false;

	// Cuts are generated only at the root node
	if (getNnodes() > options->cut_max_depth) {
		return;
	}

	if (options->dd_cuts_after_cplex) {
		// Only generate cuts after cut loop, but if we started to generate user cuts, keep doing that
		if (!isAfterCutLoop() && getNcuts((IloCplex::CutType) CPX_CUT_USER) == 0) {
			return;
		}
	}

	// Limit on number of cuts
	if (options->limit_ncuts > 0 && getNcuts((IloCplex::CutType) CPX_CUT_USER) >= options->limit_ncuts) {

		// Optional: Add objective cut at last cut
		if (options->bdd_bound_constraint_after_cuts) {
			vector<double> weights(inst->weights, inst->weights + inst->nvars);
			vector<int> path_x;
			int nvars = bdd->nvars();
			double rhs = bdd->get_optimal_sol(weights, path_x, true);
			IloExpr lhs(getEnv());
			for (int i = 0; i < nvars; ++i) {
				lhs += inst->weights[i] * vars[i];
			}
			add(lhs <= rhs).end();

			cout << "Added objective cut (RHS " << rhs << ")" << endl;
			// cout << "Added objective cut:  ";
			// for (int i = 0; i < nvars; ++i) {
			//   cout << " + " << inst->weights[i] << "<" << vars[i].getName() << ">";
			// }
			// cout << " <= " << rhs;
			// cout << endl;
		}

		abortCutLoop(); // This aborts future loops
		return;
	}

	Stats stats;
	stats.register_name("bddcut");
	stats.start_timer(0);

	int nvars = bdd->nvars();

	vector<double> x(nvars);
	for (int i = 0; i < nvars; ++i) {
		x[i] = getValue(vars[bdd->layer_to_var[i]]); // Convert x to layer indices
	}

	// Optional: Add weight based on objective to cut direction
	if (DBL_GT(options->cut_obj_weight, 0)) {
		assert(options->cut_obj_weight >= 0 && options->cut_obj_weight <= 1);
		for (int i = 0; i < nvars; ++i) {
			x[i] = (1 - options->cut_obj_weight) * x[i] + options->cut_obj_weight * inst->weights[bdd->var_to_layer[i]];
		}
	}

	// Compute interior point
	vector<double> interior_point = intpt_selector->select();

	CutInfo* cut_info = NULL;
	if (options->cut_flow_decomposition) {
		cut_info = new CutInfo(); // store flow values here to decompose
	}

	// Generate cut
	Inequality* cut = generate_bdd_inequality(bdd, x, interior_point, options, cut_info);

	// Optional: Print flow decomposition from cut
	if (options->cut_flow_decomposition) {
		vector<double> obj_layer(inst->weights, inst->weights + inst->nvars);
		for (int i = 0; i < nvars; ++i) {
			obj_layer[i] = inst->weights[bdd->var_to_layer[i]];
		}
		print_flow_decomposition_stats_cplex(bdd, getModel(), vars, cut_info->zero_arc_flow, cut_info->one_arc_flow, obj_layer);
	}

	cout << "x = ";
	for (int i = 0; i < nvars; ++i) {
		// cout << x[i] << " ";
		cout << getValue(vars[i]) << " ";
	}
	cout << endl;

	// Print cut, normalized by minimum coefficient
	double min_coeff = cut->rhs;
	for (int i = 0; i < nvars; ++i) {
		if (cut->coeffs[bdd->var_to_layer[i]] > 1e-6 && cut->coeffs[bdd->var_to_layer[i]] < min_coeff) {
			min_coeff = cut->coeffs[bdd->var_to_layer[i]];
		}
	}
	for (int i = 0; i < nvars; ++i) {
		double coeff = (double) cut->coeffs[bdd->var_to_layer[i]] / min_coeff;
		if (abs(coeff) < 1e-9) {
			coeff = 0;
		}
		cout << coeff << " ";
	}
	cout << "<= " << cut->rhs / min_coeff << endl;

	// for (int i = 0; i < nvars; ++i) {
	//   cout << cut->coeffs[bdd->var_to_layer[i]] / cut->rhs << " "; // Normalized
	// }
	// cout << "<= 1" << endl;

	// for (int i = 0; i < nvars; ++i) {
	//   cout << cut->coeffs[bdd->var_to_layer[i]] << " ";
	// }
	// cout << "<= " << cut->rhs << endl;

	if (normalize_cut) {
		for (int i = 0; i < nvars; ++i) {
			cut->coeffs[i] /= min_coeff;
			if (abs(cut->coeffs[i]) < 1e-9) {
				cut->coeffs[i] = 0;
			}
		}
		cut->rhs /= min_coeff;
	}

	// Add cut to problem
	double lhs_val = 0;
	IloExpr lhs(getEnv());
	for (int i = 0; i < nvars; ++i) {
		lhs += cut->coeffs[bdd->var_to_layer[i]] * vars[i];
		lhs_val += cut->coeffs[bdd->var_to_layer[i]] * getValue(vars[i]);
	}
	if (lhs_val > cut->rhs + 1e-6) {
		add(lhs <= cut->rhs).end();
		cout << "Cut added, violation: " << lhs_val - cut->rhs << endl;
	} else {
		cout << "Cut discarded, violation: " << lhs_val - cut->rhs << endl;
	}

	stats.end_timer(0);
	cout << "Time to generate cut: " << stats.get_time(0) << endl;

	delete cut;
}

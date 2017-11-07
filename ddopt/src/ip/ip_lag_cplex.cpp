/**
 * CPLEX cut callback functions via Lagrangian relaxation
 */

#include "ip_lag_cplex.hpp"

IloCplex::Callback BddLagrangianCutCallback(IloEnv env, IloNumVarArray vars, BDD* bdd, const vector<double>& obj_layer, Options* options)
{
	return (IloCplex::Callback(new(env) BddLagrangianCutCallbackI(env, vars, bdd, obj_layer, options)));
}


// Equivalent to:
// ILOUSERCUTCALLBACK4(BddLagrangianCutCallback, IloNumVarArray, vars, BDD*, bdd, const vector<double>&, obj_layer, Options*, options)
void BddLagrangianCutCallbackI::main()
{
	int nvars = bdd->nvars();

	// Root node only
	if (getNnodes() > 0) {
		return;
	}

	// Generate cuts only after CPLEX's cuts
	if (!isAfterCutLoop()) {
		return;
	}

	// Limit on number of cuts
	if (options->limit_ncuts > 0 && getNcuts((IloCplex::CutType) CPX_CUT_USER) >= options->limit_ncuts - 1) {
		abortCutLoop();
	}

	Stats stats;
	stats.register_name("bddcut");
	stats.start_timer(0);

	vector<double> x(nvars);
	for (int i = 0; i < nvars; ++i) {
		x[i] = getValue(vars[bdd->layer_to_var[i]]); // Convert x to layer indices

		cout << x[i] << " ";
	}
	cout << endl;

	Inequality* cut;
	if (options->cut_lagrangian_cb) {
		cut = generate_lagrangian_cut_cb(bdd, x, obj_layer, 200, 0);
	} else {
		cut = generate_lagrangian_cut_subgradient(bdd, x, obj_layer, 200, 0);
	}

	if (cut == NULL) {
		return;
	}

	// Print cut
	for (int i = 0; i < nvars; ++i) {
		cout << cut->coeffs[bdd->var_to_layer[i]] << " ";
	}
	cout << "<= " << cut->rhs << endl;

	// Add cut
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


/* Simple implementation of a subgradient algorithm
 * ------------------------------------------------
 *
 * Let x* be the optimal LP solution we want to cut off.
 * At each iteration k, generate the tightest cut with coefficients lambda(k). Stop when a cut cuts off x*.
 * Initialize lambda with the objective vector.
 * Update lambda(k+1) to lambda(k) + (x* - x(k)) / k, where x(k) is the optimal solution for lambda(k).
 */

Inequality* generate_lagrangian_cut_subgradient(BDD* bdd, const vector<double>& x, const vector<double>& obj_layer,
        int iteration_limit, int iterations_beyond_validity)
{
	vector<double> coeffs;
	int nvars = bdd->nvars();

	coeffs = obj_layer; // initialize (copy) from objective

	bool print_distance_cut_off = true;

	int cut_iterations = 0;
	for (int k = 0; k < iteration_limit; ++k) {
		vector<int> path_x(nvars);
		double lr;

		// cout << "Iteration " << k << ": ";
		// for (int i = 0; i < nvars; ++i) {
		// 	// cout << coeffs[i] << " ";
		// 	cout << coeffs[bdd->var_to_layer[i]] << " ";
		// }
		// cout << endl;

		double rhs = bdd->get_optimal_path(coeffs, path_x, true);

		// cout << "Path: ";
		// for (int i = 0; i < nvars; ++i) {
		// 	// cout << path_x[i] << " ";
		// 	cout << path_x[bdd->var_to_layer[i]] << " ";
		// }
		// cout << endl;

		double activity = 0;
		for (int i = 0; i < nvars; ++i) {
			activity += coeffs[i] * x[i];
		}
		lr = activity - rhs;
		cout << "LR: " << lr << endl;

		if (print_distance_cut_off) {
			Inequality* ineq = new Inequality();
			ineq->coeffs = coeffs;
			ineq->rhs = rhs;
			double distance_cut_off = get_distance_hyperplane_point(ineq, x);
			cout << "Distance cut off: " << distance_cut_off << endl;
		}

		if (activity > rhs + 1e-5) {

			if (cut_iterations >= iterations_beyond_validity) {
				Inequality* cut = new Inequality();
				cut->coeffs = coeffs;
				cut->rhs = rhs;
				return cut;
			}

			cut_iterations++;
		}

		for (int i = 0; i < nvars; ++i) {
			coeffs[i] += (x[i] - path_x[i]) / ((double)(k + 1));
		}
	}

	return NULL;
}


#ifndef USE_CONICBUNDLE

Inequality* generate_lagrangian_cut_cb(BDD* bdd, const vector<double>& x, const vector<double>& obj_layer,
                                       int iteration_limit, int iterations_beyond_validity)
{
	cout << "Error: ConicBundle not compiled" << endl;
	exit(1);
}

#else

Inequality* generate_lagrangian_cut_cb(BDD* bdd, const vector<double>& x, const vector<double>& obj_layer,
                                       int iteration_limit, int iterations_beyond_validity)
{
	CBSolver solver;
	LagrangianCutSubproblemCB lsp(bdd, x);
	int nvars = bdd->nvars();

	DVector lb(nvars);
	DVector ub(nvars);
	for (int i = 0; i < nvars; ++i) {
		// lb[i] = CB_minus_infinity;
		// ub[i] = CB_plus_infinity;
		lb[i] = -1;
		ub[i] = +1;
	}

	solver.init_problem(nvars, &lb, &ub);
	solver.add_function(lsp);

	// Initialize Lagrange multipliers with original objective
	solver.set_new_center_point(obj_layer);

	solver.set_out(&cout, 1);
	// solver.set_term_relprec(1e-5);
	solver.set_term_relprec(1e-7);
	// solver.set_scaling(true);

	Stats stats;
	stats.register_name("cb");

	double time_limit = 60;
	stats.start_timer(0);
	int it;
	for (it = 0; it < iteration_limit; it++) {
		solver.do_descent_step();

		if (solver.termination_code() != 0) {
			cout << "ConicBundle finished: ";
			solver.print_termination_code(cout);
			break;
		}

		if (time_limit >= 0 && stats.get_current_time(0) >= time_limit) {
			cout << "ConicBundle finished: Time limit (" << time_limit << "s)" << endl;
			break;
		}

		// DVector center;
		// solver.get_center(center);
		// cout << "Center:  ";
		// for (int i = 0; i < nvars; ++i) {
		// 	cout << center[i] << " ";
		// }
		// cout << endl;
	}
	stats.end_timer(0);

	// Output
	cout << endl;
	cout << "Number of outer iterations: " << it << endl;
	cout << "Number of inner iterations: " << lsp.get_number_evaluations() << endl;
	cout << "Dual bound: " << solver.get_objval() << endl;
	// cout << "Primal (wrt Lagrangian) bound: " << primal_value << endl;;
	cout << "Total master time: " << stats.get_time(0) - lsp.get_time() << endl;
	cout << "Total subproblem time: " << lsp.get_time() << endl;

	DVector center;
	solver.get_center(center);
	vector<int> path_x(nvars);
	double rhs = bdd->get_optimal_path(center, path_x, true);

	Inequality* cut = new Inequality();
	cut->coeffs = center;
	cut->rhs = rhs;
	return cut;
}

#endif
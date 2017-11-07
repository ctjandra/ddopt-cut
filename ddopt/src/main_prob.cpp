/** 
 * Problem-specific main processing
 */

#include "main_prob.hpp"

#include "core/solver.hpp"
#include "core/orderings.hpp"
#include "core/mergers.hpp"
#include "util/stats.hpp"
#include "ip/intpt_selector.hpp"

#ifdef SOLVER_CPLEX
#include "ip/ip_cplex.hpp"
#endif

// Independent set headers
#include "problem/indepset/indepset_instance.hpp"
#include "problem/indepset/indepset_problem.hpp"
#include "problem/indepset/indepset_mergers.hpp"
#include "problem/indepset/indepset_orderings.hpp"
#ifdef SOLVER_CPLEX
#include "problem/indepset/indepset_model_cplex.hpp"
#endif

// Binary problem headers
#include "problem/bp/bp_instance.hpp"
#include "problem/bp/bp_problem.hpp"
#include "problem/bp/bp_mergers.hpp"
#include "problem/bp/bp_orderings.hpp"
#include "problem/bp/prop_linearcons.hpp"
#ifdef SOLVER_CPLEX
#include "problem/bp/bp_reader_cplex.hpp"
#include "problem/bp/bp_model_cplex.hpp"
#endif

#define DEFAULT_INDEPSET_ORDERING 8
#define DEFAULT_INDEPSET_MERGING 1
#define DEFAULT_BP_ORDERING 4
#define DEFAULT_BP_MERGING 1



/** Main processing for an independent set problem */
void main_indepset(int order_n, int merge_n, string instance_path, string instance_filename, bool skip_dd, bool dd_only,
	Options& options)
{
	// Read independent set instance and create problem
	IndepSetInstance* inst = new IndepSetInstance();
	inst->read_DIMACS(instance_path.c_str());
	IndepSetProblem* problem = new IndepSetProblem(inst, &options);

	cout << "\n\n*** Independent set - " << instance_filename << " ***" << endl;
	cout << "\twidth: " << options.width << endl;

	srand(0);

	// Assign ordering and merger
	if (order_n < 0) {
		order_n = DEFAULT_INDEPSET_ORDERING;
	}
	if (merge_n < 0) {
		merge_n = DEFAULT_INDEPSET_MERGING;
	}
	Ordering* ordering = get_ordering_by_id_indepset(order_n, inst, options);
	if (ordering == NULL) {
		cout << "Error: invalid variable ordering" << endl;
		exit(1);
	}
	Merger* merger = get_merger_by_id_indepset(merge_n, options.width);
	if (merger == NULL) {
		cout << "Error: invalid merging scheme" << endl;
		exit(1);
	}
	problem->ordering = ordering;
	problem->merger = merger;

	// Construct DD if required
	BDD* bdd = NULL;
	if (!skip_dd) {
		Stats stats;
		stats.register_name("time-bdd");
		stats.start_timer(0);
	
		DDSolver solver(problem, &options);

		bdd = solver.construct_decision_diagram();
		assert(bdd->integrity_check()); // Sanity checks on debug mode

		stats.end_timer(0);

		cout << endl;
		cout << endl << "Upper bound: " << bdd->bound << " - width: " << solver.final_width << endl;
		cout << "Time to build BDD: " << stats.get_time(0) << endl;
	}

	// Set default interior point
	if (options.cut_interior_point < 0) {
		options.cut_interior_point = INTPT_INDEPSET;
	}

	if (!dd_only) {
#ifdef SOLVER_CPLEX
		IndepSetOptions indepset_options;
		IndepSetModelCplex model_builder(inst, &indepset_options);
		solve_ip(inst, bdd, &model_builder, &options);
#else
		cout << endl;
		cout << "Error: Compilation was done without CPLEX; cannot run IP model" << endl;
#endif
	}

	delete problem;
	delete inst;
	delete bdd;
}


#ifdef SOLVER_CPLEX

void main_bp(int order_n, int merge_n, string instance_path, string instance_filename, bool skip_dd, bool dd_only,
	Options& options)
{
	/* binary program */
	BPInstance* inst = read_bp_instance_cplex_mps(instance_path);
	for (BPRow* row : inst->rows) {
		cout << *row << endl;
	}

	// Create binary problem
	vector<BPProp*> props;
	props.push_back(new BPPropLinearcons(inst->rows));
	BinaryProblem problem(inst, props, &options);

	// Assign ordering and merger
	if (order_n < 0) {
		order_n = DEFAULT_BP_ORDERING;
	}
	if (merge_n < 0) {
		merge_n = DEFAULT_BP_MERGING;
	}
	Ordering* ordering = get_ordering_by_id_bp(order_n, inst, options);
	if (ordering == NULL) {
		cout << "Error: invalid variable ordering" << endl;
		exit(1);
	}
	Merger* merger = get_merger_by_id_bp(merge_n, options.width);
	if (merger == NULL) {
		cout << "Error: invalid merging scheme" << endl;
		exit(1);
	}
	problem.ordering = ordering;
	problem.merger = merger;

	BDD* bdd = NULL;
	if (!skip_dd) {
		Stats stats;
		stats.register_name("time-bdd");
		stats.start_timer(0);

		DDSolver solver(&problem, &options);

		bdd = solver.construct_decision_diagram();
		assert(bdd->integrity_check()); // Sanity checks on debug mode

		stats.end_timer(0);

		cout << endl;
		if (bdd == NULL) {
			cout << "Bound: Infeasible" << endl;
		} else {
			cout << "Bound: " << bdd->bound << endl;
		}
		cout << "Width: " << solver.final_width << endl;
		cout << "Time to construct BDD: " << stats.get_time(0) << endl;
	}

	// Set default interior point
	if (options.cut_interior_point < 0) {
#ifdef USE_GMP
		options.cut_interior_point = INTPT_DDCENTER;
#else
		if (options.limit_ncuts == -1 || options.limit_ncuts > 0) {
			cout << endl;
			cout << "Error: Compilation was done without GMP. The default interior point for cuts from binary programs is the center" << endl;
			cout << "       of a decision diagram, which requires GMP. A different interior point must be set with --cut-intpt." << endl;
		}
		delete inst;
		delete bdd;
		exit(1);
#endif
	}

	if (!dd_only) {
		BPModelCplex model_builder(instance_path);
		solve_ip(inst, bdd, &model_builder, &options);
	}

	delete inst;
	delete bdd;
}

#else

void main_bp(int order_n, int merge_n, string instance_path, string instance_filename, bool skip_dd, bool dd_only,
	Options& options)
{
		cout << "Error: Compilation was done without CPLEX; cannot read MPS file" << endl;
}

#endif

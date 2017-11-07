/**
 * Global options
 */

#ifndef OPTIONS_HPP_
#define OPTIONS_HPP_

#include <string>

using namespace std;

struct Options {

	// Options on what should be run
	bool   generate_cuts                        = true;    /**< generates cuts from DDs; false is equivalent to setting limit_ncuts to zero */

	// General IP options (unrelated to DDs)
	int    mip_cuts                             = -1;      /**< setting for MIP cuts, following CPLEX settings (-1: disabled, 0: automatic, 2: aggressive) */
	bool   stop_after_root                      = false;   /**< stop solving after done with the root node */
	int    root_lp                              = -1;      /**< root LP algorithm */

	// DD cut options
	int    limit_ncuts                          = 0;       /**< limit on number of cuts to generate, -1 means no limit */
	bool   bdd_bound_constraint                 = false;   /**< add objective constraint from DD bound (CPLEX only) */
	bool   bdd_bound_constraint_after_cuts      = false;   /**< add objective constraint from DD bound after DD cuts are generated (CPLEX only) */
	int    bdd_bound_constraint_val             = -1;      /**< add a given objective constraint for testing purposes (CPLEX only) */
	bool   bdd_flow_constraints                 = false;   /**< add flow constraints directly to problem (CPLEX only) */
	bool   add_trivial_bdd_var_fixing           = false;   /**< detect variables fixed by BDD and fix them in IP model (CPLEX only) */
	bool   dd_cuts_after_cplex                  = true;    /**< generate cuts after CPLEX has generated its cuts (rather than before) */

	bool   cut_perturbation_random              = false;   /**< perturb cut randomly in order to help increase dimension of face */
	bool   cut_perturbation_iterative           = false;   /**< perturb cut iteratively in order to help increase dimension of face */
	int    cut_max_depth                        = 0;       /**< maximum depth in which cuts are generated */
	bool   cut_lagrangian                       = false;   /**< use the Lagrangian method to generate cuts (CPLEX only) */
	bool   cut_lagrangian_cb                    = false;   /**< use the Lagrangian method with ConicBundle to generate cuts (CPLEX only) */
	bool   cut_flow_decomposition               = false;   /**< run flow decomposition after generating a cut */
	int    cut_interior_point                   = -1;      /**< choice of interior point to select for target cut */
	double cut_obj_weight                       = 0;       /**< weight of objective in cut direction */

	// DD construction
	int    width                                = -1;      /**< width limit for relaxation (-1 means unrestricted) */
	bool   use_long_arcs                        = true;    /**< whether long arcs should be used in the DD */
	string fixed_order_filename                 = "fixed_order.txt";  /**< input file for a fixed order for the DD */
	double order_rand_min_state_prob            = 0.8;     /**< probability for the randomized min in state ordering */
	bool   delete_old_states                    = true;    /**< free states from nodes of previous layers to reduce memory usage */

	// Output options
	bool   quiet                                = false;   /**< do not output DD construction information */

};


#endif /* OPTIONS_HPP_ */

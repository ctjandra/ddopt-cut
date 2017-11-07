/**
 * Flow decomposition methods
 */

#ifndef FLOW_DECOMP_HPP_
#define FLOW_DECOMP_HPP_

#include <vector>
#include <cassert>
#include <map>
#include "../bdd/bdd.hpp"

#ifdef SOLVER_CPLEX
#include <ilcplex/ilocplex.h>
#endif

/** Optimality tolerance of the solver (1e-6 is CPLEX default); hardcoded for convenience but ideally should get from solver */
#define OPT_TOL 1e-6


/**
 * Decompose a feasible flow into paths. At each iteration, we take the lexicographically smallest path with positive flow and
 * remove it from the flow vectors. The weight of this path is stored in path_weights. The given vector flow_vals is destroyed.
 */
void decompose_paths_from_flow(BDD* bdd, vector<vector<double>>& zero_arc_flow, vector<vector<double>>& one_arc_flow,
                               vector<vector<int>>& paths,	vector<double>& path_weights);

/**
 * Decompose a feasible flow into paths. At each iteration, we take the path with largest weight and positive flow and
 * remove it from the flow vectors. The weight of this path is stored in path_weights. The given vector flow_vals is destroyed.
 */
void decompose_paths_from_flow(BDD* bdd, const vector<double>& weights, vector<vector<double>>& zero_arc_flow,
                               vector<vector<double>>& one_arc_flow, vector<vector<int>>& paths, vector<double>& path_weights);

/**
 * Extract the lexicographically smallest path with positive flow, without removing it. Return the weight of the path.
 * If flow is zero, path is set to all zeroes and zero is returned.
 * Long arcs are assumed to be the form of (0,0,..,0) or (1,0,..,0).
 */
double extract_lexmin_path_from_flow(BDD* bdd, const vector<vector<double>>& zero_arc_flow,
                                     const vector<vector<double>>& one_arc_flow, vector<int>& path);

/** Find optimal path with positive flow w.r.t. coefficients. */
double extract_optimal_path_from_flow(BDD* bdd, const vector<double>& weights, const vector<vector<double>>& zero_arc_flow,
                                      const vector<vector<double>>& one_arc_flow, vector<int>& optimal_path);

/** Remove a given feasible path from flow vectors */
void remove_path_from_flow(BDD* bdd, vector<vector<double>>& zero_arc_flow, vector<vector<double>>& one_arc_flow,
                           const vector<int>& path, double path_weight);

/** Print paths with positive flow via depth-first search */
void print_all_paths_in_flow(BDD* bdd, vector<vector<double>>& zero_arc_flow, vector<vector<double>>& one_arc_flow);

/** Convert a path to a vector of nodes, assuming it forms a path; nodes before first or after last are considered -1. */
vector<int> bdd_convert_node_vector_to_path(BDD* bdd, const vector<Node*>& nodes);

#ifdef SOLVER_CPLEX

/** Print information on flow decomposition for a cut */
void print_flow_decomposition_stats_cplex(BDD* bdd, IloModel model, IloNumVarArray vars,
        vector<vector<double>>& zero_arc_flow, vector<vector<double>>& one_arc_flow,
        const vector<double>& obj_layer);

/** Check if a given path (in layer space) is feasible for a model (vars[i] has value sol[i]) */
bool is_feasible_cplex(BDD* bdd, const vector<int>& path, const map<int,int>& var_id_to_var, IloModel model);

#endif

#endif // FLOW_DECOMP_HPP_

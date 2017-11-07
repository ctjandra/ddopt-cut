/**
 * Main decision diagram construction
 */

#ifndef SOLVER_HPP_
#define SOLVER_HPP_

#define EXACT_BDD -1

#include "../bdd/bdd.hpp"
#include "../bdd/nodedata.hpp"
#include "../problem/problem.hpp"
#include "../problem/state.hpp"
#include "../util/options.hpp"
#include "solver_callback.hpp"

#include <vector>
#include <map>
#include <queue>

using namespace std;

class DDSolver
{
public:
	Problem*                      problem;                     /**< problem-specific functions and data */
	int                           nlayers;                     /**< number of layers in the DD */

	BDD*                          final_bdd;                   /**< decision diagram */
	int                           final_width;                 /**< final width of DD after construction */

	bool                          use_primal_pruning;          /**< if true, enables pruning with primal bound */
	double                        primal_bound;                /**< primal bound used for pruning; only used if use_primal_pruning is true */

	NodeDataMap*                  initial_node_data;           /**< initial node data */

	DDSolverCallback*             solver_callback;             /**< special solver callback for specific situations */

	Options*                      options;                     /**< options */

	DDSolver(Problem* _problem, Options* options);

	/** Construct a relaxed DD */
	BDD* construct_decision_diagram();

	/** Construct a relaxed DD starting at an initial state */
	BDD* construct_decision_diagram_at_state(State* initial_state, double initial_longest_path);

	/** Set a primal bound for possible pruning */
	void set_primal_bound(double bound);

	/** Check if node can be pruned due to the primal bound */
	bool node_can_be_pruned_by_primal_bound(Problem* prob, Node* node, Node* parent);

	/** Add NodeData to root node (key is used to recover this node_data) */
	void add_initial_node_data(string key, NodeData* node_data);

private:

	/** Merge terminal nodes if there is more than one at the end */
	Node* merge_terminal_nodes(NodeMap& terminal_node_list);
};

#endif /* SOLVER_HPP_ */

/** 
 * Main decision diagram structure
 */

#ifndef BDD_HPP_
#define BDD_HPP_

#include <map>
#include <vector>
#include "bdd_node.hpp"
#include "../util/util.hpp"
#include "../problem/state.hpp"

using namespace std;

#define DD_NODE_ID_OPEN -1


/** Decision diagram structure */
class BDD
{
public:

	vector<vector<Node*>> layers;       /**< List of nodes of the diagram per layer */
	vector<int> layer_to_var;           /**< layer_to_var[k] is the index of the variable at layer k */
	vector<int> var_to_layer;           /**< var_to_layer[k] is the layer of the k-th variable */

	double bound;                       /**< bound obtained at construction */
	bool constructed;                   /**< if false, BDD is in the middle of being constructed */


	int nvars()
	{
		return layers.size() - 1; // Number of vars, or equivalently number of layers minus one
	}

	BDD() : constructed(false) {}

	~BDD()
	{
		for (vector< vector<Node*> >::iterator itl = layers.begin(); itl != layers.end(); ++itl) {
			for (vector<Node*>::iterator it = (*itl).begin(); it != (*itl).end(); ++it) {
				delete *it;
			}
		}
	}


	// Informational functions

	/** Count number of nodes in BDD */
	int count_number_of_nodes();

	/** Count number of arcs in BDD */
	int count_number_of_arcs();

	/** Compute width of BDD */
	int get_width();

	/** Get root node layer (the first nonempty layer). This is typically 0 except in special cases. */
	int get_root_layer();

	/** Return the root node of the decision diagram */
	Node* get_root_node();

	/** Get terminal node layer (the last nonempty layer). This is typically nvars except in special cases. */
	int get_terminal_layer();

	/** Return the terminal node of the decision diagram */
	Node* get_terminal_node();

	/** Print decision diagram. Optionally use global node id instead of layer id, or add [DD] tag for easy identification. */
	void print(bool use_global_id = false, bool print_tag = false);


	// Node manipulation functions

	/** Create a featureless node (without State, longest_path, etc.) and add it to BDD */
	Node* create_node(int layer);

	/** Create a new node with the same layer and children as given node. No other features are affected, including parents. */
	Node* duplicate_node(Node* node);

	/** Remove a single node from BDD and delete it. */
	void remove_node(Node* node);

	/**
	 * Merges nodes that have the same outgoing arcs. Unlike Node::merge, this ignores states and data, so this is typically
	 * used for fully constructed BDDs.
	 */
	void merge_nodes(Node* node, Node* node_to_remove);

	/** Remove all nodes that do not have children from bottom to top */
	void remove_childless_nodes();

	/** Remove all nodes that do not have parents from top to bottom */
	void remove_parentless_nodes();

	/** Remove all nodes that are not in a path from root to terminal */
	void remove_pathless_nodes();


	// Computation of properties

	/**
	 * Stores in optimal_path the path of maximum weight using as weights coeffs for 1-arcs.
	 * The difference between this and get_optimal_sol is that everything is in the layer space rather than
	 * the variable space, including weights and the output solution. Returns total weight.
	 */
	double get_optimal_path(vector<double> coeffs_layer, vector<int>& optimal_path, bool maximize,
	                            bool ignore_relaxed_nodes = false);

	/**
	 * Stores in optimal_sol the solution of maximum weight using as weights coeffs for 1-arcs.
	 * The difference between this and get_optimal_path is that everything is in the variable space rather than
	 * the layer space, including weights and the output solution. Returns total weight.
	 */
	double get_optimal_sol(vector<double> coeffs_var, vector<int>& optimal_sol, bool maximize,
	                           bool ignore_relaxed_nodes = false);

	/**
	 * Stores in optimal_path the path of maximum or minimum weight using as weights
	 * zero_coeffs for 0-arcs and one_coeffs for 1-arcs. Returns total weight.
	 * Weights must be in terms of layers, not variables.
	 */
	double get_optimal_path_zero_one_coeffs(vector<double> zero_coeffs, vector<double> one_coeffs,
	        vector<int>& optimal_path, bool maximize, bool ignore_relaxed_nodes = false);

	/** Compute the center of a BDD */
	void get_center(vector<double>& center); // Requires GMP

	/** Identify layers that only have 0-arcs or only have 1-arcs */
	void identify_fixed_layers(vector<int>& layers_fixed_to_zero, vector<int>& layers_fixed_to_one);


	// Conversion functions

	/** Convert a vector from (problem) variable space to (DD) layer space */
	vector<double> convert_to_layer_space(const vector<double>& v);

	/** Convert a vector from (DD) layer space to (problem) variable space */
	vector<double> convert_to_var_space(const vector<double>& v);


	// Integrity check functions

	/** 
	 * Returns true if the BDD is valid: correct layers/ids and no nodes without parents or children 
	 * (except root and terminal respectively).
	 */
	bool integrity_check();

	/** Checks if all node data is empty. */
	bool empty_data_check();

private:

	/** Remove a node from BDD without updating arcs. (Internal use.) */
	void remove_node_no_arcs(Node* node);

	/** Compute value of a path */
	double compute_path_value(vector<double> zero_coeffs, vector<double> one_coeffs, vector<int>& path);
};


/** Comparator to dereference pointers to state (pointers in NodeMap are necessary because State is abstract) */
struct LessThanStatePointers {
	bool operator()(const State* lhs, const State* rhs) const
	{
		return *lhs < *rhs;
	}
};

typedef map<State*, Node*, LessThanStatePointers> NodeMap;


/**
 * Node comparator by longest path (decreasing order)
 */
struct CompareNodesLongestPath {
	bool operator()(const Node* nodeA, const Node* nodeB) const
	{
		if (DBL_EQ(nodeA->longest_path, nodeB->longest_path)) {
			return 0;
		}
		return nodeA->longest_path > nodeB->longest_path;
	}
};


#endif /* BDD_HPP_ */

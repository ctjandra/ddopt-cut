/**
 * Node structure for decision diagrams
 */

#ifndef BDD_NODE_HPP_
#define BDD_NODE_HPP_

#include <vector>
#include <boost/any.hpp>
#include "../problem/state.hpp"

using namespace std;


class NodeDataMap; // forward declaration for data in Node


/** Node of a decision diagram */
class Node
{
public:

	int             layer;
	int             id;                   /**< bdd[node->layer][node->id] == node (only after construction is done) */
	int             global_id;            /**< layer-independent identifier */

	State*          state;
	double          longest_path;

	vector<Node*>   one_ancestors;        /**< parents of node connected by a 1-arc */
	vector<Node*>   zero_ancestors;       /**< parents of node connected by a 0-arc */

	Node*           one_arc;              /**< 0-arc child */
	Node*           zero_arc;             /**< 1-arc child */

	// Auxiliary variables for finding longest paths in the BDD out of the construction (or any situation where one needs to trace back).
	// These are temporary in nature and must be manually initialized before use (always assume these are dirty).
	double          lp_value;
	Node*           lp_parent;
	int             lp_parent_arctype; // 0 or 1

	// User data stored in nodes:
	// - temp_data is used for temporary space, not assumed to be clean, and is independent from construction;
	//     it is only used after construction except for very specific cases, typically to gather information
	// - data is computed and used throughout construction and may affect the final decision diagram, such as marking nodes as infeasible;
	//     thus it requires knowing what to do when nodes are merged (whether due to equivalence or relaxation)
	boost::any      temp_data;            /**< Temporary user data attached to a node. Allocating, ensuring no concurrent use, and
                                          *  cleaning is of responsibility of the user. */
	NodeDataMap*    data;                 /**< List of user data stored in a node */

	bool            relaxed_node;         /**< indicates whether this node was merged for relaxation */


	/**
	 * Node constructor if one wishes only to create a relaxation
	 */
	Node(State* _state, double _longest_path, NodeDataMap* _data) : state(_state), longest_path(_longest_path), data(_data)
	{
		zero_arc = NULL;
		one_arc = NULL;
		layer = -1;
		id = -1;
		global_id = -1;
		relaxed_node = false;
	}

	/**
	 * Node constructor when node data is not needed
	 */
	Node(State* _state, double _longest_path) : Node(_state, _longest_path, NULL) {}

	/**
	 * Node constructor when longest path information is not needed
	 */
	Node(State* _state, NodeDataMap* _data) : Node(_state, -1, _data) {}

	~Node();


	// General functions

	/** Pulls all parents from given node and attaches them to the current one. */
	void pull_parents(Node* node);

	/** Update the optimal path value with the maximum value between this and another node */
	void update_optimal_path(Node* node);

	/**
	 * Merges this node with the given node, including merging the state and taking its parents.
	 * The given node becomes isolated (assuming no children) and ready for deletion.
	 */
	void merge(Problem* prob, Node* node, bool skip_state_merge = false);


	// Arc management functions

	/** Detach the zero arc from the node */
	void detach_zero_arc();

	/** Assign zero_node (possibly NULL) as a zero node */
	void assign_zero_arc(Node* zero_node);

	/** Detach the one arc from the node */
	void detach_one_arc();
	
	/** Assign one_node (possibly NULL) as a one node */
	void assign_one_arc(Node* one_node);

	/** Detach the arc with the given value from the node */
	void detach_arc(int val);

	/** Assign a node (possibly NULL) as a child with the  given value */
	void assign_arc(Node* child, int val);

};

#endif // BDD_NODE_HPP_

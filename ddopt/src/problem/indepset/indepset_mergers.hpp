/**
 * Merging functions specific to the independent set problem
 */

#ifndef INDEPSET_MERGERS_HPP_
#define INDEPSET_MERGERS_HPP_

#include "../../core/merge.hpp"
#include "indepset_state.hpp"


/** Return a merger for the independent set problem given an id */
Merger* get_merger_by_id_indepset(int id, int width);


/**
 * Node comparator by state size
 */
struct CompareNodesStateSizeAscending {
	bool operator()(Node* nodeA, Node* nodeB) const
	{
		IndepSetState* stateA = dynamic_cast<IndepSetState*>(nodeA->state);
		IndepSetState* stateB = dynamic_cast<IndepSetState*>(nodeB->state);
		if (stateA->get_size() != stateB->get_size()) {
			return stateA->get_size() < stateB->get_size();
		}
		if (DBL_EQ(nodeA->longest_path, nodeB->longest_path)) {
			return 0;
		}
		return nodeA->longest_path > nodeB->longest_path;
	}
};

/**
 * Node comparator by state size
 */
struct CompareNodesStateSizeDescending {
	bool operator()(Node* nodeA, Node* nodeB) const
	{
		IndepSetState* stateA = dynamic_cast<IndepSetState*>(nodeA->state);
		IndepSetState* stateB = dynamic_cast<IndepSetState*>(nodeB->state);
		if (stateA->get_size() != stateB->get_size()) {
			return stateA->get_size() > stateB->get_size();
		}
		if (DBL_EQ(nodeA->longest_path, nodeB->longest_path)) {
			return 0;
		}
		return nodeA->longest_path > nodeB->longest_path;
	}
};


// Minimum size merger
struct MinSizeMerger : Merger {
	MinSizeMerger(int _width) : Merger(_width, "min_size") {}

	void merge_layer(Problem* prob, int layer, vector<Node*>& nodes_layer)
	{
		merge_nodes_past_width_iteratively(prob, nodes_layer, width, CompareNodesStateSizeDescending());
	}
};


// Maximum size merger
struct MaxSizeMerger : Merger {
	MaxSizeMerger(int _width) : Merger(_width, "max_size") {}

	void merge_layer(Problem* prob, int layer, vector<Node*>& nodes_layer)
	{
		merge_nodes_past_width_iteratively(prob, nodes_layer, width, CompareNodesStateSizeAscending());
	}
};


/** Criterion for MinNewSolsBoundMerger */
struct MinNewSolsBound {
	double operator()(Node* nodeA, Node* nodeB) const
	{
		IndepSetState* stateA = dynamic_cast<IndepSetState*>(nodeA->state);
		IndepSetState* stateB = dynamic_cast<IndepSetState*>(nodeB->state);
		return MAX(nodeA->longest_path + stateB->get_size(),
		           nodeB->longest_path + stateA->get_size());
	}
};

/**
 * Minimum dual bound of merged node
 */
struct MinNewSolsBoundMerger : Merger {

	MinNewSolsBoundMerger(int _width) : Merger(_width, "min_new_sols_bound") {}

	void merge_layer(Problem* prob, int layer, vector<Node*>& nodes_layer)
	{
		merge_nodes_pairs_value(prob, nodes_layer, width, MinNewSolsBound(), false);
	}
};

#endif // INDEPSET_MERGERS_HPP_

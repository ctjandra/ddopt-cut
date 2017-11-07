/**
 * General-purpose merging rules for decision diagrams
 */

#ifndef MERGERS_HPP_
#define MERGERS_HPP_

#include <cassert>
#include <cstdio>
#include <vector>
#include <list>

#include "../bdd/bdd.hpp"
#include "merge.hpp"

using namespace std;



// Minimum longest path
struct MinLongestPathMerger : Merger {
	MinLongestPathMerger(int _width) : Merger(_width, "min_lp") {}

	void merge_layer(Problem* prob, int layer, vector<Node*>& nodes_layer)
	{
		merge_nodes_past_width_at_once(prob, nodes_layer, this->width, CompareNodesLongestPath());
	}
};


// Minimum longest path: Pair by pair
struct PairMinLongestPathMerger : Merger {
	PairMinLongestPathMerger(int _width) : Merger(_width, "pair_lp") {}

	void merge_layer(Problem* prob, int layer, vector<Node*>& nodes_layer)
	{
		sort(nodes_layer.begin(), nodes_layer.end(), CompareNodesLongestPath());

		// Passes no comparator because there is no need to re-sort between each iteration,
		// since order is already preserved with a longest path comparator
		merge_nodes_past_width_iteratively(prob, nodes_layer, this->width);
	}
};


// Minimum longest path: Consecutive pairs
struct ConsecutivePairLongestPathMerger : Merger {
	ConsecutivePairLongestPathMerger(int _width) : Merger(_width, "consec") {}

	void merge_layer(Problem* prob, int layer, vector<Node*>& nodes_layer)
	{
		merge_nodes_past_width_consecutive_pairs(prob, nodes_layer, this->width, CompareNodesLongestPath());
	}
};


struct NodeStateLexLessThan {
	bool operator()(const Node* lhs, const Node* rhs) const
	{
		return lhs->state < rhs->state;
	}
};

// Lexicographic merger
struct LexicographicMerger : Merger {
	LexicographicMerger(int _width) : Merger(_width, "lex") {}

	void merge_layer(Problem* prob, int layer, vector<Node*>& nodes_layer)
	{
		merge_nodes_past_width_iteratively(prob, nodes_layer, this->width, NodeStateLexLessThan());
	}
};


// Random merger
struct RandomMerger : Merger {
	RandomMerger(int _width) : Merger(_width, "random") {}

	void merge_layer(Problem* prob, int layer, vector<Node*>& nodes_layer)
	{
		random_shuffle(nodes_layer.begin(), nodes_layer.end());
		merge_nodes_past_width_at_once(prob, nodes_layer, this->width);
	}
};


#endif /* MERGERS_HPP_ */

/**
 * Functions to help with a simple top-down or bottom-up pass through a BDD
 */

#ifndef BDD_PASS_HPP_
#define BDD_PASS_HPP_

#include "bdd.hpp"
#include "../core/merge.hpp"

struct BDDPassValues {
	double top_down_val;
	double bottom_up_val;
};


class BDDPassFunc
{
public:
	virtual ~BDDPassFunc() {}

	/** Initial value stored at root if top-down, or terminal if bottom-up*/
	virtual double start_val() = 0;

	/** Initialization value at each node before the pass */
	virtual double init_val() = 0;

	/**
	 * Return value to be stored at target. Target is child if top-down, parent if bottom-up.
	 * Source is parent if top-down, child if bottom-up. Layer is always layer of parent (whether source or target).
	 */
	virtual double apply(int layer, int var, int arc_val, double source_val, double target_val,
	                     Node* source, Node* target) = 0;
};


struct CompareNodesPassValIncreasing {
	bool operator()(const Node* nodeA, const Node* nodeB) const
	{
		double tdA = boost::any_cast<BDDPassValues*>(nodeA->temp_data)->top_down_val;
		double tdB = boost::any_cast<BDDPassValues*>(nodeB->temp_data)->top_down_val;
		if (DBL_EQ(tdA, tdB)) {
			return 0;
		}
		return tdA < tdB;
	}
};


struct CompareNodesPassValDecreasing {
	bool operator()(const Node* nodeA, const Node* nodeB) const
	{
		double tdA = boost::any_cast<BDDPassValues*>(nodeA->temp_data)->top_down_val;
		double tdB = boost::any_cast<BDDPassValues*>(nodeB->temp_data)->top_down_val;
		if (DBL_EQ(tdA, tdB)) {
			return 0;
		}
		return tdA > tdB;
	}
};


// Merge nodes with largest pass values
// Note that responsibility of creating and deleting the temporary data is outside this merger
struct MaxPassValMerger : Merger {
	MaxPassValMerger(int _width) : Merger(_width, "max_pass_val") {}

	void merge_layer(Problem* prob, int layer, vector<Node*>& nodes_layer)
	{
		// Behavior is undefined if data is not set
		sort(nodes_layer.begin(), nodes_layer.end(), CompareNodesPassValIncreasing());
		merge_nodes_past_width_at_once(prob, nodes_layer, this->width);
	}
};


// Merge nodes with smallest pass values
struct MinPassValMerger : Merger {
	MinPassValMerger(int _width) : Merger(_width, "min_pass_val") {}

	void merge_layer(Problem* prob, int layer, vector<Node*>& nodes_layer)
	{
		// Behavior is undefined if data is not set
		sort(nodes_layer.begin(), nodes_layer.end(), CompareNodesPassValDecreasing());
		merge_nodes_past_width_at_once(prob, nodes_layer, this->width);
	}
};


/**
 * Store values in a top-down or bottom-up pass through the BDD. Note that bdd_pass_clean_up must always
 * be called after done with the values. NULL may be passed if only a single direction pass is needed.
 */
void bdd_pass(BDD* bdd, BDDPassFunc* top_down, BDDPassFunc* bottom_up);

/**
 * Deletes the values for top-down/bottom-up pass, assuming DD is fully constructed. Must always be called
 * after done with values.
 */
void bdd_pass_clean_up(BDD* bdd);


/**
 * Store values in a top-down pass through a BDD that is not necessarily fully constructed. Note that
 * bdd_partial_pass_clean_up must always be called after done with the values.
 */
void bdd_partial_pass(BDD* bdd, BDDPassFunc* top_down);

/**
 * Deletes the values for top-down/bottom-up pass, whether DD is fully constructed or not; i.e., includes
 * nodes not yet consolidated but added as a child of a node. Must always be called after done with values.
 */
void bdd_pass_deep_clean_up(BDD* bdd);


#endif // BDD_PASS_HPP_

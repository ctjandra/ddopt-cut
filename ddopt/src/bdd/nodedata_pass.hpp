/**
 * Classes that combine NodeData with BDDPassFunc
 */

#ifndef NODEDATA_PASS_HPP_
#define NODEDATA_PASS_HPP_

#include "nodedata.hpp"
#include "bdd_pass.hpp"

/** NodeData that uses a BDDPassFunc to do transition; requires merge implementation */
class PassFuncNodeData : public NodeData
{
public:
	BDDPassFunc* pass_func;  /**< pass function */
	double node_value;       /**< value at node */

	PassFuncNodeData(BDDPassFunc* _pass_func) : pass_func(_pass_func), node_value(_pass_func->start_val()) {}

	// Warning: This cannot be used with PassFuncs that use target node information
	NodeData* transition(Problem* prob, Node* node, State* new_state, int var, int val)
	{
		PassFuncNodeData* nd = create_from_pass_func(pass_func);
		nd->node_value = pass_func->apply(node->layer, var, val, node_value, pass_func->init_val(), node, NULL);
		return nd;
	}

	virtual void merge(Problem* prob, NodeData* rhs, State* state) = 0;

	/** Instantiate itself */
	virtual PassFuncNodeData* create_from_pass_func(BDDPassFunc* pass_func) = 0;
};


/** NodeData that uses a BDDPassFunc to do transition and takes the minimum value during merge */
class MinPassFuncNodeData : public PassFuncNodeData
{
public:
	MinPassFuncNodeData(BDDPassFunc* _pass_func) : PassFuncNodeData(_pass_func) {}

	void merge(Problem* prob, NodeData* rhs, State* state)
	{
		MinPassFuncNodeData* rhsp = dynamic_cast<MinPassFuncNodeData*>(rhs);
		if (DBL_LT(rhsp->node_value, node_value)) {
			node_value = rhsp->node_value;
		}
	}

	PassFuncNodeData* create_from_pass_func(BDDPassFunc* pass_func)
	{
		return new MinPassFuncNodeData(pass_func);
	}
};


/** NodeData that uses a BDDPassFunc to do transition and takes the maximum value during merge */
class MaxPassFuncNodeData : public PassFuncNodeData
{
public:
	MaxPassFuncNodeData(BDDPassFunc* _pass_func) : PassFuncNodeData(_pass_func) {}

	void merge(Problem* prob, NodeData* rhs, State* state)
	{
		MaxPassFuncNodeData* rhsp = dynamic_cast<MaxPassFuncNodeData*>(rhs);
		if (DBL_GT(rhsp->node_value, node_value)) {
			node_value = rhsp->node_value;
		}
	}

	PassFuncNodeData* create_from_pass_func(BDDPassFunc* pass_func)
	{
		return new MaxPassFuncNodeData(pass_func);
	}
};


struct CompareNodesPassValNodeDataIncreasing {
	string key;

	CompareNodesPassValNodeDataIncreasing(string _key) : key(_key) {}

	bool operator()(const Node* nodeA, const Node* nodeB) const
	{
		NodeData* ndA = nodeA->data->get(key);
		NodeData* ndB = nodeB->data->get(key);
		assert(ndA != NULL && ndB != NULL);
		double tdA = dynamic_cast<PassFuncNodeData*>(ndA)->node_value;
		double tdB = dynamic_cast<PassFuncNodeData*>(ndB)->node_value;
		if (DBL_EQ(tdA, tdB)) {
			return 0;
		}
		return tdA < tdB;
	}
};


struct CompareNodesPassValNodeDataDecreasing {
	string key;

	CompareNodesPassValNodeDataDecreasing(string _key) : key(_key) {}

	bool operator()(const Node* nodeA, const Node* nodeB) const
	{
		NodeData* ndA = nodeA->data->get(key);
		NodeData* ndB = nodeB->data->get(key);
		assert(ndA != NULL && ndB != NULL);
		double tdA = dynamic_cast<PassFuncNodeData*>(ndA)->node_value;
		double tdB = dynamic_cast<PassFuncNodeData*>(ndB)->node_value;
		if (DBL_EQ(tdA, tdB)) {
			return 0;
		}
		return tdA > tdB;
	}
};


/** Merge nodes with largest pass values in NodeData, using the given NodeData key */
struct MaxPassValNodeDataMerger : Merger {
	string key;

	MaxPassValNodeDataMerger(int _width, string _key) : Merger(_width, "max_pass_val_nd"), key(_key) {}

	void merge_layer(Problem* prob, int layer, vector<Node*>& nodes_layer)
	{
		sort(nodes_layer.begin(), nodes_layer.end(), CompareNodesPassValNodeDataIncreasing(key));
		merge_nodes_past_width_at_once(prob, nodes_layer, this->width);
	}
};


/** Merge nodes with smallest pass values in NodeData, using the given NodeData key */
struct MinPassValNodeDataMerger : Merger {
	string key;

	MinPassValNodeDataMerger(int _width, string _key) : Merger(_width, "min_pass_val_nd"), key(_key) {}

	void merge_layer(Problem* prob, int layer, vector<Node*>& nodes_layer)
	{
		sort(nodes_layer.begin(), nodes_layer.end(), CompareNodesPassValNodeDataDecreasing(key));
		merge_nodes_past_width_at_once(prob, nodes_layer, this->width);
	}
};


#endif // NODEDATA_PASS_HPP_
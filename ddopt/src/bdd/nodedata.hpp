/**
 * Structures to compute recursive information across nodes of a decision diagram during construction
 */

#ifndef NODEDATA_HPP_
#define NODEDATA_HPP_

#include <map>
#include "../problem/problem.hpp"
#include "../problem/state.hpp"

using namespace std;

/** Container to carry information across nodes of a decision diagram */
class NodeData
{
public:
	bool infeasible = false;       /**< if set to true during transition, communicates to solver that node is infeasible */

	// Constructor defines NodeData for DD root node

	virtual ~NodeData() {}

	/** Transition from given node with var set to val. */
	virtual NodeData* transition(Problem* prob, Node* node, State* new_state, int var, int val)
	{
		return new NodeData();
	}

	/** Merging function between two NodeDatas. This becomes the merged one and the other one is left unchanged. */
	virtual void merge(Problem* prob, NodeData* rhs, State* state) {}
};


/** Set of NodeDatas; stored by string keys */
class NodeDataMap
{
	map<string, NodeData*> data_map;

public:
	NodeDataMap() {}

	~NodeDataMap()
	{
		for (pair<const string, NodeData*>& p : data_map) {
			NodeData* nd = p.second;
			delete nd;
		}
	}

	/** Transition from given node with var set to val. */
	NodeDataMap* transition(Problem* prob, Node* node, State* new_state, int var, int val)
	{
		NodeDataMap* new_list = new NodeDataMap();
		for (pair<const string, NodeData*>& p : data_map) {
			string key = p.first;
			NodeData* nd = p.second;
			NodeData* new_nd = nd->transition(prob, node, new_state, var, val);
			new_list->add(key, new_nd);
		}
		return new_list;
	}

	/** Merging function between two NodeDataMaps. This becomes the merged one and the other one is left unchanged. */
	void merge(Problem* prob, NodeDataMap* rhs, State* state)
	{
		// Within the same DD, all NodeDataMaps must exist and have same size
		// This should be guaranteed if NodeDataMaps are only touched at the root node
		assert(rhs != NULL);
		assert(size() == rhs->size());

		for (pair<const string, NodeData*>& p : data_map) {
			string key = p.first;
			NodeData* nd = p.second;
			nd->merge(prob, rhs->get(key), state);
		}
	}

	/** Return true if at least one NodeData is infeasible */
	bool is_infeasible()
	{
		for (pair<const string, NodeData*>& p : data_map) {
			NodeData* nd = p.second;
			if (nd->infeasible) {
				return true;
			}
		}
		return false;
	}


	// Map functions

	void add(string key, NodeData* node_data)
	{
		data_map.emplace(key, node_data);
	}

	unsigned int size()
	{
		return data_map.size();
	}

	bool empty()
	{
		return data_map.empty();
	}

	NodeData* get(string key)
	{
		return data_map[key];
	}

};


#endif // NODEDATA_HPP_
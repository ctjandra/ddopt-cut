/**
 * Core merging functionality
 */

#ifndef MERGE_HPP_
#define MERGE_HPP_

#include <cassert>
#include "../bdd/bdd.hpp"


/** Node merging for decision diagrams */
struct Merger {
	int               width;
	string            name;

	Merger(int _width, string _name) : width(_width), name(_name) {}

	virtual ~Merger() {}

	/** Return vertex corresponding to given layer */
	virtual void merge_layer(Problem* prob, int layer, vector<Node*>& nodes_layer) = 0;
};


// Utility functions to help with merging. Some functions are defined in header due to use of comparator template.

/** Artificial comparator that signals to merging functions to skip sorting. */
struct NoSorting {
	bool operator()(const Node* lhs, const Node* rhs) const
	{
		assert(false); // Comparator should not be used
		exit(1);
		return false;
	}
};


/** Find an equivalent node (with respect to state) in a list of nodes */
Node* find_equivalent_state(vector<Node*>& nodes_list, Node* node);


/** Merge all nodes past a given width at once. Equivalence check is only done at the end of merging. */
template <class Compare = NoSorting>
void merge_nodes_past_width_at_once(Problem* prob, vector<Node*>& nodes_layer, int width, Compare comparator = NoSorting())
{
	bool use_sorting = !is_same<Compare,NoSorting>::value;

	// Sort nodes
	if (use_sorting) {
		sort(nodes_layer.begin(), nodes_layer.end(), comparator);
	}

	// Merge nodes
	Node* merging_node = nodes_layer[width-1];
	for (vector<Node*>::iterator node = nodes_layer.begin()+width; node != nodes_layer.end(); ++node) {
		merging_node->merge(prob, *node);
		delete(*node);
	}
	nodes_layer.resize(width);

	// Equivalence test on merging_node (i.e. check if state already exists in other nodes)
	Node* equivalent_node = find_equivalent_state(nodes_layer, merging_node);
	if (equivalent_node != NULL) {
		equivalent_node->merge(prob, merging_node, true);
		delete merging_node;
		nodes_layer.pop_back();
	}
}


/** Merge all nodes past a given width iteratively. Equivalence check is done at the end of each iteration. */
template <class Compare = NoSorting>
void merge_nodes_past_width_iteratively(Problem* prob, vector<Node*>& nodes_layer, int width, Compare comparator = NoSorting())
{
	bool use_sorting = !is_same<Compare,NoSorting>::value;
	NodeMap current_states;

	if (use_sorting) {
		sort(nodes_layer.begin(), nodes_layer.end(), comparator);
	}

	// populate current states with given nodes for equivalence checks
	current_states.clear();
	for (vector<Node*>::iterator node = nodes_layer.begin(); node != nodes_layer.end(); ++node) {
		current_states[(*node)->state] = *node;
	}

	// merge nodes from the end of the list until max. width is reached
	int current_size = nodes_layer.size();
	while (current_size > width) {

		// erase both elements from map
		current_states.erase(nodes_layer[current_size-2]->state);
		current_states.erase(nodes_layer[current_size-1]->state);

		// merge two last nodes
		// cout << "Merging " << *(nodes_layer[current_size-1]->state) << " with " << *(nodes_layer[current_size-2]->state) << endl;
		nodes_layer[current_size-2]->merge(prob, nodes_layer[current_size-1]);

		// remove last node from layer
		delete nodes_layer[current_size-1];
		nodes_layer.pop_back();
		current_size--;

		// now, we must check if the state of the new node appears in any previous node
		NodeMap::iterator map_it = current_states.find(nodes_layer[current_size-1]->state);
		if (map_it != current_states.end()) {

			// merge nodes
			// cout << "Merging " << *(nodes_layer[current_size-1]->state) << " with " << *(map_it->second->state) << endl;
			map_it->second->merge(prob, nodes_layer[current_size-1]);

			// remove last node from layer
			delete nodes_layer[current_size-1];
			nodes_layer.pop_back();
			current_size--;

		} else {
			// otherwise, we add the node to the set of current states
			current_states[nodes_layer[current_size-1]->state] = nodes_layer[current_size-1];
		}

		// ensure modified node falls into correct order
		if (use_sorting) {
			sort(nodes_layer.begin(), nodes_layer.end(), comparator);
		}
	}
}


/** Merge consecutive pairs of nodes until within width. */
template <class Compare = NoSorting>
void merge_nodes_past_width_consecutive_pairs(Problem* prob, vector<Node*>& nodes_layer, int width, Compare comparator = NoSorting())
{
	bool use_sorting = !is_same<Compare,NoSorting>::value;
	vector<Node*> old_nodes;
	Node* equivalent_node;

	// merge while maximum width is not met
	while (old_nodes.size() + nodes_layer.size() > (unsigned int)width) {

		// copy nodes to new vector
		old_nodes.resize(nodes_layer.size());
		for (int i = 0; i < (int)nodes_layer.size(); i++) {
			old_nodes[i] = nodes_layer[i];
		}
		nodes_layer.clear();  // destroy old vector

		// sort according to comparator
		if (use_sorting) {
			sort(old_nodes.begin(), old_nodes.end(), comparator);
		}

		// perform consecutive pairs merge while still possible
		while (old_nodes.size() >= 2) {

			// remove the last two nodes for merging
			Node* nodeB = old_nodes.back();
			old_nodes.pop_back();

			Node* nodeA = old_nodes.back();
			old_nodes.pop_back();

			// merge into node A
			nodeA->merge(prob, nodeB);
			delete nodeB;

			// equivalence check in old_nodes
			equivalent_node = find_equivalent_state(old_nodes, nodeA);
			if (equivalent_node != NULL) {
				equivalent_node->merge(prob, nodeA, true);
				delete nodeA;
			} else {
				// equivalence check in nodes_layer
				equivalent_node = find_equivalent_state(nodes_layer, nodeA);
				if (equivalent_node != NULL) {
					equivalent_node->merge(prob, nodeA, true);
					delete nodeA;
				} else {
					// node is new
					nodes_layer.push_back(nodeA);
				}
			}

		}

		// we insert the remaining elements of the old node at nodes_layer
		nodes_layer.insert(nodes_layer.end(), old_nodes.begin(), old_nodes.end());
		old_nodes.clear();
	}

	assert((int) nodes_layer.size() <= width);
}


/** Merge pairs that minimize or maximize a function that takes pairs of nodes into account */
template <class ValFunc>
void merge_nodes_pairs_value(Problem* prob, vector<Node*>& nodes_layer, int width, ValFunc value_function, bool maximize = true)
{
	while ((int) nodes_layer.size() > width) {
		Node* node1_to_merge = NULL;
		Node* node2_to_merge = NULL;

		double value;
		if (maximize) {
			value = -numeric_limits<double>::infinity();
		} else {
			value = numeric_limits<double>::infinity();
		}

		// Take pair of nodes that maximize/minimize the function
		for (vector<Node*>::iterator it1 = nodes_layer.begin(); it1 != nodes_layer.end(); ++it1) {
			Node* node1 = *it1;
			for (vector<Node*>::iterator it2 = it1 + 1; it2 != nodes_layer.end(); ++it2) {
				Node* node2 = *it2;
				if (node1 == node2) {
					continue;
				}

				double cur_value = value_function(node1, node2);
				if ((maximize && cur_value > value) ||
				        (!maximize && cur_value < value)) {
					node1_to_merge = node1;
					node2_to_merge = node2;
					value = cur_value;
				}
			}
		}

		// Merge pair of nodes
		node1_to_merge->merge(prob, node2_to_merge);
		nodes_layer.erase(remove(nodes_layer.begin(), nodes_layer.end(), node2_to_merge), nodes_layer.end());
		delete node2_to_merge;

		// Equivalence test on new node (i.e. check if state already exists in other nodes)
		Node* equivalent_node = find_equivalent_state(nodes_layer, node1_to_merge);
		if (equivalent_node != NULL) {
			equivalent_node->merge(prob, node1_to_merge, true);
			nodes_layer.erase(remove(nodes_layer.begin(), nodes_layer.end(), node1_to_merge), nodes_layer.end());
			delete node1_to_merge;
		}
	}
}


#endif /* MERGE_HPP_ */

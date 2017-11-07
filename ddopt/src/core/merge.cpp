/**
 * Core merging functionality
 */

#include "merge.hpp"

Node* find_equivalent_state(vector<Node*>& nodes_list, Node* node)
{
	for (Node* other : nodes_list) {
		if (other != node && other->state->equals_to(node->state)) {
			return other;
		}
	}
	return NULL;
}

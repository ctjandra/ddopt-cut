/** 
 * Node structure for decision diagrams
 */

#include "bdd_node.hpp"
#include "bdd.hpp"
#include "nodedata.hpp"

/** Node destructor */
Node::~Node()
{
	delete state;
	delete data;
}


void Node::pull_parents(Node* node)
{
	for (Node* one_ancestor : node->one_ancestors) {
		one_ancestors.push_back(one_ancestor);
		one_ancestor->one_arc = this;
	}
	node->one_ancestors.clear();

	for (Node* zero_ancestor : node->zero_ancestors) {
		zero_ancestors.push_back(zero_ancestor);
		zero_ancestor->zero_arc = this;
	}
	node->zero_ancestors.clear();
}


void Node::update_optimal_path(Node* node)
{
	// Assume we always maximize
	longest_path = MAX(longest_path, node->longest_path);
}


void Node::merge(Problem* prob, Node* node, bool skip_state_merge /*= false*/)
{
	// Merging makes sense when nodes do not have children, because otherwise we may be losing solutions
	// Removing this assumption may make the DD inconsistent as this merge does not take children into account
	assert(this->zero_arc == NULL && this->one_arc == NULL);
	assert(node->zero_arc == NULL && node->one_arc == NULL);
	assert(this != node); // Not merging with itself

	// If state is not to be merged, ensure they are equivalent
	assert(!skip_state_merge || state->equals_to(node->state));

	if (!skip_state_merge) {
		state->merge(prob, node->state);
	}
	pull_parents(node); // relaxation
	update_optimal_path(node);
	if (data != NULL) {
		data->merge(prob, node->data, node->state);
	}
	relaxed_node = true; // mark node as relaxed
}


// Arc management functions

void Node::detach_zero_arc()
{
	assert(zero_arc != NULL);
	vector<Node*>::iterator pos = find(zero_arc->zero_ancestors.begin(), zero_arc->zero_ancestors.end(), this);
	assert(pos != zero_arc->zero_ancestors.end());
	zero_arc->zero_ancestors.erase(pos);
	zero_arc = NULL;
}


void Node::assign_zero_arc(Node* zero_node)
{
	if (zero_arc != NULL) {
		detach_zero_arc();
	}
	zero_arc = zero_node;
	if (zero_node != NULL) {
		zero_node->zero_ancestors.push_back(this);
	}
}


void Node::detach_one_arc()
{
	assert(one_arc != NULL);
	vector<Node*>::iterator pos = find(one_arc->one_ancestors.begin(), one_arc->one_ancestors.end(), this);
	assert(pos != one_arc->one_ancestors.end());
	one_arc->one_ancestors.erase(pos);
	one_arc = NULL;
}


void Node::assign_one_arc(Node* one_node)
{
	if (one_arc != NULL) {
		detach_one_arc();
	}
	one_arc = one_node;
	if (one_node != NULL) {
		one_node->one_ancestors.push_back(this);
	}
}


void Node::detach_arc(int val)
{
	assert(val == 0 || val == 1);
	if (val == 1) {
		detach_one_arc();
	} else {
		detach_zero_arc();
	}
}


void Node::assign_arc(Node* child, int val)
{
	assert(val == 0 || val == 1);
	if (val == 1) {
		assign_one_arc(child);
	} else {
		assign_zero_arc(child);
	}
}

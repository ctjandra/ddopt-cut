/**
 * Completion dual bounds
 */

#ifndef COMPLETION_HPP_
#define COMPLETION_HPP_

#include "../problem/instance.hpp"

/**
 * Primal and dual bounds for the completion. That is, if we fix the variables to the path up to a node, this is a dual bound
 * for the remaining variables to the terminal. The dual bound is always an upper bound, since we always maximize in a DD;
 * similarly, the primal bound is always a lower bound.
 */
class CompletionBound
{
public:

	virtual ~CompletionBound() {}

	virtual double dual_bound(Instance* inst, Node* node, Node* parent)
	{
		cout << "Warning: Dual bound not implemented for completion; ignored primal pruning" << endl;
		return +numeric_limits<double>::infinity();
	}

	virtual double primal_bound(Instance* inst, Node* node, Node* parent)
	{
		cout << "Warning: Primal bound not implemented for completion; ignored dual pruning" << endl;
		return -numeric_limits<double>::infinity();
	}
};


// Generic dual bounds for completion

/** Use the number of remaining layers as dual bound. Only for binary problems and with objective (1,...,1). */
class NumberOfRemainingLayersCompletionBound : public CompletionBound
{
	double dual_bound(Instance* inst, Node* node, Node* parent)
	{
		// This could be a better bound if we knew exactly which layer the node will be in (which depends on long arcs), but
		// from a generic perspective we cannot know especially because the variable ordering might be dynamic
		return inst->nvars - (parent->layer + 1);
	}
};

#endif /* COMPLETION_HPP_ */

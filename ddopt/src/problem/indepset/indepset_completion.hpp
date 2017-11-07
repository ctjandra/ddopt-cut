/**
 * Completion bounds for independent set
 */

#ifndef INDEPSET_COMPLETION_HPP_
#define INDEPSET_COMPLETION_HPP_

#include "../../core/completion.hpp"
#include "indepset_state.hpp"


/** Use the size of state; that is, the number of vertices that can still be assigned to 1. Only for weights (1,...,1). */
class StateSizeCompletionBound : public CompletionBound
{
	double dual_bound(Instance* inst, Node* node, Node* parent)
	{
		IndepSetState* state = dynamic_cast<IndepSetState*>(node->state);
		return state->get_size();
	}
};

#endif /* INDEPSET_COMPLETION_HPP_ */

/**
 * Independent set problem class
 */

#include "indepset_problem.hpp"

/* Callbacks */

bool IndepSetProblem::cb_skip_var_for_long_arc(int var, State* state)
{
	IndepSetState* state_is = dynamic_cast<IndepSetState*>(state);

	return !state_is->intset.contains(var);
}

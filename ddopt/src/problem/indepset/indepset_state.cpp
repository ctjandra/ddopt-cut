/**
 * Independent set state
 */

#include "indepset_state.hpp"
#include "indepset_instance.hpp"

State* IndepSetState::transition(Problem* prob, int var, int val)
{
	assert(val == 0 || val == 1);

	if (val == 1 && !intset.contains(var)) {
		return NULL;
	}

	IndepSetInstance* insti = dynamic_cast<IndepSetInstance*>(prob->inst);
	if (insti == NULL) {
		cout << "Error: Using incompatible State and Instance" << endl;
		exit(1);
	}

	IndepSetState* new_state;
	new_state = new IndepSetState(intset);

	// Remove vertex itself
	new_state->intset.remove(var);

	// Remove neighbors of vertex if added to graph
	if (val == 1) {
		new_state->intset.set &= insti->adj_mask_compl[var].set;
	}

	return new_state;
}

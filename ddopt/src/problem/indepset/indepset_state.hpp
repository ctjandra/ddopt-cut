/**
 * Independent set state
 */

#ifndef INDEPSET_STATE_HPP_
#define INDEPSET_STATE_HPP_

#include "../state.hpp"
#include "../problem.hpp"
#include "../../util/intset.hpp"


/** State for independent set */
class IndepSetState : public State
{
public:
	IntSet intset;

	IndepSetState(IntSet _intset) : intset(_intset) {}

	State* transition(Problem* prob, int var, int val);

	void merge(Problem* prob, State* rhs)
	{
		IndepSetState* rhsi = dynamic_cast<IndepSetState*>(rhs);
		intset.union_with(rhsi->intset);
	}

	bool equals_to(State* rhs)
	{
		IndepSetState* rhsi = dynamic_cast<IndepSetState*>(rhs);
		return intset.equals_to(rhsi->intset);
	}

	bool less(const State& rhs) const
	{
		const IndepSetState& rhsi = dynamic_cast<const IndepSetState&>(rhs);
		return intset.set < rhsi.intset.set;
	}

	int get_size()
	{
		return intset.get_size();
	}

	ostream& stream_write(ostream& os) const
	{
		os << intset;
		return os;
	}
};


#endif /* INDEPSET_STATE_HPP_ */

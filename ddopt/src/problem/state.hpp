/*
 * Abstract classes that need to be implemented in order to model a new problem to be solved.
 */

#ifndef STATE_HPP_
#define STATE_HPP_

#include <iostream>
#include "instance.hpp"


class Problem; // forward declaration; state implementations need to include problem header

class State
{
public:
	virtual ~State() {}

	/** Transition from given state with var set to val. Return NULL if no transition exists (i.e. infeasible). */
	virtual State* transition(Problem* prob, int var, int val) = 0;

	/** Merging function between two states. This state becomes the merged state and the other state is left unchanged. */
	virtual void merge(Problem* prob, State* rhs) = 0;

	/** Equivalence function between two states. */
	virtual bool equals_to(State* rhs) = 0;

	/** Operator < needs to be defined for comparator in hash map */
	virtual bool less(const State& rhs) const = 0;

	/** Function for printing the state */
	virtual std::ostream& stream_write(std::ostream& os) const = 0;


	/* Operators */
	friend bool operator<(const State& lhs, const State& rhs);
	friend std::ostream& operator<<(std::ostream& os, const State& state);
};


inline bool operator<(const State& lhs, const State& rhs)
{
	return lhs.less(rhs);
}

inline std::ostream& operator<<(std::ostream& os, const State& state)
{
	return state.stream_write(os);
}


#endif /* STATE_HPP_ */

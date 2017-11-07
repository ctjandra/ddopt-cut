/**
 * Ordering structures and functions
 */

#ifndef ORDER_HPP_
#define ORDER_HPP_

#include <vector>
#include "../problem/state.hpp"

using namespace std;


/** Class representing a general ordering */
struct Ordering {
	char              name[256];

	Ordering() {}

	virtual ~Ordering() {}

	// returns vertex corresponding to particular layer
	virtual int select_next_var(int layer) = 0;

	// Callbacks for updating structures related to ordering
	virtual void cb_initialize() {}
	virtual void cb_state_created(State* state) {}
	virtual void cb_state_removed(State* state) {}
};


// Comparators to help with orderings

template <class T>
struct DecreasingComparator {
	vector<T>& v;

	DecreasingComparator(vector<T>& _v) : v(_v) { }

	bool operator()(int i, int j)
	{
		return (v[i] > v[j]);
	}
};

template <class T>
struct IncreasingComparator {
	vector<T>& v;

	IncreasingComparator(vector<T>& _v) : v(_v) { }

	bool operator()(int i, int j)
	{
		return (v[i] > v[j]);
	}
};


#endif // ORDER_HPP_

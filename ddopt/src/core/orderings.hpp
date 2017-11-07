/**
 * General-purpose variable ordering for decision diagrams
 */

#ifndef ORDERINGS_HPP_
#define ORDERINGS_HPP_

#include <cassert>
#include <vector>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/discrete_distribution.hpp>

#include "order.hpp"
#include "../problem/instance.hpp"

using namespace std;


// Random ordering
struct RandomOrdering : Ordering {

	Instance* inst;
	vector<int> v_in_layer;   // vertex at each layer

	RandomOrdering(Instance* _inst) : inst(_inst)
	{
		sprintf(name, "random");
		v_in_layer.resize(inst->nvars);
		construct_ordering();
	}

	int select_next_var(int layer)
	{
		assert(layer >= 0 && layer < inst->nvars);
		return v_in_layer[layer];
	}

private:
	void construct_ordering();
};


// FixedOrdering: read from a file
struct FixedOrdering : Ordering {

	Instance* inst;
	vector<int> v_in_layer;   // vertex at each layer

	FixedOrdering(Instance* _inst, string filename) : inst(_inst)
	{
		sprintf(name, "fixed");
		v_in_layer.resize(inst->nvars);
		read_ordering(filename);
	}

	int select_next_var(int layer)
	{
		assert(layer >= 0 && layer < inst->nvars);
		return v_in_layer[layer];
	}

private:
	void read_ordering(string filename);
};


// Same ordering as given
struct NoOrdering : Ordering {

	NoOrdering()
	{
		sprintf(name, "no-ordering");
	}

	int select_next_var(int layer)
	{
		return layer;
	}
};


#endif

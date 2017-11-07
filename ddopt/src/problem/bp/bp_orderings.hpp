/**
 * Orderings for a binary problem
 */

#ifndef BP_ORDERINGS_HPP_
#define BP_ORDERINGS_HPP_

#include <vector>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/cuthill_mckee_ordering.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/bandwidth.hpp>
#include "../../core/order.hpp"
#include "../../util/options.hpp"
#include "bp_instance.hpp"

using namespace std;


/** Return an ordering for a binary problem given an id */
Ordering* get_ordering_by_id_bp(int id, BPInstance* inst, Options& options);


/**
 * Ordering that runs the Cuthill-McKee heuristic to minimize bandwidth on constraints with
 * pairs of variables; ignores all other constraints.
 */
struct CuthillMcKeePairOrdering : Ordering {
	BPInstance* inst;
	vector<int> v_in_layer;   // vertex at each layer

	CuthillMcKeePairOrdering(BPInstance* _inst) : inst(_inst)
	{
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


#endif // BP_ORDERINGS_HPP_
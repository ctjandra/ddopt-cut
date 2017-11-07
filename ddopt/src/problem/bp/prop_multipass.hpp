/**
 * Propagator wrapper for multiple passes
 */

#ifndef PROP_MULTIPASS_HPP_
#define PROP_MULTIPASS_HPP_

#include <unordered_set>
#include "bp_prop.hpp"
#include "bprow.hpp"
#include "bpvar.hpp"

using namespace std;


/**
 * Domain propagator wrapper that runs multiple passes on a list of propagators.
 * Internal propagator has the responsibility of tracking which variables need to be repropagated on.
 */
class BPPropMultipass
{
	vector<BPProp*> propagators;           /**< list of propagators to run a number of passes on */
	int             npasses;               /**< number of propagation passes to run */

public:

	/** Default: single pass */
	BPPropMultipass(BPProp* _propagator) : BPPropMultipass(_propagator, 1) {}
	BPPropMultipass(vector<BPProp*> _propagators) : BPPropMultipass(_propagators, 1) {}

	BPPropMultipass(BPProp* _propagator, int _npasses) : npasses(_npasses)
	{
		propagators.push_back(_propagator);
	}

	BPPropMultipass(vector<BPProp*> _propagators, int _npasses) : npasses(_npasses)
	{
		propagators = _propagators;
	}

	/** Propagate by running a number of passes on propagator, assuming it tracks newly updated variables */
	void propagate(BPState* state, int v, const vector<BPVar*>& vars, const vector<BPRow*>& rows,
	               vector<double>& minactivity, vector<double>& maxactivity, bool& infeasible);
};


#endif // PROP_MULTIPASS_HPP_

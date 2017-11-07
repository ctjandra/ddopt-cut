/**
 * Propagator for binary problems
 */

#ifndef BP_PROP_HPP_
#define BP_PROP_HPP_

#include <unordered_set>
#include "bp_state.hpp"
#include "bprow.hpp"
#include "bpvar.hpp"


/** Interface for propagator for binary programs. Propagates constraints on domains used as states for BDDs. */
class BPProp
{
public:
	virtual ~BPProp() {}

	/**
	 * Run propagation over the given state where v is a newly fixed variable.
	 * Store in infeasible whether infeasibility was detected during propagation.
	 * Propagation may optionally add more fixed variables to fixed_vars_next to support further passes on these variables.
	 */
	virtual void propagate(BPState* state, int v, const vector<BPVar*>& vars, const vector<BPRow*>& rows,
	                       vector<double>& minactivity, vector<double>& maxactivity, bool& infeasible, unordered_set<int>& fixed_vars_next) = 0;

};

#endif // BP_PROP_HPP_

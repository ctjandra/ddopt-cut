/**
 * Propagator wrapper for multiple passes
 */

#include "prop_multipass.hpp"


void BPPropMultipass::propagate(BPState* state, int v, const vector<BPVar*>& vars, const vector<BPRow*>& rows,
                                vector<double>& minactivity, vector<double>& maxactivity, bool& infeasible)
{
	assert(state->domains[v] == DOM_ZERO || state->domains[v] == DOM_ONE);

	unordered_set<int> fixed_vars;
	unordered_set<int> fixed_vars_next;

	fixed_vars.insert(v);

	// Run propagator on each fixed variable, collecting the set for the next pass
	for (int pass = 0; pass < npasses && !fixed_vars.empty(); ++pass) {
		for (int cur_var : fixed_vars) {
			assert(state->domains[cur_var] == DOM_ZERO || state->domains[cur_var] == DOM_ONE);
			for (BPProp* propagator : propagators) {
				propagator->propagate(state, cur_var, vars, rows, minactivity, maxactivity, infeasible, fixed_vars_next);
				if (infeasible) {
					return;
				}
			}
		}

		fixed_vars = fixed_vars_next;
		fixed_vars_next.clear();
	}
}

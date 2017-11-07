/**
 * Propagator for linear constraints
 */

#include "prop_linearcons.hpp"


void BPPropLinearcons::propagate(BPState* state, int v, const vector<BPVar*>& vars, const vector<BPRow*>& rows,
                                 vector<double>& minactivity, vector<double>& maxactivity, bool& infeasible, unordered_set<int>& fixed_vars_next)
{
	unordered_set<int> vars_to_check;

	// Gather all unfixed variables sharing a constraint with v in order to avoid revisiting variables
	add_neighbors(v, vars_to_check, state, vars, rows, minactivity, maxactivity);

	// Check if any neighbor of v can have their domain fixed
	for (int u : vars_to_check) {
		if (state->domains[u] != DOM_ZERO_ONE) {
			continue;    // this can happen if a domain was changed during this propagation step
		}

		BPVar* var = vars[u];
		int nrows_var = var->rows.size();

		for (int i = 0; i < nrows_var && !state->infeasible; ++i) {
			int row_idx = var->rows[i];
			double coeff = var->row_coeffs[i];

			// We skip the check if the row is always feasible because this step is relatively cheap
			// Try to reduce domain
			BPDomain domain = get_smallest_domain(state, row_idx, coeff, u, rows[row_idx]->sense, minactivity[row_idx], maxactivity[row_idx]);

			if (domain != DOM_ZERO_ONE) {
				// cout << "Setting var " << u << " to " << domain << " due to row " << row_idx << endl;
				// cout << "Minact " << minactivity[row_idx] << ", Maxact " << maxactivity[row_idx] << endl;
				// cout << "Row " << row_idx << ": " << *(rows[row_idx]) << endl;
				state->set_domain(u, domain, vars, rows, minactivity, maxactivity);
				fixed_vars_next.insert(u);
				break;
			}
		}
	}
}


void BPPropLinearcons::add_neighbors(int init_var, unordered_set<int>& vars_set, BPState* state, const vector<BPVar*>& vars,
                                     const vector<BPRow*>& rows, vector<double>& minactivity, vector<double>& maxactivity)
{
	BPVar* var = vars[init_var];
	int nrows_var = var->rows.size();

	// Add all variables participating in a constraint init_var is in, except for constraints that are always feasible
	for (int i = 0; i < nrows_var; ++i) {
		int row_idx = var->rows[i];
		if (!state->is_alwaysfeasible(row_idx, rows[row_idx]->sense, minactivity, maxactivity)) {
			for (int v : rows[row_idx]->ind) {
				if (v != init_var && state->domains[v] == DOM_ZERO_ONE) {
					vars_set.insert(v);
				}
			}
		}
	}
}


BPDomain BPPropLinearcons::get_smallest_domain(BPState* state, int cons, double coeff, int var, RowSense sense,
        double minactivity, double maxactivity)
{
	assert(state->domains[var] == DOM_ZERO_ONE);

	if (sense == SENSE_GE) {
		if (coeff < 0) {
			if (DBL_LT(maxactivity + coeff, state->rhs[cons])) {
				return DOM_ZERO;
			}
		} else {
			if (DBL_LT(maxactivity - coeff, state->rhs[cons])) {
				return DOM_ONE;
			}
		}
	} else { // sense == SENSE_LE
		if (coeff < 0) {
			if (DBL_GT(minactivity - coeff, state->rhs[cons])) {
				return DOM_ONE;
			}
		} else {
			if (DBL_GT(minactivity + coeff, state->rhs[cons])) {
				return DOM_ZERO;
			}
		}
	}
	return DOM_ZERO_ONE;
}

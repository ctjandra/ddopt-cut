/**
 * Propagator for linear constraints
 */

#ifndef PROP_LINEARCONS_HPP_
#define PROP_LINEARCONS_HPP_

#include <unordered_set>
#include "bp_prop.hpp"
#include "bp_domains.hpp"
#include "bprow.hpp"
#include "bpvar.hpp"

using namespace std;


/**
 * Domain propagator for linear constraints. Applies to the constraints in the problem, as we need
 * minactivity/maxactivity and RHSs.
 */
class BPPropLinearcons : public BPProp
{
	vector<BPRow*> rows_prop;              /**< rows that need to be propagated */

public:
	BPPropLinearcons(vector<BPRow*>& _rows_prop) : rows_prop(_rows_prop) {}

	void propagate(BPState* state, int v, const vector<BPVar*>& vars, const vector<BPRow*>& rows,
	               vector<double>& minactivity, vector<double>& maxactivity, bool& infeasible, unordered_set<int>& fixed_vars_next);

private:
	/** Add to vars_set all free variables participating in the constraints v is in. */
	void add_neighbors(int init_var, unordered_set<int>& vars_set, BPState* state, const vector<BPVar*>& vars,
	                   const vector<BPRow*>& rows, vector<double>& minactivity, vector<double>& maxactivity);

	/** Return the smallest domain for the given variable w.r.t. a single constraint, assuming domain is not yet set */
	BPDomain get_smallest_domain(BPState* state, int cons, double coeff, int var, RowSense sense, double minactivity, double maxactivity);
};


#endif // PROP_LINEARCONS_HPP_

/**
 * State for binary problem
 */

#include "bp_state.hpp"
#include "bp_problem.hpp"

State* BPState::transition(Problem* prob, int var, int val)
{
	// Value must be in domain; otherwise return infeasible
	if ((val == 0 && domains[var] == DOM_ONE)
	        || (val == 1 && domains[var] == DOM_ZERO)) {
		return NULL;
	}

	assert(!infeasible);

	BinaryProblem* prob_bp = dynamic_cast<BinaryProblem*>(prob);
	if (prob_bp == NULL) {
		cout << "Error: Using incompatible State and Problem" << endl;
		exit(1);
	}
	BPInstance* inst_bp = dynamic_cast<BPInstance*>(prob_bp->inst);
	if (inst_bp == NULL) {
		cout << "Error: Using incompatible State and Instance" << endl;
		exit(1);
	}

	// Copy state and set variable to value
	BPState* state = new BPState(*this);

	state->set_var(prob, var, val, inst_bp->vars, inst_bp->rows, prob_bp->minactivity, prob_bp->maxactivity);

	if (state->infeasible) {
		return NULL;
	}
	assert(state->domains[var] == DOM_PROCESSED);

	return state;
}


void BPState::set_var(Problem* prob, int var, int val, const vector<BPVar*>& vars, const vector<BPRow*>& rows,
                      const vector<double>& init_minactivity, const vector<double>& init_maxactivity)
{
	assert(val == 0 || val == 1);
	assert(!(domains[var] == DOM_ZERO && val == 1));
	assert(!(domains[var] == DOM_ONE && val == 0));
	assert(domains[var] != DOM_PROCESSED);
	assert(!infeasible);

	/* if domain is already equal to val, just update domains */
	if (domains[var] == DOM_ZERO || domains[var] == DOM_ONE) {
		assert((domains[var] == DOM_ZERO && val == 0) || (domains[var] == DOM_ONE && val == 1));
		mark_as_processed(var);
		return;
	}

	/* otherwise, domain is free, so set the domain and propagate */
	vector<double> minactivity = init_minactivity;
	vector<double> maxactivity = init_maxactivity;
	update_activity_from_domain(vars, minactivity, maxactivity);

	set_domain(var, (val == 0) ? DOM_ZERO : DOM_ONE, vars, rows, minactivity, maxactivity);
	if (infeasible) {
		return;    /* stop processing when detected infeasibility */
	}
	propagate_domain(prob, var, vars, rows, minactivity, maxactivity);
	domains.set_domain(var, DOM_PROCESSED);
}


void BPState::propagate_domain(Problem* prob, int var_fixed, const vector<BPVar*>& vars, const vector<BPRow*>& rows,
                               vector<double>& minactivity, vector<double>& maxactivity)
{
	BinaryProblem* prob_bp = dynamic_cast<BinaryProblem*>(prob);
	if (prob_bp->propagator == NULL) {
		return;
	}

	bool infeasible_prop = false;
	prob_bp->propagator->propagate(this, var_fixed, vars, rows, minactivity, maxactivity, infeasible_prop);
	if (infeasible_prop) {
		infeasible = true;
	}
}

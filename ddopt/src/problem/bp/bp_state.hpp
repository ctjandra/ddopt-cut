/**
 * State for binary problem
 */

#ifndef BPSTATE_HPP_
#define BPSTATE_HPP_

#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_set>

#include "../../util/util.hpp"
#include "../state.hpp"
#include "../problem.hpp"
#include "bp_instance.hpp"
#include "bp_domains.hpp"

using namespace std;

/**
 * BDD state for binary programs
 */
class BPState : public State
{
public:
	vector<double> rhs;
	BPDomains domains;
	bool infeasible;

	/** Constructor */
	BPState(int nvars, int ncons);

	/** Empty constructor */
	BPState() {}

	/** Copy constructor */
	BPState(const BPState& state);


	// Standard state functions

	State* transition(Problem* prob, int var, int val);

	void merge(Problem* prob, State* state);

	bool equals_to(State* state);

	bool less(const State& state) const;

	std::ostream& stream_write(std::ostream& os) const;

	void print();

	BPState& operator=(const BPState& rhs_state);


	// Initialization functions

	void init_state(int cons, int cons_rhs);
	void init_state_from_row(int cons, BPRow* row);
	void init_state_from_rows(const vector<BPRow*>& rows);


	// State modification functions

	/** Set right-hand side of state */
	void set_rhs(int cons, double val);

	/** Set the domain of a variable. */
	void set_domain(int var, BPDomain domain, const vector<BPVar*>& vars, const vector<BPRow*>& rows,
	                vector<double>& minactivity, vector<double>& maxactivity);


	// Auxiliary functions

	/** Return true if state is feasible independent of completion */
	bool is_alwaysfeasible(int cons, RowSense sense, const vector<double>& minactivity, const vector<double>& maxactivity);

	/** Update infeasibility flag of the state. Whenever min/maxactivity or rhs changes, this must be called. */
	void update_infeasibility(int cons, double minactivity, double maxactivity, RowSense sense);

	/** Update rhs if it becomes redundant. Whenever min/maxactivity or rhs changes, this must be called. */
	void update_alwaysfeasibility(int cons, double minactivity, double maxactivity, RowSense sense);

	/** Mark a variable that has already domain {0} or {1} as processed */
	void mark_as_processed(int var);


private:

	/** Set var to val in state */
	void set_var(Problem* prob, int var, int val, const vector<BPVar*>& vars, const vector<BPRow*>& rows,
	             const vector<double>& init_minactivity, const vector<double>& init_maxactivity);
	
	/**
	 * Update minactivity and maxactivity by taking into account variables not yet visited by the solver that are set to a
	 * smaller domain. This should only be applied to the global minactivity and maxactivity; that is, they already consider
	 * variables visited during DD construction. Alters the given vectors.
	 */
	void update_activity_from_domain(const vector<BPVar*>& vars, vector<double>& minactivity, vector<double>& maxactivity);

	/** Apply propagators to the domain */
	void propagate_domain(Problem* prob, int var_fixed, const vector<BPVar*>& vars, const vector<BPRow*>& rows,
	                      vector<double>& minactivity, vector<double>& maxactivity);
};


inline BPState::BPState(int nvars, int ncons)
{
	rhs.resize(ncons);
	domains.init(nvars);
	infeasible = false;
}


inline BPState::BPState(const BPState& state)
{
	rhs = state.rhs;
	domains = state.domains;
	infeasible = state.infeasible;
}


inline void BPState::set_rhs(int cons, double val)
{
	rhs[cons] = val;
}


/** Revert domain filtering changes made to right-hand sides (from {0} or {1} to {0,1}) */
inline void revert_rhs(int var_id, const vector<BPVar*>& vars, BPState* state)
{
	// Note minactivity/maxactivity cannot be reverted due to saturation
	if (state->domains[var_id] == DOM_ONE) {
		BPVar* var = vars[var_id];
		for (int j = 0; j < (int) var->rows.size(); ++j) {
			int cons = var->rows[j];
			double coeff = var->row_coeffs[j];
			state->set_rhs(cons, state->rhs[cons] + coeff);
		}
	}
}


/** Merges another states into this one. Warning: state may be destroyed */
inline void BPState::merge(Problem* prob, State* state)
{
	BPInstance* inst_bp = dynamic_cast<BPInstance*>(prob->inst);
	BPState* state_bp = dynamic_cast<BPState*>(state);
	if (inst_bp == NULL || state_bp == NULL) {
		cout << "Error: Using incompatible State and/or Instance" << endl;
		exit(1);
	}

	// cout << "Merging:" << endl;
	// cout << *this << endl;
	// cout << *state << endl;

	assert(rhs.size() == state_bp->rhs.size());
	assert(rhs.size() == inst_bp->rows.size());
	assert(domains.size() == state_bp->domains.size());
	for (int i = 0; i < (int) domains.size(); ++i) {
		// Processed variables must be the same for both nodes (i.e. they are in the same layer)
		assert((state_bp->domains[i] == DOM_PROCESSED && domains[i] == DOM_PROCESSED)
		       || (state_bp->domains[i] != DOM_PROCESSED && domains[i] != DOM_PROCESSED));
	}

	// Take the union of domains
	for (BPDomainsUnprocIterator it = domains.begin_unproc(); it != domains.end_unproc(); ++it) {
		int i = it->var;
		if (state_bp->domains[i] != domains[i]) {
			// Revert corresponding constraints
			revert_rhs(i, inst_bp->vars, this);
			revert_rhs(i, inst_bp->vars, state_bp);

			// Relax domain to {0,1}
			domains.set_domain(i, DOM_ZERO_ONE);
		}
	}

	// Relax the right-hand side
	for (int i = 0; i < (int) rhs.size(); ++i) {
		if ((inst_bp->rows[i]->sense == SENSE_LE && state_bp->rhs[i] > rhs[i])
		        || (inst_bp->rows[i]->sense == SENSE_GE && state_bp->rhs[i] < rhs[i])) {
			set_rhs(i, state_bp->rhs[i]);
		}
	}
}


inline bool BPState::equals_to(State* state)
{
	BPState* state_bp = dynamic_cast<BPState*>(state);

	// Domains must be the same
	int nvars = (int) domains.size();
	assert(nvars == (int) state_bp->domains.size());
	for (int i = 0; i < nvars; ++i) {
		if (domains[i] != state_bp->domains[i]) {
			return false;
		}
	}

	// Right-hand sides must be the same
	int ncons = (int) rhs.size();
	assert(ncons == (int) state_bp->rhs.size());
	for (int i = 0; i < ncons; ++i) {
		if (!DBL_EQ(rhs[i], state_bp->rhs[i])) {
			return false;
		}
	}

	return true;
}


struct DomainComparator {
	bool operator()(const BPDomainNode& lhs, const BPDomainNode& rhs) const
	{
		return lhs.domain < rhs.domain;
	}
};


inline bool BPState::less(const State& state) const
{
	const BPState* stateA = this;
	const BPState* stateB = dynamic_cast<const BPState*>(&state);

	assert(!stateA->infeasible && !stateB->infeasible);

	/* lexicographically compare rhs */
	int ncons = (int) stateA->rhs.size();
	assert(ncons == (int) stateB->rhs.size());
	for (int i = 0; i < ncons; ++i) {
		if (DBL_LT(stateA->rhs[i], stateB->rhs[i])) {
			return true;
		}
		if (DBL_GT(stateA->rhs[i], stateB->rhs[i])) {
			return false;
		}
	}

	/* stateA->rhs == stateB->rhs at this point*/

	/* for speed up purposes only; changes the ordering but in a well-defined way */
	if (stateA->domains.nvars_set_zero < stateB->domains.nvars_set_zero) {
		return true;
	}
	if (stateA->domains.nvars_set_zero > stateB->domains.nvars_set_zero) {
		return false;
	}
	if (stateA->domains.nvars_set_one < stateB->domains.nvars_set_one) {
		return true;
	}
	if (stateA->domains.nvars_set_one > stateB->domains.nvars_set_one) {
		return false;
	}

	return lexicographical_compare(stateA->domains.begin_unproc(), stateA->domains.end_unproc(),
	                               stateB->domains.begin_unproc(), stateB->domains.end_unproc(),
	                               DomainComparator());
}


/** Initialize the state of a constraint */
inline void BPState::init_state(int cons, int cons_rhs)
{
	set_rhs(cons, cons_rhs);
}


/** Initialize the state from a given row */
inline void BPState::init_state_from_row(int cons, BPRow* row)
{
	init_state(cons, row->rhs);
}


/** Initialize the state from a given set of rows */
inline void BPState::init_state_from_rows(const vector<BPRow*>& rows)
{
	for (int i = 0; i < (int) rows.size(); ++i) {
		init_state_from_row(i, rows[i]);
	}
}


inline void BPState::mark_as_processed(int var)
{
	assert(domains[var] == DOM_ZERO || domains[var] == DOM_ONE);
	domains.set_domain(var, DOM_PROCESSED);
}


inline void BPState::update_activity_from_domain(const vector<BPVar*>& vars, vector<double>& minactivity, vector<double>& maxactivity)
{
	if (rhs.empty()) {
		assert(minactivity.empty() && maxactivity.empty());
		return; // certain states such as "set packing only" problems only rely on propagation; this is unnecessary
	}

	for (BPDomainsSetIterator it = domains.begin_set(); it != domains.end_set(); ++it) {
		assert(it->domain != DOM_ZERO_ONE && it->domain != DOM_PROCESSED);
		BPVar* var = vars[it->var];
		int nrows = var->rows.size();
		for (int i = 0; i < nrows; ++i) {
			int cons = var->rows[i];
			double coeff = var->row_coeffs[i];

			if (coeff < 0) {
				minactivity[cons] -= coeff;
			} else {
				maxactivity[cons] -= coeff;
			}
		}
	}
}


inline void BPState::set_domain(int var, BPDomain domain, const vector<BPVar*>& vars, const vector<BPRow*>& rows,
                                vector<double>& minactivity, vector<double>& maxactivity)
{
	/* cannot switch from {0} to {1} or vice versa */
	assert(domains[var] == DOM_ZERO_ONE || domains[var] == domain);
	if (domains[var] == domain) {
		return;
	}

	/* update minactivity, maxactivity, and rhs, as necessary */
	// cout << "Setting domain: var " << var << " from " << domains[var] << " to " << domain << endl;
	int nrows = vars[var]->rows.size();
	for (int i = 0; i < nrows; ++i) {
		int cons = vars[var]->rows[i];
		double coeff = vars[var]->row_coeffs[i];

		if (coeff < 0) {
			minactivity[cons] -= coeff;
		} else {
			maxactivity[cons] -= coeff;
		}

		if (domain == DOM_ONE) {
			set_rhs(cons, rhs[cons] - coeff);
		}

		update_infeasibility(cons, minactivity[cons], maxactivity[cons], rows[cons]->sense);
		if (infeasible) {
			// cout << "Infeasibility at row " << cons << ", setting var " << var << endl;
			// if (rows[cons]->sense == SENSE_GE)
			//     cout << "[GE] maxactivity " << maxactivity[cons] << ", rhs " << rhs[cons] << ", coeff" << coeff << endl;
			// else
			//     cout << "[LE] minactivity " << minactivity[cons] << ", rhs " << rhs[cons] << ", coeff" << coeff << endl;
			// cout << "Row: " << *(rows[cons]) << endl;
			return; /* stop processing when detected infeasibility */
		}

		update_alwaysfeasibility(cons, minactivity[cons], maxactivity[cons], rows[cons]->sense);
	}

	domains.set_domain(var, domain);
}


inline void BPState::update_infeasibility(int cons, double minactivity, double maxactivity, RowSense sense)
{
	if (sense == SENSE_GE) {
		if (DBL_LT(maxactivity, rhs[cons])) {
			infeasible = true;
		}
	} else { // sense == SENSE_LE
		if (DBL_GT(minactivity, rhs[cons])) {
			infeasible = true;
		}
	}
}


inline void BPState::update_alwaysfeasibility(int cons, double minactivity, double maxactivity, RowSense sense)
{
	if (sense == SENSE_GE) {
		if (DBL_GE(minactivity, rhs[cons])) {
			set_rhs(cons, minactivity);
		}
	} else { // sense == SENSE_LE
		if (DBL_LE(maxactivity, rhs[cons])) {
			set_rhs(cons, maxactivity);
		}
	}
}


inline bool BPState::is_alwaysfeasible(int cons, RowSense sense, const vector<double>& minactivity, const vector<double>& maxactivity)
{
	if (sense == SENSE_GE) {
		assert(rhs[cons] >= minactivity[cons]);
		return DBL_EQ(rhs[cons], minactivity[cons]);
	} else {
		assert(rhs[cons] <= maxactivity[cons]);
		return DBL_EQ(rhs[cons], maxactivity[cons]);
	}
}


/** Assignment operator */
inline BPState& BPState::operator=(const BPState& rhs_state)
{
	rhs = rhs_state.rhs;
	domains = rhs_state.domains;
	infeasible = rhs_state.infeasible;
	return *this;
}


/** Print state */
inline void BPState::print()
{
	cout << "State: ";
	for (double rhs_val : rhs) {
		cout << rhs_val << " ";
	}
	cout << endl;
	cout << "Domains: ";
	for (int i = 0; i < domains.nvars; ++i) {
		if (domains[i] == DOM_ZERO_ONE) {
			cout << "* ";
		} else if (domains[i] == DOM_PROCESSED) {
			cout << "x ";
		} else {
			cout << (int) domains[i] << " ";
		}
	}
	cout << endl;
}


/** Print state via stream */
inline std::ostream& BPState::stream_write(std::ostream& os) const
{
	os << "[ State [ ";
	for (double rhs_val : rhs) {
		os << rhs_val << " ";
	}
	os << " ] Domains [ ";
	for (int i = 0; i < domains.nvars; ++i) {
		if (domains[i] == DOM_ZERO_ONE) {
			os << "* ";
		} else if (domains[i] == DOM_PROCESSED) {
			os << "x ";
		} else {
			os << (int) domains[i] << " ";
		}
	}
	os << " ] ]";

	// os << " Unset vars [ ";
	// for (int i = 0; i < (int) domains.size(); ++i) {
	//     if (domains[i] == DOM_ZERO_ONE)
	//         os << i << " ";
	// }
	// os << " ]";

	os << " Set vars [ ";
	for (BPDomainsSetConstIterator it = domains.begin_set(); it != domains.end_set(); it++) {
		os << it->var << " ";
	}
	os << " ]";
	os << " [0: " << domains.nvars_set_zero << ", 1: " << domains.nvars_set_one << "]";
	os << " Unset vars [ ";
	for (BPDomainsUnsetConstIterator it = domains.begin_unset(); it != domains.end_unset(); it++) {
		os << it->var << " ";
	}
	os << " ]";
	// os << " Unproc vars [ ";
	// for (BPDomainsUnprocConstIterator it = domains.begin_unproc(); it != domains.end_unproc(); it++) {
	//     os << it->var << " ";
	// }
	// os << " ]";
	if (infeasible) {
		os << " [Inf]";
	}

	return os;
}


#endif /* BPSTATE_HPP_ */


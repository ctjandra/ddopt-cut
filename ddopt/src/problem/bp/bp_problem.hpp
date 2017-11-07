/**
 * Binary problem class
 */

#ifndef BP_PROBLEM_HPP_
#define BP_PROBLEM_HPP_

#include "bp_instance.hpp"
#include "bp_state.hpp"
#include "../problem.hpp"
#include "prop_multipass.hpp"


class BinaryProblem : public Problem
{
public:

	vector<double>   minactivity;         /**< lower bound on activity per row */
	vector<double>   maxactivity;         /**< upper bound on activity per row */

	BPPropMultipass* propagator;          /**< propagator for domains */

	BPInstance*      instance;            /**< casted instance for convenience */


	BinaryProblem(BPInstance* _inst, vector<BPProp*> _propagators, Options* _opts) : Problem(_inst, _opts)
	{
		instance = static_cast<BPInstance*>(inst);

		// Wrap propagators in a multipass propagator
		if (!_propagators.empty()) {
			propagator = new BPPropMultipass(_propagators);
		} else {
			propagator = NULL;
		}
	}

	BinaryProblem(BPInstance* _inst, BPProp* _propagator, Options* _opts) :
		BinaryProblem(_inst, (_propagator == NULL) ? vector<BPProp*>() : vector<BPProp*>(1, _propagator), _opts) {}

	~BinaryProblem()
	{
		delete propagator;
	}

	BPState* create_initial_state()
	{
		BPState* state = new BPState(instance->nvars, instance->nrows);
		state->init_state_from_rows(instance->rows);
		return state;
	}

	bool cb_skip_var_for_long_arc(int var, State* state);
	void cb_initialize();
	void cb_layer_end(int current_var);
};


inline void BinaryProblem::cb_initialize()
{
	minactivity.resize(instance->nrows, 0.0);
	maxactivity.resize(instance->nrows, 0.0);
	for (int i = 0; i < instance->nrows; ++i) {
		for (double coeff : instance->rows[i]->coeffs) {
			minactivity[i] += MIN(0, coeff); /* minimum between possible evaluations of term a_k * x_k */
			maxactivity[i] += MAX(0, coeff); /* maximum between possible evaluations of term a_k * x_k */
		}
	}
}

inline bool BinaryProblem::cb_skip_var_for_long_arc(int var, State* state)
{
	BPState* state_bp = dynamic_cast<BPState*>(state);

	// all variables from previous layers must be set to DOM_PROCESSED
	// if the variable has domain {0}, the node will be skipped and relevant arcs will be long arcs
	bool skip_var = (state_bp->domains[var] == DOM_ZERO);

	// If variable is to be skipped, mark it as processed
	if (skip_var) {
		state_bp->mark_as_processed(var);
	}

	return skip_var;
}

inline void BinaryProblem::cb_layer_end(int current_var)
{
	/* update minactivity and maxactivity */
	BPVar* var = instance->vars[current_var];
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


#endif /* BP_PROBLEM_HPP_ */

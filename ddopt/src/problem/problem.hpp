/*
 * Abstract classes that need to be implemented in order to model a new problem to be solved.
 */

#ifndef PROBLEM_HPP_
#define PROBLEM_HPP_

#include "../core/order.hpp"
#include "../core/merge.hpp"
#include "../core/completion.hpp"
#include "../util/options.hpp"


// Reason this is not a template is because it becomes problematic to pass it around in State functions

/* Callbacks and problem-specific state during DD construction */
class Problem
{
public:
	Ordering*                     ordering;                    /**< ordering */
	Merger*                       merger;                      /**< merging technique */
	CompletionBound*              completion;                  /**< dual bound generator for pruning; may be NULL if unused */

	Instance*                     inst;                        /**< instance */
	Options*                      options;                     /**< options */


	Problem(Instance* _inst, Options* _opts) : inst(_inst), options(_opts)
	{
		ordering = NULL;
		merger = NULL;
		completion = NULL;
	}

	virtual ~Problem()
	{
		delete ordering;
		delete merger;
		delete completion;
	}

	virtual State* create_initial_state() = 0;


	// Callbacks

	/** Callback during initialization */
	virtual void cb_initialize() {}

	/** Callback during creation of a new state */
	virtual void cb_state_created(State* state) {}

	/** Callback during removal of a state */
	virtual void cb_state_removed(State* state) {}

	/** Callback at the end of a layer (current_var is the variable of the layer) */
	virtual void cb_layer_end(int current_var) {}

	/**
	 * If true is returned, the given variable will be skipped and considered to be a long arc.
	 * If following ZDD reduction rule: we skip (return true) when the only possible child is a zero-arc.
	 */
	virtual bool cb_skip_var_for_long_arc(int var, State* state)
	{
		return false;
	}


	// Error checking

	/**
	 * For error checking purposes, indicate whether problem expects a single terminal at the end of a DD construction.
	 * Does not need to be extended, but ideally should if a single terminal is expected so an error can be output otherwise.
	 */
	virtual bool expect_single_terminal()
	{
		return false;
	}


	// Functions to call both ordering and its own callbacks

	void callback_initialize()
	{
		ordering->cb_initialize();
		this->cb_initialize();
	}

	void callback_state_created(State* state)
	{
		ordering->cb_state_created(state);
		this->cb_state_created(state);
	}

	void callback_state_removed(State* state)
	{
		ordering->cb_state_removed(state);
		this->cb_state_removed(state);
	}

};


#endif /* PROBLEM_HPP_ */

/**
 * Independent set problem class
 */

#ifndef INDEPSET_PROBLEM_HPP_
#define INDEPSET_PROBLEM_HPP_

#include "indepset_instance.hpp"
#include "indepset_state.hpp"
#include "../problem.hpp"


/** Problem class for independent set */
class IndepSetProblem : public Problem
{
public:

	IndepSetInstance* instance;        /**< casted instance for convenience */


	IndepSetProblem(IndepSetInstance* _inst, Options* _opts) : Problem(_inst, _opts)
	{
		instance = static_cast<IndepSetInstance*>(inst);
	}

	~IndepSetProblem()
	{
	}

	IndepSetState* create_initial_state()
	{
		IntSet intset;
		intset.resize(0, instance->graph->n_vertices-1, true);
		return new IndepSetState(intset);
	}

	bool cb_skip_var_for_long_arc(int var, State* state);

	bool expect_single_terminal()
	{
		return true;    // Decision diagram should be reduced
	}
};


#endif /* INDEPSET_PROBLEM_HPP_ */

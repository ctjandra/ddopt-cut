/**
 * IP model class for CPLEX
 */

#ifndef MODEL_CPLEX_HPP_
#define MODEL_CPLEX_HPP_

#include <ilcplex/ilocplex.h>
#include "instance.hpp"
#include "../util/options.hpp"
#include "../bdd/bdd.hpp"

class ModelCplex
{
public:

	virtual ~ModelCplex() {}

	/** Create an IP model for the problem. The arguments model and vars are output. */
	virtual void create_ip_model(IloEnv env, IloCplex cplex, BDD* bdd, Options* options, IloModel& model, IloNumVarArray& x) = 0;
};


#endif /* MODEL_CPLEX_HPP_ */

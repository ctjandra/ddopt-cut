/**
 * Independent set IP model class
 */

#ifndef INDEPSET_MODEL_CPLEX_HPP_
#define INDEPSET_MODEL_CPLEX_HPP_

#include <ilcplex/ilocplex.h>
#include "indepset_instance.hpp"
#include "indepset_options.hpp"
#include "../model_cplex.hpp"
#include "../../util/options.hpp"

class IndepSetModelCplex : public ModelCplex
{
public:
	IndepSetInstance* inst;
	IndepSetOptions* indepset_options;

	IndepSetModelCplex(IndepSetInstance* _inst, IndepSetOptions* _indepset_options) : inst(_inst),
		indepset_options(_indepset_options) {}

	void create_ip_model(IloEnv env, IloCplex cplex, BDD* bdd, Options* options, IloModel& model, IloNumVarArray& x);
};


#endif /* INDEPSET_MODEL_CPLEX_HPP_ */

/**
 * BP model class
 */

#ifndef BP_MODEL_CPLEX_HPP_
#define BP_MODEL_CPLEX_HPP_

#include <string>
#include <ilcplex/ilocplex.h>
#include "../model_cplex.hpp"

class BPModelCplex : public ModelCplex
{
public:
	string filename;

	BPModelCplex(string _filename) : filename(_filename) {}

	void create_ip_model(IloEnv env, IloCplex cplex, BDD* bdd, Options* options, IloModel& model, IloNumVarArray& vars);
};


#endif /* BP_MODEL_CPLEX_HPP_ */

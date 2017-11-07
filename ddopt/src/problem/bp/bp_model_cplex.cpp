/**
 * BP model class
 */

#include "bp_model_cplex.hpp"

void BPModelCplex::create_ip_model(IloEnv env, IloCplex cplex, BDD* bdd, Options* options, IloModel& model,
	IloNumVarArray& vars)
{
	IloObjective obj;
	IloRangeArray rng(env);
	cplex.importModel(model, filename.c_str(), obj, vars, rng);
}

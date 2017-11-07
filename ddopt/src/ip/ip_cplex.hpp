/**
 * Main IP modeling and solving
 */

#ifndef IP_CPLEX_HPP_
#define IP_CPLEX_HPP_

#include <ilcplex/ilocplex.h>
#include "intpt_selector.hpp"
#include "../bdd/bdd.hpp"
#include "../util/util.hpp"
#include "../util/stats.hpp"
#include "../util/options.hpp"
#include "../core/solver.hpp"
#include "../core/orderings.hpp"
#include "../core/mergers.hpp"
#include "../problem/model_cplex.hpp"


/** Model the problem as an IP and solve it */
void solve_ip(Instance* inst, BDD* bdd, ModelCplex* model_builder, Options* options);

/** Add BDD flow constraints to the model */
void add_bdd_flow_constraints(IloEnv env, IloModel model, const IloNumVarArray x, BDD* bdd);

/** Add a BDD bound constraint to the model */
void add_bdd_bound_constraint(IloEnv env, IloModel model, const IloNumVarArray x, BDD* bdd, Instance* inst);


#endif /* IP_CPLEX_HPP_ */

/**
 * DD cut generation for CPLEX
 */

#ifndef CUT_CPLEX_HPP_
#define CUT_CPLEX_HPP_

#include <ilcplex/ilocplex.h>
#include <vector>
#include <boost/unordered_map.hpp>
#include "../util/graph.hpp"
#include "../util/options.hpp"
#include "../util/util.hpp"
#include "inequality.hpp"
#include "cut_info.hpp"

using namespace std;
using boost::unordered_map;


/** Generate DD cut */
Inequality* generate_bdd_inequality(BDD* bdd, const vector<double>& x, const vector<double>& interior_point, Options* options,
	CutInfo* cut_info=NULL);


// Perturbation methods to increase cut dimension

/** Apply iterative perturbation in order to obtain a facet exactly: optimize in each direction fixing variables */
void perturb_bdd_cut_iterative(IloEnv env, IloCplex cplex, IloModel model, const IloNumVarArray u, IloObjective obj, int nvars,
	const vector<double>& x, const vector<double>& interior_point);

/** Apply random perturbation in order to obtain a facet with high probability */
void perturb_bdd_cut_random(IloEnv env, IloCplex cplex, IloModel model, const IloNumVarArray u, IloObjective obj, int nvars,
	const vector<double>& x, const vector<double>& interior_point);


#endif /* CUT_CPLEX_HPP_ */

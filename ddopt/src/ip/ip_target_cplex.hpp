/**
 * CPLEX target cut callback functions
 */

#ifndef IP_TARGET_CPLEX_HPP_
#define IP_TARGET_CPLEX_HPP_

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


// Main cut callback defined explicitly on header for use in other files (equivalent to use of ILOUSERCUTCALLBACK6 macro)
class BddTargetCutCallbackI : public IloCplex::UserCutCallbackI
{
	IloNumVarArray vars;
	BDD* bdd;
	InteriorPointSelector* intpt_selector;
	Instance* inst;
	Options* options;
	bool objective_cut;

public:
	ILOCOMMONCALLBACKSTUFF(BddTargetCutCallback)

	BddTargetCutCallbackI(IloEnv env, IloNumVarArray _vars, BDD* _bdd, InteriorPointSelector* _intpt_selector,
	                               Instance* _inst, Options* _options, bool _objective_cut)
		: IloCplex::UserCutCallbackI(env), vars(_vars), bdd(_bdd), intpt_selector(_intpt_selector), inst(_inst),
		  options(_options), objective_cut(_objective_cut) {}

	void main();
};

IloCplex::Callback BddTargetCutCallback(IloEnv env, IloNumVarArray vars, BDD* bdd, InteriorPointSelector* intpt_selector,
        Instance* inst, Options* options, bool objective_cut);


#endif /* IP_TARGET_CPLEX_HPP_ */

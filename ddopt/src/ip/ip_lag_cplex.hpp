/**
 * CPLEX cut callback functions via Lagrangian relaxation
 */

#ifndef IP_LAG_CPLEX_HPP_
#define IP_LAG_CPLEX_HPP_

#include <ilcplex/ilocplex.h>
#include "../bdd/bdd.hpp"
#include "../util/options.hpp"
#include "../cut/inequality.hpp"
#include "../cut/cut_info.hpp"
#include "../util/stats.hpp"

#ifdef USE_CONICBUNDLE

#include "CBSolver.hxx"
using namespace ConicBundle;

#endif // USE_CONICBUNDLE


// Cut callbacks defined explicitly on header for use in other files (equivalent to use of ILOUSERCUTCALLBACKx macro)

class BddLagrangianCutCallbackI : public IloCplex::UserCutCallbackI
{
	IloNumVarArray vars;
	BDD* bdd;
	vector<double> obj_layer; // objective in layer space
	Options* options;

public:
	ILOCOMMONCALLBACKSTUFF(BddLagrangianCutCallback)

	BddLagrangianCutCallbackI(IloEnv env, IloNumVarArray _vars, BDD* _bdd, const vector<double>& _obj_layer, Options* _options)
		: IloCplex::UserCutCallbackI(env), vars(_vars), bdd(_bdd), obj_layer(_obj_layer), options(_options)
	{
		assert((int) obj_layer.size() == bdd->nvars());
	}

	void main();
};

IloCplex::Callback BddLagrangianCutCallback(IloEnv env, IloNumVarArray vars, BDD* bdd, const vector<double>& obj_layer,
        Options* options);

Inequality* generate_lagrangian_cut_subgradient(BDD* bdd, const vector<double>& x, const vector<double>& obj_layer,
        int iteration_limit, int iterations_beyond_validity);

Inequality* generate_lagrangian_cut_cb(BDD* bdd, const vector<double>& x, const vector<double>& obj_layer,
                                       int iteration_limit, int iterations_beyond_validity);


#ifdef USE_CONICBUNDLE

/** Subproblem wrapper for ConicBundle */
class LagrangianCutSubproblemCB : public FunctionOracle
{
private:
	BDD* bdd;
	vector<double> x_to_separate; // in layer space
	Stats stats;
	int neval;

public:

	LagrangianCutSubproblemCB(BDD* _bdd, const vector<double>& _x_to_separate) : bdd(_bdd), x_to_separate(_x_to_separate)
	{
		stats.register_name("lrcut_subprob");
		neval = 0;
	}

	int evaluate(const DVector& lambdas, double relprec, double& objval, DVector& cut_vals,
	             vector<DVector>& subgradients, vector<PrimalData*>& primal_solutions, PrimalExtender*&)
	{
		stats.start_timer(0);
		int nvars = bdd->nvars();

		// Compute subproblem
		vector<int> path_x(nvars);
		double rhs = bdd->get_optimal_path(lambdas, path_x, true);

		double violation = 0; // negative violation (we are minimizing)
		for (int i = 0; i < nvars; ++i) {
			violation -= lambdas[i] * x_to_separate[i];
		}
		violation += rhs;

		// Return new subproblem value
		objval = violation;
		cut_vals.push_back(objval);

		// Return new subgradient
		DVector subg(nvars);
		for (int i = 0; i < nvars; ++i) {
			subg[i] = - (x_to_separate[i] - path_x[i]); // minimization
		}
		subgradients.push_back(subg);

		stats.end_timer(0);
		neval++;

		return 0;
	}

	double get_time()
	{
		return stats.get_time(0);
	}

	int get_number_evaluations()
	{
		return neval;
	}
};

#endif // USE_CONICBUNDLE

#endif // IP_LAG_CPLEX_HPP_
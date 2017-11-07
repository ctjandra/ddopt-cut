/**
 * Independent set IP model class
 */

#include "indepset_model_cplex.hpp"
#include "../../util/stats.hpp"

void IndepSetModelCplex::create_ip_model(IloEnv env, IloCplex cplex, BDD* bdd, Options* options, IloModel& model, IloNumVarArray& x)
{
	cout << "\n";
	cout << "-------------------------------------------" << endl;
	if (indepset_options->clique_formulation) {
		cout << "IP - Clique formulation" << endl;
	} else {
		cout << "IP - Edge formulation" << endl;
	}
	cout << "-------------------------------------------" << endl;
	cout << "\n";

	for (int i = 0; i < inst->graph->n_vertices; ++i) {
		if (!indepset_options->lp_relaxation) {
			x.add(IloIntVar(env, 0, 1, ("x" + to_string(i)).c_str()));
		} else {
			x.add(IloNumVar(env, 0, 1, ("x" + to_string(i)).c_str()));
		}
	}

	IloExpr obj(env);
	for (int i = 0; i < inst->graph->n_vertices; ++i) {
		obj += inst->weights[i] * x[i];
	}
	model.add(IloMaximize(env, obj));

	if (indepset_options->clique_formulation) {

		// Clique formulation
		Stats stats;
		stats.register_name("clique");
		stats.start_timer(0);

		vector<vector<int>> cliques;
		clique_decomposition(inst->graph, cliques);

		stats.end_timer(0);
		cout << "Time to generate cliques: " << stats.get_time(0) << endl;

		for (int i = 0; i < (int)cliques.size(); ++i) {
			IloExpr clique_expr(env);
			for (int j = 0; j < (int)cliques[i].size(); ++j) {
				clique_expr = clique_expr + x[cliques[i][j]];
			}
			model.add(clique_expr <= 1);
		}
	} else {

		// Edge formulation
		for (int i = 0; i < inst->graph->n_vertices; ++i) {
			for (int j = i+1; j < inst->graph->n_vertices; ++j) {
				if (inst->graph->is_adj(i, j)) {
					model.add(x[i] + x[j] <= 1);
				}
			}
		}
	}
}

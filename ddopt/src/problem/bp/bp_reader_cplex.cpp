/** 
 * Generator of a BP instance off an MPS file using CPLEX
 */

#include <cstring>
#include <algorithm>
#include "bp_reader_cplex.hpp"
#include "../../util/util.hpp"


BPInstance* read_bp_instance_cplex_mps(string mps_filename, BPReaderFilter* filter /*= NULL*/)
{
	IloEnv env;
	IloModel model(env);
	IloCplex cplex(env);

	IloObjective obj;
	IloNumVarArray vars(env);
	IloRangeArray rows(env);
	cplex.importModel(model, mps_filename.c_str(), obj, vars, rows);
	cplex.extract(model);

	vector<BPVar*> vars_in_bdd;
	vector<BPRow*> rows_in_bdd;

	int nvars = vars.getSize();
	int nrows = rows.getSize();

	// Create BPVars
	int idx = 0;
	for (int i = 0; i < nvars; ++i) {

		if (filter != NULL && filter->exclude_var(string(vars[i].getName()))) {
			continue;
		}

		// Get coefficient; unfortunately no simpler way
		IloNum obj_coeff = 0;
		for (IloExpr::LinearIterator it = obj.getLinearIterator(); it.ok(); ++it) {
			if (it.getVar().getId() == vars[i].getId()) {
				obj_coeff = it.getCoef();
				break;
			}
		}

		cout << vars[i] << " / obj: " << obj_coeff << " / id: " << vars[i].getId() << endl;

		BPVar* bpvar = new BPVar(vars[i], obj_coeff, idx);
		vars_in_bdd.push_back(bpvar);
		idx++;
	}

	// Create BPRows
	for (int i = 0; i < nrows; ++i) {

		if (filter != NULL && filter->exclude_cons(string(rows[i].getName()))) {
			continue;
		}

		IloNum lb = rows[i].getLB();
		IloNum ub = rows[i].getUB();
		cout << rows[i] << endl;
		cout << "LB: " << lb << " / UB: " << ub << endl;

		if (lb != -IloInfinity) {
			BPRow* bprow = new BPRow(rows[i], SENSE_GE, vars_in_bdd);
			if (bprow->nnonz > 0 && DBL_GT(bprow->rhs, bprow->calculate_minactivity()))	{
				rows_in_bdd.push_back(bprow);
			} else {
				delete bprow;
			}
		}

		if (ub != IloInfinity) {
			BPRow* bprow = new BPRow(rows[i], SENSE_LE, vars_in_bdd);
			if (bprow->nnonz > 0 && DBL_LT(bprow->rhs, bprow->calculate_maxactivity()))	{
				rows_in_bdd.push_back(bprow);
			} else {
				delete bprow;
			}
		}

		for (IloExpr::LinearIterator it = rows[i].getLinearIterator(); it.ok(); ++it) {
			IloNum coeff = it.getCoef();
			IloNumVar var = it.getVar();
			cout << " + " << coeff << " " << var;
		}
		cout << endl;
	}

	/* Update rows in vars */
	for (int i = 0; i < (int) vars_in_bdd.size(); ++i) {
		vars_in_bdd[i]->init_rows(rows_in_bdd);
	}

	BPInstance* inst = new BPInstance(vars_in_bdd, rows_in_bdd);

	return inst;
}

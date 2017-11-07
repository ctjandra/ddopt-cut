/**
 * Variable for binary problem 
 */

#include "bpvar.hpp"
#include "bprow.hpp"


#ifdef SOLVER_CPLEX

BPVar::BPVar(IloNumVar& var, IloNum var_obj, int _index) : index(_index)
{
	obj = -var_obj; /* CPLEX minimizes by default and we maximize */
	solver_index = var.getId();
}

#endif


void BPVar::init_rows(const vector<BPRow*>& all_rows)
{
	int nrows = all_rows.size();
	for (int i = 0; i < nrows; ++i) {
		BPRow* row = all_rows[i];

		// This could be improved with a mapping, but problem construction is not a bottleneck
		for (int j = 0; j < row->nnonz; ++j) {
			if (row->ind[j] == index) {
				rows.push_back(i);
				row_coeffs.push_back(row->coeffs[j]);
				break;
			}
		}

	}
}


/**
 * Row for binary problem 
 */

#include <algorithm>
#include <iterator>
#include "bprow.hpp"
#include "bpvar.hpp"


#ifdef SOLVER_CPLEX

BPRow::BPRow(IloRange row, RowSense _sense, const vector<BPVar*>& free_vars, bool consider_fixed)
{
	sense = _sense;
	type = "linear";
	rhs = (sense == SENSE_GE) ? row.getLB() : row.getUB();

	nnonz = 0;
	for (IloExpr::LinearIterator it = row.getLinearIterator(); it.ok(); ++it) {
		IloNum coeff = it.getCoef();
		IloNumVar var = it.getVar();

		// Take variables that are not fixed or update rhs for those that are
		if (!consider_fixed || !DBL_EQ(var.getLB(), var.getUB())) {

			int cur_ind = -1;
			int nfree_vars = free_vars.size();

			// This could be improved with a mapping, but problem construction is not a bottleneck
			for (int j = 0; j < nfree_vars; ++j) {
				if (free_vars[j]->solver_index == var.getId()) {
					assert(free_vars[j]->index == j);
					cur_ind = j;
					break;
				}
			}

			// Variable must be in the array of unfixed variables
			assert(cur_ind >= 0);

			coeffs.push_back(coeff);
			ind.push_back(cur_ind);
			nnonz++;

		} else { // i-th variable is fixed to var_lb
			rhs -= coeff * var.getLB();
		}
	}

	nnonz = ind.size();
	assert(nnonz == (int) coeffs.size());
}

#endif

double BPRow::calculate_minactivity()
{
	double minactivity = 0;
	for (double coeff : coeffs) {
		if (coeff < 0) {
			minactivity += coeff;
		}
	}
	return minactivity;
}

double BPRow::calculate_maxactivity()
{
	double maxactivity = 0;
	for (double coeff : coeffs) {
		if (coeff > 0) {
			maxactivity += coeff;
		}
	}
	return maxactivity;
}

double BPRow::get_coeff(int var)
{
	vector<int>::iterator it = std::find(ind.begin(), ind.end(), var);
	if (it == ind.end()) { // not found
		return 0;
	}
	int i = std::distance(ind.begin(), it);
	return coeffs[i];
}

ostream& operator<<(std::ostream& os, const BPRow& row)
{
	for (int i = 0; i < row.nnonz; ++i) {
		os << " + " << row.coeffs[i] << "<x" << row.ind[i] << ">";
	}
	if (row.sense == SENSE_LE) {
		os << " <= ";
	} else {
		os << " >= ";
	}
	os << row.rhs << " ";
	return os;
}

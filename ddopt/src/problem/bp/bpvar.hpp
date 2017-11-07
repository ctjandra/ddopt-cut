/**
 * Variable for binary problem 
 */

#ifndef BPVAR_HPP_
#define BPVAR_HPP_

#ifdef SOLVER_CPLEX
#include <ilcplex/ilocplex.h>
#endif

#include <vector>
#include <string>

using namespace std;

struct BPRow; /* forward declaration */

/**
 * Variable
 */
struct BPVar {
	double obj;                /**< objective value for variable */
	vector<int> rows;          /**< row indices in which this variable has a nonzero coefficient */
	vector<double> row_coeffs; /**< row coefficients for each row in rows */
	int index;                 /**< unique identifier for this variable; will be used in ind in BPRows */

	int solver_index;          /**< solver's index of var (used only at initialization of rows/vars) */

#ifdef SOLVER_CPLEX
	/** Constructor from CPLEX var (does not set rows; use init_rows) */
	BPVar(IloNumVar& var, IloNum var_obj, int _index);
#endif

	/** Initialize rows and row_coeffs based on given rows */
	void init_rows(const vector<BPRow*>& all_rows);
};

#endif /* BPVAR_HPP_ */

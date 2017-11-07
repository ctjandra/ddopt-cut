/**
 * Row for binary problem 
 */

#ifndef BPROW_HPP_
#define BPROW_HPP_

#include "bpvar.hpp"

#include <vector>
#include <iostream>

#include "../../util/util.hpp"

using namespace std;

enum RowSense {
	SENSE_LE = 0,
	SENSE_GE
};

/**
 * Row representing a constraint (with only the relevant variables, unlike the solver's row structure)
 */
struct BPRow {
	double rhs;              /**< right-hand side of constraint */
	RowSense sense;          /**< sense of constraint (<= or >=) */
	vector<double> coeffs;   /**< coefficients of the constraint */
	vector<int> ind;         /**< variable indices of the constraint (w.r.t. a given array at construction) */
	int nnonz;               /**< number of coefficients/variables in the constraint */
	const char* type;        /**< type of constraint */

#ifdef SOLVER_CPLEX
	BPRow(IloRange row, RowSense _sense, const vector<BPVar*>& free_vars, bool consider_fixed=true);
#endif

	/** Calculate the minimum left-hand side value given that variables are binary */
	double calculate_minactivity();

	/** Calculate the maximum left-hand side value given that variables are binary */
	double calculate_maxactivity();

	/** Returns the coefficient of the given variable. Linear search. */
	double get_coeff(int var);

	/** Prints row */
	friend ostream& operator<<(std::ostream& os, const BPRow& row);
};

#endif /* BPROW_HPP_ */

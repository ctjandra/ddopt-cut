/**
 * Simple inequality structure
 */

#ifndef INEQUALITY_HPP_
#define INEQUALITY_HPP_

struct Inequality {
	vector<double>	coeffs;
	double			rhs;

	Inequality() {}

	Inequality(const vector<double>& _coeffs, double _rhs) : coeffs(_coeffs), rhs(_rhs) {}
};

#endif // INEQUALITY_HPP_
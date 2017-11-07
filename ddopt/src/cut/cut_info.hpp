/** 
 * Functions to extract information about cut generated for investigative purposes; some of these may be time-consuming
 */

#ifndef CUT_INFO_HPP_
#define CUT_INFO_HPP_

#include <vector>
#include "../bdd/bdd.hpp"
#include "../bdd/bdd_pass.hpp"
#include "inequality.hpp"

using namespace std;

/** Structure to store information about a cut after it is generated */
struct CutInfo {
	vector<vector<double>> zero_arc_flow;
	vector<vector<double>> one_arc_flow;
};

/** Pass function that stores minimum partial distance to a given point */
class MinDistanceToPointPassFunc : public BDDPassFunc
{
	vector<double> point;
	bool layer_space;        /**< point is given in layer space if true, variable space if false */
	bool manhattan;          /**< Manhattan distance if true, Euclidean distance if false */

public:
	MinDistanceToPointPassFunc(const vector<double>& _point, bool _layer_space, bool _manhattan=false) :
		point(_point), layer_space(_layer_space), manhattan(_manhattan) {}

	double start_val()
	{
		return 0;
	}

	double init_val()
	{
		return numeric_limits<double>::infinity(); /* minimum */
	}

	double apply(int layer, int var, int arc_val, double source_val, double target_val, Node* source, Node* target)
	{
		int idx = (layer_space) ? layer : var;
		double distance = source_val;
		if (manhattan) {
			distance += abs(point[idx] - arc_val);
		} else {
			distance += (point[idx] - arc_val) * (point[idx] - arc_val);
		}
		if (DBL_LT(distance, target_val)) {
			return distance;
		} else {
			return target_val;
		}
	}
};


/** Pass function that stores minimum dot product to a given point */
class MinDotProductToPointPassFunc : public BDDPassFunc
{
	vector<double> point;
	bool layer_space;        /**< point is given in layer space if true, variable space if false */

public:
	MinDotProductToPointPassFunc(const vector<double>& _point, bool _layer_space) :
		point(_point), layer_space(_layer_space) {}

	double start_val()
	{
		return 0;
	}

	double init_val()
	{
		return numeric_limits<double>::infinity(); /* minimum */
	}

	double apply(int layer, int var, int arc_val, double source_val, double target_val, Node* source, Node* target)
	{
		int idx = (layer_space) ? layer : var;
		double prodval = source_val + point[idx] * arc_val;
		if (DBL_LT(prodval, target_val)) {
			return prodval;
		} else {
			return target_val;
		}
	}
};


/** Print distances from separating point at each node */
void print_distances_separating_point(BDD* bdd, const vector<double>& x_layer);

/** Print values of a pass function */
void print_pass_func_values(BDD* bdd, BDDPassFunc* pass_func);

/** Print dot product w.r.t. separating point at each node */
void print_dot_product_point(BDD* bdd, const vector<double>& x_layer);

/** Calculate distance between a hyperplane defined by an inequality and a point x (distance cut off / efficacy) */
double get_distance_hyperplane_point(Inequality* inequality, const vector<double>& x);

/**
 * Calculate the cosine of the angle between (the normal vector of) two inequalities.
 * If include_rhs is true, interpret them as (coefficients, rhs); if false, as coefficients vector only.
 */
double get_cos_angle_inequalities(Inequality* ineq1, Inequality* ineq2, bool include_rhs=true);

/**
 * Calculate the angle between (the normal vector of) two inequalities.
 * If include_rhs is true, interpret them as (coefficients, rhs); if false, as coefficients vector only.
 */
double get_angle_inequalities(Inequality* ineq1, Inequality* ineq2, bool include_rhs=true);

#endif // CUT_INFO_HPP_
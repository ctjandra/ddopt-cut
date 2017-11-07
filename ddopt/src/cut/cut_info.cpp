/** 
 * Functions to extract information about cut generated for investigative purposes; some of these may be time-consuming
 */

#include "cut_info.hpp"


void print_distances_separating_point(BDD* bdd, const vector<double>& x_layer)
{
	MinDistanceToPointPassFunc pass_func(x_layer, true, false);
	bdd_pass(bdd, &pass_func, &pass_func);

	int bdd_size = bdd->layers.size();
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = bdd->layers[layer].size();
		cout << "Layer " << layer << ":  ";
		for (int k = 0; k < size; ++k) {
			Node* node = bdd->layers[layer][k];
			double td_val = boost::any_cast<BDDPassValues*>(node->temp_data)->top_down_val;
			// double bu_val = boost::any_cast<BDDPassValues*>(node->temp_data)->bottom_up_val;
			cout << td_val << " ";
		}
		cout << endl;
	}
	cout << endl;

	bdd_pass_clean_up(bdd);
}


void print_pass_func_values(BDD* bdd, BDDPassFunc* pass_func)
{
	bdd_pass(bdd, pass_func, pass_func);

	int bdd_size = bdd->layers.size();
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = bdd->layers[layer].size();
		cout << "Layer " << layer << ":  ";
		for (int k = 0; k < size; ++k) {
			Node* node = bdd->layers[layer][k];
			double td_val = boost::any_cast<BDDPassValues*>(node->temp_data)->top_down_val;
			// double bu_val = boost::any_cast<BDDPassValues*>(node->temp_data)->bottom_up_val;
			cout << td_val << " ";
		}
		cout << endl;
	}
	cout << endl;

	bdd_pass_clean_up(bdd);
}


void print_dot_product_point(BDD* bdd, const vector<double>& x_layer)
{
	BDDPassFunc* pass_func = new MinDotProductToPointPassFunc(x_layer, true);
	print_pass_func_values(bdd, pass_func);
	delete pass_func;
}


double get_distance_hyperplane_point(Inequality* inequality, const vector<double>& x)
{
	int nvars = x.size();
	assert(nvars == (int) inequality->coeffs.size());
	double lhs = 0;
	double norm = 0;
	for (int i = 0; i < nvars; ++i) {
		lhs += inequality->coeffs[i] * x[i];
		norm += inequality->coeffs[i] * inequality->coeffs[i];
	}
	double distance = abs(lhs - inequality->rhs) / sqrt(norm);
	return distance;
}


double get_cos_angle_inequalities(Inequality* ineq1, Inequality* ineq2, bool include_rhs)
{
	int nvars = ineq1->coeffs.size();
	assert(ineq1->coeffs.size() == ineq2->coeffs.size());
	double dp = 0;
	double norm1 = 0;
	double norm2 = 0;
	for (int i = 0; i < nvars; ++i) {
		dp += ineq1->coeffs[i] * ineq2->coeffs[i];
		norm1 += ineq1->coeffs[i] * ineq1->coeffs[i];
		norm2 += ineq2->coeffs[i] * ineq2->coeffs[i];
	}
	if (include_rhs) {
		dp += ineq1->rhs * ineq2->rhs;
		norm1 += ineq1->rhs * ineq1->rhs;
		norm2 += ineq2->rhs * ineq2->rhs;
	}
	norm1 = sqrt(norm1);
	norm2 = sqrt(norm2);
	return dp / (norm1 * norm2);
}


double get_angle_inequalities(Inequality* ineq1, Inequality* ineq2, bool include_rhs)
{
	double cosangle = get_cos_angle_inequalities(ineq1, ineq2, include_rhs);
	if (DBL_EQ(cosangle, 1)) {
		return 0;
	}
	return acos(cosangle);
}

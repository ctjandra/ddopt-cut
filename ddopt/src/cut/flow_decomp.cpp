/**
 * Flow decomposition methods
 */

#include "flow_decomp.hpp"


void decompose_paths_from_flow(BDD* bdd, vector<vector<double>>& zero_arc_flow, vector<vector<double>>& one_arc_flow,
                               vector<vector<int>>& paths, vector<double>& path_weights)
{
	vector<double> weights(0);
	decompose_paths_from_flow(bdd, weights, zero_arc_flow, one_arc_flow, paths, path_weights);
}


void decompose_paths_from_flow(BDD* bdd, const vector<double>& weights, vector<vector<double>>& zero_arc_flow,
                               vector<vector<double>>& one_arc_flow, vector<vector<int>>& paths, vector<double>& path_weights)
{
	paths.clear();
	path_weights.clear();

	double path_weight = +numeric_limits<double>::infinity();
	while (path_weight > 0) {
		vector<int> path;

		if (weights.size() == 0) {
			path_weight = extract_lexmin_path_from_flow(bdd, zero_arc_flow, one_arc_flow, path);
		} else {
			path_weight = extract_optimal_path_from_flow(bdd, weights, zero_arc_flow, one_arc_flow, path);
		}

		if (path_weight > 0) {
			remove_path_from_flow(bdd, zero_arc_flow, one_arc_flow, path, path_weight);
			paths.push_back(path);
			path_weights.push_back(path_weight);
		}
	}
}


double extract_lexmin_path_from_flow(BDD* bdd, const vector<vector<double>>& zero_arc_flow,
                                     const vector<vector<double>>& one_arc_flow, vector<int>& path)
{
	// Implicit in initializing path is the assumption that long arcs are all zero except for the first variable
	path.resize(bdd->nvars());
	fill(path.begin(), path.end(), 0);

	double path_weight = +numeric_limits<double>::infinity();

	Node* node = bdd->get_root_node();
	Node* terminal = bdd->get_terminal_node();

	// Solver optimality tolerance is used to determine if flow is zero or not
	// The floating epsilon of 1e-9 used throughout this code is too small and causes problems

	// Return 0 if flow is zero
	if ((node->zero_arc == NULL || DBL_EQ_TOL(zero_arc_flow[node->layer][node->id], 0, OPT_TOL))
	        && (node->one_arc == NULL || DBL_EQ_TOL(one_arc_flow[node->layer][node->id], 0, OPT_TOL))) {
		return 0;
	}

	// Extract lexicographically smallest path (i.e. following 0-arcs first, then 1-arcs)
	while (node != terminal) {
		int layer = node->layer;
		int id = node->id;
		if (node->zero_arc != NULL && DBL_GT_TOL(zero_arc_flow[layer][id], 0, OPT_TOL)) {
			double flow_val = zero_arc_flow[layer][id];
			path[layer] = 0;
			node = node->zero_arc;

			if (DBL_LT_TOL(flow_val, path_weight, OPT_TOL)) {
				path_weight = flow_val;
			}

		} else if (node->one_arc != NULL && DBL_GT_TOL(one_arc_flow[layer][id], 0, OPT_TOL)) {
			double flow_val = one_arc_flow[layer][id];
			path[layer] = 1;
			node = node->one_arc;

			if (DBL_LT_TOL(flow_val, path_weight, OPT_TOL)) {
				path_weight = flow_val;
			}

		} else {
			double inflow = 0;
			for (Node* zero_ancestor : node->zero_ancestors) {
				inflow += zero_arc_flow[zero_ancestor->layer][zero_ancestor->id];
			}
			for (Node* one_ancestor : node->one_ancestors) {
				inflow += one_arc_flow[one_ancestor->layer][one_ancestor->id];
			}
			double outflow = 0;
			if (node->zero_arc != NULL) {
				outflow += zero_arc_flow[layer][id];
			}
			if (node->one_arc != NULL) {
				outflow += one_arc_flow[layer][id];
			}
			cout << "Error: Flow decomposition reached a node with flow imbalance" << endl;
			cout << "       Layer: " << layer << " / Id: " << id << endl;
			cout << "       Inflow: " << inflow << " / Outflow: " << outflow << endl;
			// exit(1);
			return 0;
		}
	}

	// assert(DBL_GT_TOL(path_weight, 0, OPT_TOL));

	return path_weight;
}


double extract_optimal_path_from_flow(BDD* bdd, const vector<double>& weights, const vector<vector<double>>& zero_arc_flow,
                                      const vector<vector<double>>& one_arc_flow, vector<int>& optimal_path)
{
	int bdd_size = bdd->layers.size();

	assert(bdd->layers.size() > 0);
	assert((int) weights.size() == bdd->nvars());

	int initial_layer = bdd->get_root_layer();

	// Initialize auxiliary variables
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = bdd->layers[layer].size();
		for (int k = 0; k < size; ++k) {
			bdd->layers[layer][k]->lp_value = -numeric_limits<double>::infinity();
			bdd->layers[layer][k]->lp_parent = NULL;
			bdd->layers[layer][k]->lp_parent_arctype = -1;
		}
	}
	bdd->layers[initial_layer][0]->lp_value = 0;

	// Compute weights
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = bdd->layers[layer].size();
		for (int k = 0; k < size; ++k) {
			// cout << "At node " << layer << ", " << k << ":";
			// if (bdd->layers[layer][k]->zero_arc != NULL)
			// 	cout << " f0 -> " << zero_arc_flow[layer][k];
			// if (bdd->layers[layer][k]->one_arc != NULL)
			// 	cout << " f1 -> " << one_arc_flow[layer][k];
			// cout << endl;

			if (bdd->layers[layer][k]->zero_arc != NULL && DBL_GT_TOL(zero_arc_flow[layer][k], 0, OPT_TOL) &&
			        (bdd->layers[layer][k]->lp_value > bdd->layers[layer][k]->zero_arc->lp_value)) {
				bdd->layers[layer][k]->zero_arc->lp_value = bdd->layers[layer][k]->lp_value;
				bdd->layers[layer][k]->zero_arc->lp_parent = bdd->layers[layer][k];
				bdd->layers[layer][k]->zero_arc->lp_parent_arctype = 0;
				// cout << "    0-arc to " << bdd->layers[layer][k]->zero_arc->layer << ", " << bdd->layers[layer][k]->zero_arc->id << endl;
			}
			if (bdd->layers[layer][k]->one_arc != NULL && DBL_GT_TOL(one_arc_flow[layer][k], 0, OPT_TOL) &&
			        (bdd->layers[layer][k]->lp_value + weights[layer] > bdd->layers[layer][k]->one_arc->lp_value)) {
				bdd->layers[layer][k]->one_arc->lp_value = bdd->layers[layer][k]->lp_value + weights[layer];
				bdd->layers[layer][k]->one_arc->lp_parent = bdd->layers[layer][k];
				bdd->layers[layer][k]->one_arc->lp_parent_arctype = 1;
				// cout << "    1-arc to " << bdd->layers[layer][k]->one_arc->layer << ", " << bdd->layers[layer][k]->one_arc->id << endl;
			}
		}
	}

	// Extract optimal path
	Node* node = bdd->layers[bdd_size-1][0];

	if (node->lp_parent == NULL) {
		// Terminal node was unreachable due to pruning + skipping relaxed nodes
		optimal_path.resize(0);
		return 0;
	}

	optimal_path.resize(bdd_size - 1);
	fill(optimal_path.begin(), optimal_path.end(), 0);
	double path_flow_val = numeric_limits<double>::infinity();

	while (node->lp_parent != NULL) {
		double flow_val;
		if (node->lp_parent_arctype == 0) {
			flow_val = zero_arc_flow[node->lp_parent->layer][node->lp_parent->id];
		} else { // node->lp_parent_arctype == 1
			flow_val = one_arc_flow[node->lp_parent->layer][node->lp_parent->id];
		}
		if (DBL_LT_TOL(flow_val, path_flow_val, OPT_TOL)) {
			path_flow_val = flow_val;
		}

		optimal_path[node->lp_parent->layer] = node->lp_parent_arctype;
		node = node->lp_parent;
	}
	assert(node->layer == initial_layer);

	return path_flow_val;
}


void remove_path_from_flow(BDD* bdd, vector<vector<double>>& zero_arc_flow, vector<vector<double>>& one_arc_flow,
                           const vector<int>& path, double path_weight)
{
	// Remove path from flow; path must be valid
	Node* node = bdd->get_root_node();
	Node* terminal = bdd->get_terminal_node();
	while (node != terminal) {
		int layer = node->layer;
		assert(path[layer] == 0 || path[layer] == 1);
		if (path[layer] == 0) {
			if (node->zero_arc == NULL) {
				cout << "Error: Infeasible path in path decomposition" << endl;
				exit(1);
			}
			assert(DBL_GT_TOL(zero_arc_flow[layer][node->id], 0, OPT_TOL));
			zero_arc_flow[layer][node->id] -= path_weight;
			node = node->zero_arc;
		} else { // path[layer] == 1
			if (node->one_arc == NULL) {
				cout << "Error: Infeasible path in path decomposition" << endl;
				exit(1);
			}
			assert(DBL_GT_TOL(one_arc_flow[layer][node->id], 0, OPT_TOL));
			one_arc_flow[layer][node->id] -= path_weight;
			node = node->one_arc;
		}
	}

}


void print_all_paths_in_flow(BDD* bdd, vector<vector<double>>& zero_arc_flow, vector<vector<double>>& one_arc_flow)
{
	int count = 0;
	Node* root = bdd->get_root_node();
	Node* terminal = bdd->get_terminal_node();

	vector<Node*> node_stack;
	vector<int> last_traversed_stack; // node_stack of last arc traversed; -1 means no arc traversed yet

	node_stack.push_back(root);
	last_traversed_stack.push_back(-1);

	while (node_stack.size() > 0) {
		assert(node_stack.size() == last_traversed_stack.size());

		for (Node* node : node_stack) {
			cout << node->layer << " ";
		}
		cout << endl;
		for (int lt : last_traversed_stack) {
			cout << lt << " ";
		}
		cout << endl;
		cout << endl;

		Node* node = node_stack.back();
		int last_traversed = last_traversed_stack.back();

		if (node == NULL) {
			node_stack.pop_back();
			last_traversed_stack.pop_back();
			continue; // Go back
		}

		if (node == terminal) {
			// Print path
			vector<int> path = bdd_convert_node_vector_to_path(bdd, node_stack);
			count++;
			cout << "Path " << count << ":  ";
			for (int k : path) {
				cout << k << " ";
			}
			cout << endl;
			node_stack.pop_back();
			last_traversed_stack.pop_back();
			continue;
		}

		if (last_traversed == -1) {
			// No arc traversed yet; traverse 0-arc now (NULL cases handled above)
			if (DBL_GT_TOL(zero_arc_flow[node->layer][node->id], 0, OPT_TOL)) {
				node_stack.push_back(node->zero_arc);
			} else {
				node_stack.push_back(NULL);
			}
			last_traversed_stack.pop_back();
			last_traversed_stack.push_back(0);
			last_traversed_stack.push_back(-1);
		} else if (last_traversed == 0) {
			// Last arc traversed is 0-arc; traverse 1-arc now (NULL cases handled above)
			if (DBL_GT_TOL(one_arc_flow[node->layer][node->id], 0, OPT_TOL)) {
				node_stack.push_back(node->one_arc);
			} else {
				node_stack.push_back(NULL);
			}
			last_traversed_stack.pop_back();
			last_traversed_stack.push_back(1);
			last_traversed_stack.push_back(-1);
		} else if (last_traversed == 1) {
			// Done with node, remove from stack
			node_stack.pop_back();
			last_traversed_stack.pop_back();
		}
	}

	cout << "Number of positive flow paths: " << count << endl;
}


vector<int> bdd_convert_node_vector_to_path(BDD* bdd, const vector<Node*>& nodes)
{
	vector<int> path(bdd->nvars(), -1);

	int size = nodes.size();
	for (int i = 0; i < size - 1; ++i) {
		Node* node = nodes[i];
		Node* next = nodes[i+1];
		if (node->zero_arc != next && node->one_arc != next) {
			cout << "Error: Converting node vector to path failed (nodes not in sequence)" << endl;
			exit(1);
		}
		assert(node->layer < next->layer); // must be true if next is a child of node

		if (node->zero_arc == next) {
			path[node->layer] = 0;
		} else if (node->one_arc == next) {
			path[node->layer] = 1;
		}

		// Fill in for long arcs
		for (int i = node->layer + 1; i < next->layer; ++i) {
			path[i] = 0;
		}
	}

	return path;
}


#ifdef SOLVER_CPLEX

void print_flow_decomposition_stats_cplex(BDD* bdd, IloModel model, IloNumVarArray vars,
        vector<vector<double>>& zero_arc_flow, vector<vector<double>>& one_arc_flow,
        const vector<double>& obj_layer)
{
	map<int, int> var_id_to_var;
	int nvars = bdd->nvars();

	for (int i = 0; i < nvars; ++i) {
		var_id_to_var[vars[i].getId()] = i;
	}

	// weights for flow decomposition: we maximize sum of satisfiability (minus RHSs), i.e., -1^T A
	vector<double> weights_fd(nvars, 0);

	for (IloModel::Iterator iter(model); iter.ok(); ++iter) {
		if ((*iter).isConstraint()) {
			IloRangeI* range_impl = dynamic_cast<IloRangeI*>((*iter).asConstraint().getImpl());
			if (!range_impl) {
				continue;
			}

			IloRange row(range_impl);

			// If both <= and >=, or equality, we cannot do anything (they would cancel out)
			if (row.getLB() != -IloInfinity && row.getUB() != +IloInfinity) {
				continue;
			}

			// Iterate through coefficients
			for (IloExpr::LinearIterator row_it = row.getLinearIterator(); row_it.ok(); ++row_it) {
				IloNum coeff = row_it.getCoef();
				int var_idx = var_id_to_var[row_it.getVar().getId()];

				if (row.getLB() != -IloInfinity) {
					weights_fd[bdd->var_to_layer[var_idx]] += coeff;
				} else {
					weights_fd[bdd->var_to_layer[var_idx]] += -coeff;
				}
			}
		}
	}

	cout << "Decomposition weights:  ";
	for (int i = 0; i < nvars; ++i) {
		cout << weights_fd[i] << " ";
	}
	cout << endl;

	vector<vector<int>> paths;
	vector<double> path_weights;
	decompose_paths_from_flow(bdd, weights_fd, zero_arc_flow, one_arc_flow, paths, path_weights);

	bool print_paths = true; // For testing/analysis purposes

	int npaths = paths.size();
	int nfeasible = 0;
	double best_objval = -numeric_limits<double>::infinity();
	for (int i = 0; i < npaths; ++i) {
		bool feasible = is_feasible_cplex(bdd, paths[i], var_id_to_var, model);

		double objval = 0;
		for (int k : paths[i]) {
			if (k == 1) {
				objval += obj_layer[i];
			}
		}
		if (feasible && DBL_GT(objval, best_objval)) {
			best_objval = objval;
		}

		if (print_paths) {
			cout << "Path " << i;
			cout << ", weight " << path_weights[i];
			cout << ", objval " << objval;
			if (feasible) {
				cout << ", Feasible:  ";
			} else {
				cout << ", Infeasible:  ";
			}
			for (int k : paths[i]) {
				cout << k << " ";
			}
			cout << endl;
		}

		if (feasible) {
			nfeasible++;
		}
	}

	cout << "Feasible solutions in decomposition:  " << nfeasible << " / " << npaths << endl;
	cout << "Best feasible objective in decomposition:  " << best_objval << endl;
}


bool is_feasible_cplex(BDD* bdd, const vector<int>& path, const map<int,int>& var_id_to_var, IloModel model)
{
	for (IloModel::Iterator iter(model); iter.ok(); ++iter) {
		if ((*iter).isConstraint()) {
			IloRangeI* range_impl = dynamic_cast<IloRangeI*>((*iter).asConstraint().getImpl());
			if (!range_impl) {
				continue;
			}

			IloRange row(range_impl);

			IloNum activity = 0;
			// Iterate through coefficients
			for (IloExpr::LinearIterator row_it = row.getLinearIterator(); row_it.ok(); ++row_it) {
				IloNum coeff = row_it.getCoef();
				int var_idx = var_id_to_var.at(row_it.getVar().getId());
				activity += path[bdd->var_to_layer[var_idx]] * coeff;
			}

			if (!(DBL_GE(activity, row.getLB()) && DBL_LE(activity, row.getUB()))) {
				return false;
			}
		}
	}

	return true;
}

#endif

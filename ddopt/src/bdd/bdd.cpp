/** 
 * Main decision diagram structure
 */

#include <iostream>
#include <cassert>
#include "bdd.hpp"
#include "../util/util.hpp"
#include "../util/stats.hpp"

#ifdef USE_GMP
#include <gmpxx.h>
#endif



// Informational functions

int BDD::count_number_of_nodes()
{
	int nnodes = 0;
	int bdd_size = layers.size();
	for (int layer = 0; layer < bdd_size; ++layer) {
		nnodes += layers[layer].size();
	}
	return nnodes;
}


int BDD::count_number_of_arcs()
{
	int narcs = 0;
	int bdd_size = layers.size();
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = layers[layer].size();
		for (int k = 0; k < size; ++k) {
			Node* node = layers[layer][k];
			if (node->zero_arc != NULL) {
				narcs++;
			}
			if (node->one_arc != NULL) {
				narcs++;
			}
		}
	}
	return narcs;
}


int BDD::get_width()
{
	int width = 0;
	int bdd_size = layers.size();
	for (int layer = 0; layer < bdd_size; ++layer) {
		width = MAX(width, (int) layers[layer].size());
	}
	return width;
}


int BDD::get_root_layer()
{
	int bdd_size = layers.size();
	int initial_layer = -1;
	for (int layer = 0; layer < bdd_size; ++layer) {
		if (layers[layer].size() > 0) {
			initial_layer = layer;
			break;
		}
	}
	assert(initial_layer >= 0); // Assumes given DD is nonempty
	assert(layers[initial_layer].size() == 1);
	return initial_layer;
}


Node* BDD::get_root_node()
{
	return layers[get_root_layer()][0];
}


int BDD::get_terminal_layer()
{
	int bdd_size = layers.size();
	int final_layer = -1;
	for (int layer = bdd_size - 1; layer >= 0; --layer) {
		if (layers[layer].size() > 0) {
			final_layer = layer;
			break;
		}
	}
	assert(final_layer >= 0); // Assumes given DD is nonempty
	assert(layers[final_layer].size() == 1);
	return final_layer;
}


Node* BDD::get_terminal_node()
{
	return layers[get_terminal_layer()][0];
}


void BDD::print(bool use_global_id /* = false */, bool print_tag /* = false */)
{
	int bdd_size = layers.size();

	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = layers[layer].size();
		if (print_tag) {
			cout << "[DD] ";
		}
		cout << layer << " :";
		for (int k = 0; k < size; ++k) {
			Node* node = layers[layer][k];
			if (node->zero_arc != NULL) {
				if (use_global_id) {
					cout << "  (" << node->global_id << ",0)->" << node->zero_arc->global_id;
				} else {
					cout << "  (" << node->id << ",0)->(" << node->zero_arc->layer << "," << node->zero_arc->id << ")";
				}
			}
			if (node->one_arc != NULL) {
				if (use_global_id) {
					cout << "  (" << node->global_id << ",1)->" << node->one_arc->global_id;
				} else {
					cout << "  (" << node->id << ",1)->(" << node->one_arc->layer << "," << node->one_arc->id << ")";
				}
			}
			if (node->zero_arc == NULL && node->one_arc == NULL) {
				cout << "  (" << node->id << ",T)"; // indicate terminal node
			}
			if (node->relaxed_node) {
				cout << "  (" << node->id << ",R)"; // indicate relaxed node
			}
		}
		cout << endl;
	}
	cout << endl;
}



// Node manipulation functions

Node* BDD::create_node(int layer)
{
	Node* node = new Node(NULL, -1);
	node->layer = layer;
	node->id = layers[layer].size();
	layers[layer].push_back(node);
	return node;
}


Node* BDD::duplicate_node(Node* node)
{
	Node* new_node = create_node(node->layer);
	if (node->zero_arc != NULL) {
		new_node->assign_zero_arc(node->zero_arc);
	}
	if (node->one_arc != NULL) {
		new_node->assign_one_arc(node->one_arc);
	}
	return new_node;
}


void BDD::remove_node_no_arcs(Node* node)
{
	// Remove node from BDD, updating ids
	int layer = node->layer;
	int size = layers[layer].size();
	for (int i = node->id; i < size - 1; ++i) {
		layers[layer][i] = layers[layer][i+1];
		assert(layers[layer][i]->id == i+1);
		layers[layer][i]->id = i;
	}
	layers[layer].pop_back();
}


void BDD::remove_node(Node* node)
{
	// Detach the node's children
	Node* zero_node = node->zero_arc;
	Node* one_node = node->one_arc;
	if (zero_node != NULL) {
		vector<Node*>::iterator pos = find(zero_node->zero_ancestors.begin(), zero_node->zero_ancestors.end(), node);
		assert(pos != zero_node->zero_ancestors.end());
		zero_node->zero_ancestors.erase(pos);
		node->zero_arc = NULL;
	}
	if (one_node != NULL) {
		vector<Node*>::iterator pos = find(one_node->one_ancestors.begin(), one_node->one_ancestors.end(), node);
		assert(pos != one_node->one_ancestors.end());
		one_node->one_ancestors.erase(pos);
		node->one_arc = NULL;
	}

	// Detach the node's parents
	int ancestors_size;
	ancestors_size = node->zero_ancestors.size();
	for (int k = 0; k < ancestors_size; ++k) {
		node->zero_ancestors[k]->zero_arc = NULL;
	}
	ancestors_size = node->one_ancestors.size();
	for (int k = 0; k < ancestors_size; ++k) {
		node->one_ancestors[k]->one_arc = NULL;
	}

	// Remove node from BDD, updating ids
	remove_node_no_arcs(node);

	delete node;
}


void BDD::merge_nodes(Node* node, Node* node_to_remove)
{
	// Assumes the outgoing arcs are the same
	assert(node->zero_arc == node_to_remove->zero_arc);
	assert(node->one_arc == node_to_remove->one_arc);
	assert(node->layer == node_to_remove->layer);

	// Let parents of node_to_remove point to node
	int ancestors_size = node_to_remove->zero_ancestors.size();
	for (int k = 0; k < ancestors_size; ++k) {
		// It cannot be a duplicate since a parent cannot have both node and node_to_remove as a zero child
		node->zero_ancestors.push_back(node_to_remove->zero_ancestors[k]);
		node_to_remove->zero_ancestors[k]->zero_arc = node;
	}
	ancestors_size = node_to_remove->one_ancestors.size();
	for (int k = 0; k < ancestors_size; ++k) {
		// It cannot be a duplicate since a parent cannot have both node and node_to_remove as a one child
		node->one_ancestors.push_back(node_to_remove->one_ancestors[k]);
		node_to_remove->one_ancestors[k]->one_arc = node;
	}

	// Detach node_to_remove's children
	if (node_to_remove->zero_arc != NULL) {
		node_to_remove->detach_zero_arc();
	}
	if (node_to_remove->one_arc != NULL) {
		node_to_remove->detach_one_arc();
	}

	// Remove node from BDD
	remove_node_no_arcs(node_to_remove);

	delete node_to_remove;
}


void BDD::remove_childless_nodes()
{
	int terminal_layer = get_terminal_layer();
	for (int layer = terminal_layer - 1; layer >= 0; --layer) {
		vector<Node*> nodes_to_remove;
		int size = layers[layer].size();
		for (int k = 0; k < size; ++k) {
			Node* node = layers[layer][k];
			if (node->zero_arc == NULL && node->one_arc == NULL) {
				nodes_to_remove.push_back(node);
			}
		}
		// cout << "Layer " << layer << ": Removed " << nodes_to_remove.size() << " childless nodes" << endl;
		for (Node* node_to_remove : nodes_to_remove) {
			remove_node(node_to_remove);
		}
	}
}


void BDD::remove_parentless_nodes()
{
	int bdd_size = layers.size();
	int root_layer = get_root_layer();
	for (int layer = root_layer + 1; layer < bdd_size; ++layer) {
		vector<Node*> nodes_to_remove;
		int size = layers[layer].size();
		for (int k = 0; k < size; ++k) {
			Node* node = layers[layer][k];
			if (node->zero_ancestors.size() == 0 && node->one_ancestors.size() == 0) {
				nodes_to_remove.push_back(node);
			}
		}
		// cout << "Layer " << layer << ": Removed " << nodes_to_remove.size() << " parentless nodes" << endl;
		for (Node* node_to_remove : nodes_to_remove) {
			remove_node(node_to_remove);
		}
	}
}


void BDD::remove_pathless_nodes()
{
	remove_childless_nodes();
	remove_parentless_nodes();
}



// Computation of properties

double BDD::get_optimal_path(vector<double> coeffs_layer, vector<int>& optimal_path, bool maximize,
                            bool ignore_relaxed_nodes /* = false */)
{
	vector<double> zero_coeffs(coeffs_layer.size(), 0);
	return get_optimal_path_zero_one_coeffs(zero_coeffs, coeffs_layer, optimal_path, maximize,
	        ignore_relaxed_nodes);
}


double BDD::get_optimal_sol(vector<double> coeffs_var, vector<int>& optimal_sol, bool maximize,
                           bool ignore_relaxed_nodes /* = false */)
{
	vector<double> zero_coeffs(coeffs_var.size(), 0);

	// Convert from variable space to layer space
	vector<double> one_coeffs(coeffs_var.size(), 0);
	for (int var = 0; var < (int) coeffs_var.size(); ++var) {
		one_coeffs[var_to_layer[var]] = coeffs_var[var];
	}

	// Get optimal path
	vector<int> optimal_path;
	double opt_val = get_optimal_path_zero_one_coeffs(zero_coeffs, one_coeffs, optimal_path, maximize,
	                 ignore_relaxed_nodes);

	// Convert path from layer space to variable space
	int size = optimal_path.size();
	optimal_sol.resize(size);
	for (int layer = 0; layer < size; ++layer) {
		optimal_sol[layer_to_var[layer]] = optimal_path[layer];
	}

	return opt_val;
}


double BDD::get_optimal_path_zero_one_coeffs(vector<double> zero_coeffs, vector<double> one_coeffs,
        vector<int>& optimal_path, bool maximize, bool ignore_relaxed_nodes /* = false */)
{
	int bdd_size = layers.size();

	assert(layers.size() > 0);
	assert((int) zero_coeffs.size() == nvars());
	assert((int) one_coeffs.size() == nvars());

	int initial_layer = get_root_layer();

	// Initialize auxiliary variables
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = layers[layer].size();
		for (int k = 0; k < size; ++k) {
			if (maximize) {
				layers[layer][k]->lp_value = -numeric_limits<double>::infinity();
			} else {
				layers[layer][k]->lp_value = numeric_limits<double>::infinity();
			}
			layers[layer][k]->lp_parent = NULL;
			layers[layer][k]->lp_parent_arctype = -1;
		}
	}
	layers[initial_layer][0]->lp_value = 0;

	// Compute weights
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = layers[layer].size();
		for (int k = 0; k < size; ++k) {
			if (ignore_relaxed_nodes && layers[layer][k]->relaxed_node) {
				continue;
			}
			if (layers[layer][k]->zero_arc != NULL &&
			        ((maximize && layers[layer][k]->lp_value + zero_coeffs[layer] > layers[layer][k]->zero_arc->lp_value) ||
			         (!maximize && layers[layer][k]->lp_value + zero_coeffs[layer] < layers[layer][k]->zero_arc->lp_value))) {
				layers[layer][k]->zero_arc->lp_value = layers[layer][k]->lp_value + zero_coeffs[layer];
				layers[layer][k]->zero_arc->lp_parent = layers[layer][k];
				layers[layer][k]->zero_arc->lp_parent_arctype = 0;
			}
			if (layers[layer][k]->one_arc != NULL &&
			        ((maximize && layers[layer][k]->lp_value + one_coeffs[layer] > layers[layer][k]->one_arc->lp_value) ||
			         (!maximize && layers[layer][k]->lp_value + one_coeffs[layer] < layers[layer][k]->one_arc->lp_value))) {
				layers[layer][k]->one_arc->lp_value = layers[layer][k]->lp_value + one_coeffs[layer];
				layers[layer][k]->one_arc->lp_parent = layers[layer][k];
				layers[layer][k]->one_arc->lp_parent_arctype = 1;
			}
		}
	}

	// // Sanity check (check if all nodes are visited; not true if ignoring relaxed nodes)
	// if (!ignore_relaxed_nodes) {
	//   for (int layer = initial_layer + 1; layer < bdd_size; ++layer) {
	//     for (int k = 0; k < (int) layers[layer].size(); ++k) {
	//       assert(!maximize || layers[layer][k]->lp_value != -numeric_limits<double>::infinity());
	//       assert( maximize || layers[layer][k]->lp_value != numeric_limits<double>::infinity());
	//       assert(layers[layer][k]->lp_parent != NULL);
	//       assert(layers[layer][k]->lp_parent_arctype != -1);
	//     }
	//   }
	// }

	// Extract optimal path
	Node* node = layers[bdd_size-1][0];

	if (node->lp_parent == NULL) {
		// Terminal node was unreachable due to pruning + skipping relaxed nodes
		optimal_path.resize(0);
		return maximize ? -numeric_limits<double>::infinity() : numeric_limits<double>::infinity();
	}

	optimal_path.resize(bdd_size - 1);
	for (int layer = 0; layer < bdd_size - 1; ++layer) {
		optimal_path[layer] = 0; // Set everything to zero to consider long arcs
	}
	while (node->lp_parent != NULL) {
		optimal_path[node->lp_parent->layer] = node->lp_parent_arctype;
		node = node->lp_parent;
	}
	assert(node->layer == initial_layer);

	double optimal_value = layers[bdd_size-1][0]->lp_value;

	// Sanity check
	assert(DBL_EQ(compute_path_value(zero_coeffs, one_coeffs, optimal_path), optimal_value));

	return optimal_value;
}


double BDD::compute_path_value(vector<double> zero_coeffs, vector<double> one_coeffs, vector<int>& path)
{
	double value = 0;
	int bdd_size = layers.size();

	for (int layer = 0; layer < bdd_size - 1; ++layer) {
		if (path[layer] == 1) {
			value += one_coeffs[layer];
		} else {
			value += zero_coeffs[layer];
		}
	}
	return value;
}


#ifdef USE_GMP // Computing the center of a BDD requires arbitrary precision integers due to handling of very large values

struct CenterData {
	mpz_class top_down_val = 0;
	mpz_class bottom_up_val = 0;
	// int top_down_val = 0;
	// int bottom_up_val = 0;
};

/** Compute the center of a BDD */
void BDD::get_center(vector<double>& center)
{
	CenterData* cd;
	CenterData* cd2;
	int bdd_size = layers.size();

	Stats stats;
	stats.register_name("time-center");
	stats.start_timer(0);

	assert(layers.size() > 0);
	assert(layers[0].size() == 1);
	assert(layers[bdd_size-1].size() == 1);

	// Initialize auxiliary variables
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = layers[layer].size();
		for (int k = 0; k < size; ++k) {
			layers[layer][k]->temp_data = new CenterData();
		}
	}

	// Initialize root and terminal values
	int root_layer = get_root_layer();
	int terminal_layer = get_terminal_layer();
	boost::any_cast<CenterData*>(layers[root_layer][0]->temp_data)->top_down_val = 1;
	boost::any_cast<CenterData*>(layers[terminal_layer][0]->temp_data)->bottom_up_val = 1;

	// Compute number of paths from root to each node
	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = layers[layer].size();
		for (int k = 0; k < size; ++k) {
			cd = boost::any_cast<CenterData*>(layers[layer][k]->temp_data);
			if (layers[layer][k]->zero_arc != NULL) {
				cd2 = boost::any_cast<CenterData*>(layers[layer][k]->zero_arc->temp_data);
				cd2->top_down_val += cd->top_down_val;
			}
			if (layers[layer][k]->one_arc != NULL) {
				cd2 = boost::any_cast<CenterData*>(layers[layer][k]->one_arc->temp_data);
				cd2->top_down_val += cd->top_down_val;
			}
			// cout << "TD: layer " << layer << ", l " << k << ": " << cd->top_down_val << endl;
		}
	}

	// Compute number of paths from each node to terminal
	for (int layer = bdd_size - 1; layer >= 0; --layer) {
		int size = layers[layer].size();
		for (int k = 0; k < size; ++k) {
			cd = boost::any_cast<CenterData*>(layers[layer][k]->temp_data);
			if (layers[layer][k]->zero_arc != NULL) {
				cd2 = boost::any_cast<CenterData*>(layers[layer][k]->zero_arc->temp_data);
				cd->bottom_up_val += cd2->bottom_up_val;
			}
			if (layers[layer][k]->one_arc != NULL) {
				cd2 = boost::any_cast<CenterData*>(layers[layer][k]->one_arc->temp_data);
				cd->bottom_up_val += cd2->bottom_up_val;
			}
			// cout << "BU: layer " << layer << ", l " << k << ": " << cd->bottom_up_val << endl;
		}
	}

	assert(boost::any_cast<CenterData*>(layers[root_layer][0]->temp_data)->bottom_up_val
	       == boost::any_cast<CenterData*>(layers[terminal_layer][0]->temp_data)->top_down_val);
	mpz_class total_npaths = boost::any_cast<CenterData*>(layers[terminal_layer][0]->temp_data)->top_down_val;
	// int total_npaths = boost::any_cast<CenterData*>(layers[terminal_layer][0]->temp_data)->top_down_val;

	// Compute center
	center.resize(bdd_size - 1);
	for (int layer = 0; layer < bdd_size - 1; ++layer) {
		int size = layers[layer].size();
		mpz_class center_sum = 0;
		// int center_sum = 0;
		for (int k = 0; k < size; ++k) {
			if (layers[layer][k]->one_arc != NULL) {
				// cout << "k=" << k << ", " <<  boost::any_cast<CenterData*>(layers[layer][k]->temp_data)->top_down_val;
				// cout << " -- " << boost::any_cast<CenterData*>(layers[layer][k]->one_arc->temp_data)->bottom_up_val << endl;
				center_sum += boost::any_cast<CenterData*>(layers[layer][k]->temp_data)->top_down_val
				              + boost::any_cast<CenterData*>(layers[layer][k]->one_arc->temp_data)->bottom_up_val;
			}
		}
		mpq_class center_val(center_sum, total_npaths);
		center[layer] = center_val.get_d(); // convert back to double
		// center[layer] = (double) center_sum / total_npaths;
		assert(center[layer] >= 0 && center[layer] <= 1);
	}

	for (int layer = 0; layer < bdd_size; ++layer) {
		int size = layers[layer].size();
		for (int k = 0; k < size; ++k) {
			delete boost::any_cast<CenterData*>(layers[layer][k]->temp_data);
		}
	}

	stats.end_timer(0);
	cout << "Time to calculate center: " << stats.get_time(0) << endl;
}

#else

void BDD::get_center(vector<double>& center)
{
	cout << "Error: Using DD center as interior point requires compilation with GMP library" << endl;
	exit(1);
}

#endif



void BDD::identify_fixed_layers(vector<int>& layers_fixed_to_zero, vector<int>& layers_fixed_to_one)
{
	int bdd_size = layers.size();

	vector<bool> found_zero(bdd_size - 1, false);
	vector<bool> found_one(bdd_size - 1, false);

	for (int layer = 0; layer < bdd_size - 1; ++layer) {
		int size = layers[layer].size();
		for (int k = 0; k < size; ++k) {
			Node* node = layers[layer][k];
			if (node->zero_arc != NULL) {
				found_zero[layer] = true;
				for (int j = layer + 1; j < node->zero_arc->layer; ++j) {
					found_zero[j] = true; // long arc (0,0,...,0)
				}
			}
			if (node->one_arc != NULL) {
				found_one[layer] = true;
				for (int j = layer + 1; j < node->one_arc->layer; ++j) {
					found_zero[j] = true; // long arc (1,0,...,0)
				}
			}
		}
	}

	layers_fixed_to_one.clear();
	layers_fixed_to_zero.clear();
	for (int i = 0; i < bdd_size - 1; ++i) {
		assert(found_zero[i] || found_one[i]);
		if (!found_zero[i]) {
			layers_fixed_to_one.push_back(i);
		}
		if (!found_one[i]) {
			layers_fixed_to_zero.push_back(i);
		}
	}
}



// Conversion functions

vector<double> BDD::convert_to_layer_space(const vector<double>& v)
{
	int nv = nvars();
	assert((int) v.size() == nv);

	vector<double> converted_vector(nv);
	for (int i = 0; i < nv; ++i) {
		converted_vector[i] = v[var_to_layer[i]];
	}
	return converted_vector;
}


vector<double> BDD::convert_to_var_space(const vector<double>& v)
{
	int nv = nvars();
	assert((int) v.size() == nv);

	vector<double> converted_vector(nv);
	for (int i = 0; i < nv; ++i) {
		converted_vector[i] = v[layer_to_var[i]];
	}
	return converted_vector;
}



// Integrity check functions

bool BDD::integrity_check()
{
	int bdd_size = layers.size();
	for (int i = 0; i < bdd_size; ++i) {
		int size = layers[i].size();
		for (int j = 0; j < size; ++j) {
			Node* node = layers[i][j];

			if (node->id >= size || node->layer >= bdd_size || layers[node->layer][node->id] != node) {
				cout << "** BDD integrity check error: Node with incorrect id or layer (node id = " << node->id << ", node layer = " << node->layer << ")";
				if (node->id >= size) {
					cout << " / Exceeded size: " << size;
				} else if (node->layer >= bdd_size) {
					cout << " / Exceeded number of layers: " << bdd_size;
				} else if (layers[node->layer][node->id] != node) {
					cout << " / Invalid position (node at id/layer has id = " << layers[node->layer][node->id]->id << ", layer = " << layers[node->layer][node->id]->layer << ")";
				}
				cout << endl;
				return false; // Incorrect id or layer
			}

			if (i > 0 && (node->zero_ancestors.empty() && node->one_ancestors.empty())) {
				cout << "** BDD integrity check error: Node (" << node->layer << "," << node->id << ") with no ancestors" << endl;
				return false; // No ancestors
			}

			if (i < bdd_size - 1 && (node->zero_arc == NULL && node->one_arc == NULL)) {
				cout << "** BDD integrity check error: Node (" << node->layer << "," << node->id << ") with no children" << endl;
				return false; // No children
			}

			// Check if arcs are two-way
			for (Node* zero_ancestor : node->zero_ancestors) {
				if (zero_ancestor->zero_arc != node) {
					cout << "** BDD integrity check error: Inconsistent (zero-)arc parent at (" << node->layer << "," << node->id << ")";
					cout << ", parent (" << zero_ancestor->layer << "," << zero_ancestor->id << ") pointing at ";
					if (zero_ancestor->zero_arc == NULL) {
						cout << "null" << endl;
					} else {
						cout << "(" << zero_ancestor->zero_arc->layer << "," << zero_ancestor->zero_arc->id << ")" << endl;
					}
					return false;
				}
			}

			for (Node* one_ancestor : node->one_ancestors) {
				if (one_ancestor->one_arc != node) {
					cout << "** BDD integrity check error: Inconsistent (one-)arc parent at (" << node->layer << "," << node->id << ")" << endl;
					cout << ", parent (" << one_ancestor->layer << "," << one_ancestor->id << ") pointing at ";
					if (one_ancestor->one_arc == NULL) {
						cout << "null" << endl;
					} else {
						cout << "(" << one_ancestor->one_arc->layer << "," << one_ancestor->one_arc->id << ")" << endl;
					}
					return false;
				}
			}

			if (node->zero_arc != NULL) {
				if (find(node->zero_arc->zero_ancestors.begin(), node->zero_arc->zero_ancestors.end(), node) == node->zero_arc->zero_ancestors.end()) {
					cout << "** BDD integrity check error: Inconsistent (zero-)arc child at (" << node->layer << "," << node->id << ")" << endl;
					return false;
				}
			}

			if (node->one_arc != NULL) {
				if (find(node->one_arc->one_ancestors.begin(), node->one_arc->one_ancestors.end(), node) == node->one_arc->one_ancestors.end()) {
					cout << "** BDD integrity check error: Inconsistent (one-)arc child at (" << node->layer << "," << node->id << ")" << endl;
					return false;
				}
			}

			// Check if data is empty
			if (!node->temp_data.empty()) {
				cout << "** BDD integrity check error: Node (" << node->layer << "," << node->id << ") data not empty" << endl;
				return false;
			}
		}
	}
	return true;
}


bool BDD::empty_data_check()
{
	int bdd_size = layers.size();
	for (int i = 0; i < bdd_size; ++i) {
		int size = layers[i].size();
		for (int j = 0; j < size; ++j) {
			Node* node = layers[i][j];
			// Check if data is empty
			if (!node->temp_data.empty()) {
				return false;
			}
		}
	}
	return true;
}

/**
 * Main decision diagram construction
 */

#include <cassert>
#include "solver.hpp"
#include "../util/util.hpp"
#include "../util/stats.hpp"


BDD* DDSolver::construct_decision_diagram()
{
	BDD* bdd = construct_decision_diagram_at_state(problem->create_initial_state(), 0);
	if (bdd != NULL) {
		assert(bdd->layers[0].size() == 1);
		assert(bdd->layers[bdd->layers.size()-1].size() == 1);
	}
	return bdd;
}


BDD* DDSolver::construct_decision_diagram_at_state(State* initial_state, double initial_longest_path)
{
	// Initialization

	Stats stats;
	stats.register_name("time_construct_dd");
	stats.start_timer(0);

	final_width = -1;

	int width = -1;
	if (problem->merger != NULL) {
		width = problem->merger->width;
	}

	vector<Node*> nodes_layer; // current layer
	if (width != EXACT_BDD) {
		nodes_layer.reserve(2 * width * 100);
	} else {
		nodes_layer.reserve(1e8);
	}

	NodeMap node_list; // hash map of states to nodes
	node_list.clear();
	int global_id = 0;

	problem->callback_initialize();


	// Decision diagram construction

	Node* initial_node = new Node(initial_state, initial_longest_path, initial_node_data);
	node_list[initial_state] = initial_node;
	problem->callback_state_created(initial_state);
	initial_node->global_id = global_id++;

	NodeMap::iterator node_it, existing_node_it;

	// iterate through layers to construct nodes
	for (int layer = 0; layer < nlayers - 1; ++layer) {

		// select next variable
		int current_var = problem->ordering->select_next_var(layer);
		assert(current_var >= 0 && current_var < problem->inst->nvars);

		// error if variable was already visited
		if (final_bdd->var_to_layer[current_var] != -1) {
			cout << "Error: Variable selected more than once" << endl;
			exit(1);
		}

		// update variable-layer translation maps
		final_bdd->layer_to_var[layer] = current_var;
		final_bdd->var_to_layer[current_var] = layer;

#ifdef DEBUG
		cout << "\n\n\n\n ====================================================== \n\n";
		cout << "Layer " << layer << " - current variable: " << current_var << endl;
		// iterate through the nodes in the node list
		cout << "(Before) state list: " << endl;
		for (NodeMap::iterator node_it = node_list.begin(); node_it != node_list.end(); ++node_it) {
			cout << "\tstate: " << *(node_it->first);
			cout << " - longest path: " << node_it->second->longest_path;
			cout << endl;
		}
		cout << current_var << endl;
#endif

		/*
		 * ===============================================================================
		 * 1. Take nodes that have the current variable in their state
		 *    We will create arcs from those nodes in this layer
		 * ===============================================================================
		 */
		nodes_layer.clear();

		node_it = node_list.begin();
		while (node_it != node_list.end())	{

			// if a node does not contain the variable, it will be skipped and corresponding arcs will be long arcs
			if (options->use_long_arcs && problem->cb_skip_var_for_long_arc(current_var, node_it->second->state)) {
				++node_it;
				continue;
			}

			assert((int)final_bdd->layers.size() > layer);

			problem->callback_state_removed(node_it->first);

			// add node to current layer list
			nodes_layer.push_back(node_it->second);

			// erase element from the list (erasing on-the-fly for a map)
			node_list.erase(node_it++);
		}

#ifdef DEBUG
		cout << "\nBefore merging: " << endl;
		for (Node* node : nodes_layer) {
			cout << "\t " << *(node->state) << " - " << node->longest_path;
		}
		cout << endl;
#endif

		// Print layer information
		if (!options->quiet) {
			cout << "Layer " << layer << " - current variable: " << current_var;
			cout << " - pool size: " << node_list.size();
			cout << " - before merge: " << nodes_layer.size();
			cout << " - total: " << node_list.size() + nodes_layer.size();
			cout << endl;
		}


		/*
		 * ===============================================================================
		 * 2. Merging
		 * ===============================================================================
		 */
		if (width != EXACT_BDD && (int) nodes_layer.size() > width) {

			if (solver_callback != NULL) {
				solver_callback->cb_pre_merge(final_bdd, nodes_layer, node_list, width, layer);
			}

			// cout << "Merging " << (int) nodes_layer.size() << " max " << width << endl;
			assert(problem->merger != NULL);
			problem->merger->merge_layer(problem, layer, nodes_layer);

			if (solver_callback != NULL) {
				solver_callback->cb_post_merge(final_bdd, nodes_layer, node_list, width, layer);
			}
		}

		final_width = MAX(final_width, (int) nodes_layer.size());

#ifdef DEBUG
		cout << " - after merge: " << nodes_layer.size() << endl;
		cout << "\nAfter merging: " << endl;
		for (Node* node : nodes_layer) {
			cout << "\t " << *(node->state) << " - " << node->longest_path;
		}
		cout << endl;

		int nrelaxed_nodes = 0;
		for (Node* node : nodes_layer) {
			if (node->relaxed_node) {
				nrelaxed_nodes++;
			}
		}
		cout << "Layer " << layer << ": " << nrelaxed_nodes << " relaxed nodes" << endl;
#endif


		/*
		 * ===============================================================================
		 * 3. Branching
		 * ===============================================================================
		 */
		for (Node* branch_node : nodes_layer) {

			// add node to final BDD representation
			assert(branch_node->layer == DD_NODE_ID_OPEN);
			branch_node->layer = layer;
			branch_node->id = final_bdd->layers[layer].size();
			final_bdd->layers[layer].push_back(branch_node);

			Node* new_node;

			for (int val = 0; val <= 1; ++val) {

				State* new_state = branch_node->state->transition(problem, current_var, val);

				// // Debugging info
				// cout << "[T]  Set " << current_var << " to " << val << "  /  State " << *(branch_node->state) << " / Value " << branch_node->longest_path << endl;
				// if (new_state != NULL) {
				//   cout << "[T]   -- Result: " << *new_state << " / Value " << branch_node->longest_path + val * problem->inst->weights[current_var] << endl;
				// } else {
				//   cout << "[T]   -- Result: Infeasible" << endl;
				// }

				if (new_state != NULL) {

					// create new node data
					NodeDataMap* nd = NULL;
					if (branch_node->data != NULL) {
						assert(!branch_node->data->is_infeasible());
						nd = branch_node->data->transition(problem, branch_node, new_state, current_var, val);

						if (nd->is_infeasible()) {
							if (val == 1) {
								branch_node->one_arc = NULL;
							} else { // val == 0
								branch_node->zero_arc = NULL;
							}
							continue;
						}
					}

					// create a new (potential) node
					new_node = new Node(new_state, branch_node->longest_path + val * problem->inst->weights[current_var], nd);

					// prune node if bounds allow
					if ((use_primal_pruning && node_can_be_pruned_by_primal_bound(problem, new_node, branch_node))) {
						if (val == 1) {
							branch_node->one_arc = NULL;
						} else { // val == 0
							branch_node->zero_arc = NULL;
						}
						delete new_node;
						continue;
					}

					// check if node with this new state already exists
					// stats.register_name("find");
					// stats.start_timer(1);
					existing_node_it = node_list.find(new_node->state);
					// stats.end_timer(1);
					// cout << "Time find: " << stats.get_time(1) << endl;

					if (existing_node_it != node_list.end()) {
						// node already exists: delete newly created node and point to existing node

						Node* existing_node = existing_node_it->second;
						existing_node->update_optimal_path(new_node);
						if (existing_node->data != NULL) {
							existing_node->data->merge(problem, new_node->data, new_node->state);
						}
						delete new_node;
						new_node = existing_node_it->second;

					} else {
						// node does not exist: point to new node

						// stats.register_name("assign");
						// stats.start_timer(2);
						node_list[new_node->state] = new_node;
						// stats.end_timer(2);
						// cout << "Time assign: " << stats.get_time(2) << endl;
						new_node->global_id = global_id++;
						problem->callback_state_created(new_node->state);
					}

					// update node links (either existing or new node)
					assert(val != 1 || branch_node->one_arc == NULL);
					assert(val != 0 || branch_node->zero_arc == NULL);
					branch_node->assign_arc(new_node, val);

					// // Debugging info
					// cout << " (" << val << ") From " << endl;
					// cout << "\t" << *(branch_node->state) << endl;
					// cout << " to " << endl;
					// cout << "\t" << *(new_node->state) << endl;
					// cout << endl;
				}
			}

			// Optional: Delete states from previous nodes to reduce memory usage
			if (options->delete_old_states) {
				delete branch_node->state;
				branch_node->state = NULL;
			}

		}

#ifdef DEBUG
		// iterate through the nodes in the node list
		cout << "(After) state list: " << endl;
		for (NodeMap::iterator node_it = node_list.begin(); node_it != node_list.end(); ++node_it) {
			cout << "\tstate: " << *(node_it->first);
			cout << " - longest path: " << node_it->second->longest_path;
			cout << endl;
		}
		cout << endl;
#endif

		problem->cb_layer_end(current_var);
		if (solver_callback != NULL) {
			solver_callback->cb_layer_end(final_bdd, nodes_layer, node_list, width, layer, options);
		}
	}


	// Final steps

	// If no nodes are left, BDD is infeasible or all nodes were pruned
	if (node_list.size() == 0) {
		stats.end_timer(0);
		// cout << "DD construction time: " << stats.get_time(0) << endl;
		delete final_bdd;
		return NULL;
	}

	// Merge terminal nodes into one (unless a single terminal is expected)
	Node* terminal_node;
	if (!problem->expect_single_terminal()) {
		terminal_node = merge_terminal_nodes(node_list);
	} else {
		assert(node_list.size() <= 1);
		if (node_list.size() > 1) {
			cout << "Error: More than one terminal at the end of BDD construction" << endl;
			exit(1);
		}
		terminal_node = node_list.begin()->second;
	}
	node_list.clear();

	// Finish setting up terminal node
	terminal_node->layer = nlayers-1;
	terminal_node->id = 0;
	final_bdd->layers[nlayers-1].push_back(terminal_node);
	final_bdd->bound = terminal_node->longest_path;

	// Final sanity checks
#ifndef NDEBUG
	for (int i = 0; i < nlayers; i++) {
		int id = 0;
		for (Node* node : final_bdd->layers[i]) {
			assert(node->layer == i);
			if (options->use_long_arcs) {
				assert(node->zero_arc == NULL || node->zero_arc->layer > i);
				assert(node->one_arc == NULL || node->one_arc->layer > i);
			} else {
				assert(node->zero_arc == NULL || node->zero_arc->layer == i+1);
				assert(node->one_arc == NULL || node->one_arc->layer == i+1);
			}
			assert(node->id == id);
			id++;
			// cout << "   Node " << node->id << ": " << (node->zero_ancestors.size() + node->one_ancestors.size()) << " parents" << endl;
		}
		// cout << "Layer " << i << " width: " << final_bdd->layers[i].size() << endl;
	}
	assert(final_bdd->layers[0].size() == 1);
	assert(final_bdd->layers[nlayers-1].size() == 1);
	// cout << endl;
#endif

	// Finalize construction
	final_bdd->constructed = true;
	if (solver_callback != NULL) {
		solver_callback->cb_solver_end(final_bdd, options);
	}

	stats.end_timer(0);
	// cout << "DD construction time: " << stats.get_time(0) << endl;

	return final_bdd;
}


Node* DDSolver::merge_terminal_nodes(NodeMap& terminal_node_list)
{
	NodeMap::iterator node_it = terminal_node_list.begin();
	Node* terminal_node = node_it->second;
	terminal_node_list.erase(node_it++);

	while (node_it != terminal_node_list.end()) {
		Node* other = node_it->second;

		// Update arcs
		for (Node* parent : other->zero_ancestors) {
			parent->assign_zero_arc(terminal_node);
		}
		for (Node* parent : other->one_ancestors) {
			parent->assign_one_arc(terminal_node);
		}

		terminal_node->update_optimal_path(other);
		if (terminal_node->data != NULL) {
			terminal_node->data->merge(problem, other->data, other->state);
		}
		terminal_node_list.erase(node_it++);
		delete other;
	}

	return terminal_node;
}


DDSolver::DDSolver(Problem* _problem, Options* _options) : problem(_problem), options(_options)
{
	nlayers = problem->inst->nvars + 1;

	// Note: BDD must be freed manually after use, since it may outlive the solver in certain applications
	final_bdd = new BDD();
	final_bdd->layers.resize(nlayers);
	final_bdd->layer_to_var.resize(problem->inst->nvars);
	final_bdd->var_to_layer.resize(problem->inst->nvars);

	// Initialize vectors with -1 for error detection and other usages (such as passes)
	fill(final_bdd->layer_to_var.begin(), final_bdd->layer_to_var.end(), DD_NODE_ID_OPEN);
	fill(final_bdd->var_to_layer.begin(), final_bdd->var_to_layer.end(), DD_NODE_ID_OPEN);
	final_width = -1;

	use_primal_pruning = false;
	primal_bound = -numeric_limits<double>::infinity();

	initial_node_data = NULL;
	solver_callback = NULL;
}


void DDSolver::set_primal_bound(double bound)
{
	use_primal_pruning = true;
	primal_bound = bound;
}


bool DDSolver::node_can_be_pruned_by_primal_bound(Problem* prob, Node* node, Node* parent)
{
	/* prune if partial solution value + completion dual bound <= primal bound */
	/* Note that we are always maximizing in a DD */
	assert(prob->completion != NULL);
	double completion_bound = prob->completion->dual_bound(prob->inst, node, parent);
	bool pruned = DBL_LE(node->longest_path + completion_bound, primal_bound);
	// if (pruned) {
	//   cout << "Pruned by primal bound: bound " << node->longest_path << " + " <<  completion_bound << " = "
	//        << node->longest_path + completion_bound << " <= " << primal_bound << endl;
	// }
	return pruned;
}


void DDSolver::add_initial_node_data(string key, NodeData* node_data)
{
	if (initial_node_data == NULL) {
		initial_node_data = new NodeDataMap();
	}
	initial_node_data->add(key, node_data);
}

/**
 * Variable ordering for the independent set problem
 */

#include <algorithm>

#include "indepset_orderings.hpp"
#include "../../core/orderings.hpp"

using namespace std;


Ordering* get_ordering_by_id_indepset(int id, IndepSetInstance* inst, Options& options)
{
	// Read ordering type
	switch (id) {
	case 1:
		return new RandomOrdering(inst);
	case 2:
		return new MaximalPathDecomp(inst);
	case 3:
		return new MinInState(inst);
	case 4:
		return new CutVertexDecomposition(inst, false);
	case 5:
		return new CutVertexDecomposition(inst);
	case 6:
		if (options.order_rand_min_state_prob < 0 || options.order_rand_min_state_prob > 1) {
			cout << "Error: probability for randomized min in state ordering must be between 0 and 1\n\n";
			exit(1);
		}
		return new MinInState(inst, options.order_rand_min_state_prob);
	case 7:
		if (options.fixed_order_filename.empty()) {
			cout << "Error: text file required for ordering\n\n";
			exit(1);
		}
		return new FixedOrdering(inst, options.fixed_order_filename);
	case 8:
		return new MinDegreeOrdering(inst);
	case 9:
		return new NoOrdering();
	}
	return NULL;
}

void MinInState::cb_initialize()
{
	// initialize in-state counter with initial state
	memset(in_state_counter, 0, sizeof(int)*inst->graph->n_vertices);
}

void MinInState::cb_state_created(State* state)
{
	IndepSetState* state_is = dynamic_cast<IndepSetState*>(state);

	// increment active state counter
	int v = state_is->intset.get_first();
	while (v != state_is->intset.get_end()) {
		in_state_counter[v]++;
		v = state_is->intset.get_next(v);
	}
}

void MinInState::cb_state_removed(State* state)
{
	IndepSetState* state_is = dynamic_cast<IndepSetState*>(state);

	// decrement active state counter
	int v = state_is->intset.get_first();
	while (v != state_is->intset.get_end()) {
		in_state_counter[v]--;
		v = state_is->intset.get_next(v);
	}
}

// min in state heuristic
int MinInState::select_next_var(int layer)
{
	if (prob == 1) {
		return select_vertex_with_min_in_state(layer);
	}

	// check if we will randomize our selection or not
	double probabilities[] = {prob, 1.0 - prob};
	boost::random::discrete_distribution<> dist(probabilities);

	if (dist(gen) == 0) {
		return select_vertex_with_min_in_state(layer);
	}

	return select_vertex_randomly(layer);
}

int MinInState::select_vertex_with_min_in_state(int layer)
{
	int min = INF;
	int selected_vertex = -1;
	for (int i = 0; i < inst->graph->n_vertices; i++) {
		if (in_state_counter[i] > 0 && in_state_counter[i] < min) {
			selected_vertex = i;
			min = in_state_counter[i];
		}
	}

	assert(selected_vertex >= 0);

	return selected_vertex;
}

int MinInState::select_vertex_randomly(int layer)
{
	vector<int> selectable_vertices;
	selectable_vertices.clear();
	for (int v = 0; v < inst->graph->n_vertices; v++) {
		if (in_state_counter[v] > 0) {
			selectable_vertices.push_back(v);
		}
	}

	assert(selectable_vertices.size() > 0);
	boost::random::uniform_int_distribution<> vertex_selector(0, (selectable_vertices.size()-1));

	return selectable_vertices[ vertex_selector(gen) ];
}


// minimum degree ordering
void MinDegreeOrdering::construct_ordering()
{

	v_in_layer.clear();

	// compute vertex degree
	vector<int> degree(inst->graph->n_vertices, 0);
	for (int i = 0; i < inst->graph->n_vertices; ++i) {
		for (int j = i+1; j < inst->graph->n_vertices; ++j) {
			if (inst->graph->adj_m[i][j]) {
				++(degree[i]);
				++(degree[j]);
			}
		}
	}

	vector<bool> selected(inst->graph->n_vertices, false);

	while ((int)v_in_layer.size() < inst->graph->n_vertices) {

		int min = INF;
		int v = -1;

		for (int i = 0; i < inst->graph->n_vertices; ++i) {
			if (degree[i] > 0 && degree[i] < min && !selected[i]) {
				min = degree[i];
				v = i;
			}
		}

		if (v == -1) {
			for (int i = 0; i < inst->graph->n_vertices; ++i) {
				if (!selected[i]) {
					//cout << "\n selected vertex " << i << " --> degree: " << degree[i] << endl;
					v_in_layer.push_back(i);
				}
			}
		} else {
			selected[v] = true;
			v_in_layer.push_back(v);
			//cout << "\n selected vertex " << v << " --> degree: " << degree[v] << endl;
			for (int i = 0; i < inst->graph->n_vertices; ++i) {
				if (i != v && inst->graph->adj_m[i][v]) {
					--(degree[i]);
				}
			}
		}
	}
}


// maximal path decomposition
void MaximalPathDecomp::construct_ordering()
{
	int n_maximal_paths = 0;

	v_in_layer.resize(inst->graph->n_vertices);
	vector<bool> visited(inst->graph->n_vertices, false);

	int n = 0;  // number of vertices already considered in the path

	// partial orderings
	vector<int> left;
	vector<int> right;

	while (n < inst->graph->n_vertices) {
		left.clear();
		right.clear();

		int middle = -1;
		// take first unvisited vertex
		for (int v = 0; v < inst->graph->n_vertices; v++) {
			if (!visited[v]) {
				middle = v;
				break;
			}
		}
		visited[middle] = true;

		// right composition
		int current = middle;
		while (current != -1) {
			int next = -1;
			for (int v = 0; v < inst->graph->n_vertices; v++) {
				if (!visited[v] && inst->graph->is_adj(current, v)) {
					next = v;
					right.push_back(next);
					visited[next] = true;
					break;
				}
			}
			current = next;
		}

		// left composition
		current = middle;
		while (current != -1) {
			int next = -1;
			for (int v = 0; v < inst->graph->n_vertices; v++) {
				if (!visited[v] && inst->graph->is_adj(current, v)) {
					next = v;
					left.push_back(next);
					visited[next] = true;
					break;
				}
			}
			current = next;
		}

		// compose path from left to right
		for (int i = (int)left.size()-1; i>=0; i--) {
			v_in_layer[n++] = left[i];
		}
		v_in_layer[n++] = middle;
		for (int i = 0; i < (int)right.size(); i++) {
			v_in_layer[n++] = right[i];
		}
		n_maximal_paths++;
	}

	cout << "\nnumber of maximal paths in decomposition: " << n_maximal_paths << endl << endl;

	// Sanity check
	if (n_maximal_paths == 1) {
		for (int v = 0; v < inst->graph->n_vertices-1; v++) {
			if (!inst->graph->is_adj(v_in_layer[v], v_in_layer[v+1])) {
				cout << "Error: Maximal path decomposition\n";
				exit(1);
			}
		}
	}

}


void CutVertexDecomposition::identify_components(vector< vector<int> >& comps, vector<bool>& is_in_graph)
{

	vector<int> label(inst->graph->n_vertices, -1);
	int num_comps = -1;

	vector<int> stack;

	vector<bool> visited(inst->graph->n_vertices, false);
	for (int i = 0; i < inst->graph->n_vertices; i++) {

		if (is_in_graph[i] && !visited[i]) {

			num_comps++;
			stack.push_back(i);

			while (!stack.empty()) {

				int v = stack.back();
				stack.pop_back();

				label[v] = num_comps;
				visited[v] = true;

				for (int w = 0; w < inst->graph->n_vertices; w++) {
					if (w == v) {
						continue;
					}
					if (is_in_graph[w] && inst->graph->is_adj(v,w) && !visited[w]) {
						stack.push_back(w);
					}
				}
			}
		}
	}

	comps.clear();
	comps.resize(num_comps+1);

	for (int v = 0; v < inst->graph->n_vertices; v++) {
		if (label[v] != -1) {
			comps[label[v]].push_back(v);
		}
	}
}


vector<int> CutVertexDecomposition::find_ordering(vector<bool> is_in_graph)
{

	int size = 0;
	for (int i = 0; i < inst->graph->n_vertices; i++) {
		size += (is_in_graph[i] ? 1 : 0);
	}

	// find vertex with all components less than half the size of the graph
	vector< vector<int> > comps;
	for (int i = 0; i < inst->graph->n_vertices; i++) {
		if (is_in_graph[i]) {

			// try removing vertex
			is_in_graph[i] = false;
			identify_components(comps, is_in_graph);

			//cout << "components when removing vertex " << i << endl;

			bool all_valid = true;
			for (int j = 0; j < (int)comps.size() && all_valid; j++) {
				all_valid = ((int)comps[j].size() <= size/2);
				//cout << "\t" << comps[j].size() << endl;
			}

			if (all_valid) {

				vector<int> ordering;

				// compose ordering for each component separately
				vector<bool> is_in_graph_new(inst->graph->n_vertices, false);
				for (int c = 0; c < (int)comps.size(); c++) {

					//cout << "*** Component " << c << endl;

					for (int v = 0; v < (int)comps[c].size(); v++) {
						is_in_graph_new[comps[c][v]] = true;
					}

					vector<int> order_bck = find_ordering(is_in_graph_new);

					for (int v = 0; v < (int)comps[c].size(); v++) {
						is_in_graph_new[comps[c][v]] = false;
						ordering.push_back(order_bck[v]);
					}
				}
				ordering.push_back(i);
				return ordering;
			}

			// put vertex back again
			is_in_graph[i] = true;
		}
	}
	return (vector<int>(1,-1));
}


void CutVertexDecomposition::construct_ordering()
{
	vector<bool> is_in_graph(inst->graph->n_vertices, true);
	vector<int> ordering = find_ordering(is_in_graph);
	for (int i = 0; i < (int)ordering.size(); i++) {
		v_in_layer[i] = ordering[i];
	}
}


void CutVertexDecomposition::restrict_graph()
{
	vector<pair<int,int>> edges;

	vector<int> vertices(inst->graph->n_vertices);
	vector<int> degrees(inst->graph->n_vertices);
	for (int i = 0; i < inst->graph->n_vertices; i++) {
		vertices[i] = i;
		degrees[i] = inst->graph->adj_list[i].size()-1;
	}
	DecreasingComparator<int> int_comp(degrees);
	sort(vertices.begin(), vertices.end(), int_comp);

	vector<bool> is_taken(inst->graph->n_vertices, false);

	cout << "vertices ordered by degree: " << endl;
	for (int i = 0; i < inst->graph->n_vertices; i++) {
		cout << vertices[i] << " ";
	}
	cout << endl;

	is_taken[vertices[0]] = true;
	int n_taken = 1;

	while (n_taken != inst->graph->n_vertices) {
		for (int i = 0; i < inst->graph->n_vertices; i++) {

			if (!is_taken[vertices[i]]) {
				continue;
			}

			int w = vertices[i];
			bool new_edge = false;

			for (int v = 0; v < inst->graph->n_vertices; v++) {
				if (v != w && inst->graph->is_adj(v, w) && !is_taken[v]) {

					pair<int,int> t;
					t.first = v;
					t.second = w;
					edges.push_back(t);

					is_taken[v] = true;
					n_taken++;

					new_edge = true;
					// cout << "took " << v << " due to " << w << endl;
				}
			}

			if (new_edge) {
				break;
			}
		}
	}

	// cout << endl;
	// for (int i = 0; i < (int)edges.size(); i++) {
	//   cout << edges[i].first << "," << edges[i].second << endl;
	// }
	// cout << endl;

	bool** new_adj = new bool*[inst->graph->n_vertices];
	for (int i = 0; i < inst->graph->n_vertices; i++) {
		new_adj[i] = new bool[inst->graph->n_vertices];
	}

	for (int i = 0; i < inst->graph->n_vertices; i++) {
		for (int j = i+1; j < inst->graph->n_vertices; j++) {
			new_adj[i][j] = false;
			new_adj[j][i] = false;
		}
	}

	for (int i = 0; i < (int)edges.size(); i++) {
		new_adj[edges[i].first][edges[i].second] = true;
		new_adj[edges[i].second][edges[i].first] = true;
	}

	original_adj_matrix = inst->graph->adj_m;
	inst->graph->adj_m = new_adj;
}


void CutVertexDecomposition::regenerate_graph()
{
	inst->graph->adj_m = original_adj_matrix;
}

/**
 * Orderings for a binary problem
 */

#include "bp_orderings.hpp"
#include "../../core/orderings.hpp"

using namespace boost;


Ordering* get_ordering_by_id_bp(int id, BPInstance* inst, Options& options)
{
	// Read ordering type
	switch (id) {
	case 1:
		return new RandomOrdering(inst);
	case 2:
		return new CuthillMcKeePairOrdering(inst);
	case 3:
		if (options.fixed_order_filename.empty()) {
			cout << "Error: text file required for ordering\n\n";
			exit(1);
		}
		return new FixedOrdering(inst, options.fixed_order_filename);
	case 4:
		return new NoOrdering();
	}
	return NULL;
}


void CuthillMcKeePairOrdering::construct_ordering()
{
	typedef adjacency_list<vecS, vecS, undirectedS,
	        property<vertex_color_t, default_color_type, property<vertex_degree_t,int>>> Graph;
	typedef graph_traits<Graph>::vertex_descriptor Vertex;

	map<int,int> var_to_graph;
	vector<int> graph_to_var;

	// Find relevant variables first
	int k = 0;
	for (BPRow* row : inst->rows) {
		if (row->nnonz == 2) {
			if (var_to_graph.find(row->ind[0]) == var_to_graph.end()) {
				var_to_graph[row->ind[0]] = k++;
				graph_to_var.push_back(row->ind[0]);
			}
			if (var_to_graph.find(row->ind[1]) == var_to_graph.end()) {
				var_to_graph[row->ind[1]] = k++;
				graph_to_var.push_back(row->ind[1]);
			}
		}
	}

	Graph graph(graph_to_var.size());

	for (BPRow* row : inst->rows) {
		if (row->nnonz == 2) {
			add_edge(var_to_graph[row->ind[0]], var_to_graph[row->ind[1]], graph);
		}
	}

	// typedef graph_traits<Graph>::edge_iterator edge_iterator;
	// pair<edge_iterator, edge_iterator> ei = edges(graph);
	// for (edge_iterator it = ei.first; it != ei.second; ++it) {
	// 	cout << "(" << source(*it, graph) << ", " << target(*it, graph) << ")" << endl;
	// }

	property_map<Graph, vertex_index_t>::type index_map = get(vertex_index, graph);
	vector<Vertex> perm(num_vertices(graph));

	// Run heuristic
	cuthill_mckee_ordering(graph, perm.begin(), get(vertex_color, graph), get(vertex_degree, graph));

	v_in_layer.clear();

	// Add variables not in a pair constraint
	for (BPVar* var : inst->vars) {
		if (var_to_graph.find(var->index) == var_to_graph.end()) {
			v_in_layer.push_back(var->index);
		}
	}

	// Add variables to ordering
	for (vector<Vertex>::const_iterator i = perm.begin(); i != perm.end(); ++i) {
		v_in_layer.push_back(graph_to_var[index_map[*i]]);
	}
}

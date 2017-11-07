/**
 * Graph data structure
 */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "graph.hpp"

#define MIN(a,b) a < b ? a : b
#define MAX(a,b) a > b ? a : b

using namespace std;


void Graph::read_dimacs(const char* filename)
{

	string buffer;
	char   command;

	ifstream input(filename);

	if (!input.is_open()) {
		cerr << "Error: could not open DIMACS graph file " << filename << endl << endl;
		exit(1);
	}

	int read_edges = 0;
	n_edges = -1;

	int source, target;

	while (read_edges != n_edges) {

		input >> command;

		if (command == 'c') {
			// read comment
			getline(input, buffer);

		} else if (command == 'p') {
			// read 'edge' or 'col'
			input >> buffer;

			// read number of vertices and edges
			input >> n_vertices;
			input >> n_edges;

			// allocate adjacent matrix
			adj_m = new bool*[n_vertices];
			for (int i = 0; i < n_vertices; i++) {
				adj_m[i] = new bool[n_vertices];
				memset(adj_m[i], false, sizeof(bool)*n_vertices);
			}

			// allocate adjacent list
			adj_list.resize(n_vertices);


		} else if (command == 'e') {
			// read edge
			input >> source;
			source--;

			input >> target;
			target--;

			set_adj(source, target);
			set_adj(target, source);

			read_edges++;
		}

	}

	input.close();

	int count_edges = 0;
	for (int i = 0; i < n_vertices; i++) {
		for (int j = i+1; j < n_vertices; j++) {
			if (is_adj(i,j)) {
				count_edges++;
			}
		}
	}

	cout << "\tedges updated: " << count_edges << " - " << n_edges << endl;
	n_edges = count_edges;
	cout << "\tvertices: " << n_vertices << endl;
}


void Graph::export_to_gml(const char* output)
{

	ofstream file(output);
	file << "graph [\n";

	for (int i = 0; i < n_vertices; i++) {
		file << "node [\n";
		file << "\tid " << i << "\n";
		file << "\tlabel \"" << i << "\"\n";

		file << "\t graphics [ \n";
		file << "\t\t type \"ellipse\"\n";
		file << "\t\t hasFill 0 \n";
		file << "\t\t ] \n";

		file << "\t]\n" << endl;
	}
	int total_edges = 0;
	for (int i = 0; i < n_vertices; i++) {
		for (int j = i+1; j < n_vertices; j++) {
			if (!is_adj(i, j)) {
				continue;
			}
			file << "edge [\n";
			file << "\t source " << i << "\n";
			file << "\t target " << j << "\n";
			file << "\t]\n";
			total_edges++;
		}
	}
	file << "\n]";
	file.close();
	cout << "TOTAL EDGES: " << total_edges << endl;
}


/**
 * Create an isomorphic graph according to a vertex mapping
 * Mapping description: mapping[i] = position where vertex i is in new ordering
 */
Graph::Graph(Graph* graph, vector<int>& mapping)
	: n_vertices(graph->n_vertices), n_edges(graph->n_edges)
{
	// allocate adjacent matrix
	adj_m = new bool*[n_vertices];
	for (int i = 0; i < n_vertices; i++) {
		adj_m[i] = new bool[n_vertices];
		memset(adj_m[i], false, sizeof(bool)*n_vertices);
	}

	// allocate adjacent list
	adj_list.resize(n_vertices);

	// construct graph according to mapping
	for (int i = 0; i < graph->n_vertices; i++) {
		for (int j : graph->adj_list[i]) {
			set_adj(mapping[i], mapping[j]);
		}
	}
}


void Graph::print()
{
	cout << "Graph" << endl;
	for (int v = 0; v < n_vertices; ++v) {
		if (adj_list[v].size() != 0) {
			cout << "\t" << v << " --> ";
			for (int u : adj_list[v]) {
				cout << u << " ";
			}
			cout << endl;
		}
	}
	cout << endl;
}


Graph* Graph::create_subgraph(vector<int> subgraph_vertices)
{
	Graph* subgraph = new Graph(subgraph_vertices.size());
	for (int u = 0; u < subgraph->n_vertices; ++u) {
		for (int v = u; v < subgraph->n_vertices; ++v) {
			if (is_adj(subgraph_vertices[u], subgraph_vertices[v])) {
				subgraph->add_edge(u, v);
			}
		}
	}

	return subgraph;
}


/** Extract a clique from the graph. If graph has no edges, return NULL. */
Graph* extract_clique(Graph* graph, vector<vector<int>>& cliques)
{
	// find vertex with largest degree
	int vertex = -1;
	int max_degree = 0;
	for (int v = 0; v < graph->n_vertices; ++v) {
		if (graph->degree(v) > max_degree) {
			vertex = v;
			max_degree = graph->degree(v);
		}
	}

	// if maximum degree is zero, graph has no edges
	if (max_degree == 0) {
		return NULL;
	}

	// vertices already in clique (using bool* might be slightly more efficient)
	vector<bool> in_clique(graph->n_vertices, false);
	vector<int> clique;
	clique.push_back(vertex);
	in_clique[vertex] = true;

	// grow clique by taking vertices that are adjacent to 'vertex' with maximum degree
	while (true) {

		vector<int> subgraph;
		for (int u : graph->adj_list[vertex]) {
			// check if vertex is adjacent to all vertices in clique
			bool is_adj_all = !in_clique[u];
			for (int j = 1; j < (int)clique.size() && is_adj_all; ++j) {
				is_adj_all = graph->is_adj(clique[j], u);
			}
			if (is_adj_all) {
				subgraph.push_back(u);
			}
		}

		if (subgraph.empty()) {
			break;
		}

		max_degree = -1;
		int selected_u = -1;
		for (int i = 0; i < (int)subgraph.size(); ++i) {
			int degree = 0;
			for (int j = 0; j < (int)subgraph.size(); ++j) {
				if (i == j) {
					continue;
				}
				if (graph->is_adj(subgraph[i], subgraph[j])) {
					degree++;
				}
			}
			if (degree > max_degree) {
				selected_u = subgraph[i];
				max_degree = degree;
			}
		}

		// add vertex to clique
		clique.push_back(selected_u);
		in_clique[selected_u] = true;
	}

	// build graph representing clique, and remove clique from original graph
	for (int i = 0; i < (int)clique.size(); ++i) {
		for (int j = i+1; j < (int)clique.size(); ++j) {
			graph->remove_edge(clique[i], clique[j]);
		}
	}

	// add clique to set of cliques
	cliques.push_back(clique);

	return graph;
}


/** Decompose the graph into cliques */
void clique_decomposition(Graph* graph, vector<vector<int>>& cliques)
{
	// Create copy of the graph
	Graph* graph_copy = new Graph(graph->n_vertices);
	for (int v = 0; v < graph->n_vertices; ++v) {
		if (graph->adj_list[v].size() != 0) {
			for (int u : graph->adj_list[v]) {
				graph_copy->add_edge(v, u);
			}
		}
	}
	Graph* graph_copy_ptr = graph_copy; // only to keep pointer for deletion

	cliques.clear();

	// Extract cliques
	while (graph_copy != NULL) {
		graph_copy = extract_clique(graph_copy, cliques);

		// cout << "Clique " << cliques.size() << ": ";
		// for( int i = 0; i < (int)cliques.back().size(); ++i ) {
		//  cout << cliques.back()[i] << " ";
		// }
		// cout << endl;
	}

	delete graph_copy_ptr;
}

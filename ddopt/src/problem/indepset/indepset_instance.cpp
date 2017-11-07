/**
 * Independent set instance
 */

#include <cstdlib>
#include <iostream>
#include <vector>

#include "indepset_instance.hpp"

using namespace std;


/**
 * Create independent set instance with no costs from graph
 */
void IndepSetInstance::create_from_graph(Graph* orig_graph)
{
	vector<double> obj(orig_graph->n_vertices, 1);  // default weights of 1
	create_from_graph(orig_graph, obj);
}


void IndepSetInstance::create_from_graph(Graph* orig_graph, const vector<double>& obj)
{
	graph = orig_graph;

	nvars = graph->n_vertices;
	assert((int) obj.size() == graph->n_vertices);
	weights = new double[graph->n_vertices];
	for (int i = 0; i < graph->n_vertices; i++) {
		weights[i] = obj[i];
	}

	// create complement mask of adjacencies
	adj_mask_compl = new IntSet[graph->n_vertices];
	for (int v = 0; v < graph->n_vertices; v++) {

		adj_mask_compl[v].resize(0, graph->n_vertices-1, true);
		for (int w = 0; w < graph->n_vertices; w++) {
			if (graph->is_adj(v,w)) {
				adj_mask_compl[v].remove(w);
			}
		}

		// a vertex is adjacent to itself
		adj_mask_compl[v].remove(v);

	}
}


/**
 * Read DIMACS independent set instance with optional weights file
 */
void IndepSetInstance::read_DIMACS(const char* filename, const char* weights_file)
{

	cout << "Reading instance " << filename << endl;

	// Read graph
	graph = new Graph;
	graph->read_dimacs(filename);

	cout << "\tnumber of vertices: " << graph->n_vertices << endl;
	cout << "\tnumber of edges: " << graph->n_edges << endl;

	create_from_graph(graph);

	// Assign weights
	if (weights_file != NULL) {
		ifstream weightsfile(weights_file);
		for (int i = 0; i < graph->n_vertices; i++) {
			weightsfile >> weights[i];
		}
		weightsfile.close();
	}

	cout << "\tdone." << endl;
}

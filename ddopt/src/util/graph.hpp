/**
 * Graph data structure
 */

#ifndef GRAPH_HPP_
#define GRAPH_HPP_

#include <cassert>
#include <cstring>
#include <vector>

using namespace std;


/** 
 * Simple graph structure that assumes that arcs/nodes are not removed once inserted. It keeps a redundant representation
 * as an adjacent matrix and list for fast iteration and adjacency check.
 */
struct Graph {

	bool**                      adj_m;              /**< adjacent matrix */
	vector< vector<int> >       adj_list;           /**< adjacent list */

	int                         n_vertices;         /**< |V| */
	int                         n_edges;            /**< |E| */

	/** Set two vertices as adjacents */
	void set_adj(int i, int j);

	/** Check if two vertices are adjancent */
	bool is_adj(int i, int j);

	/** Empty constructor */
	Graph();

	/** Create an isomorphic graph according to a vertex mapping */
	Graph(Graph* graph, vector<int>& mapping);

	/** Destructor */
	~Graph();

	/** Read graph from a DIMACS format */
	void read_dimacs(const char* filename);

	/** Export to GML format */
	void export_to_gml(const char* output);

	/** Constructor with number of vertices */
	Graph(int num_vertices);

	/** Add edge */
	void add_edge(int i, int j);

	/** Remove edge */
	void remove_edge(int i, int j);

	/** Return degree of a vertex */
	int degree(int v)
	{
		return adj_list[v].size();
	}

	/** Return the density of the graph */
	double density();

	/** Print graph */
	void print();

	/**
	 * Create a vertex-induced subgraph of a graph for vertices such that subgraph_vertices[i] == true.
	 * The vertices will be mapped corresponding to the order in the original graph.
	 */
	Graph* create_subgraph(vector<int> subgraph_vertices);
};


/**
 * ----------------------------------------------
 * Inline implementations
 * ----------------------------------------------
 */

/**
 * Empty constructor
 */
inline Graph::Graph() : n_vertices(0), n_edges(0)
{

}

/**
 * Destructor
 */
inline Graph::~Graph()
{
	for (int i = 0; i < n_vertices; ++i) {
		delete[] adj_m[i];
	}
	delete[] adj_m;
}

/**
 * Check if two vertices are adjacent
 */
inline bool Graph::is_adj(int i, int j)
{
	assert(i >= 0);
	assert(j >= 0);
	assert(i < n_vertices);
	assert(j < n_vertices);
	return adj_m[i][j];
}


/**
 * Set two vertices as adjacent
 */
inline void Graph::set_adj(int i, int j)
{
	assert(i >= 0);
	assert(j >= 0);
	assert(i < n_vertices);
	assert(j < n_vertices);

	// check if already adjacent
	if (adj_m[i][j]) {
		return;
	}

	// add to adjacent matrix and list
	adj_m[i][j] = true;
	adj_list[i].push_back(j);
}


/**
 * Constructor with number of vertices
 */
inline Graph::Graph(int num_vertices)
	: n_vertices(num_vertices), n_edges(0)
{
	adj_m = new bool*[ num_vertices ];
	for (int i = 0; i < num_vertices; ++i) {
		adj_m[i] = new bool[ num_vertices ];
		memset(adj_m[i], false, sizeof(bool) * num_vertices);
	}
	adj_list.resize(num_vertices);
}

/**
 * Add edge
 */
inline void Graph::add_edge(int i, int j)
{
	assert(i >= 0);
	assert(j >= 0);
	assert(i < n_vertices);
	assert(j < n_vertices);

	// check if already adjacent
	if (adj_m[i][j]) {
		return;
	}

	// add to adjacent matrix and list
	adj_m[i][j] = true;
	adj_m[j][i] = true;
	adj_list[i].push_back(j);
	adj_list[j].push_back(i);

	n_edges++;
}

/**
 * Remove edge
 */
inline void Graph::remove_edge(int i, int j)
{
	assert(i >= 0);
	assert(j >= 0);
	assert(i < n_vertices);
	assert(j < n_vertices);

	// check if already adjacent
	if (!adj_m[i][j]) {
		return;
	}

	// add to adjacent matrix and list
	adj_m[i][j] = false;
	adj_m[j][i] = false;

	for (int v = 0; v < (int)adj_list[i].size(); ++v) {
		if (adj_list[i][v] == j) {
			adj_list[i][v] = adj_list[i].back();
			adj_list[i].pop_back();
			break;
		}
	}

	for (int v = 0; v < (int)adj_list[j].size(); ++v) {
		if (adj_list[j][v] == i) {
			adj_list[j][v] = adj_list[j].back();
			adj_list[j].pop_back();
			break;
		}
	}
}

/**
 * Return the density of the graph
 */
inline double Graph::density()
{
	return (double) n_edges / (n_vertices * (n_vertices - 1) / 2);
}

/** Decompose the graph into cliques */
void clique_decomposition(Graph* graph, vector<vector<int>>& cliques);

#endif


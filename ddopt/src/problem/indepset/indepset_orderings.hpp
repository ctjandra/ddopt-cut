/**
 * Variable ordering for the independent set problem
 */

#ifndef INDEPSET_ORDERINGS_HPP_
#define INDEPSET_ORDERINGS_HPP_

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/discrete_distribution.hpp>

#include "../../core/order.hpp"
#include "indepset_instance.hpp"
#include "indepset_state.hpp"

using namespace std;


/** Return an ordering for the independent set problem given an id */
Ordering* get_ordering_by_id_indepset(int id, IndepSetInstance* inst, Options& options);


// Vertex that it is in the least number of states - with option to choose randomly with probability (1-prob)
struct MinInState : Ordering {

	IndepSetInstance* inst;
	boost::random::mt19937 gen;
	int* in_state_counter;          /**< number of states containing each variable */
	double prob;			        /**< probability min in state is applied (otherwise random) */

	MinInState(IndepSetInstance* _inst, double _prob = 1) : inst(_inst), prob(_prob)
	{
		sprintf(name, "min_in_state");

		assert(prob >= 0 && prob <= 1);
		if (prob < 1) {
			gen.seed(inst->graph->n_vertices + inst->graph->n_edges);
		}

		in_state_counter = new int[inst->graph->n_vertices];
	}

	~MinInState()
	{
		delete[] in_state_counter;
	}

	int select_next_var(int layer);
	int select_vertex_with_min_in_state(int layer);
	int select_vertex_randomly(int layer);

	void cb_initialize();
	void cb_state_created(State* state);
	void cb_state_removed(State* state);
};


// Maximal Path Decomposition
struct MaximalPathDecomp : Ordering {

	IndepSetInstance* inst;
	vector<int> v_in_layer;   // vertex at each layer

	MaximalPathDecomp(IndepSetInstance* _inst) : inst(_inst)
	{
		sprintf(name, "maxpath");
		construct_ordering();
	}

	int select_next_var(int layer)
	{
		assert(layer >= 0 && layer < inst->graph->n_vertices);
		return v_in_layer[layer];
	}

private:
	void construct_ordering();
};


// Minimum degree ordering
struct MinDegreeOrdering : Ordering {

	IndepSetInstance* inst;
	vector<int> v_in_layer;   // vertex at each layer

	MinDegreeOrdering(IndepSetInstance* _inst) : inst(_inst)
	{
		sprintf(name, "mindegree");
		construct_ordering();
	}

	int select_next_var(int layer)
	{
		assert(layer >= 0 && layer < inst->graph->n_vertices);
		return v_in_layer[layer];
	}

private:
	void construct_ordering();
};


// Cut vertex decomposition
struct CutVertexDecomposition : Ordering {

	IndepSetInstance* inst;
	vector<int> v_in_layer;      // vertex at each layer
	bool** original_adj_matrix;

	CutVertexDecomposition(IndepSetInstance* _inst, bool general = true) : inst(_inst)
	{
		sprintf(name, "cut-vertex");
		v_in_layer.resize(inst->graph->n_vertices);

		// If a general graph (not a tree), take a spanning tree
		if (general) {
			restrict_graph();
		}
		construct_ordering();
		if (general) {
			regenerate_graph();
		}
	}

	int select_next_var(int layer)
	{
		assert(layer >= 0 && layer < inst->graph->n_vertices);
		return v_in_layer[layer];
	}

private:
	void        restrict_graph();
	void        regenerate_graph();
	void        construct_ordering();
	void        identify_components(vector< vector<int> >& comps, vector<bool>& is_in_graph);
	vector<int> find_ordering(vector<bool> is_in_graph);
};


#endif

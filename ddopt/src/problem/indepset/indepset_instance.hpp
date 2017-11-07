/**
 * Independent set instance
 */

#ifndef INDEPSET_INSTANCE_HPP_
#define INDEPSET_INSTANCE_HPP_

#include <fstream>
#include <iostream>
#include <cstring>

#include "../../util/graph.hpp"
#include "../../util/intset.hpp"

#include "../instance.hpp"

using namespace std;


/**
 * Independent set instance structure
 */
class IndepSetInstance : public Instance
{
public:
	Graph*              graph;             /**< independent set graph */
	IntSet*             adj_mask_compl;	   /**< complement mask of adjacencies */


	~IndepSetInstance()
	{
		delete graph;
		delete[] adj_mask_compl;
		delete[] weights;
	}

	/** Create independent set instance with no weights from graph */
	void create_from_graph(Graph* graph);

	/** Create independent set instance with weights from obj */
	void create_from_graph(Graph* orig_graph, const vector<double>& obj);

	/** Read DIMACS independent set instance with optional weights file */
	void read_DIMACS(const char* filename, const char* weights_file = NULL);
};


#endif /* INDEPSET_INSTANCE_HPP_ */

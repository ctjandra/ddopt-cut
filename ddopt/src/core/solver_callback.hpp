/**
 * Callback interface for decision diagram construction
 */

#ifndef DD_SOLVER_CALLBACK_HPP_
#define DD_SOLVER_CALLBACK_HPP_

#include "../bdd/bdd.hpp"
#include "../util/options.hpp"

/** Interface for a special callback to be called during DD construction */
class DDSolverCallback
{
public:
	virtual ~DDSolverCallback() {}

	virtual void cb_layer_end(BDD* bdd, const vector<Node*>& nodes_layer, NodeMap& node_list, int width, int current_layer,
	                          Options* options) {}

	virtual void cb_pre_merge(BDD* bdd, const vector<Node*>& nodes_layer, const NodeMap& node_list, int width, int current_layer) {}

	virtual void cb_post_merge(BDD* bdd, const vector<Node*>& nodes_layer, const NodeMap& node_list, int width, int current_layer) {}

	virtual void cb_solver_end(BDD* bdd, Options* options) {}
};


#endif // DD_SOLVER_CALLBACK_HPP_

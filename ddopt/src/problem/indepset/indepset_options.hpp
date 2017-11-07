/**
 * Options for the independent set problem
 */

#ifndef INDEPSET_OPTIONS_HPP_
#define INDEPSET_OPTIONS_HPP_

 struct IndepSetOptions {
	bool   clique_formulation                   = true;    /**< use clique cover formulation instead of edge */
	bool   lp_relaxation                        = false;   /**< run LP relaxation only (set variables to continuous) */
};

#endif /* INDEPSET_OPTIONS_HPP_ */

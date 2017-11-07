/** 
 * Generator of a BP instance off an MPS file using CPLEX
 */

#ifndef BP_READER_HPP_
#define BP_READER_HPP_

#include <ilcplex/ilocplex.h>
#include <vector>
#include <map>
#include <string>
#include "bpvar.hpp"
#include "bprow.hpp"
#include "bp_instance.hpp"
#include "../../util/options.hpp"

#define NODD_TAG               "nodd" /* tag that indicates that constraint should not be added to the decision diagram 
                                       * (i.e. if the name of a constraint ends with this string, it will not be considered) */


// Note: Error checking is not robust; it is up to the user to ensure consistency
/** Class to filter out unwanted variables and constraints in problem */
class BPReaderFilter
{
public:

	/** Return true if variable must be excluded from problem */
	virtual bool exclude_var(string var_name) = 0;

	/** Return true if constraint must be excluded from problem */
	virtual bool exclude_cons(string cons_name) = 0;
};

/** Read instance off MPS file using CPLEX */
BPInstance* read_bp_instance_cplex_mps(string mps_filename, BPReaderFilter* filter = NULL);


#endif // BP_READER_HPP_

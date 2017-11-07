/** 
 * Problem-specific main processing
 */

#ifndef MAIN_PROB_HPP_
#define MAIN_PROB_HPP_

#include <string>
#include "util/options.hpp"

/** Main processing for an independent set problem */
void main_indepset(int order_n, int merge_n, string instance_path, string instance_filename, bool skip_dd, bool dd_only,
	Options& options);

/** Main processing for a binary program */
void main_bp(int order_n, int merge_n, string instance_path, string instance_filename, bool skip_dd, bool dd_only,
	Options& options);

#endif // MAIN_PROB_HPP_

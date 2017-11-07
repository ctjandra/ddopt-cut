/**
 * Merging functions specific to binary problems
 */

#ifndef BP_MERGERS_HPP_
#define BP_MERGERS_HPP_

#include "../../core/merge.hpp"


/** Return a merger for a binary problem given an id */
Merger* get_merger_by_id_bp(int id, int width);

#endif // BP_MERGERS_HPP_

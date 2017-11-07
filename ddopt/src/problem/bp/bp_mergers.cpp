/**
 * Merging functions specific to binary problems
 */

#include "bp_mergers.hpp"
#include "../../core/mergers.hpp"

Merger* get_merger_by_id_bp(int id, int width)
{
	// Read merge type
	switch (id) {
	case 1:
		return new MinLongestPathMerger(width);
	case 2:
		return new PairMinLongestPathMerger(width);
	case 3:
		return new ConsecutivePairLongestPathMerger(width);
	case 4:
		return new LexicographicMerger(width);
	case 5:
		return new RandomMerger(width);
	}
	return NULL;
}

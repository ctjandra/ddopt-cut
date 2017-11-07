/**
 * Merging functions specific to the independent set problem
 */

#include "indepset_mergers.hpp"
#include "../../core/mergers.hpp"

Merger* get_merger_by_id_indepset(int id, int width)
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
		return new MinSizeMerger(width);
	case 5:
		return new MaxSizeMerger(width);
	case 6:
		return new LexicographicMerger(width);
	case 7:
		return new MinNewSolsBoundMerger(width);
	case 8:
		return new RandomMerger(width);
	}
	return NULL;
}

/**
 * General-purpose variable ordering for decision diagrams
 */

#include <fstream>
#include <algorithm>
#include "orderings.hpp"

using namespace std;


// fixed ordering
void FixedOrdering::read_ordering(string filename)
{
	ifstream ordering(filename);
	if (!ordering.is_open()) {
		cout << "ERROR - could not open ordering file " << filename << endl;
		exit(1);
	}

	for (int v = 0; v < inst->nvars; v++) {
		ordering >> v_in_layer[v];
	}

	ordering.close();
}


void RandomOrdering::construct_ordering()
{
	for (int i = 0; i < inst->nvars; i++) {
		v_in_layer[i] = i;
	}

	random_shuffle(v_in_layer.begin(), v_in_layer.end());
}

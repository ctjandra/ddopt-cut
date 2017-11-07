/**
 * Binary problem instance
 */

#ifndef BP_INSTANCE_HPP_
#define BP_INSTANCE_HPP_

#include "../instance.hpp"
#include "bpvar.hpp"
#include "bprow.hpp"

using namespace std;


/**
 * Binary program instance structure
 */
class BPInstance : public Instance
{
public:
	vector<BPVar*>                  vars;                        /**< variables */
	vector<BPRow*>                  rows;                        /**< constraints */

	// int nvars in base Instance class
	int                             nrows;

	BPInstance(vector<BPVar*> _vars, vector<BPRow*> _rows) : vars(_vars), rows(_rows)
	{
		nvars = _vars.size();
		nrows = _rows.size();

		weights = new double[nvars];
		for (int i = 0; i < nvars; i++) {
			weights[i] = _vars[i]->obj;
		}
	}

	~BPInstance()
	{
		delete[] weights;
	}
};


#endif /* BP_INSTANCE_HPP_ */

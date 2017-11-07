/**
 * Classes to compute an interior point for target cuts
 */

#include "intpt_selector.hpp"

InteriorPointSelector* get_interior_point_selector_from_id(InteriorPointSelectorId id, Instance* inst, BDD* bdd)
{
	switch (id) {
	case INTPT_DEFAULT: // default
		return new InteriorPointSelectorDDCenter(bdd);
		break;
	case INTPT_ZERO:
		return new InteriorPointSelectorZero(inst->nvars);
		break;
	case INTPT_ONE:
		return new InteriorPointSelectorOne(inst->nvars);
		break;
	case INTPT_INDEPSET:
		return new InteriorPointSelectorIndepSet(inst);
		break;
	case INTPT_DDCENTER:
		return new InteriorPointSelectorDDCenter(bdd);
		break;
	default:
		cout << "Error: No interior point for target cuts selected" << endl;
		exit(1);
	}
}

/**
 * Classes to compute an interior point for target cuts
 */

#ifndef INTPT_SELECTOR_HPP_
#define INTPT_SELECTOR_HPP_

#include <vector>
#include "../problem/indepset/indepset_instance.hpp"
#include "../bdd/bdd.hpp"

enum InteriorPointSelectorId {
	INTPT_DEFAULT = -1,
	INTPT_ZERO = 0,
	INTPT_ONE,
	INTPT_INDEPSET,
	INTPT_DDCENTER
};


/** Interface class: Compute an interior point for the problem */
class InteriorPointSelector
{
public:

	virtual ~InteriorPointSelector() {}

	/** Select a valid interior point */
	virtual vector<double> select() = 0;

	/** Return true if the point is in variable space and needs to be converted to layer space */
	virtual bool in_var_space() = 0;
};


/** Return an interior point selector from its id */
InteriorPointSelector* get_interior_point_selector_from_id(InteriorPointSelectorId id, Instance* inst, BDD* bdd);


// Specific implementations of interior point selector

/** Compute an interior point for independent set */
class InteriorPointSelectorIndepSet : public InteriorPointSelector
{
private:
	int n_vertices;

public:
	InteriorPointSelectorIndepSet(Instance* inst)
	{
		IndepSetInstance* inst_is = dynamic_cast<IndepSetInstance*>(inst);
		if (inst_is == NULL) {
			cout << "Error: Independent set interior point selector only works with independent set instances" << endl;
			exit(1);
		}
		n_vertices = inst_is->graph->n_vertices;
	}

	vector<double> select()
	{
		vector<double> interior_point(n_vertices);
		for (int i = 0; i < n_vertices; ++i) {
			interior_point[i] = 1.0 / (2 * n_vertices);
		}
		return interior_point;
	}

	bool in_var_space()
	{
		return false;
	}
};


/** Use the origin as an interior point */
class InteriorPointSelectorZero : public InteriorPointSelector
{
private:
	int nvars;

public:
	InteriorPointSelectorZero(int _nvars) : nvars(_nvars) {}

	vector<double> select()
	{
		vector<double> interior_point(nvars, 0);
		return interior_point;
	}

	bool in_var_space()
	{
		return false;
	}
};


/** Use (1,...,1) as an interior point */
class InteriorPointSelectorOne : public InteriorPointSelector
{
private:
	int nvars;

public:
	InteriorPointSelectorOne(int _nvars) : nvars(_nvars) {}

	vector<double> select()
	{
		vector<double> interior_point(nvars, 1);
		return interior_point;
	}

	bool in_var_space()
	{
		return false;
	}
};


/** Compute the center of a decision diagram as an interior point */
class InteriorPointSelectorDDCenter : public InteriorPointSelector
{
private:
	BDD* bdd;

public:
	InteriorPointSelectorDDCenter(BDD* _bdd) : bdd(_bdd) {}

	vector<double> select()
	{
		int nvars = bdd->nvars();
		vector<double> interior_point(nvars, 0);
		bdd->get_center(interior_point);

		cout << "Interior point (center): ";
		for (int i = 0; i < nvars; ++i) {
			cout << interior_point[i] << " ";
		}
		cout << endl;

		return interior_point;
	}

	bool in_var_space()
	{
		return false;
	}
};


#endif // INTPT_SELECTOR_HPP_

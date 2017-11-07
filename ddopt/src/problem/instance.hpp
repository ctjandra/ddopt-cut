/*
 * Abstract classes that need to be implemented in order to model a new problem to be solved.
 */

#ifndef INSTANCE_HPP_
#define INSTANCE_HPP_


class Instance
{
public:
	double*                    weights;                     /**< (linear) objective function (maximize) */
	int                        nvars;                       /**< number of variables in instance */

	virtual ~Instance() {}
};


#endif /* INSTANCE_HPP_ */

/**
 * General utilities
 */

#ifndef UTIL_HPP_
#define UTIL_HPP_


/**
 * -------------------------------------------------------------
 * Macros
 * -------------------------------------------------------------
 */

#include <math.h>
#include <limits>

#ifndef MIN
#define MIN(_a_,_b_)              ((_a_ < _b_) ? _a_ : _b_)
#endif
#ifndef MAX
#define MAX(_a_,_b_)              ((_a_ > _b_) ? _a_ : _b_)
#endif

#define FLOOR(_a_)                ( (int)_a_ )
#define CEIL(_a_)                 ( ((int)_a_ == _a_) ? _a_ : (((int)_a_)+1) )

#define EPSILON 1e-9
#define DBL_EQ(a,b) (fabs((a) - (b)) <= EPSILON)
#define DBL_GE(a,b)    ((a) >= (b) - EPSILON)
#define DBL_LE(a,b)    ((a) <= (b) + EPSILON)
#define DBL_GT(a,b)    ((a) > (b) + EPSILON)
#define DBL_LT(a,b)    ((a) < (b) - EPSILON)

#define DBL_EQ_TOL(a,b,tol)    (fabs((a) - (b)) <= tol)
#define DBL_GE_TOL(a,b,tol)    ((a) >= (b) - tol)
#define DBL_LE_TOL(a,b,tol)    ((a) <= (b) + tol)
#define DBL_GT_TOL(a,b,tol)    ((a) > (b) + tol)
#define DBL_LT_TOL(a,b,tol)    ((a) < (b) - tol)

/**
 * -------------------------------------------------------------
 * Constants
 * -------------------------------------------------------------
 */

const int   INF_WIDTH   = -1;         /**< infinite width */
const int   INF   		= std::numeric_limits<int>::max();

#endif /* UTIL_HPP_ */

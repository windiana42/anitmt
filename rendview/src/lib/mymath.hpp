#ifndef _RENDVIEW_MATH_H_
#define _RENDVIEW_MATH_H_ 1

#include "config.h"

#include <math.h>

#if HAVE_NAN_H
# include <nan.h>
#endif

#if HAVE_IEEEFP_H
# include <ieeefp.h>
#endif

/*   FrameClockVal: UNUSED 
#if HAVE_ISINF
// Then, allright.
#elif HAVE_ISINFD
#  define isinf(x) isinfd(x)
#elif HAVE_FPCLASS
#  define isinf(x) (fpclass(x)==FP_NINF || fpclass(x)==FP_PINF)
#else
#  error "No isinf() and no known equivalent."
#endif

#if HAVE_ISNAN
// Then, allright. 
#elif HAVE_ISNAND
#  define isnan(x) isnand(x)
#elif HAVE_FPCLASS
#  define isnan(x) (fpclass(x)==FP_SNAN || fpclass(x)==FP_QNAN)
#else
#  error "No isnan() and no known equivalent."
#endif
*/

#endif  /* _RENDVIEW_MATH_H_ */

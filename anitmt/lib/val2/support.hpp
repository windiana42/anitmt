#ifndef _NS_vect_support_HPP_
#define _NS_vect_support_HPP_ 1

#include <stddef.h>
#include <math.h>

#include <ostream.h>

namespace vect
{

extern double epsilon;  // Max. difference for comparisons. 

// Inline function which computes x*x; often useful if you want to 
// calculate the square of some non-trivial expression. 
inline double sqr(double x)  {  return(x*x);  }

inline double deg2rad(double x)  {  return(x*M_PI/180.0);  }
inline double rad2deg(double x)  {  return(x*180.0/M_PI);  }

// Template forward declarations: 
class Flag;
class Scalar;
template<int N> class Vector;
template<int C,int R> class Matrix;
class String;

}  // namespace end 

#endif  /* _vect_support_HPP_ */

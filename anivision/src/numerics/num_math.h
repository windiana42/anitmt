/*
 * numerics/num_math.h
 * 
 * Numerics library general maths header. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _WW_NUMERICS_MATH_H_
#define _WW_NUMERICS_MATH_H_

// MUST BE FIRST: 
#include <numerics/config.h>

#include <math.h>

namespace NUM
{

// Global: machine epsilon: smallest value with 1.0+macheps1>1.0. 
extern const double macheps1;
// Square root of the above constant: 
extern const double sqrt_macheps1;

// Precision macheps1: smallest value with 1.0+macheps1_prec>1.0 
// when vars are in registers. 
extern const double macheps1_prec;


//------------------------------------------------------------------------------

// Returns x*x. 
#ifdef SQR
#error SQR defined
#endif
template<typename T>inline T SQR(T x)  {  return(x*x);  }

// I wonder what this function does...
#ifdef fabs
#error fabs defined
#endif
// See NUM_OVERLOADED(fabs) below, too. 
template<typename T>inline T fabs(T x)  {  return(x<0 ? -x : x);  }

#define NUM_OVERLOADED(func) \
	inline double func(double x)  {  return(::func(x));  } \
	inline float func(float x)    {  return(::func ## f(x));  } \
	inline long double func(long double x)  {  return(::func ## l(x));  }
#define NUM_OVERLOADED2(func) \
	inline double func(double x,double y)  {  return(::func(x,y));  } \
	inline float func(float x,float y)     {  return(::func ## f(x,y));  } \
	inline long double func(long double x,long double y) \
		{  return(::func ## l(x,y));  }

#if defined(sqrt) || defined(sin) || defined(cos) || defined(tan) || defined(exp) || defined(log)
#error defined math functions
#endif
NUM_OVERLOADED(fabs)
NUM_OVERLOADED(sqrt)  NUM_OVERLOADED(cbrt)  NUM_OVERLOADED2(pow)
NUM_OVERLOADED(sin)   NUM_OVERLOADED(asin)
NUM_OVERLOADED(cos)   NUM_OVERLOADED(acos)
NUM_OVERLOADED(tan)   NUM_OVERLOADED(atan)  NUM_OVERLOADED2(atan2)
NUM_OVERLOADED(exp)   NUM_OVERLOADED(log)
NUM_OVERLOADED(exp10) NUM_OVERLOADED(log10)
NUM_OVERLOADED(exp2)  NUM_OVERLOADED(log2)
NUM_OVERLOADED(rint)  NUM_OVERLOADED(nearbyint)
NUM_OVERLOADED(ceil)  NUM_OVERLOADED(floor)
NUM_OVERLOADED(round) NUM_OVERLOADED(trunc)
// isinf, isnan, finite -> are actually #defined and work correctly for 
//  float, double and long double (via if(sizeof(x)==sizeof(float)...)
#undef NUM_OVERLOADED
#undef NUM_OVERLOADED2

#if defined(sincos)
#error defined math function sincos
#endif
inline void sincos(double x,double *sinx,double *cosx)
	{  ::sincos(x,sinx,cosx);  }
inline void sincos(float x,float *sinx,float *cosx)
	{  ::sincosf(x,sinx,cosx);  }
inline void sincos(long double x,long double *sinx,long double *cosx)
	{  ::sincosl(x,sinx,cosx);  }


// x^y in integer arithmetic. y MUST BE >=0. 
inline int powii(int x,int y)
{
	if(y<0) return(-1);
	int res=1;
	for(;y;x*=x)  {  if(y&1) res*=x;  y/=2;  }
	return(res);
}
// x^y for integer y. 
template<typename T>inline T powi(T x,int y)
{
	bool neg=(y<0 ? (y=-y,1) : 0);
	T res=1;
	for(;y;x*=x)  {  if(y&1) res*=x;  y/=2;  }
	return(neg ? 1/res : res);
}


// Return sign, i.e. -1, 0 or 1. 
#ifdef SGN
#error SGN defined
#endif
template<typename T>inline int SGN(T x)
	{  return(x>0.0 ? 1 : (x<0.0 ? -1 : 0));  }

#if defined(MAX) || defined(MIN)
#error MAX or MIN defined
#endif
template<typename T>inline T MAX(T a,T b)
	{  return(a<b ? b : a);  }
template<typename T>inline T MIN(T a,T b)
	{  return(a<b ? a : b);  }

// Numerically stable version of hypot; don't know if 
// it is really needed or if hypot() is clever enough. 
// Computes sqrt(a^2 + b^2) without destructive underflow or overflow 
// (algorithm from Numerical Recipes). 
template<typename T>inline T HYPOT(T a,T b)
{
	a=fabs(a); b=fabs(b);
	return(a>b ? (a * sqrt(1.0 + SQR(b/a))) : 
		(b==0.0 ? 0.0 : b * sqrt(1.0 + SQR (a/b))) );
}


// Returns euclidic norm of passed vector with dimension dim. 
template<typename T>inline T NormEuclidic(const T *x,int dim)
{
	if(dim==1)  return(fabs(*x));
	T sum=0.0;
	for(int i=0; i<dim; i++)
	{  sum+=SQR(x[i]);  }
	return(sqrt(sum));
}

// Returns the distance between two points (euclidic norm): 
// Points x,y specified as arrays of size dim. 
template<typename T>inline T DistEuclidic(const T *x,const T *y,int dim)
{
	if(dim==1)  return(fabs(*y-*x));
	T sum=0.0;
	for(int i=0; i<dim; i++)
	{  sum+=SQR(y[i]-x[i]);  }
	return(sqrt(sum));
}


// Return maximum norm, i.e. max{ a[0],...,a[dim-1] }
// Returns 0 if dim=0. 
template<typename T>inline T NormMaximum(const T *a,int dim)
{
	if(!dim) return(0);
	T max=a[0];
	for(int i=1; i<dim; i++)
		if(max<a[i]) max=a[i];
	return(max);
}

// Equivalent for minumum. Note that this actually is NOT a "norm", 
// hence the prefix "Find". 
template<class T>inline T FindMinimum(const T *a,int dim)
{
	if(!dim) return(0);
	T min=a[0];
	for(int i=1; i<dim; i++)
		if(min>a[i]) min=a[i];
	return(min);
}


// Normalize vector, i.e. scale it to length 1: 
// Returns length of vector before normalisation. 
template<typename T>inline T NormalizeVector(T *x,int dim)
{
	T len=NormEuclidic(x,dim),fact=1.0/len;
	for(int i=0; i<dim; i++)
	{  x[i]*=fact;  }
	return(len);
}
template<typename T>inline T NormalizeVector(T *dest,const T *src,int dim)
{
	T len=NormEuclidic(src,dim),fact=1.0/len;
	for(int i=0; i<dim; i++)
	{  dest[i]=src[i]*fact;  }
	return(len);
}

// Calculate cross product res=a x b. All vectors are 3dim, of course. 
template<typename T>inline void CrossProduct(T *dest,const T *a,const T *b)
{
	dest[0] = a[1]*b[2] - a[2]*b[1];
	dest[1] = a[2]*b[0] - a[0]*b[2];
	dest[2] = a[0]*b[1] - a[1]*b[0];
}


// array[0..size-1] is an array of elements sorted in ascenting 
// order. This function searches the index i with the following 
// condition: array[i]<=value<array[i+1], i.e. the index of the 
// interval which holds the value. 
// Returns -1  if value<array[0]       and 
//        size if array[size-1]<=value
// Returns 0 if size=0. 
// This function does a binary search and thus has log(size) runtime. 
template<typename T>inline int ArrayBinsearchInterval(T *array,int size,T value)
{
	if(!size)  return(0);
	// Prevent us from crossing the array borders below: 
	if(value<array[0])  return(-1);
	if(value>=array[size-1])  return(size);
nc_assert(size>=2);
	int a=0,b=size-1;
	while(b-a>1)
	{
		int m=(a+b)/2;
		if(value>=array[m])  a=m;
		else  b=m;
	}
nc_assert(array[a]<=value);
nc_assert(value<array[b]);
	return(a);
}


// Check if the passed t values are increasing steadily. 
// Actually, it checks if array[i]-array[i-1]<=val_eps. 
// Return value: 
//   0 -> OK
//  >0 -> index of the array element which is smaller than its 
//        predecessor. 
// Returns 0 if array==NULL. 
template<typename T>inline int TestOrderAscenting(const T *array,int n,
	double val_eps=1e-7)
{
	if(!array) return(0);
	for(int i=1; i<n; i++)
		if(array[i]-array[i-1]<=val_eps)
			return(i);
	return(0);
}

//------------------------------------------------------------------------------


extern void _AllocFailure(size_t size);

// Use that only for simple (POD) types like int or double. 
// Allocate an array of nelem elements: 
template<class T>inline T *ALLOC(size_t nelem)
{
	nelem*=sizeof(T);
	T *ptr=(T*)LMalloc(nelem);
	if(!ptr && nelem)  _AllocFailure(nelem);
	return(ptr);
}
// Free an array as allocated by ALLOC(): 
template<class T>inline T *FREE(T *ptr)
	{  return((T*)LFree(ptr));  }
// Reallocation, just as uaual...
template<class T>inline T *REALLOC(T *ptr,size_t nelem)
{
	nelem*=sizeof(T);
	T *nptr=(T*)LRealloc(ptr,nelem);
	if(!nptr && nelem)  _AllocFailure(nelem);
	return(nptr);
}

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_MATH_H_ */

/*
 * numerics/interpolate/extr0ebase.h
 * 
 * Base class for vector extrapolators to 0 with error. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _WW_NUMERICS_INTERPOLATE_EXTR0EBASE_H_
#define _WW_NUMERICS_INTERPOLATE_EXTR0EBASE_H_ 1

#include <numerics/num_math.h>


namespace NUM
{

// Base class for vector extrapolators to 0 with error estimate. 
// Derived classes include Extrapolator_0E_Poly and
// Extrapolator_0E_Rational. 
class Extrapolator_0E_Base
{
	protected:
		int dim;     // dimenstion of the used vectors
		int max_n;   // max number of interpolation points
		int n;       // number of points already added
		double *t;   // "time" or "x" values [max_n]
		
	public:
		Extrapolator_0E_Base(int _max_n,int _dim) : dim(_dim),max_n(_max_n),n(0)
			{  t=ALLOC<double>(max_n);  }
		virtual ~Extrapolator_0E_Base()
			{  t=FREE(t);  }
		
		// Set first interpolation point at (_t,_p[0..dim-1]). 
		// Returns extrapolation in px[0..dim-1] and estimated error 
		// in dp[0..dim-1]. 
		virtual void FirstPoint(double _t,const double *_p,
			double *px,double *dp);
		
		// Add more points to the interpolator. 
		// Returns extrapolation in px[0..dim-1] and estimated error 
		// in dp[0..dim-1]. 
		virtual void AddPoint(double _t,const double *_p,
			double *px,double *dp);
};

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_INTERPOLATE_EXTR0EBASE_H_ */

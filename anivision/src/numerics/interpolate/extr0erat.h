/*
 * numerics/interpolate/extr0erat.h
 * 
 * Rational extrapolation to 0 (for vectors and with error). 
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

#ifndef _WW_NUMERICS_INTERPOLATE_EXTR0ERAT_H_
#define _WW_NUMERICS_INTERPOLATE_EXTR0ERAT_H_ 1

#include <numerics/interpolate/extr0ebase.h>


namespace NUM
{

// Rational extrapolator to 0 for vectors with error estimation. 
// Just like Extrapolator_0E_Poly, but with rational extrapolation 
// instead of polynomial extrapolation. 
class Extrapolator_0E_Rational : public Extrapolator_0E_Base
{
	private:
		// In Extrapolator_0E_Base: 
		//int dim;     // dimenstion of the used vectors
		//int max_n;   // max number of interpolation points
		//int n;       // number of points already added
		//double *t;   // "time" or "x" values [max_n]
		double *_p;  // internal storage for interpolation [max_n*dim]
		
		// d: 0..dim-1, k: 0..max_n-1
		inline double &p(int d,int k)
			{  return(_p[k*dim+d]);  }
	public:
		// max_n: max number of interpolation points. 
		// dim: dimenstion of the used vectors. 
		Extrapolator_0E_Rational(int _max_n,int _dim) : 
			Extrapolator_0E_Base(_max_n,_dim)
			{  _p=ALLOC<double>(max_n*dim);  }
		~Extrapolator_0E_Rational()
			{  _p=FREE(_p);  }
		
		// Description: See base class. 
		// [overriding virtuals]
		void FirstPoint(double _t,const double *_p,double *px,double *dp);
		void AddPoint(double _t,const double *_p,double *px,double *dp);
};

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_INTERPOLATE_EXTR0ERAT_H_ */

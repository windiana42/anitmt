/*
 * numerics/interpolate/extrr0poly.h
 * 
 * Polynomial extrapolation to 0 for single values and without 
 * error using the Neville algorithm. 
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

#ifndef _WW_NUMERICS_INTERPOLATE_EXTRR0POLY_H_
#define _WW_NUMERICS_INTERPOLATE_EXTRR0POLY_H_ 1

#include <numerics/num_math.h>


namespace NUM
{

// Quite specialized class: 
// This is a polynomial extrapolator to 0. 
// You feed it a series of value pairs (t,x) with t>0 and t decreasing 
// continuously. Each time you add such a point using AddPoint(), it 
// calculates the polynomial extrapolation of x for the value t=0. 
// This is useful for Richardson's deferred approach to the limit. 
// This is R^1 in x, only, hence no vectors. In case vectors are 
// needed, use Extrapolator_0_Poly (below). 
class Extrapolator_R_0_Poly
{
	private:
		int max_n;
		int n;       // number of points already added 
		double *p;   // size: [max_n]
		double *t;   // size: [max_n]
		
		void _assertfail();
		
		// These are forbidden: 
		Extrapolator_R_0_Poly(const Extrapolator_R_0_Poly &) {}
		Extrapolator_R_0_Poly &operator=(const Extrapolator_R_0_Poly &) { return(*this); }
	public:  _CPP_OPERATORS
		// max_n is the max. number of used interpolation points. 
		Extrapolator_R_0_Poly(int _max_n) : max_n(_max_n),n(0)
			{  p=ALLOC<double>(max_n);  t=ALLOC<double>(max_n);  }
		~Extrapolator_R_0_Poly()
			{  t=FREE(t);  p=FREE(p);  }
		
		// Set first interpolation point at (t,p). 
		inline void FirstPoint(double _t,double _p)
			{  *t=_t;  *p=_p;  n=1;  }
		
		// Add interpolation point at (t,p). 
		// Note that the t values must all be >0 and monotoniously 
		//      falling. 
		// Returns the extrapolation to t=0. 
		double AddPoint(double _t,double _p)
		{
			#ifndef NDEBUG
			if(n>=max_n)  _assertfail();  //assert(n<max_n);   // correct
			#endif
			
			t[n]=_t;
			for(int k=0; k<n; k++)
			{	double tmp=_p-p[k];  p[k]=_p;
				_p=p[k] + tmp/( t[n-k-1]/_t - 1.0 );  }
			p[n]=_p;
			return(p[n++]);
		}
		
		// Retruns the current extrapolation to 0 
		// (just as AddPoint() does): 
		inline double Extrapolate0() const
			{  return(p[n-1]);  }
};


// Just like Extrapolator_R_0_Poly but for uses vectors of 
// dimension dim for the x values in (t,x). 
// --> To be placed in different file. 
class Extrapolator_0_Poly
{
	// [Implement based on Extrapolator_R_0_Poly above when actually needed.]
};

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_INTERPOLATE_EXTRR0POLY_H_ */

/*
 * numerics/interpolate/cebyshev.h
 * 
 * Cebyshev interpolation header. 
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

#ifndef _WW_NUMERICS_INTERPOLATE_CEBYSHEV_H_
#define _WW_NUMERICS_INTERPOLATE_CEBYSHEV_H_ 1

#include <numerics/function.h>

namespace NUM
{

class Interpolator_Chebyshev
{
	private:
		// Order of poly; use n+1 interpolation points. 
		// Position of these points is chosen by the 
		// interpolation algorithm. 
		int n;
		
		// The interpolation range: 
		double a,b;
		
		// Chebyshev coefficients for Chebyshev-Poly series of 
		// the interpolation poly. (Not the coeffs of the 
		// poly itself.) 
		double *gamma;
		
		// The actual polynomial coefficients (only valid if 
		// PolyCalcCoeffs() was called): 
		double *coeff;
		
		void _Reset(int new_n);
		
	public:  _CPP_OPERATORS
		Interpolator_Chebyshev();
		~Interpolator_Chebyshev();
		
		// Return degree of poly (number of interpolation points - 1): 
		int N() const
			{  return(n);  }
		
		// Set up the interpolator: 
		// Interpolate passed function from a to b 
		// using n+1 interpolation points. 
		// Time: O(n^2)  (n+1 function calls)
		// Return value: 
		//   0 -> OK
		//  -2 -> n too small (must be at least 1) 
		int Interpolate(Function_R_R &f,
			double a,double b,int n);
		
		// Set up interpolator using the (analytical) 
		// derivation of the passed Chebyshev poly. 
		// (Does NOT need PolyCalcCoeffs().)
		// Do not pass "this" as source; pp.Derive(pp) won't work. 
		// Time: O(n)
		// Return value: 
		//   0 -> OK
		//  -2 -> Inproper source poly (must have n>=2). 
		int Derive(const Interpolator_Chebyshev &src);
		
		// Evaluate interpolation polynomial. 
		// Passed value must be in range a..b. 
		// Uses Clanshaw method which is numerically stable. 
		// Time: O(n)
		// Returns NAN if not set up. 
		double Eval(double x);
		
		// Calculate the polynomial coefficients of the currently 
		// interpolated function. 
		// (i.e. the a-values in a0 + a1 x + a2 x^2 + a3 x^3 + ...
		// Time: O(n^2)
		// Numerical stability unknown but good in experiments. 
		// Return value: 
		//    0 -> OK
		//   -2 -> call Interpolate() first 
		int PolyCalcCoeffs();
		
		// Use coefficients calculated by PolyCalcCoeffs() and 
		// evaluate interpolation polynomial or first or second 
		// derivation. 
		// Passed value must be in range a..b. 
		// Time: O(n)
		// Numerical stability unknown but reasonable in experiments. 
		// Returns NAN if not set up or PolyCalcCoeffs() not called. 
		double PolyEval(double x);
		double PolyEvalD(double x);
		double PolyEvalDD(double x);
};

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_INTERPOLATE_CEBYSHEV_H_ */

/*
 * numerics/diff_int/derice.cc
 * 
 * Implementation of numerical derivation methods. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "derive.h"

namespace NUM  // numerics
{

double DiffCentral_GSL(Function_R_R &f,double x,double *abserr)
{
	// Borrowed from GSL. 

	/* Construct a divided difference table with a fairly large step
	   size to get a very rough estimate of f'''.  Use this to estimate
	   the step size which will minimize the error in calculating f'. */
	
	/* Algorithm based on description on pg. 204 of Conte and de Boor
	   (CdB) - coefficients of Newton form of polynomial of degree 3. */
	
	double h=sqrt_macheps1;
	double a[4],d[4],a3;
	for(int i=0; i<4; i++)
	{
		a[i] = x + (i-2) * h;
		d[i] = f(a[i]);
	}
	
	for(int k=1; k<4; k++)  // originally k<5
	{
		for(int i=0; i<4-k; i++)
		{
			d[i] = (d[i+1]-d[i]) / (a[i+k]-a[i]);
		}
	}
	
	/* Adapt procedure described on pg. 282 of CdB to find best
	   value of step size. */
	
	a3 = fabs(d[0] + d[1] + d[2] + d[3]);
	if(a3 < 100.0 * sqrt_macheps1)
	{  a3 = 100.0 * sqrt_macheps1;  }
	
	h = pow(sqrt_macheps1 / (2.0 * a3), 1.0 / 3.0);
	if(h > 100.0 * sqrt_macheps1)
	{  h = 100.0 * sqrt_macheps1;  }
	
	if(abserr)
	{  *abserr = fabs(100.0 * a3 * h * h);  }
	return((f(x+h) - f(x-h)) / (2.0 * h));
}

}  // end of namespace NUM

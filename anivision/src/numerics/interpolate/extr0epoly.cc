/*
 * numerics/interpolate/extr0epoly.cc
 * 
 * Polynomial extrapolation to 0 (Neville algorithm, for vectors and 
 * with error estimate). 
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

#include "extr0epoly.h"


namespace NUM
{

void Extrapolator_0E_Poly::FirstPoint(double _t,const double *_p,
	double *px,double *dp)
{
	*t=_t;
	for(int d=0; d<dim; d++)
	{
		double tmp=_p[d];
		p(d,0)=tmp;
		px[d]=tmp;
		dp[d]=tmp;
	}
	n=1;
}


void Extrapolator_0E_Poly::AddPoint(double _t,const double *_p,
	double *px,double *dp)
{
	assert(n<max_n);   // correct
	
	// This implements the Neville algorithm for the polynomial 
	// interpolation with added code for the error estimation. 
	
	t[n]=_t;
	
	// We need a copy of _p because it is const. 
	// [The 1dim algo in Extrapolator_R_0_Poly modifies the 
	//  passed value (allowed as it is on the stack).]
	double c[dim];
	for(int d=0; d<dim; d++)
	{
		double tmp=_p[d];
		px[d]=tmp;
		dp[d]=tmp;
		c[d]=tmp;
	}
	
	for(int k=0; k<n; k++)
	{
		// Especially tricky algorithm... :)
		double tmp1=t[n-k-1];
		double tmp2=1.0/(tmp1-_t);
		tmp1*=tmp2;
		tmp2*=_t;
		for(int d=0; d<dim; d++)
		{
			double tmp=c[d]-p(d,k);
			p(d,k)=dp[d];
			c[d]=tmp1*tmp;
			dp[d]=tmp2*tmp;
			px[d]+=dp[d];
		}
	}
	for(int d=0; d<dim; d++)
		p(d,n)=dp[d];
	
	++n;  // Oops... don't loose that one :)
}

}  // end of namespace NUM

/*
 * numerics/interpolate/extr0erat.cc
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

#include "extr0erat.h"


namespace NUM
{

void Extrapolator_0E_Rational::FirstPoint(double _t,const double *_p,
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


void Extrapolator_0E_Rational::AddPoint(double _t,const double *_p,
	double *px,double *dp)
{
	assert(n<max_n);   // correct
	
	t[n]=_t;
	
	// Predivided "time" values: 
	double _tt[n];
	double *tt=_tt-1;
	for(int i=1; i<=n; i++)
		tt[i]=t[n-i]/_t;
	
	for(int j=0; j<dim; j++)
	{
		double pp=p(j,0);
		double c=_p[j];
		
		p(j,0)=c;
		
		double pxsum=c,ddp;
		for(int i=1; i<=n; i++)
		{
			double b1=tt[i]*pp;
			double b=b1-c;
			if(b==0.0)
			{  ddp=pp;  }
			else
			{
				b=(c-pp)/b;
				ddp=c*b;
				c=b1*b;
			}
			
			// Strange: Test case seemed to work better with 
			//          if(i+1<n) [in one case only]. 
			if(i<n) pp=p(j,i);  
			
			p(j,i)=ddp;
			pxsum+=ddp;
		}
		dp[j]=ddp;
		px[j]=pxsum;
	}
	
	++n;  // Oops... don't loose that one :)
}

}  // end of namespace NUM

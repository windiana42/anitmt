/*
 * numerics/fit/linearfit.cc
 * 
 * Linear regression. 
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

#include "fit.h"

namespace NUM
{

// Perform linear fit (linear regression), i.e. fit passed data 
// (x,y) with specified y standard deviation against a straight line. 
// Return coefficients of y = a + b*x in *a and *b and their 
// respective standard deviations in *a_sigma,*b_sigma. 
// Return value: chi^2 of the fit or negative values in case of error.
double LinearFit(
	const double *x,const double *y,const double *y_sigma,int ndata,
	double *_a,double *a_sigma,
	double *_b,double *b_sigma)
{
	// Implementation based on formulas 15.2.2 to 15.2.22 in 
	// Numerical Recipes in C: The Art of Scientific Computing 
	// (William  H. Press, Brian P. Flannery, Saul A. Teukolsky, 
	// William T. Vetterling; New York: Cambridge University Press) 
	// Written & verified to work correctly 06/2004 by Wolfgang Wieser. 
	
	double S=0.0,Sx=0.0,Sy=0.0;
	for(int i=0; i<ndata; i++)
	{
		double ss=1.0/SQR(y_sigma[i]);
		S+=ss;
		Sx+=x[i]*ss;
		Sy+=y[i]*ss;
	}
	//fprintf(stderr,"S=<%g %g %g>\n",S,Sx,Sy);
	
	double SxS=Sx/S;
	double b=0.0,Stt=0.0;
	for(int i=0; i<ndata; i++)
	{
		double ti=(x[i]-SxS)/y_sigma[i];
		b+=ti*y[i]/y_sigma[i];
		Stt+=ti*ti;
	}
	b/=Stt;
	//fprintf(stderr,"<b=%g,Stt=%g>\n",b,Stt);
	
	double a = (Sy - Sx*b) / S;
	if(_a) *_a=a;
	if(_b) *_b=b;
	if(a_sigma)  *a_sigma=sqrt((1.0+Sx*Sx/(S*Stt))/S);
	if(b_sigma)  *b_sigma=sqrt(1.0/Stt);
	// Cov(a,b)=-Sx/(S*Stt);
	// r_ab=Cov(a,b)/(sigma_a*sigma_b)
	
	double chi2=0.0;
	for(int i=0; i<ndata; i++)
		chi2+=SQR((y[i]-a-b*x[i])/y_sigma[i]);
	
	return(chi2);
}

}  // end of namespace NUM

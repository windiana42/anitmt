/*
 * numerics/fit/svdfit_e.cc
 * 
 * Implementing least squares linear fit algorithm for unknown errors 
 * with SVD "solver" (singular value decomposition). 
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
#include <numerics/linalg/linalg.h>


namespace NUM
{

// Do least square fitting for function *basefunc which is linear 
// in its coefficients (appling SVDecomposition "solver"). 
// Calculate coeffs in base function to be match passed data 
// as well "as possible" (i.e. fit funtion to data). 
// Pass (measured) input points using arrays x[] and y[] of size ndata. 
// Stores coeffs b (arrays of size ncoeff). 
// Note: This method assumes that the fit is okay and that all errors 
//       of the input variables are the same. Use this method in case you 
//       cannot supply proper error values for the measured values and 
//       (try to) make sure (by other means) that the fit will be "okay". 
// The algorithm calculates the error of the fit and the parameters (see 
// note above) and returns the latter in and db[] (array [ncoeff] 
// holding the standard deviations) (Pass NULL if not interested.). 
// Return value: 
//  >=0 -> squared error of the fit, i.e. SQUARED standard deviation
//   -1 -> SVD bailed out (SVDecompose())
//   -2 -> error in SVDFwBackSubst(). 
double SVD_Fit_E(
	const double *x,const double *y,int ndata,
	double *b,double *db,int ncoeff,
	void (*basefunc)(double x,double *X,int ncoeff),
	double w_tolerance)
{
	// Set up linear system: 
	SMatrix<double> A(ndata,ncoeff);
	double X[ncoeff];
	
	for(int i=0; i<ndata; i++)
	{
		(*basefunc)(x[i],X,ncoeff);
		for(int j=0; j<ncoeff; j++)
		{  A[i][j]=X[j];  }
	}
	
	// M=ndata; N=ncoeff. 
	double W[ncoeff];
	SMatrix<double> V(ncoeff,ncoeff);
	int rv=SVDecompose(A,W,V);
	if(rv)
	{  assert(rv==1);  return(-1.0);  }  // SVD bailed out
	
	// NOTE A got replaced by U here. 
	
	#if 0
	{
		// Compute min/max w value: 
		double wmin=W[0],wmax=W[0];
		for(int i=1; i<ncoeff; i++)
			if(wmax<W[i])  wmax=W[i];
			else if(wmin>W[i])  wmin=W[i];
		printf("  wmax=%g; wmin=%g;  condition number: %g\n",wmax,wmin,
			wmax/wmin);
	}
	#endif
	
	if(w_tolerance>0.0)
	{
		// Ignore small singular values (the W's are all >=0): 
		double wmax=0.0;
		for(int i=0; i<ncoeff; i++)
			if(wmax<W[i])  wmax=W[i];
		double wlimit=w_tolerance*wmax;
		for(int i=0; i<ncoeff; i++)
			if(W[i]<wlimit)  W[i]=0.0;
	}
	
	rv=SVDFwBackSubst(A,W,V,y,b);
	if(rv) return(-2.0);
	
	// Calculate chi^2 in the usual way: 
	double chi2=0.0;
	for(int i=0; i<ndata; i++)
	{
		(*basefunc)(x[i],X,ncoeff);
		double sum=0.0;
		for(int j=0; j<ncoeff; j++)
			sum+=b[j]*X[j];
		chi2+=SQR(y[i]-sum);
	}
	// Assume that expected chi^2 value (ndata-ncoeff = 
	// data sets - degrees of freedom) equals real chi^2 value. 
	// This allows calculation of standard deviation ("error") 
	// which was previously unknown. 
	double sigma2=chi2/(ndata-ncoeff);
	
	// Compute standard deviations: 
	if(db) for(int j=0; j<ncoeff; j++)
	{
		double sum=0.0;
		for(int i=0; i<ncoeff; i++)
			if(W[i]!=0.0)
				sum+=SQR(V[j][i]/W[i]);
		db[j]=sqrt(sum*sigma2);
	}
	
	return(sigma2);
}

}  // end of namespace NUM

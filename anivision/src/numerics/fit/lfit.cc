/*
 * numerics/fit/lfit.cc
 * 
 * Implementing least squares linear fit algorithm for unknown errors 
 * with Gaussian "solver" (LU decompose). 
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
// in its coefficients (appling Gaussian LU decomposition "solver"). 
// Calculate coeffs in base function to be match passed data 
// as well "as possible" (i.e. fit funtion to data). 
// Pass (measured) input points using arrays x[] and y[] with standard 
// deviation of y in y_sigma[], all of size ndata. 
// Stores coeffs b (arrays of size ncoeff). 
// The algorithm calculates the error of the fit and the parameters (see 
// note above) and returns the latter in and db[] (array [ncoeff] 
// holding the standard deviations) (Pass NULL if not interested.). 
// Return value: chi^2 of the fit or negative values in case of error. 
double LFit(
	const double *x,const double *y,const double *y_sigma,int ndata,
	double *b,double *db,int ncoeff,
	void (*basefunc)(double x,double *X,int ncoeff))
{
	SMatrix<double> A(SMatrix<double>::NullMat,ncoeff,ncoeff);
	double X[ncoeff];
	for(int i=0; i<ncoeff; i++) b[i]=0.0;
	
	// Wieser-algorithm :)
	for(int i=0; i<ndata; i++)
	{
		// Evaluate function to fit: 
		(*basefunc)(x[i],X,ncoeff);
		double ssqri=1.0/SQR(y_sigma[i]);
		
		for(int j=0; j<ncoeff; j++)
		{
			for(int k=0; k<ncoeff; k++)  A[k][j]+=X[k]*X[j]*ssqri;
			b[j]+=y[i]*X[j]*ssqri;
		}
	}
	
	// Solve A*x=b storing the result in b again. 
	// Side effect: A gets LU decomposed (i.e. modified). 
	int p[A.R()-1];  // (permutation array)
	if(LUDecompose(A,p)) return(-1.0);
	if(LUFwBackSubst(A,p,b)) return(-1.0);
	
	// Calculate error using diag elems of inverse matrix of A. 
	// It comes handy that A is already LU decomposed...
	double ev[ncoeff];
	for(int ei=0; ei<ncoeff; ei++)
	{
		// FIXME: This could be made faster: 
		for(int i=0; i<ncoeff; i++) ev[i] = i==ei ? 1.0 : 0.0;
		if(LUFwBackSubst(A,p,ev)) return(-1.0);
		if(ev[ei]<0.0)  return(-2.0);
		db[ei]=sqrt(ev[ei]);
	}
	
	// Chi^2 minimisation done. 
	double chisqr=0.0;
	for(int i=0; i<ndata; i++)
	{
		(*basefunc)(x[i],X,ncoeff);
		double sum=0.0;
		for(int j=0; j<ncoeff; j++)  sum+=b[j]*X[j];
		chisqr+=SQR((y[i]-sum)/y_sigma[i]);
	}
	
	return(chisqr);
}

}  // end of namespace NUM

/*
 * numerics/fit/hhlfit.cc
 * 
 * Implementing least squares linear fit algorithm for unknown errors 
 * with Householder transformation. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de)
 * Based heavily on linfit.cc (c) 2004 by Andreas Holzner. 
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


// This is like LFit_E() but using Householder transformation which 
// is numerically more stable while not much slower. 
double HHLFit_E(
	const double *x,const double *y,int ndata,
	double *b,double *db,int ncoeff,
	void (*basefunc)(double x,double *X,int ncoeff))
{
	SMatrix<double> C(ndata,ncoeff);
	double t[ncoeff];
	
	// Evaluate function to fit: 
	for(int i=0; i<ndata; i++)
		(*basefunc)(x[i],C[i],ncoeff);
	
	// Apply Householder transformation on C
    // This is based on code in Schwarz, Numerische Mathematik, Kap. 7.2
	for(int l=0; l<ncoeff; l++)
	{
		double gamma=0.0;
		for(int i=l; i<ndata; i++)
			gamma += SQR(C[i][l]); 
		gamma = (C[l][l]<0.0) ? -sqrt(gamma) : sqrt(gamma);
		t[l] = -gamma;
		double beta = -sqrt( 2.0*gamma*(gamma+C[l][l]) );
		C[l][l] = (C[l][l]+gamma)/beta;
		for(int k=l+1; k<ndata; k++)
			C[k][l] /= beta;
		for(int j=l+1; j<ncoeff; j++)
		{
			double p=0.0;
			for(int k=l; k<ndata; k++)
				p += C[k][l]*C[k][j];
			p+=p;  // *2
			for(int i=l; i<ndata; i++)
				C[i][j] -= p*C[i][l];
		}
	}
	
	// Copy y[] values into d[] (because we need to modify them): 
	double *d=ALLOC<double>(ndata);
	for(int i=0; i<ndata; i++)
		d[i]=-y[i];
	// Apply Householder transformation on d=y: 
    for(int l=0; l<ncoeff; l++)
	{
		double p=0.0;
		for(int k=l; k<ndata; k++)
			p += C[k][l]*d[k];
		p+=p;  // *2
		for(int k=l; k<ndata; k++)
			d[k] -= p*C[k][l];
	}
	
	// Residuum vector is given by d[ncoeff..ndata-1]
	// calculate solution
	for(int i=ncoeff-1; i>=0; i--)
	{
		double p=d[i];
		for(int k=i+1; k<ncoeff; k++)
			p += C[i][k]*b[k];
		b[i] = -p/t[i];
	}
	
	// Compute errors: 
	// First inv(trans(C)*C)[j][j] = inv(R)*inv(trans(R))
	// This is based on Lawson, Hanson, Solving Least Squares Problems, 
	// chapter 12. 
	for(int i=0; i<ncoeff; i++)
		C[i][i] = 1.0/t[i];
	for(int i=0; i<ncoeff-1; i++) for(int j=i+1; j<ncoeff; j++)
	{
		double p=0.0;
		for(int l=i; l<j; l++)
			p += C[i][l]*C[l][j];
		C[i][j] = -p*C[j][j];
	}
	for(int i=0; i<ncoeff; i++)
	{
		double p=0.0;
		for(int l=i; l<ncoeff; l++)
			p += SQR(C[i][l]);
		t[i] = p;
	}
	// Compute standard deviation of fit parameters: 
	double p=0.0;
	for(int i=ncoeff; i<ndata; i++)
		p += SQR(d[i]);
	p /= (ndata-ncoeff);
	for(int i=0; i<ncoeff; i++)
		db[i] = sqrt(p*t[i]);
	
	// Calculate chi^2 in the usual way: 
	double chi2=0.0;
	double X[ncoeff];
	for(int i=0; i<ndata; i++)
	{
		(*basefunc)(x[i],X,ncoeff);
		double sum=0.0;
		for(int j=0; j<ncoeff; j++)
			sum+=b[j]*X[j];
		chi2+=SQR(y[i]-sum);
	}
	
	d=FREE(d);
	
	return(chi2);
}

}  // end of namespace NUM

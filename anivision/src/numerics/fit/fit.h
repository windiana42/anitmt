/*
 * numerics/fit/fit.h
 * 
 * Numerics library fitting functions. 
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

#ifndef _WW_NUMERICS_FIT_FIT_H_
#define _WW_NUMERICS_FIT_FIT_H_ 1

#include <numerics/la_basics.h>


namespace NUM  // numerics
{

// Perform linear fit (linear regression), i.e. fit passed data 
// (x,y) with specified y standard deviation against a straight line. 
// Return coefficients of y = a + b*x in *a and *b and their 
// respective standard deviations in *a_sigma,*b_sigma. 
// Return value: chi^2 of the fit or negative values in case of error.
extern double LinearFit(
	const double *x,const double *y,const double *y_sigma,int ndata,
	double *_a,double *a_sigma,
	double *_b,double *b_sigma);


// Do least square fitting for function *basefunc which is linear 
// in its coefficients (appling Gaussian LU decomposition "solver"). 
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
// Return value: Squared error of the fit, i.e. SQUARED standard 
//               deviation or negative in case of error. 
extern double LFit_E(
	const double *x,const double *y,int ndata,
	double *b,double *db,int ncoeff,
	void (*basefunc)(double x,double *X,int ncoeff));


// This is like LFit_E() but using Householder transformation which 
// is numerically more stable while not much slower. 
extern double HHLFit_E(
	const double *x,const double *y,int ndata,
	double *b,double *db,int ncoeff,
	void (*basefunc)(double x,double *X,int ncoeff));

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
extern double SVD_Fit_E(
	const double *x,const double *y,int ndata,
	double *b,double *db,int ncoeff,
	void (*basefunc)(double x,double *X,int ncoeff),
	double w_tolerance);


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
extern double LFit(
	const double *x,const double *y,const double *y_sigma,int ndata,
	double *b,double *db,int ncoeff,
	void (*basefunc)(double x,double *X,int ncoeff));

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_FIT_FIT_H_ */

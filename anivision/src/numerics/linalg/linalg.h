/*
 * numerics/linalg/linalg.h
 * 
 * Numerics library linear algebra. 
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

#ifndef _WW_NUMERICS_LINALG_LINALG_H_
#define _WW_NUMERICS_LINALG_LINALG_H_ 1

#include <numerics/la_basics.h>


namespace NUM  // numerics
{

// See "ludcmp.cc" for details: 
extern int LUDecompose(SMatrix<double> &a,int *p,double *_det=NULL);
extern int LUDecompose(SMatrix<float> &a,int *p,float *_det=NULL);

extern int LUFwBackSubst(const SMatrix<double> &a,const int *p,double *b);
extern int LUFwBackSubst(const SMatrix<float> &a,const int *p,float *b);

extern int LUInvert(SMatrix<double> &res,const SMatrix<double> &a,const int *p);
extern int LUInvert(SMatrix<float> &res,const SMatrix<float> &a,const int *p);

extern int LUSolve(const SMatrix<double> &a,double *x,const double *b);
extern int LUSolve(const SMatrix<float> &a,float *x,const float *b);


// See "choleskydcmp.cc" for details: 
extern int CholeskyDecompose(SMatrix<double> &m);
extern int CholeskyDecompose(SMatrix<float> &m);

extern int CholeskyFwBackSubst(const SMatrix<double> &m,double *b);
extern int CholeskyFwBackSubst(const SMatrix<float> &m,float *b);


// Special cholesky decompostion for diagonally dominant systems. 
// See "choleskyddom.cc" for details. 
extern int CholeskySolveDiagDominant(const double *a,const double *b,
	double c,int n,double *x,const double *d);
extern int CholeskySolveDiagDominant(const float *a,const float *b,
	float c,int n,float *x,const float *d);


// Standard methods for solving and evaluating tri-diagonal systems: 
// See "tridiag.cc" for details. 
extern void SolveTridiag(int n,const double *o,double *d,const double *u,
	double *y,double *x);
extern void SolveTridiag(int n,const float *o,float *d,const float *u,
	float *y,float *x);

extern void SolveTridiag_TDMA(int n,const double *o,double *d,const double *u,
	double *y,double *x);
extern void SolveTridiag_TDMA(int n,const float *o,float *d,const float *u,
	float *y,float *x);

extern void SolveTridiag_M1_TDMA(int n,double *d,double *y,double *x);
extern void SolveTridiag_M1_TDMA(int n,float *d,float *y,float *x);

extern void EvalTridiag(int n,const double *o,const double *d,const double *u,
	const double *x,double *y);
extern void EvalTridiag(int n,const float *o,const float *d,const float *u,
	const float *x,float *y);


// See "svdcmp.cc" for details: 
extern int SVDecompose(SMatrix<double> &a,double *w,
	SMatrix<double> &v,int maxiter=30);
extern int SVDecompose(SMatrix<float> &a,float *w,
	SMatrix<float> &v,int maxiter=30);

extern int SVDFwBackSubst(const SMatrix<double> &u,const double *w,
	const SMatrix<double> &v,const double *b,double *x);
extern int SVDFwBackSubst(const SMatrix<float> &u,const float *w,
	const SMatrix<float> &v,const float *b,float *x);

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_LINALG_LINALG_H_ */

/*
 * numerics/linalg/ludcmp.cc
 * 
 * LU decomposition, forward/back substitution. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de)
 * 
 * Based heavily on sample code lu.cpp which is a more or less direct 
 * translation of lu.f90...
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */ 

#include "linalg.h"

namespace NUM
{

// LU decomposition of matrix a (must be N x N square matrix). 
// LU decomposed matrix replaces passed matrix a. 
// Determinant returned in *_det if non-NULL. 
// Permutation array stored in p[] (size N-1). 
// Runtime: O(N^3)
// Return value: 
//   0 -> OK
//  -1 -> singular matrix
//  -2 -> non-square matrix (or N<1)
template<typename T>static inline
	int _LUDecompose(SMatrix<T> &a,int *p,T *_det)
{
	// LU decomposition with relative column maximum strategy. 
	
	// Input matrix a will be replaced by LU decomposit matrix on output 
	// p will store index matrix (book-keeping for row exchanges) 
	// det will store the determinat of matrix 
	
	if(a.R()!=a.C() || a.R()<1)
	{  return(-2);  }
	int np=a.R();
	
	T det=1.0;
	
	for(int k=0; k<np-1; k++)
	{
		T max=0.0;
		p[k]=-1;
		for(int i=k; i<np; i++)
		{
			T s=0.0;
			for(int j=k; j<np; j++)  s+=fabs(a[i][j]);
			T q=fabs(a[i][k])/s;
			if(q>max)
			{  max=q;  p[k]=i;  }
		}
		
		if(max==0.0 /* p[k]<0*/)
		{  return(-1);  }
		
		if(p[k]!=k)
		{
			det=-det;
			// Swap lines: 
			for(int j=0,pk=p[k]; j<np; j++)
			{
				T h=a[k][j];
				a[k][j]=a[pk][j];
				a[pk][j]=h;
			}
		}
		
		det*=a[k][k];
		for(int i=k+1; i<np; i++)
		{
			T fac=a[i][k]/a[k][k];
			a[i][k]=fac;
			for(int j=k+1; j<np; j++)  a[i][j]-=fac*a[k][j];
		}
	}
	
	det*=a[np-1][np-1];
	
	if(_det) *_det=det;
	
	return(0);
}

int LUDecompose(SMatrix<double> &a,int *p,double *_det)
{  return(_LUDecompose(a,p,_det));  }
int LUDecompose(SMatrix<float> &a,int *p,float *_det)
{  return(_LUDecompose(a,p,_det));  }


// Forward/back substitution. 
// Pass an LU decomposed N x N matrix (a) and the permutation 
// array as returned by the LU decomposition (p). 
// The function will solve the equation A*x=b and store 
// the result for x in b (where b is a array (vector) of size N). 
// Runtime: O(N^2)
// Return value: 
//   0 -> OK
//  -2 -> matrix not square
template<typename T>static inline
	int _LUFwBackSubst(const SMatrix<T> &a,const int *p,T *b)
{
	// forward/backsubstitution to obtain solution A*x=b, with;
	
	// LU decomposit matrix a;
	// index matrix p;
	// constant vector b, which will be replaced by solution x on output;
	
	if(a.R()!=a.C())
	{  return(-2);  }
	int np=a.R();
	
	// Swap "lines"...
	for(int k=0; k<np-1; k++)
	{
		if(p[k]==k)  continue;
		T h=b[k];
		b[k]=b[p[k]];
		b[p[k]]=h;
	}
	
	// L matrix: 
	for(int i=1; i<np; i++)
		for(int j=0; j<i; j++)
			b[i]-=a[i][j]*b[j];
	
	// U matrix: 
	for(int i=np-1; i>=0; i--)
	{
		T h=b[i];
		for(int k=i+1; k<np; k++)  h-=a[i][k]*b[k];
		b[i]=h/a[i][i];
	}
	
	return(0);
}

int LUFwBackSubst(const SMatrix<double> &a,const int *p,double *b)
{  return(_LUFwBackSubst(a,p,b));  }
int LUFwBackSubst(const SMatrix<float> &a,const int *p,float *b)
{  return(_LUFwBackSubst(a,p,b));  }


// Invert an LU-decomposed matrix. 
// Pass an LU decomposed N x N matrix (a) and the permutation 
// array as returned by the LU decomposition (p). 
// Resulting matrix is stored in res. 
// Time: O(N^3)
// Return value: 
//   0 -> OK
//  -2 -> matrix not square or size mismatch
template<typename T>static inline
	int _LUInvert(SMatrix<T> &res,const SMatrix<T> &a,const int *p)
{
	if(a.R()!=a.C() || a.R()!=res.C() || a.R()!=res.R())
	{  return(-2);  }
	int np=a.R();
	
	T b[np];  // <-- ersatzweise: ALLOC<T>() .. FREE()
	for(int ei=0; ei<np; ei++)
	{
		// Compute ei-th column in result matrix. 
		
		// Set up result vector: 
		for(int i=0; i<np; i++) b[i]=i==ei ? 1.0 : 0.0;
		
		LUFwBackSubst(a,p,b);
		
		// Store matrix column: 
		for(int i=0; i<np; i++) res[i][ei]=b[i];
	}
	
	return(0);
}

int LUInvert(SMatrix<double> &res,const SMatrix<double> &a,const int *p)
{  return(_LUInvert(res,a,p));  }
int LUInvert(SMatrix<float> &res,const SMatrix<float> &a,const int *p)
{  return(_LUInvert(res,a,p));  }


// Just for convenience; there is probably little point in 
// using this function because you normally won't LU decompose 
// just to solve one single equation. 
//--
// Solve A*x=b using LU decomposition; 
// A,b not modified, result stored in x. 
// Returns 0 on success. 
template<typename T>static inline
	int _LUSolve(const SMatrix<T> &a,T *x,const T *b)
{
	// Copy matrix because LUDecompose() will modify it: 
	SMatrix<T> mat(a);
	int p[a.R()-1];
	int rv=LUDecompose(mat,p);
	if(rv) return(rv);
	
	// LU decomposition done; proceed with fw/back subst. 
	// First copy solution to x which is then modified by 
	// LUFwBackSubst(). 
	for(int i=0; i<a.C(); i++)  x[i]=b[i];
	rv=LUFwBackSubst(mat,p,x);
	return(rv);
}

int LUSolve(const SMatrix<double> &a,double *x,const double *b)
{  return(LUSolve(a,x,b));  }
int LUSolve(const SMatrix<float> &a,float *x,const float *b)
{  return(LUSolve(a,x,b));  }

}  // end of namespace NUM

/*
 * numerics/linalg/choleskydcmp.cc
 * 
 * Cholesky decomposition and associated forward/back substitution. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de)
 * 
 * Based heavily on sample code in quad_matrix.cpp from numerics course 
 * at the LMU. 
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

// Cholesky decomposition of matrix m. 
// Matrix must be symmetric and positively definite. 
// Passed matrix will be decomposed (and hence modified). 
// Return value: 
//   0 -> OK
//  -1 -> unsuitable matrix (NOTE: NOT getting retval -1 will NOT 
//        necessarily mean that the matrix is suitable; make sure to 
//        pass only positively definite symmetric matrices)
//  -2 -> non-square matrix (or N<1)
template<typename T>static inline
	int _CholeskyDecompose(SMatrix<T> &m)
{
	if(m.R()!=m.C() || m.R()<1)  return(-2);
	int n=m.R();
	
	// First, need a copy of the matrix: 
	SMatrix<T> l(m.R(),m.C());
	
	for(int k=0; k<n; k++)
	{
		if(m[k][k]<=0.0)  return(-1);
		T lkk=sqrt(m[k][k]);
		l[k][k]=lkk;
		lkk=1.0/lkk;
		for(int i=k+1; i<n; i++)
		{
			l[i][k]=m[i][k]*lkk;
			for(int j=k+1; j<=i; j++)
			{  m[i][j]-=l[i][k]*l[j][k];  }
		}
	}
	
	// "Mirror..."
	for(int i=0; i<n; i++) for(int j=0; j<=i; j++)
		m[i][j]=l[i][j];
	
	return(0);
}

int CholeskyDecompose(SMatrix<double> &m)
	{  return(_CholeskyDecompose(m));  }
int CholeskyDecompose(SMatrix<float> &m)
	{  return(_CholeskyDecompose(m));  }


// Cholesky forward/back substitution to solve m*a=b. 
// The result vector b will be updated to store the result a. 
// Pass a Cholesky decomposed matrix as matrix m 
// (see _CholeskyDecompose()). 
// Return value: 
//   0 -> OK
//  -2 -> non-square matrix
template<typename T>static inline
	int _CholeskyFwBackSubst(const SMatrix<T> &m,T *b)
{
	if(m.R()!=m.C())  return(-2);
	int n=m.R();
	
	// Forward substitution: 
	for(int i=0; i<n; i++)
	{
		T s=b[i];
		for(int j=0; j<i; j++)  s-=m[i][j]*b[j];
		b[i]=s/m[i][i];
	}
	
	// Backward substitution: 
	for(int i=n-1; i>=0; i--)
	{
		T s=b[i];
		for(int j=i+1; j<n; j++)  s-=m[j][i]*b[j];
		b[i]=s/m[i][i];
	}
	
	return(0);
}

int CholeskyFwBackSubst(const SMatrix<double> &m,double *b)
	{  return(_CholeskyFwBackSubst(m,b));  }
int CholeskyFwBackSubst(const SMatrix<float> &m,float *b)
	{  return(_CholeskyFwBackSubst(m,b));  }

}  // end of namespace NUM

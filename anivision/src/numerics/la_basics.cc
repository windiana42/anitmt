/*
 * numerics/la_basics.cc
 * 
 * Linear algebra basics...
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de)
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */ 

#include "la_basics.h"

namespace NUM
{

// Matrix multiplication. 
// Time: O(N^3)
// Return value: 
//   0 -> OK
//  -2 -> matrix size mismatch
template<typename T>static inline
	int _MatMult(SMatrix<T> &res,
		const SMatrix<T> &a1,const SMatrix<T> &a2)
{
	if(a1.C()!=a2.R() || res.R()!=a1.R() || res.C()!=a2.C())
		return(-2);
	
	for(int i=0; i<res.R(); i++)
		for(int j=0; j<res.C(); j++)
		{
			T h=0.0;
			for(int k=0; k<a1.C(); k++)  h+=a1[i][k]*a2[k][j];
			res[i][j]=h;
		}
	
	return(0);
}

int MatMult(SMatrix<double> &res,
	const SMatrix<double> &a1,const SMatrix<double> &a2)
{  return(_MatMult(res,a1,a2));  }
int MatMult(SMatrix<float> &res,
	const SMatrix<float> &a1,const SMatrix<float> &a2)
{  return(_MatMult(res,a1,a2));  }


// Multiplication matrix * diagonal matrix. 
// Time: O(N^2)
// Return value: 
//   0 -> OK
//  -2 -> matrix size mismatch
template<typename T>static inline
	int _MatMultMD(SMatrix<T> &res,
		const SMatrix<T> &a,const T *diag)
{
	if(a.R()!=a.C() || res.R()!=a.R() || res.C()!=a.C())
		return(-2);
	
	for(int i=0; i<res.R(); i++)
		for(int j=0; j<res.C(); j++)
			res[i][j]=a[i][j]*diag[j];
	
	return(0);
}

int MatMultMD(SMatrix<double> &res,
	const SMatrix<double> &a,const double *diag)
{  return(_MatMultMD(res,a,diag));  }
int MatMultMD(SMatrix<float> &res,
	const SMatrix<float> &a,const float *diag)
{  return(_MatMultMD(res,a,diag));  }


// Multiplication matrix * vector. 
// Time: O(N^2)
// Return value: 
//   0 -> OK
template<typename T>static inline
	int _MatMultMV(T *res,const SMatrix<T> &a,const T *vec)
{
	for(int i=0; i<a.R(); i++)
	{
		T h=0.0;
		for(int j=0; j<a.C(); j++)
			h+=a[i][j]*vec[j];
		res[i]=h;
	}
	
	return(0);
}

int MatMultMV(double *res,const SMatrix<double> &a,const double *vec)
{  return(_MatMultMV(res,a,vec));  }
int MatMultMV(float *res,const SMatrix<float> &a,const float *vec)
{  return(_MatMultMV(res,a,vec));  }

}  // end of namespace NUM

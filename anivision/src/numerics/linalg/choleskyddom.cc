/*
 * numerics/linalg/choleskyddom.cc
 * 
 * Solving diagonally dominant systems using Cholesky decomposition. 
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

#include "linalg.h"

namespace NUM
{

// Compute solution for equation 
//   A * x = d
// Where A is specified using a[], b[] and c as described below. 
// You may pass x=d to get in-place solution (d[] overwritten with 
// solution vector). 
// 
//   / a0  b0                c   \ .
//   | b0  a1  b1                |
//   |     b1  a2  b2            |
//   |     b2  a3  ...           |  =: A
//   |         ... ...           |
//   |                    b[n-2] |
//   \ c           b[n-2] a[n-1] /
//
// Make sure the input is suitable for decomposition, i.e. 
// positively definite, otherwise the sqrt() of a negative 
// number occurs and the result is all NaN. 
// NOTE that n>=3 required for this algorithm. 
// Return value is always 0 currently. 
template<typename T>static inline
	int _CholeskySolveDiagDominant(const T *a,const T *b,T c,int n,
	T *x,const T *d)
{
	// Perform Cholesky decomposition on the following matrix: 
	// 
	//   / a0  b0                c   \ .
	//   | b0  a1  b1                |
	//   |     b1  a2  b2            |
	//   |     b2  a3  ...           |  =: A
	//   |         ... ...           |
	//   |                    b[n-2] |
	//   \ c           b[n-2] a[n-1] /
	//
	// Result: 
	// 
	//   / l0                            \ .
	//   | m0  l1                        |
	//   |     m1  l2                    |
	//   |         m2  l3                |  =: L
	//   |             ... ...           |
	//   |                               |
	//   \ e0  e1  e2..    m[n-2] l[n-1] /
	// 
	// where L * L^T = A. 
	
	// This algorithm only works for n>=3. 
	assert(n>=3);
	
	T l[n];
	T m[n-1];
	T e[n-2];
	
	l[0]=sqrt(a[0]);
	e[0]=c/l[0];
	T s=0.0;
	int i=0;
	for(; i<n-2; i++)
	{
		m[i]=b[i]/l[i];
		if(i) e[i]=-e[i-1]*m[i-1]/l[i];
		l[i+1]=sqrt(a[i+1]-SQR(m[i]));
		s+=SQR(e[i]);
	}
	m[i]=(b[i]-e[n-3]*m[n-3])/l[i];
	++i;
	l[i]=sqrt(a[i]-SQR(m[n-2])-s);
	
	// Decomposition done. 
	
	// Forward substitution: 
	T y[n];
	y[0]=-d[0]/l[0];
	s=0.0;
	for(i=1; i<n-1; i++)
	{
		y[i]=-(d[i]+m[i-1]*y[i-1])/l[i];
		s+=e[i-1]*y[i-1];
	}
	y[i]=-(d[i]+m[n-2]*y[n-2]+s)/l[i];
	
	// Backsubsitiution: 
	i=n-1;  // ...which should already be the case. 
	x[i]=-y[i]/l[i];
	s=x[i--];
	x[i]=-(y[i]+m[i]*s)/l[i];
	for(--i; i>=0; i--)
	{  x[i]=-(y[i]+m[i]*x[i+1]+e[i]*s)/l[i];  }
	
	// Done. Result now in x[]. 
	
	return(0);
}


int CholeskySolveDiagDominant(const double *a,const double *b,
	double c,int n,double *x,const double *d)
	{  return(_CholeskySolveDiagDominant(a,b,c,n,x,d));  }
int CholeskySolveDiagDominant(const float *a,const float *b,
	float c,int n,float *x,const float *d)
	{  return(_CholeskySolveDiagDominant(a,b,c,n,x,d));  }


#if 0
int main()
{
	const int n=4;
	double a[n]={12,4,6,7};
	double b[n-1]={4,-1,2};
	double c=5;
	double d[n]={19,5,9,7};
	
	CholeskySolveDiagDominant(a,b,c,n,d,d);
	for(int i=0; i<n; i++)
	{  printf("  %g\n",d[i]);  }
	
	// Calculate result r=A*d (with solved "d" which is actually "x"): 
	double r[n];
	r[0]=a[0]*d[0]+b[0]*d[1]+c*d[n-1];
	for(int i=1; i<n-1; i++)
	{  r[i] = b[i-1]*d[i-1]+a[i]*d[i]+b[i]*d[i+1];  }
	r[n-1]=c*d[0]+b[n-2]*d[n-2]+a[n-1]*d[n-1];
	
	printf("\n");
	
	for(int i=0; i<n; i++)
	{  printf("  %g\n",r[i]);  }
	
	return(0);
}
#endif

}  // end of namespace NUM

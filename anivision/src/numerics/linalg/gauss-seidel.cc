/*
 * gauss-seidel.cc
 * 
 * Gauss-Seidel iterative algorithm: Iterative sover for sparse matrix 
 * equations. 
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
#include "gauss-seidel.h"

namespace NUM
{

// n:     number of rows (and hence the number of unknowns). 
// eps:   stop iterations if the max. change of the x values in one 
//        iteration is smaller than eps. Use eps<0 to enfoce the 
//        specified number of iterations. 
// maxiter: max number of iterations to do. Set 0 for unlimited. 
//        Iterations will be alternating top-down and down-up. 
//        First iteration is top-down if maxiter>0 and down-up if 
//        maxiter<0. (In this case the iteration limit is -maxiter). 
// omega: overrelaxation factor. Good values are in range 1..2. 
//        use -1 (something <0) to disable overrelaxation. 
// Return value:
//   Max difference between old and new x value in last iteration. 
//   In case retval<eps, it converged, othterwise it was bailout due 
//   to iteration limit. 
template<typename T>T _GaussSeidelIterate(int n,const GaussSeidelRow<T> *row,
	T eps,int maxiter,T omega)
{
	bool overrelax=(omega>=0.0);
	double omega1=1.0-omega;
	
	double maxdiff;
	int idir=+1;
	if(maxiter<0)
	{  idir=-1;  maxiter=-maxiter;  }
	for(int iter=0; !maxiter || iter<maxiter; iter++)
	{
		maxdiff=0.0;
		
		// Run through the rows up-to-down and down-to-up: 
		for(int i=idir<0 ? n-1 : 0,
		     iend=idir<0 ? -1 : n;
			i!=iend; i+=idir)
		{
			const GaussSeidelRow<T> *r=row+i;
			T sum=0.0;
			for(int j=1/*correct*/; j<r->nent; j++)
				sum+=r->A[j]*(*r->x[j]);
			sum=(r->y-sum)/r->A[0];
			
			if(overrelax)
			{
				double oldx = **r->x;
				**r->x = sum*omega + omega1*oldx;
				
				sum=fabs(**r->x-oldx);
				if(maxdiff<sum)
				   maxdiff=sum;
			}
			else
			{
				double diff=fabs(**r->x-sum);
				**r->x = sum;
				
				if(maxdiff<diff)
				   maxdiff=diff;
			}
		}
		
		// Bailout test (converged?): 
		if(maxdiff<eps)  break;
		
		idir=-idir;
	}
	
	return(maxdiff);
}


double GaussSeidelIterate(int n,const GaussSeidelRow<double> *row,
	double eps,int maxiter,double omega)
	{  return(_GaussSeidelIterate(n,row,eps,maxiter,omega));  }
float GaussSeidelIterate(int n,const GaussSeidelRow<float> *row,
	float eps,int maxiter,float omega)
	{  return(_GaussSeidelIterate(n,row,eps,maxiter,omega));  }


// Special version of the above function if all non-zero non-diag elements 
// of the matrix are -1.0. 
template<typename T>T _GaussSeidelIterate_M1(int n,
	const GaussSeidelRow_M1<T> *row,T eps,int maxiter,T omega)
{
	bool overrelax=(omega>=0.0);
	double omega1=1.0-omega;
	
	double maxdiff;
	int idir=+1;
	if(maxiter<0)
	{  idir=-1;  maxiter=-maxiter;  }
	for(int iter=0; !maxiter || iter<maxiter; iter++)
	{
		maxdiff=0.0;
		
		// Run through the s up-to-down and down-to-up: 
		for(int i=idir<0 ? n-1 : 0,
		     iend=idir<0 ? -1 : n;
			i!=iend; i+=idir)
		{
			const GaussSeidelRow_M1<T> *r=row+i;
			T sum=0.0;
			for(int j=1/*correct*/; j<r->nent; j++)
				sum+=*r->x[j];
			sum=(r->y+sum)/r->A0;
			
			if(overrelax)
			{
				double oldx = **r->x;
				**r->x = sum*omega + omega1*oldx;
				
				sum=fabs(**r->x-oldx);
				if(maxdiff<sum)
				   maxdiff=sum;
			}
			else
			{
				double diff=fabs(**r->x-sum);
				**r->x = sum;
				
				if(maxdiff<diff)
				   maxdiff=diff;
			}
		}
		
		// Bailout test (converged?): 
		if(maxdiff<eps)  break;
		
		idir=-idir;
	}
	
	return(maxdiff);
}


double GaussSeidelIterate_M1(int n,const GaussSeidelRow_M1<double> *row,
	double eps,int maxiter,double omega)
	{  return(_GaussSeidelIterate_M1(n,row,eps,maxiter,omega));  }
float GaussSeidelIterate_M1(int n,const GaussSeidelRow_M1<float> *row,
	float eps,int maxiter,float omega)
	{  return(_GaussSeidelIterate_M1(n,row,eps,maxiter,omega));  }

}  // end of namespace NUM

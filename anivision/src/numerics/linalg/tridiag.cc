/*
 * numerics/linalg/tridiag.cc
 * 
 * Solving tridiagonal equation systems. 
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

// All these routines accept the tridiagonal matrix equation in the 
// following form: 
// 
//  / d[0]  o[0]                           \   / x[0] \   / y[0] \
//  | u[0]  d[1]  o[1]                     |   | x[1] |   | y[1] |
//  |       u[1]  d[2]  o[2]               |   | x[2] |   | y[2] |
//  |             u[2] ...                 | * |  .   | = |  .   |
//  |                  ........     o[n-1] |   |  .   |   |  .   |
//  \                       u[n-1]   d[n]  /   \ x[n] /   \ y[n] /
// 
// ...and we search the solution for x[0..n] where d[0..n], o[0..n-1], 
// u[0..n-1] and y[0..n] are known. 
// 

// Standard method for solving tri-diagonal systems using one forward 
// and one backward substitution step. Runtime is O(n). 
// Cannot be used for d[0]==0 (and there is no way to solve such a 
// system easily since adding other lines would destroy the tridiagonal 
// form). 
// CHANGES: d[], y[] and x[] of course. 
// NOTE: - In case o[i]=u[i], you can pass o=u (i.e. the same pointers).  
//         because these values are not internally changed by the algorithm. 
//       - In case you wish to get the system solved "in-situ", you may 
//         pass x=y and the solution will overwrite the old values in y[]. 
//       - Can savely be used for n=0. 
template<typename T>static inline
	void _SolveTridiag(int n,const T *o,T *d,const T *u,T *y,T *x)
{
	// Forward elimination: 
	for(int i=0; i<n; i++)
	{
		T tmp=u[i]/d[i];
		d[i+1]-=o[i]*tmp;
		y[i+1]-=y[i]*tmp;
	}
	// Backward substitution: 
	x[n]=y[n]/d[n];
	for(int i=n-1; i>=0; i--)
		x[i]=(y[i]-o[i]*x[i+1])/d[i];
}

void SolveTridiag(int n,const double *o,double *d,const double *u,
	double *y,double *x)
	{  _SolveTridiag(n,o,d,u,y,x);  }
void SolveTridiag(int n,const float *o,float *d,const float *u,
	float *y,float *x)
	{  _SolveTridiag(n,o,d,u,y,x);  }


// This implements the TDMA algorithm (Thomas tri-diagonal matrix algorithm) 
// for solution of the tridiagonal system. The functionality is essentially 
// the same as for _SolveTridiag() above but this version is somewhat 
// faster (n FPU divisions less) while seeming to be equally stable. 
// CHANGES: d[], y[] and x[] of course. 
// NOTE: - In case o[i]=u[i], you can pass o=u (i.e. the same pointers). 
//         because these values are not internally changed by the algorithm. 
//       - In case you wish to get the system solved "in-situ", you may 
//         pass x=y and the solution will overwrite the old values in y[]. 
//       - Can savely be used for n=0. 
template<typename T>static inline
	void _SolveTridiag_TDMA(int n,const T *o,T *d,const T *u,T *y,T *x)
{
	if(n>=1)
	{
		// Forward elimination: 
		double tmp=1.0/d[0];
		d[0]=-o[0]*tmp;
		y[0]*=tmp;
		for(int i=1; ; i++)
		{
			tmp=-1.0/(u[i-1]*d[i-1]+d[i]);
			y[i]=(u[i-1]*y[i-1]-y[i])*tmp;
			if(i>=n)  break;
			d[i]=o[i]*tmp;
		}
		// Backward substitution: 
		x[n]=y[n];
		for(int i=n-1; i>=0; i--)
			x[i]=d[i]*x[i+1]+y[i];
	}
	else if(!n)
		x[n]=y[n]/d[n];
	
	// The above is actually a modified TDMA which uses d[] to store 
	// the intermediate values. This one modifies o[] instead of d[] 
	// and is otherwise equal. 
	#if 0
		// Forward elimination: 
		double tmp=1.0/d[0];
		o[0]*=-tmp;
		y[0]*=tmp;
		for(int i=1; ; i++)
		{
			tmp=-1.0/(u[i-1]*o[i-1]+d[i]);
			y[i]=(u[i-1]*y[i-1]-y[i])*tmp;
			if(i>=n)  break;
			o[i]*=tmp;
		}
		// Backward substitution: 
		x[n]=y[n];
		for(int i=n-1; i>=0; i--)
			x[i]=o[i]*x[i+1]+y[i];
	#endif
}

void SolveTridiag_TDMA(int n,const double *o,double *d,const double *u,
	double *y,double *x)
	{  _SolveTridiag_TDMA(n,o,d,u,y,x);  }
void SolveTridiag_TDMA(int n,const float *o,float *d,const float *u,
	float *y,float *x)
	{  _SolveTridiag_TDMA(n,o,d,u,y,x);  }


// This is a special version of the _SolveTridiag_TDMA() if o[i]=u[i]=-1 
// for all i. This is a type of system which can appear in iterative 
// PDE solving... Otherwise identical to the _SolveTridiag_TDMA(). 
template<typename T>static inline
	void _SolveTridiag_M1_TDMA(int n,T *d,T *y,T *x)
{
	if(n>=1)
	{
		// Forward elimination: 
		double tmp=1.0/d[0];
		d[0]=tmp;
		y[0]*=tmp;
		for(int i=1; ; i++)
		{
			tmp=1.0/(d[i]-d[i-1]);
			y[i]=(y[i-1]+y[i])*tmp;
			if(i>=n)  break;
			d[i]=tmp;
		}
		// Backward substitution: 
		x[n]=y[n];
		for(int i=n-1; i>=0; i--)
			x[i]=d[i]*x[i+1]+y[i];
	}
	else if(!n)
		x[n]=y[n]/d[n];
}

void SolveTridiag_M1_TDMA(int n,double *d,double *y,double *x)
	{  _SolveTridiag_M1_TDMA(n,d,y,x);  }
void SolveTridiag_M1_TDMA(int n,float *d,float *y,float *x)
	{  _SolveTridiag_M1_TDMA(n,d,y,x);  }


// Evaluate tridiagonal system, i.e. compute y = A * x with A and x 
// known. Of course, this is O(n). 
// NOTE: - Of course, you can use o=u if o[i]=u[i] for all i. 
//       - You CANNOT use x=y; the result for y cannot overwrite the x values. 
//       - Can be savely used for n=0 and 1. 
template<typename T>static inline
	void _EvalTridiag(int n,const T *o,const T *d,const T *u,const T *x,T *y)
{
	if(n>=1)
	{
		y[0] = d[0]*x[0] + o[0]*x[1];
		--n;
		for(int i=0; i<n; )
		{
			double tmp=u[i]*x[i];
			++i;
			y[i] = tmp + d[i]*x[i] + o[i]*x[i+1];
		}
		double tmp=u[n]*x[n];
		++n;
		y[n] = tmp + d[n]*x[n];
	}
	else if(n==0)
	{  y[0] = d[0]*x[0];  }
}

void EvalTridiag(int n,const double *o,const double *d,const double *u,
	const double *x,double *y)
	{  _EvalTridiag(n,o,d,u,x,y);  }
void EvalTridiag(int n,const float *o,const float *d,const float *u,
	const float *x,float *y)
	{  _EvalTridiag(n,o,d,u,x,y);  }

}  // end of namespace NUM


#if 0
#include <stdlib.h>

// Small test program. 
int main()
{
	for(int iter=0; iter<10; iter++)
	{
		int n=30;
		double o[n],u[n],d[n+1],y[n+1];
		double _o[n],_u[n],_d[n+1],_y[n+1];
		double x[n+1];
		
		// Generate a tridiag system: 
		for(int i=0; ; i++)
		{
			_d[i]=d[i]=double(rand())/RAND_MAX * 100.0;
			_y[i]=y[i]=double(rand())/RAND_MAX * 100.0;
			x[i]=y[i];
			if(i>=n)  break;
			_o[i]=o[i]=double(rand())/RAND_MAX * 100.0;
			_u[i]=u[i]=o[i]; //double(rand())/RAND_MAX * 100.0 / 100.0;
		}
		
		// Solve the system: 
		#if 0
		for(int j=0; j<100000; j++)
		{
			for(int i=0; ; i++)
			{
				d[i]=_d[i];
				y[i]=_y[i];
				if(i>=n)  break;
				o[i]=_o[i];
				u[i]=_u[i];
			}
			
			NUM::SolveTridiag_TDMA(n,o,d,u,y,x);
		}
		#else
		NUM::SolveTridiag_TDMA(n,o,d,o,y,x);
		#endif
		
		// Test-substitute: 
		NUM::EvalTridiag(n,_o,_d,_u,x,y);
		
		// Check result: 
		double sqerr=0.0;
		for(int i=0; i<=n; i++)
		{
			double d=y[i]-_y[i];
			//fprintf(stderr,"  %g %g %g (%g)\n",x[i],y[i],_y[i],d);
			sqerr+=d*d;
		}
		fprintf(stderr,"Error=%g\n",sqrt(sqerr));
	}
	
	return(0);
}
#endif

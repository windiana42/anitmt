/*
 * numerics/spline/aspline.cc
 * 
 * Numerics library Akima spline. 
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

#include "aspline.h"

namespace NUM
{

const SplineBase::Properties *AkimaSpline::SplineProperties() const
{
	static const Properties prop=
	{
		splinetype: ST_ASpline,
		name: "aspline",
		cont_order: 1 // Not 2. 
	};
	return(&prop);
}


void AkimaSpline::Clear()
{
	SplineBase::Clear();
	
	valid_idx=-1;
	B=FREE(B);
	C=D=NULL;
}


int AkimaSpline::_ComputeCoeffs(double *t)
{
	// Compute the index of the polynomial: 
	int retval;
	int idx=_EvalFindIdx(*t,&retval);
	
	// Normalize t: 
	*t-=get_t(idx);
	
	// See if we can answer from cache: 
	if(valid_idx==idx)  return(retval);
	valid_idx=idx;
	
	if(n<=1)
	{
		assert(n==1);
		
		// Perform linear interpolation. 
		if(tv)
		{
			double h=tv[1]-tv[0];
			for(int j=0; j<dim; j++)
			{
				B[j] = (xv(1)[j]-xv(0)[j]) / h;
				C[j]=0.0;
				D[j]=0.0;
			}
		}
		else for(int j=0; j<dim; j++)
		{
			B[j] = xv(1)[j]-xv(0)[j];
			C[j]=0.0;
			D[j]=0.0;
		}
		
		return(retval);
	}
	
	// Compute the coefficients: 
	const int kpermA[]={ 0,1,2,3,-1,-2};
	const int mcalcA[]={10,0,1,2,-1,-2};
	const int kpermB[]={ 0,1,-1,-2,2,3};
	const int mcalcB[]={10,0,-1,-2,1,2};
	const int *kperm,*mcalc;
	if(idx+1>=n)  {  kperm=kpermB;  mcalc=mcalcB;  }
	else          {  kperm=kpermA;  mcalc=mcalcA;  }
	
	double _x[6],*x=&_x[2];  // x[-2..+3]
	double _m[5],*m=&_m[2];  // m[-2..+2]
	double T[2];
	
	// The Akima spline interpolation only needs a context of the nearest 
	// 6 points of the actual interpolation location. 
	// I.e. if you have data points at 1 2 3 4 5 6 7 8 9 10 and want to 
	// interpolate at 4.3, then the points 2 3 4 5 6 7 are needed for 
	// the interpolation. For the borders, the not available outside points 
	// are "interpolated" by the algorithm as well based on the three 
	// available points at the border, hence at least 3 points are needed 
	// for the Akima interpolation. (For 2 points (n=1), the linear 
	// interpolation above is used.) 
	
	if(tv)
	{
		double _t[6],*t=&_t[2];  // t[-2..+3]
		// Compute the t[] values...
		for(int _k=0; _k<6; _k++)
		{
			int k=kperm[_k],kk=k+idx;
			if(kk>=0 && kk<=n)  t[k] = get_t(kk);
			else if(kk<0)
			{
				if(kk==-2)  t[k] = 2.0*tv[0]-tv[2];
				else        t[k] = tv[0] + tv[1] - tv[2];
			}
			else if(kk>n)
			{
				if(kk==n+2)  t[k] = 2.0*tv[n]-get_t(n-2);
				else         t[k] = tv[n] + get_t(n-1) - get_t(n-2);
			}
			//else assert(0);
		}
		// ...then the x[] values and the coefficients: 
		for(int j=0; j<dim; j++)
		{
			for(int _k=0; _k<6; _k++)
			{
				int k=kperm[_k],kk=k+idx;
				if(kk>=0 && kk<=n)  x[k] = xv(kk)[j];
				else if(kk<0) x[k] = (t[k+1]-t[k])*(m[k+2]-2.0*m[k+1]) + x[k+1];
				else if(kk>n) x[k] = (t[k]-t[k-1])*(2.0*m[k-2]-m[k-3]) + x[k-1];
				//else assert(0);
				
				if((k=mcalc[_k])!=10)  m[k] = (x[k+1]-x[k]) / (t[k+1]-t[k]);
			}
			
			for(int k=0; k<2; k++)
			{
				double mk1k =  fabs(m[k+1]-m[k]);
				double mk1k2 = fabs(m[k-1]-m[k-2]);
				double den = mk1k + mk1k2;
				T[k] = (den==0.0) ? 0.0 : ( (mk1k*m[k-1]+mk1k2*m[k]) / den );
			}
			
			double dt=tv[idx+1]-tv[idx];
			B[j]=T[0];
			C[j]=(3.0*m[0]-2.0*T[0]-T[1])/dt;
			D[j]=(T[0]+T[1]-2.0*m[0])/(dt*dt);
		}
	}
	else for(int j=0; j<dim; j++)
	{
		for(int _k=0; _k<6; _k++)
		{
			int k=kperm[_k],kk=k+idx;
			if(kk>=0 && kk<=n)  x[k] = xv(kk)[j];
			else if(kk<0)  x[k] = (m[k+2]-2.0*m[k+1]) + x[k+1];
			else if(kk>n)  x[k] = (2.0*m[k-2]-m[k-3]) + x[k-1];
			//else assert(0);
			
			if((k=mcalc[_k])!=10)  m[k] = x[k+1]-x[k];
		}
		
		for(int k=0; k<2; k++)
		{
			double mk1k =  fabs(m[k+1]-m[k]);
			double mk1k2 = fabs(m[k-1]-m[k-2]);
			double den = mk1k + mk1k2;
			T[k] = (den==0.0) ? 0.0 : ( (mk1k*m[k-1]+mk1k2*m[k]) / den );
		}
		
		B[j]=T[0];
		C[j]=3.0*m[0]-2.0*T[0]-T[1];
		D[j]=T[0]+T[1]-2.0*m[0];
	}
	
	return(retval);
}


int AkimaSpline::Eval(double t,double *result) const
{
	if(!n)
	{  for(int j=0; j<dim; j++)  result[j]=0.0;  return(-2);  }
	
	int retval=((AkimaSpline*)this)->_ComputeCoeffs(&t);
	
	// Evaluate the spline: 
	for(int j=0; j<dim; j++)
	{  result[j] = xv(valid_idx)[j] + t*( B[j] + t*( C[j] + t*D[j] ));  }
	
	return(retval);
}

int AkimaSpline::EvalD(double t,double *result) const
{
	if(!n)
	{  for(int j=0; j<dim; j++)  result[j]=0.0;  return(-2);  }
	
	int retval=((AkimaSpline*)this)->_ComputeCoeffs(&t);
	
	// Evaluate the spline: 
	for(int j=0; j<dim; j++)
	{  result[j] = B[j] + t*( 2.0*C[j] + t*3.0*D[j] );  }
	
	return(retval);
}

int AkimaSpline::EvalDD(double t,double *result) const
{
	if(!n)
	{  for(int j=0; j<dim; j++)  result[j]=0.0;  return(-2);  }
	
	int retval=((AkimaSpline*)this)->_ComputeCoeffs(&t);
	
	// Evaluate the spline: 
	for(int j=0; j<dim; j++)
	{  result[j] = 2.0*C[j] + t*6.0*D[j];  }
	
	return(retval);
}


int AkimaSpline::Create(const VectorArray<double> &cpoints,const double *tvals)
{
	if(cpoints.NVectors()<2 || cpoints.Dim()<(tvals==TVALS_CPTS ? 2 : 1))
	{  return(-2);  }
	
	// Check t values if specified: 
	if(tvals && tvals!=TVALS_DIST && 
		( (tvals==TVALS_CPTS) ? 
			_TestAscentingOrder_LastCompo(cpoints) : 
			_TestAscentingOrder(tvals,cpoints.NVectors()) ) )
	{  return(-4);  }
	
	// Get rid of old stuff...
	// ...and copy the control points and t values, if present: 
	_CopyPointsAndTVals(cpoints,tvals);  // calls Clear(). 
	
	//valid_idx=-1; and B,C,D=NULL because Clear() was called. 
	B=ALLOC<double>(3*dim);
	C=&B[dim];
	D=&C[dim];
	
	return(0);
}


AkimaSpline::AkimaSpline() : 
	SplineBase()
{
	valid_idx=-1;
	B=C=D=NULL;
}

AkimaSpline::~AkimaSpline()
{
	Clear();
}

}  // end of namespace NUM

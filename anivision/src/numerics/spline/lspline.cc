/*
 * numerics/spline/lspline.cc
 * 
 * Numerics library linear spline. 
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

#include "lspline.h"

namespace NUM
{

const SplineBase::Properties *LinearSpline::SplineProperties() const
{
	static const Properties prop=
	{
		splinetype: ST_LSpline,
		name: "lspline",
		cont_order: 0
	};
	return(&prop);
}


void LinearSpline::Clear()
{
	SplineBase::Clear();
}


int LinearSpline::Eval(double t,double *result) const
{
	if(!n)
	{  for(int j=0; j<dim; j++)  result[j]=0.0;  return(-2);  }
	
	// Compute the index of the polynomial: 
	int retval;
	int idx=_EvalFindIdx(t,&retval);
	
	// Normalize t: 
	t-=get_t(idx);
	
	// Evaluate the "spline": 
	#if 1
	double h = tv ? (tv[idx+1]-tv[idx]) : 1.0;
	double p = t/h;
	double p1 = 1.0-p;
	for(int j=0; j<dim; j++)
	{  result[j] = xv(idx+1)[j]*p + xv(idx)[j]*p1;  }
	#elif 0
	// This is just for testing: cosine interpolation. 
	// Will work nice if the interpolation points are up-down-up-down-up...
	// Only useful for x-t interpolation, no difference for x-y interpolation. 
	double h = tv ? (tv[idx+1]-tv[idx]) : 1.0;
	double p = t/h;
	double p1 = 1.0-p;
	double pp = 0.5*(1.0-cos(p*M_PI));
	double pp1 = 1.0-pp;
	for(int j=0; j<dim; j++)
	{  result[j] = xv(idx+1)[j]*pp + xv(idx)[j]*pp1;  }
	#elif 0
	// This is for testing only: cubic interpolation. 
	// Only useful for x-t interpolation, no difference for x-y interpolation. 
	// Looks pretty much like x-splines but cannot interpolate first and 
	// last points (without additional work / assumptions). 
	double h = tv ? (tv[idx+1]-tv[idx]) : 1.0;
	double p = t/h;
	if(idx>0 && idx<n-1) for(int j=0; j<dim; j++)
	{
		double a0 = xv(idx+2)[j] - xv(idx+1)[j] + xv(idx)[j] - xv(idx-1)[j];
		double a1 = xv(idx-1)[j] - xv(idx)[j] - a0;
		double a2 = xv(idx+1)[j] - xv(idx-1)[j];
		double a3 = xv(idx)[j];
		result[j] = ((a0*p + a1)*p + a2)*p + a3;
	}
	#elif 0
	// This is for testing only: hermite interpolation. 
	// Only useful for x-t interpolation, no difference for x-y interpolation. 
	// Quite similar to cubic interpolation but with additional tension 
	// and bias. Cannot interpolate first and last points (without 
	// additional work / assumptions). 
	double h = tv ? (tv[idx+1]-tv[idx]) : 1.0;
	double p = t/h;
	double p2=p*p,p3=p2*p;
	double tension=0.0;  // +1 -> high, 0 -> normal, -1 -> low
	double bias=0.0;     // >0 (<0) -> towards first (last) segment
	double tens1bp = (1.0-tension)*(1.0+bias);
	double tens1bm = (1.0-tension)*(1.0-bias);
	if(idx>0 && idx<n-1) for(int j=0; j<dim; j++)
	{
		double y0 = xv(idx-1)[j];
		double y1 = xv(idx  )[j];
		double y2 = xv(idx+1)[j];
		double y3 = xv(idx+2)[j];
		double m0 = 0.5 * ( (y1-y0)*tens1bp + (y2-y1)*tens1bm );
		double m1 = 0.5 * ( (y2-y1)*tens1bp + (y3-y2)*tens1bm );
		double a0 =  2.0*p3 - 3.0*p2 + 1.0;
		double a1 =      p3 - 2.0*p2 + p;
		double a2 =      p3 -     p2;
		double a3 = -2.0*p3 + 3.0*p2;
		result[j] = a0*y1+a1*m0+a2*m1+a3*y2;
	}
	#endif
	
	return(retval);
}

int LinearSpline::EvalD(double t,double *result) const
{
	if(!n)
	{  for(int j=0; j<dim; j++)  result[j]=0.0;  return(-2);  }
	
	// Compute the index of the polynomial: 
	int retval;
	int idx=_EvalFindIdx(t,&retval);
	
	// t not needed below here. 
	
	// "Evaluate" the "spline": 
	double h = tv ? (tv[idx+1]-tv[idx]) : 1.0;
	for(int j=0; j<dim; j++)
	{  result[j] = (xv(idx+1)[j]-xv(idx)[j])/h;  }
	
	return(retval);
}

int LinearSpline::EvalDD(double t,double *result) const
{
	// See description. 
	for(int j=0; j<dim; j++)
	{  result[j]=0.0;  }
	
	return(0);
}


int LinearSpline::Create(const VectorArray<double> &cpoints,const double *tvals)
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
	
	return(0);
}


double LinearSpline::CalcLength(NUM::Integrator_R_R * /*integrator*/,
	int /*int_by_piece*/)
{
	if(!n) return(0.0);
	double len=0.0;
	double *p0,*p1=xv(0);
	for(int i=1; i<=n; i++)
	{
		p0=p1;
		p1=xv(i);
		len+=NUM::DistEuclidic(p0,p1,dim);
	}
	return(len);
}


LinearSpline::LinearSpline() : 
	SplineBase()
{
	// nothing to do
}

LinearSpline::~LinearSpline()
{
	Clear();
}

}  // end of namespace NUM

/*
 * objdesc/curvetmap_linear.cc
 * 
 * Curve time mapping function for linar (=constant speed) movement. 
 * This is part of the AniVision project. 
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

#include "curvetmap_linear.h"

#include <numerics/spline/cspline.h>


namespace NUM
{

double CurveTMapFunction_Linear::function(double x)
{
	assert(ialf);
	// Okay, so we're seeing values curve_t0..curve_t1 for x here 
	// and must first linerly transform them into the range 
	// 0..totlen. 
	double res;
	ialf->Eval( (x-curve_t0) * totlen / (curve_t1-curve_t0) ,&res);
	//fprintf(stderr,"tmap_linear: %g -> %g\n",x,res);
	return(res);
}

double CurveTMapFunction_Linear::diff(double x)
{
	assert(ialf);
	double scale = totlen / (curve_t1-curve_t0);
	double tt = (x-curve_t0) * scale;
	double res;
	ialf->EvalD(tt,&res);
	return( res * scale );
}

double CurveTMapFunction_Linear::ddiff(double x)
{
	assert(ialf);
	double scale = totlen / (curve_t1-curve_t0);
	double tt = (x-curve_t0) * scale;
	double res;
	ialf->EvalDD(tt,&res);
	return( res * scale*scale );
}


int CurveTMapFunction_Linear::Create(CurveFunctionBase *curve,
	double _curve_t0,double _curve_t1,EvalContext *ectx)
{
	// Constant moving function using spline interpolation. 
	assert(!ialf);
	
	// We interpolate using nseq interpolation segments. 
	// For splines, the number of interpoaltion segments is the number 
	// of spline curve pieces. Other functions return -1 indicating 
	// that there is no real preference for the distribution of the 
	// interpolation points. 
	int nseg=curve->TMapCreator_GetPreferredInterpolSegCount();
	
	// Number of control points per interpolation segment. 
	// Use 12 (or so) for splines (which have several interpolation segments) 
	// and much more for functions which do not have an interpol. segment 
	// count preference. 
	// FIXME: should be tunable. 
	const int ppsp = nseg<0 ? 120 : 12;
	
	// Set up the integrator to be used to calculate the curve length. 
	// FIXME: should be tunable. 
	Integrator_R_R_NewtonCotes ncintegrator;
	ncintegrator.nintervals=3;  // decrease if ppsp increase 
	ncintegrator.order=4;
	
	// We need at least one segment (especially if nseg=-1 [special]). 
	bool have_preferred_segs=(nseg>=0);
	if(nseg<0)  nseg=1;
	
	CurveFunctionBase::DevLenFunction_ctx len_func(curve);
	len_func.ectx=ectx;
	
	// Calc number of control points: 
	int nctlpts=ppsp*nseg+1;
	VectorArray<double> tv(nctlpts,1);
	VectorArray<double> sv(nctlpts,1);
	
	double ti0,ti1=curve->TMapCreator_GetInterpolSegITime(0);
	double len=0.0;
	// Compute control points for the inverse arc length 
	// function: 
	*tv[0]=ti1;
	*sv[0]=0.0;
	int vidx=1;
	for(int i=0; i<nseg; i++)
	{
		ti0=ti1;
		ti1=curve->TMapCreator_GetInterpolSegITime(i+1);
		double dt=(ti1-ti0)/double(ppsp);
		double t0,t1=ti0;
		for(int j=0; j<ppsp; j++,vidx++)
		{
			t0=t1;
			t1+=dt;
			// Calculate curve length until t. 
			// I.e. add length of curve in t0..t1 (=l) to 
			//      length ti0..t0 (=len). 
			double l=ncintegrator(len_func,t0,t1);
			if(l<=1e-10)
			{
				fprintf(stderr,"ERROR: l=%g too small at t=%g..%g (seg=%d/%d): "
					"identical spline points?\n",
					l,t0,t1,i,nseg);
				abort();
			}
			assert(l>0.0);
			len+=l;
			*sv[vidx]=len;
			*tv[vidx]=t1;
		}
	}
	assert(vidx==nctlpts);
	
	
	fprintf(stderr,"Linear tmap: nctlpts=%d*%d; totlen=%g; (%d calls) "
		"for curve map %g..%g -> %g..%g\n",
		nseg,ppsp,
		/*totlen=*/ *sv[nctlpts-1],len_func.ncalls,
		_curve_t0,_curve_t1,
		curve->iT0(),curve->iT1());
	
	// We need to create the inverse arc length function: 
	//   l^{-1}(s_i)=t_i  (s_i=sv[i]; t_i=tv[i])
	// The spline is a function [arc length] -> [internal_curve_time]
	// FIXME: Should probably use Hermite boundary condition. 
	CubicPolySpline *spline=new CubicPolySpline();
	int rv=spline->Create(
		/*cpoints=*/tv,/*tvals=*/sv.GetArrayBase(),
		SBC_Natural,NULL,NULL);
	if(rv)
	{
		// Should report some error. 
		assert(0);
		delete spline;
		return(1);
	}
	
	ialf=spline;
	totlen=*sv[nctlpts-1];
	curve_t0=_curve_t0;
	curve_t1=_curve_t1;
	
	return(0);
}


CurveTMapFunction_Linear::CurveTMapFunction_Linear() : 
	CurveTMapFunction()
{
	ialf=NULL;
	
	totlen=NAN;
	curve_t0=NAN;
	curve_t1=NAN;
}

CurveTMapFunction_Linear::~CurveTMapFunction_Linear()
{
	DELETE(ialf);
}

}  // end of namespace NUM

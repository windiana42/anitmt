/*
 * objdesc/curve_cspline.cc
 * 
 * Cubic poly spline curve class. 
 * This is part of the AniVision project. 
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

#include "curve_cspline.h"


namespace NUM
{

int CurveFunction_CSpline::Eval(double t,double *result,EvalContext *) const
{
	assert(cspline);
	return(cspline->Eval(t,result));
}

int CurveFunction_CSpline::EvalD(double t,double *result,EvalContext *) const
{
	assert(cspline);
	return(cspline->EvalD(t,result));
}

int CurveFunction_CSpline::EvalDD(double t,double *result,EvalContext *) const
{
	assert(cspline);
	return(cspline->EvalDD(t,result));
}


int CurveFunction_CSpline::TMapCreator_GetPreferredInterpolSegCount()
{
	assert(cspline);
	return(cspline->N());
}

double CurveFunction_CSpline::TMapCreator_GetInterpolSegITime(int n)
{
	assert(cspline);
	return(cspline->GetT(n));
}


int CurveFunction_CSpline::SetAddPoints(const char *val)
{
	int rv=CurveFunction_Spline_Base::SetAddPoints(val);
	if(rv==2 && !strcmp(val,"periodic"))
	{  udata->addpts=APS_Periodic;  rv=0;  }
	return(rv);
}


int CurveFunction_CSpline::Create(EvalContext *)
{
	assert(udata);
	assert(!cspline);
	
	if(!udata->pts)
	{
		// FIXME: report error
		assert(!"report me");
		return(0);
	}
	
	const double *tvals=NULL;
	switch(udata->tvals_mode)
	{
		case TVM_Unset:  // fall through
		case TVM_Inc:    tvals=NULL;  break;
		case TVM_Dist:   tvals=SplineBase::TVALS_DIST;  break;
		case TVM_Pts_T:  tvals=SplineBase::TVALS_CPTS;  break;
		default:  assert(0);
	}
	
	SplineBoundaryCondition sbc=SBC_Natural;
	double *paramA=NULL,*paramB=NULL;
	switch(udata->addpts)
	{
		case APS_Unset:   // fall through
		case APS_None:      sbc=SBC_Natural;  break;
		case APS_Periodic:  sbc=SBC_Periodic;  break;
		case APS_Start:
		case APS_End:
		case APS_Both:
			assert(!"!implemented");
			break;
		default:  assert(0);  break;
	}
	
	cspline=new CubicPolySpline();
	int rv=cspline->Create(*udata->pts,tvals,sbc,paramA,paramB);
	if(rv)
	{
		// FIXME: report error
		assert(!"report me");
		
		DELETE(cspline);
		return(1);
	}
	
	// Important: Create must set these: 
	dim=udata->pts->Dim();
	it0=cspline->GetT(0);
	it1=cspline->GetT(udata->pts->NVectors()-1);  // CORRECT.
	
	fprintf(stderr,"Created cspline(pts=%d*%d, it=%g..%g): rv=%d\n",
		udata->pts->NVectors(),udata->pts->Dim(),
		it0,it1,
		rv);
	
	DELETE(udata); assert(!udata);
	return(0);
}


CurveFunction_CSpline::CurveFunction_CSpline() : 
	CurveFunction_Spline_Base()
{
	cspline=NULL;
	udata=new UData_Base();
}

CurveFunction_CSpline::~CurveFunction_CSpline()
{
	DELETE(udata);
	DELETE(cspline);
}

}  // end of namespace NUM

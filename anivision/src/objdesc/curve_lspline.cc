/*
 * objdesc/curve_lspline.cc
 * 
 * Linear spline curve class. 
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

#include "curve_lspline.h"


namespace NUM
{

int CurveFunction_LSpline::Eval(double t,double *result,EvalContext *) const
{
	assert(lspline);
	return(lspline->Eval(t,result));
}

int CurveFunction_LSpline::EvalD(double t,double *result,EvalContext *) const
{
	assert(lspline);
	return(lspline->EvalD(t,result));
}

int CurveFunction_LSpline::EvalDD(double t,double *result,EvalContext *) const
{
	assert(lspline);
	return(lspline->EvalDD(t,result));
}


int CurveFunction_LSpline::TMapCreator_GetPreferredInterpolSegCount()
{
	assert(lspline);
	return(lspline->N());
}

double CurveFunction_LSpline::TMapCreator_GetInterpolSegITime(int n)
{
	assert(lspline);
	return(lspline->GetT(n));
}


int CurveFunction_LSpline::Create(EvalContext *)
{
	assert(udata);
	assert(!lspline);
	
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
	
	switch(udata->addpts)
	{
		case APS_Unset:   // fall through
		case APS_None:      break;
		case APS_Periodic:  assert(0);  break;   // <-- not allowed here
		case APS_Start:
		case APS_End:
		case APS_Both:
			assert(!"!implemented");
			break;
		default:  assert(0);  break;
	}
	
	lspline=new LinearSpline();
	int rv=lspline->Create(*udata->pts,tvals);
	if(rv)
	{
		// FIXME: report error
		assert(!"report me");
		
		DELETE(lspline);
		return(1);
	}
	
	// Important: Create must set these: 
	dim=udata->pts->Dim();
	it0=lspline->GetT(0);
	it1=lspline->GetT(udata->pts->NVectors()-1);  // CORRECT.
	
	fprintf(stderr,"Created lspline(pts=%d*%d, it=%g..%g): rv=%d\n",
		udata->pts->NVectors(),udata->pts->Dim(),
		it0,it1,
		rv);
	
	DELETE(udata); assert(!udata);
	return(0);
}


CurveFunction_LSpline::CurveFunction_LSpline() : 
	CurveFunction_Spline_Base()
{
	lspline=NULL;
	udata=new UData_Base();
}

CurveFunction_LSpline::~CurveFunction_LSpline()
{
	DELETE(udata);
	DELETE(lspline);
}

}  // end of namespace NUM

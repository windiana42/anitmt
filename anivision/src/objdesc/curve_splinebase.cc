/*
 * objdesc/curve_splinebase.cc
 * 
 * Base class for spline curve classes. Just for the interface :) 
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

#include "curve_splinebase.h"


namespace NUM
{

int CurveFunction_Spline_Base::SetAddPoints(const char *val)
{
	assert(udata);
	if(udata->addpts!=APS_Unset)  return(1);
	if(!strcmp(val,"none"))
	{  udata->addpts=APS_None;  }
	else if(!strcmp(val,"start"))
	{  udata->addpts=APS_Start;  }
	else if(!strcmp(val,"end"))
	{  udata->addpts=APS_End;  }
	else if(!strcmp(val,"both"))
	{  udata->addpts=APS_Both;  }
	// "periodic" is accepted by the derived cspline, only. 
	else return(2);
	return(0);
}

int CurveFunction_Spline_Base::SetTValueMode(const char *val)
{
	assert(udata);
	if(udata->tvals_mode==TVM_Pts_T)  return(3);
	if(udata->tvals_mode!=TVM_Unset)  return(1);
	if(!strcmp(val,"inc"))
	{  udata->tvals_mode=TVM_Inc;  }
	else if(!strcmp(val,"dist"))
	{  udata->tvals_mode=TVM_Dist;  }
	else return(2);
	return(0);
}


int CurveFunction_Spline_Base::SetPoints(VectorArray<double> &cpts,
	bool with_tvals)
{
	assert(udata);
	if(udata->pts)  return(1);
	if(cpts.NVectors()<2)  return(2);
	if(with_tvals && udata->tvals_mode!=TVM_Unset && 
		udata->tvals_mode!=TVM_Inc)  return(3);
	udata->pts=new VectorArray<double>(VectorArray<double>::Transfer,cpts);
	if(with_tvals) udata->tvals_mode=TVM_Pts_T;
	return(0);
}


CurveFunction_Spline_Base::CurveFunction_Spline_Base() : 
	CurveFunctionBase()
{
	// Derived class' constructor is responsible for the (de)allocation. 
	udata=NULL;
}

CurveFunction_Spline_Base::~CurveFunction_Spline_Base()
{
	// Derived class' constructor is responsible for the (de)allocation. 
	assert(!udata);
}


//------------------------------------------------------------------------------

CurveFunction_Spline_Base::UData_Base::UData_Base()
{
	pts=NULL;
	tvals_mode=TVM_Unset;
	addpts=APS_Unset;
}

CurveFunction_Spline_Base::UData_Base::~UData_Base()
{
	DELETE(pts);
}

}  // end of namespace NUM

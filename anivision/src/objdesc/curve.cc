/*
 * objdesc/curve.cc
 * 
 * Curve class to move object position around in space. 
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

#include "curve.h"
#include "curvetmap_none.h"


namespace NUM
{

int OCurve::Eval(double t,double *result,EvalContext *ectx) const
{
	assert(curvefunc);
	double my_t = tmapfunc ? (*tmapfunc)(t) : t;
	return(curvefunc->Eval(my_t,result,ectx));
}

int OCurve::EvalD(double t,double *result,EvalContext *ectx) const
{
	assert(curvefunc);
	
	if(!tmapfunc)
	{  return(curvefunc->EvalD(t,result,ectx));  }
	
	double my_t = (*tmapfunc)(t);
	int rv=curvefunc->EvalD(my_t,result,ectx);
	
	// Nachdifferenzieren: 
	double diff=tmapfunc->diff(t);
	for(int i=0; i<curvefunc->Dim(); i++)
		result[i]*=diff;
	
	return(rv);
}

int OCurve::EvalDD(double t,double *result,EvalContext *ectx) const
{
	assert(curvefunc);
	
	if(!tmapfunc)
	{  return(curvefunc->EvalDD(t,result,ectx));  }
	
	double my_t = (*tmapfunc)(t);
	int rv=curvefunc->EvalDD(my_t,result,ectx);
	
	double tmp[curvefunc->Dim()];
	curvefunc->EvalD(my_t,tmp,ectx);
	
	// Nachdifferenzieren: 
	double diff2=SQR(tmapfunc->diff(t));
	double ddiff=tmapfunc->ddiff(t);
	for(int i=0; i<curvefunc->Dim(); i++)
		result[i] = result[i]*diff2 + tmp[i]*ddiff;
	
	return(rv);
}


int OCurve::SetPosCurve(CurveFunctionBase *_curvefunc)
{
	if(curvefunc)
	{  DELETE(_curvefunc);  return(1);  }
	
	curvefunc=_curvefunc;
	
	return(0);
}


int OCurve::SetTMapFunction(CurveTMapFunction *_tmapfunc)
{
	if(tmapfunc)
	{  DELETE(_tmapfunc);  return(1);  }
	
	tmapfunc=_tmapfunc;
	
	return(0);
}


void OCurve::_SetTime_DoFix()
{
	if(isnan(delta_t))
	{
		if(t_range.Valid()==Value::RValidAB)
		{  delta_t=t_range.length();  }
	}
	else
	{
		switch(t_range.Valid())
		{
			case Value::RInvalid: break;
			case Value::RValidA:  t_range.set_b(t_range.val_a()+delta_t); break;
			case Value::RValidB:  t_range.set_a(t_range.val_b()-delta_t); break;
			case Value::RValidAB:  // fall through
			default: assert(0);
		}
	}
}


int OCurve::Set_DT(double dt)
{
	if(!isnan(delta_t))  return(1);
	// If delta_t==NAN, the t_range may not be RValidAB (time known otherwise). 
	if(t_range.Valid()==Value::RValidAB) assert(0);
	
	delta_t=dt;
	_SetTime_DoFix();
	
	return(0);
}

int OCurve::Set_T(const Range &t)
{
	if(t_range.Valid()!=Value::RInvalid)  return(1);
	
	if(t.Valid()==Value::RValidAB && !isnan(delta_t))  return(2);
	
	t_range=t;
	_SetTime_DoFix();
	
	return(0);
}


int OCurve::Create(EvalContext *ectx)
{
	// FIXME: This is currently just a quick-n-dirty implementation. 
	// We need a better one with continuacy etc. 
	
	// This will only work if the time is set fix. 
	assert(t_range.Valid()==Value::RValidAB);
	
	assert(curvefunc);
	int rv=curvefunc->Create(ectx);
	if(rv)
	{
		// We canot go on in this case. 
		return(1);
	}
	
	// This must be set by Create(): 
	dim=3;
	it0=T0();  // <-- t_range.val_a()
	it1=T1();  // <-- t_range.val_b()
	assert(!isnan(it0) && !isnan(it1));
	
	int errors=0;
	
	if(!tmapfunc)
	{
		// We need a tmap function, at least the "none" version 
		// which does just linear time re-scaling. 
		tmapfunc=new CurveTMapFunction_None();
	}
	
	// We have a time mapping function. 
	// Need to create it _after_ creating the actual curve 
	// function: 
	rv=tmapfunc->Create(curvefunc,T0(),T1(),ectx);
	if(rv)
	{  ++errors;  DELETE(tmapfunc);  }
	
	return(errors);
}


OCurve::OCurve() : 
	CurveFunctionBase(),
	t_range()
{
	curvefunc=NULL;
	tmapfunc=NULL;
	
	delta_t=NAN;
}

OCurve::~OCurve()
{
	DELETE(curvefunc);
	DELETE(tmapfunc);
}


//------------------------------------------------------------------------------

int CurveFunctionBase::Create(EvalContext *)
{
	// Must be overridden when called. 
	assert(0);
}


int CurveFunctionBase::Eval(double /*t*/,double * /*res*/,EvalContext *) const
{
	// Must be overridden when called. 
	assert(0);
}

int CurveFunctionBase::EvalD(double /*t*/,double * /*res*/,EvalContext *) const
{
	// Must be overridden when called. 
	assert(0);
}

int CurveFunctionBase::EvalDD(double /*t*/,double * /*res*/,EvalContext *) const
{
	// Must be overridden when called. 
	assert(0);
}


//------------------------------------------------------------------------------

double CurveFunctionBase::DevLenFunction_ctx::function(double x)
{
	if(!curve) return(0.0);
	int dim=curve->Dim();
	double tmp[dim];
	curve->EvalD(x,tmp,ectx);
	return(NormEuclidic(tmp,dim));
}

double CurveFunctionBase::TMapCreator_GetInterpolSegITime(int n)
{
	// Must be overridden if called with n!=0 or 1. 
	switch(n)
	{
		case 0:  return(it0);
		case 1:  return(it1);
		default:  assert(0);
	}
	return(NAN);
}


//------------------------------------------------------------------------------

double CurveTMapFunction::diff(double /*x*/)
{
	// Must be overridden. 
	assert(0);
	return(NAN);
}

double CurveTMapFunction::ddiff(double /*x*/)
{
	// Must be overridden. 
	assert(0);
	return(NAN);
}


int CurveTMapFunction::Create(CurveFunctionBase *,double,double,EvalContext *)
{
	// Must be overridden. 
	assert(0);
	return(1);
}

}  // end of namespace NUM

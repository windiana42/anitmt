/*
 * objdesc/curve_fixed.cc
 * 
 * Fixed "curve" class. 
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

#include "curve_fixed.h"


namespace NUM
{

int CurveFunction_Fixed::Eval(double /*t*/,double *result,EvalContext *) const
{
	for(int i=0; i<dim; i++)
		result[i]=pos[i];
	return(0);
}

int CurveFunction_Fixed::EvalD(double /*t*/,double *result,EvalContext *) const
{
	for(int i=0; i<dim; i++)
		result[i]=0.0;
	return(0);
}

int CurveFunction_Fixed::EvalDD(double /*t*/,double *result,EvalContext *) const
{
	for(int i=0; i<dim; i++)
		result[i]=0.0;
	return(0);
}


int CurveFunction_Fixed::SetPos(double *_pos,int _dim)
{
	if(pos)  return(1);
	dim=_dim;
	pos=(double*)LMalloc(dim*sizeof(dim));
	for(int i=0; i<dim; i++)
		pos[i]=_pos[i];
	return(0);
}


int CurveFunction_Fixed::Create(EvalContext *)
{
	if(!pos)
	{
		// FIXME: If no pos is specified, should use continuacy. 
		dim=3;
		pos=(double*)LMalloc(dim*sizeof(dim));
		pos[0]=pos[1]=pos[2]=0.0;
	}
	
	return(0);
}


CurveFunction_Fixed::CurveFunction_Fixed() : 
	CurveFunctionBase()
{
	pos=NULL;
}

CurveFunction_Fixed::~CurveFunction_Fixed()
{
	pos=(double*)LFree(pos);
}

}  // end of namespace NUM

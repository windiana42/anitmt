/*
 * objdesc/upspec_gravity.cc
 * 
 * Class for the up position of object based upon gravity model. 
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

#include "upspec_gravity.h"


namespace NUM
{

int OUpSpec_Gravity::Create(OCurve * /*ocurve*/,EvalContext *)
{
	if(!force_up_set)
	{  force_up_set=1;  }   // Default is already set in force_up[]. 
	
	if(use_accel_scale)
	{
		if(fabs(accel_scale)<1e-14)
		{  use_accel_scale=0;  }
	}
	
	return(0);
}


int OUpSpec_Gravity::Eval(OCurve *ocurve,const double *pos_already_known,
	const double *front_vector,double t,double *result,EvalContext *ectx) const
{
	// Get front vector if not specified. 
	double _pos_tmp[3];
	if(!pos_already_known)
	{
		ocurve->Eval(t,_pos_tmp,ectx);
		pos_already_known=_pos_tmp;
	}
	
	// This one must be specified: 
	assert(front_vector);
	
	double prim_up[3];
	if(is_center)
	{
		for(int i=0; i<3; i++)
			prim_up[i]=pos_already_known[i]-force_up[i];
		NormalizeVector(prim_up,3);
	}
	else
	{
		for(int i=0; i<3; i++)
			prim_up[i]=force_up[i];
		// No normalisation step here. 
	}
	
	if(use_accel_scale)
	{
		double accl[3];
		ocurve->EvalDD(t,accl,ectx);
		for(int i=0; i<3; i++)
			prim_up[i] += accl[i]*accel_scale;
		//fprintf(stderr,"ACCL(t=%g)=<%g,%g,%g>\n",t,accl[0],accl[1],accl[2]);
	}
	
	// The primary UP vector is prim_up (y); rotate it along the right 
	// vetor (i.e. front x up) until it is perpendicular to front 
	// (i.e. do another cross product): 
	double tmp_right[3];
	CrossProduct(tmp_right,front_vector,prim_up);
	CrossProduct(result,tmp_right,front_vector);
	
	return(0);
}


int OUpSpec_Gravity::SetForce(double *_force_up,bool _is_center)
{
	if(force_up_set)
		return(is_center==_is_center ? 1 : 3);
	
	is_center=_is_center;
	for(int i=0; i<3; i++)
		force_up[i]=_force_up[i];
	
	return(0);
}

int OUpSpec_Gravity::SetAccelScale(double _accel_scale)
{
	if(use_accel_scale) return(1);
	accel_scale=_accel_scale;
	use_accel_scale=1;
	return(0);
}


OUpSpec_Gravity::OUpSpec_Gravity() : 
	OUpSpec()
{
	force_up[0]=0.0;
	force_up[1]=1.0;
	force_up[2]=0.0;
	is_center=0;
	force_up_set=0;
	
	use_accel_scale=0;
	accel_scale=0.0;
}

OUpSpec_Gravity::~OUpSpec_Gravity()
{
}

}  // end of namespace NUM

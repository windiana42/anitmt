/*
 * calccore/ccif.cc
 * 
 * Calc core interface, wrappers and other stuff. 
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

#include "ccif.h"
#include <objdesc/omatrix.h>

#include <assert.h>


namespace CC
{

int CCIF_Setting::SetTimeRange(const Range & /*tr*/)
{
	assert(0);
	return(0);
}


int CCIF_Setting::ComputeAll()
{
	assert(0);
	return(0);
}


int CCIF_Setting::TimeStep()
{
	assert(0);
	return(1);  // 1 -> animation end
}


CCIF_Setting::CCIF_Setting(CCIF_Factory *_factory)
{
	factory=_factory;
}

CCIF_Setting::~CCIF_Setting()
{
	factory=NULL;
}

//------------------------------------------------------------------------------

int CCIF_Object::ProvidesVal(OVal ov) const
{
	switch(ov)
	{
		case ValPos:    // fall
		case ValFront:  //   trough
		case ValUp:     //     to 
		case ValSpeed:  //       here
		case ValAccel:  //          ...
		case ValMatrix: return((vflags & VF_Location) ? 1 : 0);
		case ValTime:   return((vflags & VF_Time) ? 1 : 0);
		case ValActive: return((vflags & VF_Active) ? 1 : 0);
	}
	return(0);
}


int CCIF_Object::GetVal(OVal ov,int *store_here,CCIF_ExecThread *)
{
	// Just the default implementation. 
	if(!ProvidesVal(ov)) return(1);
	switch(ov)  // Fall through switch. 
	{
		case ValPos:
		case ValFront:
		case ValUp:
		case ValSpeed:
		case ValAccel:
		case ValMatrix:
		case ValTime:
			return(2);  // incompatible type
		case ValActive:
			*store_here=1;   // default: active
			return(0);
	}
	assert(0);
}


int CCIF_Object::GetVal(OVal ov,double *store_here,CCIF_ExecThread *)
{
	// Just the default implementation. 
	if(!ProvidesVal(ov)) return(1);
	switch(ov)  // Fall through switch. 
	{
		case ValPos:
		case ValFront:
		case ValUp:
		case ValSpeed:
		case ValAccel:
			for(int i=0; i<3; i++)
			{  store_here[i]=0.0;  }
			return(0);
		case ValTime:
			*store_here=0.0;
			return(0);
		case ValMatrix:
		case ValActive:
			return(2);  // incompatible type
	}
	assert(0);
}


int CCIF_Object::GetVal(OVal ov,NUM::ObjectMatrix *store_here,CCIF_ExecThread *)
{
	// Just the default implementation. 
	if(!ProvidesVal(ov)) return(1);
	switch(ov)  // Fall through switch. 
	{
		case ValPos:
		case ValFront:
		case ValUp:
		case ValSpeed:
		case ValAccel:
		case ValTime:
		case ValActive:
			return(2);  // incompatible type
		case ValMatrix:
			*store_here=NUM::ObjectMatrix::IdentMat;
			return(0);
	}
	assert(0);
}


int CCIF_Object::EvalAniDescBlock(ADTNAniDescBlock * /*adb*/,ExecThread *et)
{
	// This function must be overruidden. 
	assert(et);
	assert(0);
	return(0);
}


CCIF_Object::CCIF_Object(CCIF_Setting *_setting,int _vflags,
	AniObjectInstance *_ani_object)
{
	setting=_setting;
	vflags=_vflags;
	ani_object_inst=_ani_object;
	assert(ani_object_inst);
}

CCIF_Object::~CCIF_Object()
{
	ani_object_inst=NULL;
}

//------------------------------------------------------------------------------

CCIF_Value::CCIF_Value(CCIF_Setting *_setting,int _dimension)
{
	setting=_setting;
	dimension=_dimension;
}

CCIF_Value::~CCIF_Value()
{
}

//------------------------------------------------------------------------------

void _ccif_failassert(int line)
{
	fprintf(stderr,"_ccif_failassert(%d)\n",line);
	abort();
}

}  // end of namespace CC

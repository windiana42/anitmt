/*
 * calccore/scc/object.cc
 * 
 * Simple calc core object main file. 
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

#include "ccif_simple.h"

#include <objdesc/omatrix.h>

//#include <objdesc/curve_fixed.h>
//#include <objdesc/curve_lspline.h>
//#include <objdesc/curve_cspline.h>
//#include "curve_function.h"

//#include <objdesc/curvetmap_none.h>
//#include <objdesc/curvetmap_linear.h>

//#include <objdesc/frontspec_speed.h>

//#include <objdesc/upspec_gravity.h>


#warning "FIXME: rewrite this file."
//FIXME: This file is just a very quick hack and needs to be rewritten. 

namespace CC { namespace S
{

NUM::OMovement *CCObject::_FindMovementByTime(double setting_time)
{
	// First, query cache: 
	do {
		if(!_prev_looked_up_movement)  break;
		if(_prev_looked_up_movement->T0()<=setting_time && 
			_prev_looked_up_movement->T1()>setting_time)
		{  return(_prev_looked_up_movement);  }
		
		// See if it happens to be the next movement: 
		NUM::OMovement *next=_prev_looked_up_movement->next;
		if(!next)  break;
		if(next->T0()<=setting_time && next->T1()>setting_time)
		{  _prev_looked_up_movement=next;  return(next);  }
	} while(0);
	
	// Perform lookup: 
	NUM::OMovement *close=NULL;  // Just the > vs >= issue...
	for(NUM::OMovement *m=movements.first(); m; m=m->next)
	{
		if(m->T0()<=setting_time && m->T1()>=setting_time)
		{
			close=m;
			if(m->T1()>setting_time)  break;
		}
	}
	_prev_looked_up_movement=close;
	return(close);
}


int CCObject::GetVal(OVal ov,int *store_here,CCIF_ExecThread *cet)
{
	fprintf(stderr,"CC:S: Implement GetVal()\n");
	return(CCIF_Object::GetVal(ov,store_here,cet));
}


int CCObject::GetVal(OVal ov,double *store_here,CCIF_ExecThread *cet)
{
	// First, get the current movement: 
	double setting_time = setting->CurrentSettingTime();
	NUM::OMovement *m=_FindMovementByTime(setting_time);
	
	EvalContext_SCC _ectx(cet,this);
	EvalContext_SCC *ectx=&_ectx;
	
	switch(ov)
	{
		case ValTime:
			// FIXME: Should return object time(?) For now, I am using setting time. 
			// Only used in povwriter to write a comment, currently. 
			*store_here=setting_time;
			return(0);
		case ValPos:
			if(!m)
			{  NUM::OMovement::DefaultPos(store_here);  return(0);  }
			m->GetPos(setting_time,store_here,ectx);
			return(0);
		case ValFront:
			if(!m)
			{  NUM::OMovement::DefaultFront(store_here);  return(0);  }
			m->GetFront(setting_time,store_here,/*pos_already_known=*/NULL,ectx);
			return(0);
		case ValUp:
			if(!m)
			{  NUM::OMovement::DefaultUp(store_here);  return(0);  }
			m->GetUp(setting_time,store_here,
				/*pos_already_known=*/NULL,/*front_already_known=*/NULL,ectx);
			return(0);
		case ValSpeed:
		case ValAccel:
			assert(!"!implemented");
			return(0);
		// NO: default: assert(0); NO!
	}
	
	// Return correct error value: 
	return(CCIF_Object::GetVal(ov,store_here,cet));
}


int CCObject::GetVal(OVal ov,NUM::ObjectMatrix *store_here,CCIF_ExecThread *cet)
{
	if(ov==ValMatrix)
	{
		EvalContext_SCC _ectx(cet,this);
		EvalContext_SCC *ectx=&_ectx;
		
		// First, get the current movement: 
		double setting_time = setting->CurrentSettingTime();
		NUM::OMovement *m=_FindMovementByTime(setting_time);
		if(m)
		{
			double pos[3],front[3],up[3];

			m->GetPos(setting_time,pos,ectx);
			m->GetFront(setting_time,front,
				/*pos_already_known=*/pos,ectx);
			m->GetUp(setting_time,up,
				/*pos_already_known=*/pos,
				/*front_already_known=*/front,ectx);

			// FIXME: (a) allow right vector; (b) use obj_{front,right,up}
			store_here->SetObjPos(pos,front,up,NULL);
			return(0);
		}
		else
		{
			// No movement for current time. 
			fprintf(stderr,"** No movement for current time (%g) "
				"(not implemented).\n",setting_time);
			assert(!"!implemented");
		}
	}
	
	// *store_here=NUM::ObjectMatrix::IdentMat;
	
	// Return correct error value: 
	return(CCIF_Object::GetVal(ov,store_here,cet));
}


CCObject::CCObject(CCSetting *_setting,int _vflags,AniObjectInstance *_inst) : 
	CCIF_Object(_setting,_vflags,_inst),
	LinkedListBase<CCObject>(),
	movements()
{
	_prev_looked_up_movement=NULL;
	
	Setting()->RegisterObject(this);
}

CCObject::~CCObject()
{
	Setting()->UnregisterObject(this);
	
	while(!movements.is_empty())
	{  delete movements.popfirst();  }
}

}}  // end of namespace CC::S

/*
 * calccore/anitmt/setting.cc
 * 
 * AniTMT calc core setting main file. 
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

#include "factory.h"
#include "ccif_anitmt.h"


namespace CC { namespace AniTMT
{

int CCSetting::SetTimeRange(const Range &tr)
{
	timerange=tr;
	// Assume we're at the beginning of the time range. 
	assert(timerange.Valid()&Value::RValidA);
	current_time=timerange.val_a();
	return(0);
}


int CCSetting::ComputeAll()
{
	fprintf(stderr,"ComputeAll()...\n");
	
	return(0);
}


int CCSetting::TimeStep()
{
	// Increase time: 
	// FIXME: Make fps value global and tunable. 
	const double frames_per_time=25.0;
	++n_time_steps;
	current_time = timerange.val_a() + n_time_steps/frames_per_time;
	
	// Check for animation end: 
	assert(timerange.Valid()&Value::RValidB);
	int ani_end=(current_time>timerange.val_b());
	
	fprintf(stderr,"SCC: TimeStep: -> setting time: %g%s\n",
		current_time,ani_end ? " [end]" : "");
	
	return(ani_end);
}


void CCSetting::RegisterValue(CCValue *val)
{
	if(!val) return;
	vallist.append(val);
}

void CCSetting::UnregisterValue(CCValue *val)
{
	if(!val) return;
	vallist.dequeue(val);
}


void CCSetting::RegisterObject(CCObject *obj)
{
	if(!obj) return;
	objlist.append(obj);
}

void CCSetting::UnregisterObject(CCObject *obj)
{
	if(!obj) return;
	objlist.dequeue(obj);
}


CCSetting::CCSetting(CCIF_Factory_AniTMT *_factory) : 
	CCIF_Setting(_factory),
	objlist(),
	vallist()
{
	n_time_steps=0;
}

CCSetting::~CCSetting()
{
	// Clean object and value lists: 
	if(!vallist.is_empty())
	{
		fprintf(stderr,"OOPS: CCSetting: Val list not empty\n");
		while(!vallist.is_empty())
		{  delete vallist.first();  }  // <-- destructor will unregister
	}
	if(!objlist.is_empty())
	{
		fprintf(stderr,"OOPS: CCSetting: Obj list not empty\n");
		while(!objlist.is_empty())
		{  delete objlist.first();  }  // <-- destructor will unregister
	}
}

}}  // end of namespace CC::S

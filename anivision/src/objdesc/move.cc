/*
 * objdesc/move.cc
 * 
 * Class to move object (pos,front,up) around in space. 
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

#include "move.h"
#include "frontspec_speed.h"
#include "upspec_gravity.h"

#include <ctype.h>


namespace NUM
{

void OMovement::DefaultPos(double *store_here)
{
	store_here[0]=0.0;
	store_here[1]=0.0;
	store_here[2]=0.0;
}

void OMovement::DefaultFront(double *store_here)
{
	store_here[0]=1.0;
	store_here[1]=0.0;
	store_here[2]=0.0;
}

void OMovement::DefaultUp(double *store_here)
{
	store_here[0]=0.0;
	store_here[1]=1.0;
	store_here[2]=0.0;
}


int OMovement::GetPos(double t,double *store_here,EvalContext *ectx) const
{
	if(!ocurve)
	{
		DefaultPos(store_here);
		return(-2);
	}
	
	t-=ctime;
	return(ocurve->Eval(t,store_here,ectx));
}


int OMovement::GetFront(double t,double *store_here,
	const double *pos_already_known,
	EvalContext *ectx) const
{
	if(!ocurve || !ofront)
	{  DefaultFront(store_here);  return(-2);  }
	
	t-=ctime;
	
	int rv=ofront->Eval(ocurve,pos_already_known,t,store_here,ectx);
	if(do_normalize_front)
	{
		double len=NormalizeVector(store_here,3);
		if(len==0.0)
		{
			fprintf(stderr,"OOPS: len=0: problem with front vector normalisation.\n");
			// FIXME: Should use previous front vector. 
			DefaultFront(store_here);
		}
	}
	
	return(rv);
}


int OMovement::GetUp(double t,double *store_here,
	const double *pos_already_known,const double *front_already_known,
	EvalContext *ectx) const
{
	if(!ocurve)
	{
		DefaultUp(store_here);
		return(-2);
	}
	
	// Get front vector for the up eval function. 
	// Note that this is done before we exec "t-=ctime" below. 
	double front_tmp[3];
	if(!front_already_known)
	{
		GetFront(t,front_tmp,pos_already_known,ectx);
		front_already_known=front_tmp;
	}
	
	t-=ctime;
	
	int rv=oup->Eval(ocurve,pos_already_known,front_already_known,
		t,store_here,ectx);
	if(do_normalize_up)
	{
		double len=NormalizeVector(store_here,3);
		if(len==0.0)
		{
			fprintf(stderr,"OOPS: len=0: problem with up vector normalisation.\n");
			// FIXME: Should use previous up vector. 
			DefaultUp(store_here);
		}
	}
	
	return(rv);
}


int OMovement::Create(double current_setting_time,EvalContext *ectx)
{
	int nerrors=0;
	
	ctime=current_setting_time;
	
	if(!ocurve)
	{
		// FIXME: Continuacy...
		assert(!"!implemented");
		
		return(1);
	}
	
	fprintf(stderr,"IMPLEMENT: Continuacy etc. starting at OMovement::Create()\n");
	
	// For now. simply create the curve: 
	int rv=ocurve->Create(ectx);
	if(rv)
	{  ++nerrors;  }
	
	// Then, set up the front spec: 
	if(!ofront)
	{
		// Fall back to default: 
		ofront=new OFrontSpec_Speed();
	}
	rv=ofront->Create(ocurve,ectx);
	if(rv)
	{  ++nerrors;  }
	
	// Then, set up the up spec: 
	if(!oup)
	{
		// Fall back to default: 
		oup=new OUpSpec_Gravity();
	}
	rv=oup->Create(ocurve,ectx);
	if(rv)
	{  ++nerrors;  }
	
	fprintf(stderr,"Movement: ctime=%g, n_errors=%d\n",
		ctime,nerrors);
	
	return(nerrors);
}


int OMovement::SetPosCurve(OCurve *_ocurve)
{
	if(ocurve)
	{  DELETE(_ocurve);  return(1);  }
	
	ocurve=_ocurve;
	
	return(0);
}


int OMovement::SetFrontSpec(OFrontSpec *_ofront)
{
	if(ofront)
	{  DELETE(_ofront);  return(1);  }
	
	ofront=_ofront;
	
	return(0);
}

int OMovement::SetUpSpec(OUpSpec *_oup)
{
	if(oup)
	{  DELETE(_oup);  return(1);  }
	
	oup=_oup;
	
	return(0);
}


int OMovement::SetFrontMode(const char *str)
{
	bool yesno=1;
	while(isspace(*str))  ++str;
	while(*str=='!')
	{  yesno=!yesno;  ++str;  }
	if(!strcmp(str,"normalize"))
	{  do_normalize_front=yesno;  }
	else return(2);
	return(0);
}

int OMovement::SetUpMode(const char *str)
{
	bool yesno=1;
	while(isspace(*str))  ++str;
	while(*str=='!')
	{  yesno=!yesno;  ++str;  }
	if(!strcmp(str,"normalize"))
	{  do_normalize_up=yesno;  }
	else return(2);
	return(0);
}


OMovement::OMovement() : 
	LinkedListBase<OMovement>()
{
	ocurve=NULL;
	ofront=NULL;
	oup=NULL;
	
	ctime=NAN;
	
	do_normalize_front=1;
	do_normalize_up=1;
}

OMovement::~OMovement()
{
	DELETE(oup);
	DELETE(ofront);
	DELETE(ocurve);
}

}  // end of namespace NUM

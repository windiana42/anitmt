/*
 * parconsumer.cc
 * 
 * Implementation of parameter consumer. 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <hlib/prototypes.h>

#include <string.h>

#include "parconsumer.h"
#include "parmanager.h"

#ifndef TESTING
#define TESTING 1
#endif


#if TESTING
#warning TESTING switched on (uses assert())
#include <assert.h>
#else
#define assert(x)
#endif


namespace par
{


int ParameterConsumer::CheckParam(ParamInfo *pi)
{
	int complain=0;
	switch(pi->spectype)
	{
		case STNone:  break;
		case STAtLeastOnce:  complain=(!pi->nspec);  break;
		case STExactlyOnce:  complain=(pi->nspec!=1);  break;
		case STMaxOnce:      complain=(pi->nspec>1);   break;
		default:  complain=1;  break;
	}
	assert(!pi->nspec || pi->is_set);
	if(complain)
	{  CheckParamError(pi);  }
	return(complain ? 1 : 0);
}


PAR::ParamInfo *ParameterConsumer::AddParam(
	const char *name,
	ParameterType ptype,
	const char *helptext,
	void *valptr,
	ValueHandler *hdl,
	int flags)
{
	if(!name || !curr_section || (flags && flags<5))
		return(NULL);
	
	_AddParInfo info;
	info.section=curr_section;
	info.name=name;
	info.helptext=helptext;
	info.ptype=ptype;
	info.valptr=valptr;
	info.hdl=hdl;
	info.exclusive_hdl=(flags & PExclusiveHdl) ? 1 : 0;
	info.skip_in_help=(flags & PSkipInHelp) ? 1 : 0;
	info.has_default=!(flags & PNoDefault);
	info.spectype=PSpecType(flags & _STMask);
	
	ParamInfo *pi=parmanager()->AddParam(this,&info);
	if(!pi)
	{  ++add_failed;  }

	return(pi);
}

int ParameterConsumer::DelParam(ParamInfo *pi)
{
	return(parmanager()->DelParam(pi));
}

void ParameterConsumer::RecursiveDeleteParams(Section *top=NULL)
{
	#warning what to do if the curr_section is about to get removed...? \
	  at least should document that this sets curr_section=NULL. 
	curr_section=NULL;
	parmanager()->RecursiveDeleteParams(this,top);
}


int ParameterConsumer::SetSection(const char *sect_name,
	const char *helptext,Section *top,int flags)
{
	Section *s=parmanager()->RegisterSection(this,
		sect_name,helptext,top,flags);
	if(!s)  return(-1);
	curr_section=s;
	
	return(0);
}


PAR::Section *ParameterConsumer::FindSection(const char *name,Section *top)
{
	return(parmanager()->FindSection(name,top));
}


int ParameterConsumer::AddSpecialHelp(SpecialHelpItem *shi)
{
	int rv=parmanager()->AddSpecialHelp(this,shi);
	if(rv)
	{  ++add_failed;  }
	return(rv);
}


ParameterConsumer::ParameterConsumer(ParameterManager *_manager,
	int *failflag) : 
	shelp(failflag)
{
	int failed=0;
	
	manager=_manager;
	curr_section=NULL;
	add_failed=0;
	
	if(parmanager()->RegisterParameterConsumer(this))
	{  --failed;  }
	
	if(failflag)
	{  *failflag+=failed;  }
	else if(failed)
	{  ConstructorFailedExit();  }
}


ParameterConsumer::~ParameterConsumer()
{
	parmanager()->UnregisterParameterConsumer(this);
	
	// Make sure to delete the special help items: 
	while(!shelp.is_empty())
	{  delete shelp.popfirst();  }
	
	manager=NULL;
}

}  // namespace end 

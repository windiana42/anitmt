#include <hlib/prototypes.h>

#include <string.h>
#include <stdlib.h>
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
		case STAtLeastOnce:  complain=(!pi->nspec);    break;
		case STExactlyOnce:  complain=(pi->nspec!=1);  break;
		case STMaxOnce:      complain=(pi->nspec>1);   break;
		default:  complain=1;  break;
	}
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
	info.has_default=(flags & PNoDefault);
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


int ParameterConsumer::SetSection(const char *sect_name,
	const char *helptext,Section *top)
{
	Section *s=parmanager()->RegisterSection(this,
		sect_name,helptext,top);
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

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
	return(parmanager()->AddParam(this,&info));
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


ParameterConsumer::ParameterConsumer(ParameterManager *_manager,
	int *failflag)
{
	int failed=0;
	
	manager=_manager;
	curr_section=NULL;
	
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
	
	manager=NULL;
}


#warning hack me...



#warning REMOVE ME. 
#if 0
inline ParParseState ParameterConsumer::_CallParseFunc(
	IParDesc *ipd,ParamArg *arg)
{
	return((ipd->parser) ? 
		( (*(ipd->parser))(this,ipd,arg) ) : 
		( this->parse(ipd,arg) ) );
}

ParParseState ParameterConsumer::_ParseArg(
	IParDesc *ipd,ParamArg *arg,bool /*switch_only*/)
{
	// ipd->name is arg->name here. 
	ParParseState pps;
	
	if(ipd->prev_otype!=ParamArg::_FromNowhere)
	{  ParameterParser::manager->WarnAlreadySet(arg,ipd,
		ipd->prev_otype,ipd->prev_origin,ipd->prev_opos);  }
	
	if(ipd->ptype==PTOption)
	{
		if(arg->atype==ParamArg::Assignment)
		{  return(PPSOptAssign);  }
		assert(arg->atype==ParamArg::Option);
		pps=_CallParseFunc(ipd,arg);
		if(pps<=0)  // success
		{
			if(ipd->valptr)
			{  ++(*(int*)(ipd->valptr));  }
		}
	}
	else
	{
		assert(arg->atype==ParamArg::Option || 
		       arg->atype==ParamArg::Assignment);
		pps=_CallParseFunc(ipd,arg);
	}
	
	if(pps<=0)   // success
	{
		++ipd->is_set;
		
		ipd->prev_otype=arg->otype;
		ipd->prev_origin=arg->origin;
		ipd->prev_opos=arg->opos;
	}
	if(pps!=PPSSuccess)
	{
		ValParseError(ipd,arg,pps);
	}
	
	return(pps);
}


		// You will normally not have to call this function; it is normally 
		// called by the parser: 
		// par_name is the name of the parameter with all recognized 
		#warning TIDY ME UP!!!! (throw out; this belongs to ParamSource)
		// sections skipped (thus pointing into memory allocated at 
		// arg->arg). 
		// switch_only is set to true if `no-' was at the beginning of the arg. 
		ParParseState LookupParse(ParamArg *arg,const char *par_name,
			bool switch_only);
		

		#if 0
	protected:
		// Function to parse in the value from the passed ParamArg. 
		// The parser function returns 0 on success and !=0 on 
		// failure (see enum ParParseState for possible values). 
		// The value must be stored under *p->valptr; *p->valptr 
		// must not be modified if the return value is !=0. 
		// This virtual method parse() is only called if there is ????!!!!
		#warning (!!!fix description!!!)
		// The parser routines shall not modify any members of 
		// ParDesc (unless you really know what you do). 
		// Return value: 
		//    0 -> success (values tored under *ParDesc::valptr) 
		//  !=0 -> failed (*ParDesc::valptr must stay untouched)
		//         See the enum above for possible values. 
		// Note: if you use arg->next, you have to set arg->next->used 
		//       to the correct value (-2 -> error; -1 -> used; 
		//       0 -> not used; arg->next->namelen -> used)
		//       (you need not use -2 as this is set automatically if 
		//       you return an error and used further args.)
		virtual ParParseState parse(ParDesc *pd,ParamArg *)
			{  return((pd->ptype==PTOption) ? PPSSuccess : PPSIllegalArg);  }
		
		// Called when the passed argument belongs to the section this 
		// handler also belongs to but was not recognized among the 
		// handlers of the section. 
		// See LookupParse() for explanation of the args. 
		virtual ParParseState unknown(ParamArg *,const char * /*par_name*/,
			bool /*switch_only*/)  {  return(PPSUnknown);  }
		
		// Called if parsing failed or returned a warning: 
		// (Normally calls ParameterParser::manager->ValParseError().) 
		// Return value ignored. 
		virtual int ValParseError(ParDesc *pd,ParamArg *arg,ParParseState pps);
		#endif


#endif

}  // namespace end 

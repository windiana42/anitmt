/*
 * parintrnl.cc
 * 
 * Parameter system internal shared routines. 
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

#include "valhdl.h"

#include <string.h>


#ifndef TESTING
#define TESTING 1
#endif 


#ifdef TESTING
#warning TESTING switched on
#endif 


namespace par
{

static inline int ABS(int x)
{  return((x<0) ? (-x) : x);  }


// Returns static data:
const char *PAR::ParamTypeString(ParameterType ptype)
{
	switch(ptype)
	{
		case PTParameter:  return("parameter");
		case PTSwitch:     return("switch");
		case PTOption:     return("option");
		case PTOptPar:     return("optparam");
		// default: fall through to return("???);
	}
	return("???");
}


PAR::Section::Section(const char *_name,ssize_t namelen,int *failflag) : 
	sub(), pilist()
{
	int failed=0;
	
	name=NULL;
	helptext=NULL;
	sect_hdl=NULL;
	up=NULL;
	flags=0;
	
	if(_name)
	{
		if(namelen<0)  namelen=strlen(name);
		char *tmpname=(char*)LMalloc(namelen+1);
		if(!tmpname)
		{  --failed;  }
		else
		{
			strncpy(tmpname,_name,namelen);
			tmpname[namelen]='\0';
			name=tmpname;
		}
	}
	
	if(failflag)
	{  *failflag+=failed;  }
	else if(failed)
	{  ConstructorFailedExit();  }
}


PAR::Section::~Section()
{
	if(name)  name=(char*)LFree((char*)name);
	
	#if TESTING
	if(sub.first())
	{  fprintf(stderr,"PAR::Section::~Section(): still existing subsections!\n");  }
	if(pilist.first())
	{  fprintf(stderr,"PAR::Section::~Section(): still containing parameters!\n");  }
	#endif
	
	sect_hdl=NULL;
	up=NULL;
}


// Return value: 0 -> found; 1 -> not found
int PAR::ParamInfo::NameCmp(const char *search,size_t slen)
{
	if(!search || !slen)  return(1);
	
	for(int i=0,sp0=0,sp=0; sp>=0; sp0=sp,i++)
	{
		sp=synpos[i];
		int nlen=ABS(sp)-sp0-1;
		#if TESTING
		if(nlen<=0)
		{  fprintf(stderr,"parintrnl.cc: nlen=%d<0 (%s)\n",nlen,name);
			abort();  }
		#endif
		if(slen!=size_t(nlen))  continue;
		if(strncmp(&name[sp0],search,slen))  continue;
		return(0);
	}
	return(1);
}


// Static special allocator: 
PAR::ParamInfo *PAR::ParamInfo::NewPi(const char *name)
{
	if(!name)  return(NULL);
	
	// Count the synonyms...
	int n=1;
	for(const char *c=name; *c; c++)
	{  if(*c=='|')  ++n;  }
	
	// sympos[] is of type int... but with GNU C++ we don't relay on that. 
	#if __GNUC__
	#  define synpostype typeof(*(((ParamInfo*)NULL)->synpos))
	#else
	#  define synpostype int
	#endif
	// Allocate the synpos[] array and the ParamInfo at 
	// the same memory chunk. 
	int fflag=0;
	ParamInfo *pi=NEWplus<ParamInfo>(n*sizeof(synpostype),&fflag);
	if(fflag)
	{  ParamInfo::DelPi(pi);  pi=NULL;  }
	if(pi)
	{
		pi->name=name;
		pi->synpos=(synpostype*)(pi+1);
		
		const char *c=name;
		for(n=0; *c; c++)
		{
			if(*c=='|')
			{  pi->synpos[n++]=(c-name+1);  }
		}
		pi->synpos[n]=-(c-name+1);
	}
	#undef synpostype
	return(pi);
}


PAR::ParamInfo::ParamInfo(int *failflag)
{
	int failed=0;
	
	pc=NULL;
	section=NULL;
	name=NULL;
	helptext=NULL;
	ptype=ParameterType(-1);
	vhdl=NULL;
	exclusive_vhdl=0;
	skip_in_help=0;
	valptr=NULL;
	is_set=0;
	spectype=STNone;
	locked=0;
	nspec=0;
	
	if(failflag)
	{  *failflag+=failed;  }
	else if(failed)
	{  ConstructorFailedExit();  }
}
	

PAR::ParamInfo::~ParamInfo()
{
	if(exclusive_vhdl && vhdl)
	{  delete vhdl;  }
	vhdl=NULL;
	
	// name not copied. 
	name=NULL;
	section=NULL;
}


PAR::ParamCopy::ParamCopy(int * /*failflag*/=NULL) : 
	LinkedListBase<ParamCopy>(),
	porigin(ParamArg::_FromNowhere,NULL,-1)
{
	info=NULL;
	copyval=NULL;
	nspec=0;
}


PAR::SpecialHelpItem::SpecialHelpItem(int * /*failflag*/=NULL) : 
	LinkedListBase<SpecialHelpItem>()
{
	optname=NULL;
	descr=NULL;
	helptxt=NULL;
	item_id=-1;
}

}  // namespace end 

/*
 * srccmd.cc
 * 
 * Implementation of parameter source capable of reading in the 
 * parameters/arguments passed on the command line. 
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

#define HLIB_IN_HLIB 1
#include "srccmd.h"

#include <string.h>


#ifndef TESTING
#define TESTING 1
#endif


#if TESTING
#warning TESTING switched on (using assert())
#include <assert.h>
#else
#define assert(x) do{}while(0)
#endif


namespace par
{
#warning introduce ParamArg::ignore; must call ParamSource::approve() \
	[delete global handler]. 
#warning should introduce possibility for custom virtual parser in SectionParameterHandler 
		

int ParameterSource_CmdLine::ReadCmdLine(ParamArg *pa_array,int argc,
	Section *topsect)
{
	if(!topsect)
	{  topsect=manager->TopSection();  }
	
	// Section depth stack: 
	// Stores number of sections we went down at the last "section" 
	// command. 
	int *sdstack=NULL;   // array of size [sds_dim]
	int sds_dim=0;  // stack size
	int sds_top=-1;  // top index sdstack[sds_top]
	int ignore_above=-2;  // ignore params in non-existant sections
	
	Section *curr_sect=topsect;
	
	int errors=0;
	for(int i=0 /*YES*/; i<argc; i++)
	{
		ParamArg *pa=&pa_array[i];
		if(pa->pdone)  continue;  // Needed. 
		
		// Check for fancy section specs ( -sect-{ ... -} )
		do {
			if(pa->atype!=ParamArg::Option)  break;
			if(pa->namelen>=2 && pa->name[pa->namelen-1]=='{')
			{
				char *sname=(char*)pa->name;
				size_t snamelen=pa->namelen-1;
				if(pa->name[snamelen-1]=='-')  --snamelen;
				sname[snamelen]='\0';
				
				Section *down=manager->FindSection(sname,curr_sect,
					/*tell_section_handler=*/&pa->origin);
				if(!down)
				{
					PreprocessorError(PPUnknownSection,&pa->origin,
						curr_sect,sname);
					++errors;
					if(ignore_above==-2)  ignore_above=sds_top;
					pa->pdone=-1;  // This will ensure that Parse() skips it. 
				}
				// Alyways put it on the stack because that is more 
				// user-friendly (aviod error for too many "-}" because 
				// of misspelled section name. 
				
				// Check subsection depth: 
				int depth=0;
				if(down)
				{
					Section *si=down;
					for(; si && si!=curr_sect; si=si->up,depth++);
					assert(si && depth>0);  // Otherwise: down not below curr_sect. 
				}
				if(sds_top+1>=sds_dim)  // need to realloc stack
				{
					sds_dim+=16;
					int *oldptr=sdstack;
					sdstack=(int*)LRealloc(sdstack,sds_dim*sizeof(int));
					if(!sdstack)
					{
						sdstack=oldptr;  sds_dim-=16;
						errors=-1;  // alloc failure
						goto breakfor;
					}
				}
				sdstack[++sds_top]=depth;  // "push"
				if(down)
				{
					curr_sect=down;
					pa->pdone=1;  // This will ensure that Parse() skips it. 
				}
			}
			else if(pa->namelen==1 && pa->name[0]=='}')
			{
				if(sds_top<0)
				{
					too_many_ends:;
					PreprocessorError(PPTooManyEndStatements,&pa->origin,
						curr_sect,NULL);
					++errors;
					pa->pdone=-1;  // This will ensure that Parse() skips it. 
					ignore_above=-2;
					break;
				}
				for(int depth=sdstack[sds_top--]; depth; depth--)
				{
					if(curr_sect==topsect)
					{  goto too_many_ends;  }
					curr_sect=curr_sect->up;
					assert(curr_sect);
				}
				if(ignore_above!=-2 && sds_top<=ignore_above)
				{  ignore_above=-2;  }
				pa->pdone=1;  // This will ensure that Parse() skips it. 
			}
		} while(0);
		
		// Ignore params in unknown sections to prevents lots 
		// of unecessary errors. 
		if(ignore_above!=-2)  continue;
		
		int rv=Parse(pa,curr_sect);
		if(rv<0)
		{  ++errors;  }
	}
	breakfor:;
	
	LFree(sdstack);
	
	return(errors);
}


int ParameterSource_CmdLine::Parse(ParamArg *pa,Section *topsect)
{
	if(pa->pdone)
	{  return(1);  }
	
	if(!topsect)
	{  topsect=manager->TopSection();  }
	
	// Check for implicit args: 
	if(parmanager()->CheckHandleHelpOpts(pa,topsect))
	{
		++n_iquery_opts;
		return(10);
	}
	
	return(FindCopyParseParam(pa,NULL,NULL,topsect));
}


ParameterSource_CmdLine::ParameterSource_CmdLine(
	ParameterManager *_manager,int *failflag) : 
	ParameterSource(_manager,failflag)
{
	n_iquery_opts=0;
}

ParameterSource_CmdLine::~ParameterSource_CmdLine()
{
	// empty
}

}  // namespace end 

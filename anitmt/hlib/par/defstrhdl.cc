/*
 * defstrhdl.cc
 * 
 * Implementation of default string value handler. 
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

#include "valuehandler.h"
#include "cmdline.h"

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifndef TESTING
#define TESTING 1
#endif


#if TESTING
#warning TESTING switched on. TESTING using assert()
#include <assert.h>
#else
#define assert(x) 
#endif


namespace par
{

void *StringValueHandler::alloc(ParamInfo *)
{
	return(NEW<RefString>());
}

void StringValueHandler::free(ParamInfo *,void *valptr)
{
	if(valptr)
	{
		RefString *str=(RefString*)valptr;
		delete str;
	}
}


int StringValueHandler::copy(ParamInfo *,void *_dest,void *_src,int operation)
{
	RefString *dest=(RefString*)_dest;
	RefString *src=(RefString*)_src;
	
	// Both strings must be refs to '\0'-terminated strings:
	assert(dest->stype()!=1);
	assert(src->stype()!=1);
	
	int rv=0;
	const char *srcstr=src->str();
	switch((SpecialOp)operation)
	{
		case SOPCopy:
			*dest=*src;  // easy case (it's RefString after all)
			break;
		case SOPAdd:
			if(srcstr)
			{  rv=dest->append(srcstr);  }
			break;
		case SOPSub:
			if(srcstr)
			{  rv=dest->prepend(srcstr);  }
			break;
		default:  return(-2);  // operation not supported
	}
	
	return(rv ? (-1) : 0);
}


PAR::ParParseState StringValueHandler::parse(ParamInfo *,void *valptr,
	ParamArg *arg)
{
	const char *value=arg->value;
	if(internal::ValueInNextArg(arg))
	{
		ParamArg *nx=arg->next;  // not NULL here 
		if(nx->atype==ParamArg::Unknown || 
		   nx->atype==ParamArg::Filename )
		{
			value=arg->next->arg;
			arg->next->pdone=1;   // Parser will set that to -1 on error. 
		}
	}
	
	if(!value)
	{  return(PAR::PPSValOmitted);  }
	
	// skip whitespace
	const char *p=value;
	while(isspace(*p))  ++p;
	if(!(*p) || (arg->origin.otype==ParamArg::FromFile && *p=='#'))
	{  return(PAR::PPSValOmitted);  }
	
	// Check length of whitespace at the end of the string: 
	const char *pend=p+strlen(p);
	while(isspace(*(--pend)));
	++pend;
	// *pend is where the '\0' should be. 
	
	size_t str_len=pend-p;
	
	// We need a temporary copy: 
	char *tmp=(char*)LMalloc(str_len+1);
	if(!tmp)
	{  return(PAR::PPSAssFailed);  }
	
	strncpy(tmp,p,str_len);
	tmp[str_len]='\0';
	
	// Okay, tmp now contains a trimmed copy of the string. 
	// The rest of the algorithm will modify that trimmed version and 
	// convert it to the string we will actually store. 
	
	PAR::ParParseState rv=PAR::PPSSuccess;
	
	// How it works: 
	// First, the string is trimmed (all whitespace at the beginning 
	// and the end gets deleted). 
	// If no char is left, this is an error (specify empty strings 
	// as ""). 
	// * Then, it is taken as it is, if the first character is not `"'. 
	// * If the first char is `"', then everything between the first 
	//   and the second `"' is taken as string. If ``\\'' or ``\"'' 
	//   is encountered, it is replaced by `\' and `"' respectively 
	//   (with ``\"'' not counting as parsing end). All other excapes 
	//   STAY UNCHANGED. 
	#warning could support string auto-concatenation...
	
	if(*tmp=='\"')
	{
		// Parse it (changing tmp)
		char *dest=tmp;
		char *src=tmp+1;
		for(; *src; src++)
		{
			if(*src=='\\')
			{
				if(src[1]=='\\' || src[1]=='\"')
				{  *(dest++)=*(++src);  }
				else
				{  *(dest++)=*src;  }
			}
			else if(*src=='\"')
			{  *dest='\0';  break;  }
			else
			{  *(dest++)=*src;  }
		}
		if(*dest)
		{
			*dest='\0';
			if(!rv)
			{  rv=PAR::PPSArgUnterminated;  }
		}
		else if(src[1])
		{
			// src[1] is the char after the terminating `"'. 
			// Check if the rest of the line is a comment: 
			for(++src; *src; src++)
			{
				if(*src=='#' && arg->origin.otype==ParamArg::FromFile)
				{  break;  }
				if(!isspace(*src))
				{
					if(!rv)
					{  rv=PAR::PPSGarbageEnd;  }
					break;
				}
			}
		}
	}
	else
	{
		// Well, we take the complete string. 
		// Unless, of course, there is a comment at the end: 
		if(arg->origin.otype==ParamArg::FromFile)
		{
			for(char *c=tmp; *c; c++)
			{
				if(*c=='#')
				{  *c='\0';  break;  }
			}
		}
	}
	
	// Assign value: 
	RefString *str=(RefString*)valptr;
	int arv=0;
	switch(arg->assmode)
	{
		case '\0':  arv=str->set(tmp);  break;
		case '+':   arv=str->append(tmp);  break;
		case '-':   arv=str->prepend(tmp);  break;
		default:  rv=PAR::PPSIllegalAssMode;  break;
	}
	if(arv && rv<=0)
	{  rv=PAR::PPSAssFailed;  }
	
	// Important: free string again: 
	tmp=(char*)LFree(tmp);
	
	return(rv);
}


char *StringValueHandler::print(ParamInfo *,void *valptr,
	char *dest,size_t len)
{
	RefString *str=(RefString*)valptr;
	if(str->stype()==1)  // not a '\0'-terminated string
	{
		assert(str->stype()!=1);
		return(NULL);
	}
	
	// Calculate the required length: 
	size_t need_len=str->len()+3;  // +2 for `"' +1 for '\0'
	if(str->str())
	{
		for(const char *c=str->str(); *c; c++)
		{
			if(*c=='\\' || *c=='\"')
			{  ++need_len;  }
		}
	}
	
	// Allocate memory if not enough mem was supplied: 
	if(len<need_len)
	{
		dest=ValueHandler::stralloc(need_len);
		if(!dest)  return(dest);
		len=need_len;
	}
	
	// Copy the string: 
	char *d=dest;
	*(d++)='\"';
	if(str->str())
	{
		for(const char *s=str->str(); *s; s++)
		{
			if(*s=='\\' || *s=='\"')  *(d++)='\\';
			*(d++)=*s;
		}
	}
	*d='\"';
	
	return(dest);
}


// Make them available: 
static StringValueHandler static_string_handler;
ValueHandler *default_string_handler=&static_string_handler;

}  // end of namespace par

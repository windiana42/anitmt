/*
 * defstrhdl.cc
 * 
 * Implementation of default string value handler. 
 * 
 * Copyright (c) 2001--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#define HLIB_IN_HLIB 1
#include "valuehandler.h"
#include "cmdline.h"

#if HAVE_LIMITS_H
#include <limits.h>
#endif
#include <string.h>
#include <ctype.h>

#ifndef TESTING
#define TESTING 1
#endif


#if TESTING
#warning TESTING switched on. TESTING using assert()
#include <assert.h>
#else
#define assert(x) do{}while(0)
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


int StringValueHandler::copy(ParamInfo *,void *_dest,const void *_src,
	int operation)
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


PAR::ParParseState StringValueHandler::parse(ParamInfo *pi,void *valptr,
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
	
	PAR::ParParseState rv=PAR::PPSSuccess;
	
	const char *p=value;
	
	if(!p)
	{  rv=PAR::PPSValOmitted;  }
	else
	{
		// skip whitespace
		while(isspace(*p))  ++p;
		if(!(*p) || (arg->origin.otype==ParamArg::FromFile && *p=='#'))
		{  rv=PAR::PPSValOmitted;  }
	}
	if(rv!=PAR::PPSSuccess)
	{
		if(rv==PAR::PPSValOmitted && pi->ptype==PAR::PTOptPar)
		{
			return(PAR::PPSSuccess);
		}
		return(rv);
	}
	
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
	
	
	// How it works: 
	// First, the string is trimmed (all whitespace at the beginning 
	// and the end gets deleted). 
	// If no char is left, this is an error (specify empty strings 
	// as ""). 
	// Then the string is parsed. If a `"' appears, the closing `"' is 
	// looked for. If a `"' appears in between, this part is treated 
	// correctly and the `"' removed 
	// ( '-this="blah blah"' -> '-this=blah blah' )
	// If ``\\'', ``\"'' or ``\ '' is encountered, it is replaced by 
	// `\', `"', ` '  respectively (with neither of them counting as 
	// parsing end). 
	// All other excapes STAY UNCHANGED. 
	
	// Note: This parser is very tolerant. Maybe too tolerant. 
	// It won't even generate PAR::PPSGarbageEnd. 
	
	int in_quot=0;
	int saw_quote=0;
	// Parse it (changing tmp)
	char *dest=tmp;
	char *src=tmp;
	for(;;src++)
	{
		if(*src=='\\')
		{
			if(src[1]=='\\' || src[1]=='\"' || src[1]==' ')
			{  *(dest++)=*(++src);  }
			else
			{  *(dest++)=*src;  }
		}
		else if(*src=='\"')
		{
			in_quot=(in_quot ? 0 : 1);
			saw_quote=1;
		}
		else if(*src=='\0')
		{
			if(in_quot)
			{  rv=PAR::PPSArgUnterminated;  }
			*dest='\0';
			break;
		}
		else if(!in_quot && *src=='#' && 
			arg->origin.otype==ParamArg::FromFile)
		{
			*dest='\0';
			break;
		}
		else if(isspace(*src))
		{
			if(in_quot)
			{  *(dest++)=*src;  }
			else if(!saw_quote)
			{
				// Chomp spaces unless we did not yet see a quote. 
				*(dest++)=*src;
			}
		}
		else
		{  *(dest++)=*src;  }
	}
	if(*dest)
	{
		*dest='\0';
		if(!rv)
		{  rv=PAR::PPSArgUnterminated;  }
	}
	else if(!saw_quote && rv<=0)
	{
		// Cut away whitespace at the end: 
		--dest;
		while(dest>=tmp && isspace(*dest))  --dest;
		if(dest<tmp)
		{  rv=PAR::PPSValOmitted;  }
		else
		{  *(++dest)='\0';  }
	}
	
	// Assign value: 
	RefString *str=(RefString*)valptr;
	int arv=0;
	switch(arg->assmode)
	{
		//OLD assmode '\0' should not happen (no value!) I think... 
		//OLD case '\0':  assert(0);  rv=PAR::PPSIllegalAssMode;  break;
		case '\0':  // fall through
		case '=':   arv=str->set(tmp);  break;
		case '+':   arv=str->append(tmp);  break;
		case '-':   arv=str->prepend(tmp);  break;
		default:  rv=PAR::PPSIllegalAssMode;  break;
	}
	if(arv && rv<=0)
	{  rv=PAR::PPSAssFailed;  }
	
	//fprintf(stderr,"STRING PARSED=<%s>, rv=%d\n",str->str(),rv);
	
	// Important: free string again: 
	tmp=(char*)LFree(tmp);
	
	return(rv);
}


char *StringValueHandler::print(ParamInfo *,const void *valptr,
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

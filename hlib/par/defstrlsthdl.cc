/*
 * defstrlsthdl.cc
 * 
 * Implementation of default string list value handler. 
 * 
 * Copyright (c) 2001 -- 2004 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include <hlib/refstrlist.h>

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

void *StringListValueHandler::alloc(ParamInfo *)
{
	return(NEW<RefStrList>());
}

void StringListValueHandler::free(ParamInfo *,void *valptr)
{
	if(valptr)
	{
		RefStrList *sl=(RefStrList*)valptr;
		delete sl;
	}
}


int StringListValueHandler::copy(ParamInfo *,void *_dest,const void *_src,
	int operation)
{
	RefStrList *dest=(RefStrList*)_dest;
	RefStrList *src=(RefStrList*)_src;
	
	int rv=-2;
	switch((SpecialOp)operation)
	{
		case SOPCopy:
			dest->clear();
			// fall through
		case SOPAdd:
			rv=dest->append(src) ? (-1) : 0;
			break;
		case SOPSub:
			// Remove those entries in src from dest: 
			// see also parse()
			fprintf(stderr,"defstrlsthdl.cc:%d: NOT HACKED\n",__LINE__);
			abort();
			break;
		// defaut: operation not supported (default rv=-2)
	}
	
	return(rv);
}


PAR::ParParseState StringListValueHandler::parse(ParamInfo *,void *valptr,
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
	
	// Where to store the passed strings: 
	RefStrList list;
	
	char *tmp=NULL;
	if(value)
	{
		// skip whitespace
		const char *p=value;
		while(isspace(*p))  ++p;
		if(!(*p) || (arg->origin.otype==ParamArg::FromFile && *p=='#'))
		{  value=NULL;  }
		else
		{
			// Check length of whitespace at the end of the string: 
			const char *pend=p+strlen(p);
			while(isspace(*(--pend)));
			if(*pend=='\\' && pend[1])  ++pend;
			++pend;
			// *pend is where the '\0' should be. 
			
			size_t str_len=pend-p;
			
			// We need a temporary copy: 
			tmp=(char*)LMalloc(str_len+1);
			if(!tmp)
			{  return(PAR::PPSAssFailed);  }
			
			strncpy(tmp,p,str_len);
			tmp[str_len]='\0';
		}
	}
	
	// Okay, tmp now contains a trimmed copy of the string 
	// (or tmp=NULL if no value is passed [= empty string list]). 
	// The rest of the algorithm will modify that trimmed version and 
	// convert it to the string we will actually store. 
	
	PAR::ParParseState rv=PAR::PPSSuccess;
	
	if(tmp)
	{
		char *tc=tmp;
		for(;;)
		{
			// Skip all spacing separating two strings: 
			while(isspace(*tc))  ++tc;
			if(!(*tc))  break;  // found end of list
			if(arg->origin.otype==ParamArg::FromFile && *tc=='#')  break;  // rest of line is comment 
			
			// Parse in string beginning with *tc. 
			char *begin=tc;
			if(*begin=='\"') // encapsulated string
			{
				char *dest=begin;
				for(++tc;;tc++)
				{
					switch(*tc)
					{
						case '\\':
							if(tc[1]=='\\' || tc[1]=='\"' || tc[1]==' ')
							{  *(dest++)=*(++tc);  }
							else
							{  *(dest++)=*tc;  }
							break;
						case '\"':
							*dest='\0';
							if(list.append(begin))
							{  rv=PAR::PPSAssFailed;  }
							++tc;
							goto breakfor;  // next string
						case '\0':
							rv=PAR::PPSArgUnterminated;
							goto breakout;
						default:  *(dest++)=*tc;  break;
					}
				}
				breakfor:;
			}
			else  // non-encapsulated string 
			{
				char *dest=begin;
				int in_quot=0;
				for(;;tc++)
				{
					if(*tc=='\\')
					{
						if(tc[1]==' ' || tc[1]=='\"' || tc[1]=='\\')
						{  *(dest++)=*(++tc);  }
						else
						{  *(dest++)=*tc;  }
					}
					else if(*tc=='\"')
					{
						// Oops, there is a '"'. It's something like 
						// -this-string="has to be concatenated". 
						in_quot=(in_quot ? 0 : 1);
					}
					else if(*tc=='\0')
					{
						if(!in_quot)  break;
						rv=PAR::PPSArgUnterminated;
						goto breakout;
					}
					else if(!in_quot && isspace(*tc))
					{  ++tc;  break;  }
					else
					{  *(dest++)=*tc;  }
				}
				
				*dest='\0';
				if(list.append(begin))
				{  rv=PAR::PPSAssFailed;  goto breakout;  }
			}
		}
		breakout:;
	}
	
	#if 0
	char *_str=print(NULL,&list,NULL,0);
	fprintf(stderr,"READ IN STRLIST[%s]",_str);
	LFree(_str);
	#endif
	
	// Assign value: 
	RefStrList *sl=(RefStrList*)valptr;
	int arv=0;
	switch(arg->assmode)
	{
		// assmode '\0' should not happen (no value!) I think... 
		case '\0':  assert(0);  rv=PAR::PPSIllegalAssMode;  break;
		case '=':  sl->clear();  // fall through
		case '+':  arv=sl->append(&list);  break;
		case '-':
			// remove from list
			// (see also copy())
			fprintf(stderr,"defstrlsthdl.cc:%d: NOT HACKED\n",__LINE__);
			abort();
		default:  rv=PAR::PPSIllegalAssMode;  break;
	}
	if(arv && rv<=0)
	{  rv=PAR::PPSAssFailed;  }
	
	// Important: free string again: 
	tmp=(char*)LFree(tmp);
	
	return(rv);
}


char *StringListValueHandler::print(ParamInfo *,const void *valptr,
	char *dest,size_t len)
{
	RefStrList *sl=(RefStrList*)valptr;
	
	// Calculate the required length: 
	size_t need_len=0;
	for(const RefStrList::Node *i=sl->first(); i; i=i->next)
	{
		need_len+=i->len()+3;  // +2 for `"' +1 for ' '
		if(!i->str())  continue;
		assert(i->stype()!=1);  // must be a '\0'-terminated string
		for(const char *c=i->str(); *c; c++)
		{
			if(*c=='\\' || *c=='\"')
			{  ++need_len;  }
		}
	}
	if(!sl->first())
	{  ++need_len;  }
	
	// Allocate memory if not enough mem was supplied: 
	if(len<need_len)
	{
		dest=ValueHandler::stralloc(need_len);
		if(!dest)  return(dest);
		len=need_len;
	}
	
	// Format the string list into the string: 
	char *d=dest;
	for(const RefStrList::Node *i=sl->first(); i; i=i->next)
	{
		*(d++)='\"';
		if(i->str())
		{
			for(const char *s=i->str(); *s; s++)
			{
				if(*s=='\\' || *s=='\"')  *(d++)='\\';
				*(d++)=*s;
			}
		}
		*(d++)='\"';
		if(i->next)
		{  *(d++)=' ';  }
	}
	*d='\0';
	
	return(dest);
}


// Make them available: 
static StringListValueHandler static_stringlist_handler;
ValueHandler *default_stringlist_handler=&static_stringlist_handler;

}  // end of namespace par


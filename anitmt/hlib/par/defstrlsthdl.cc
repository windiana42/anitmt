/*
 * defstrlsthdl.cc
 * 
 * Implementation of default string list value handler. 
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
#include <hlib/refstrlist.h>

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


int StringListValueHandler::copy(ParamInfo *,void *_dest,void *_src,int operation)
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
							if(tc[1]=='\\' || tc[1]=='\"')
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
				if(arg->origin.otype==ParamArg::FromFile)
				{
					while(*tc && !isspace(*tc) && *tc!='#')  ++tc;
					if(*tc=='#')  *tc='\0';  // Rest of line is comment
				}
				else
				{  while(*tc && !isspace(*tc))  ++tc;  }
				
				if(*tc)
				{
					*(tc++)='\0';
					if(list.append(begin))
					{  rv=PAR::PPSAssFailed;  }
				}
				else
				{
					if(list.append(begin))
					{  rv=PAR::PPSAssFailed;  }
					break;   // success: reached list end
				}
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
		case '\0':  sl->clear();  // fall through
		case '+':   arv=sl->append(&list);  break;
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


char *StringListValueHandler::print(ParamInfo *,void *valptr,
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


#if 0
//##############################################################################

/*
 * conv.cpp
 * 
 * Conversion of strings into values and vice versa. 
 * 
 * This is a part of the aniTMT animation project. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser
 * Bugs, suggestions to wwieser@gmx.de. 
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 * 
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 * 
 * This program is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the 
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * 
 * See the GNU General Public License for details.
 * If you have not received a copy of the GNU General Public License,
 * write to the 
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * Revision History:
 *   Jan 2001   started writing
 *
 */

#include <errno.h>
#include <limits.h>
#include <iostream>
#include "params.hpp"

namespace anitmt
{

inline int String_Value_Converter::is_trim(char c)
{
	if(!c)  return(0);
	return(isspace(int(c)));
}


// Be sure dest[] is at least the size of str. 
// Return value: 
//    true -> OK; 
//    false -> nothing left of strig (*tmp is set to '\0') 
bool String_Value_Converter::trim_str(char *str,char *dest)
{
	*dest='\0';
	while(is_trim(*str))  ++str;
	if(!(*str))  return(false);
	char *end=(str+strlen(str));
	for(;;)
	{
		if((--end)<str)     return(false);
		if(!is_trim(*end))  break;
	}
	++end;
	//char tmp[end-str+1];
	//strncpy(tmp,str,end-str);
	//tmp[end-str]='\0';
	strncpy(dest,str,end-str);
	dest[end-str]='\0';
	return(true);
}


// Modifies str; 
// looks for first appearance of `#', overwrites it with '\0' and 
// trims the string end. 
// Return value: false -> nothing left of strig 
bool String_Value_Converter::simple_find_comment(char *str)
{
	if(comments_in_line)
		for(char *c=str,*last=NULL; *c; c++)
		{
			if(is_trim(*c) || *c=='#')
			{  if(!last)  last=c;  }	
			else
			{  last=NULL;  }
			if(*c=='#')  // Found comment. 
			{
				if(!last)  last=c;
				*last='\0';
				return(last!=str);
			}
		}
	
	return((*str=='\0') ? false : true);
}


// type: 0 -> warning; 1 -> error 
std::ostream &String_Value_Converter::_Prefix(int type)
{
	if(cmd_mode)
	{
		(*errstream) << "Option " << 
			((optname.length()>0 && optname[0]=='[') ? "" : 
				( (optname.length()==1) ? "-" : "--" )) << 
			optname << ": ";
	}
	else
	{
		(*errstream) << filename << ":" << linecnt;
		if(linecnt!=linecnt2)
			(*errstream) << "--" << linecnt2;
		(*errstream) << ": Parameter " << optname << ": ";
	}
	(*errstream) << (type ? "Error" : "Warning");
	return(*errstream);
}


bool String_Value_Converter::Str_To_Value(char *str,std::string &val)
{
	// First, trim (and copy) string: 
	char tmp[strlen(str)+1];
	trim_str(str,tmp);
	
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
			Warning() << ": Unterminated string" << std::endl;
		}
		else if(src[1])
		{
			// src[1] is the char after the terminating `"'. 
			// Check if the rest of the line is a comment: 
			for(++src; *src; src++)
			{
				if(comments_in_line && *src=='#')  break;
				if(!is_trim(*src))
				{
					Error() << ": Garbage at end of string" << std::endl;
					return(false);
				}
			}
		}
	}
	else
	{
		if(!simple_find_comment(tmp))
		{
			// This is an error. If the user wants to explicitly specify 
			// an empty string, he may pass "" as str. 
			Error() << ": Required value omitted "
				"(use \"\" to specify an empty string)" << std::endl;
			return(false);
		}
	}
	
	val.assign(tmp);
	return(true);
}

bool String_Value_Converter::Str_To_Value(char *str,stringlist &val)
{
	// Copy original so that we can restore it in case of an error. 
	stringlist original(val);
	
	// First, trim...
	char tmp[strlen(str)+1];
	if(!trim_str(str,tmp))  return(true);  // OK here -- empty list
	
	// ...then parse...
	char *tc=tmp;
	for(;;)
	{
		// Skip all spacing separating two strings: 
		while(isspace(*tc))  ++tc;
		if(!(*tc))  break;  // found end of list
		if(comments_in_line && *tc=='#')  break;  // rest of line is comment 
		
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
						if(tc[1]=='\\' || tc[1]=='\"')
						{  *(dest++)=*(++tc);  }
						else
						{  *(dest++)=*tc;  }
						break;
					case '\"':
						*dest='\0';
						val.add(begin);
						++tc;
						goto breakfor;  // next string
					case '\0':
						Error() << ": Unexpected end of stringlist: "
							"unterminated string" << std::endl;
						// Restore original string list. 
						val=original;
						return(false);
					default:  *(dest++)=*tc;  break;
				}
			}
			breakfor:;
		}
		else  // non-encapsulated string 
		{
			if(comments_in_line)
			{
				while(*tc && !isspace(*tc) && *tc!='#')  ++tc;
				if(*tc=='#')  *tc='\0';  // Rest of line is comment
			}
			else
			{  while(*tc && !isspace(*tc))  ++tc;  }
			
			if(*tc)
			{  *(tc++)='\0';  val.add(begin);  }
			else
			{  val.add(begin);  break;  }  // success: reached list end
		}
	}
	
	return(true);
}


/******************************************************************************/


std::ostream &String_Value_Converter::Print_Value(std::ostream &os,const std::string &val)
{
	os << "\"";
	int end=val.length();
	for(int i=0;i<end;i++)
	{
		char c=val[i];
		if(c=='\\' || c=='\"')  os << '\\';
		os << c;
	}
	os << "\"";
	return(os);
}

std::ostream &String_Value_Converter::Print_Value(std::ostream &os,const stringlist &val)
{
	if(!val.empty())
	{
		stringlist::const_iterator i=val.begin();
		assert(i!=val.end());
		Print_Value(os,*i);
		for(++i; i!=val.end(); i++)
		{
			os << " ";  // strings separated by space
			Print_Value(os,*i);
		}
	}
	return(os);
}


}  /* end of namespace anitmt */

#endif

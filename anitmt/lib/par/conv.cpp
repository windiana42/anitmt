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
#include "pars.hpp"

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
ostream &String_Value_Converter::_Prefix(int type)
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


String_Value_Converter::String_Value_Converter()
{
	comments_in_line=true;
	errstream=&cerr;
	linecnt=linecnt2=-1;
}

String_Value_Converter::~String_Value_Converter()
{
}


/******************************************************************************/

bool String_Value_Converter::Value_Omitted()
{
	Error() << ": Required value omitted." << endl;
	return(false);
}


// These functions really read in the values: 
// str is read and the result is stored in val. 
// str is trimmed if necessary. 
// Return value: true -> OK; no error
//               false -> error in str; val unchanged. 
bool String_Value_Converter::Str_To_Value(char *str,bool &val,bool silent=false)
{
	// trim str: 
	char tmp[strlen(str)+1];
	trim_str(str,tmp);
	if(!simple_find_comment(tmp))
	{  return(Value_Omitted());  }
	
	if(!strcasecmp(tmp,"yes")  ||
	   !strcasecmp(tmp,"on")   ||
	   !strcasecmp(tmp,"true") ||
	   !strcasecmp(tmp,"1")    )
	{  val=true;  return(true);  }
	if(!strcasecmp(tmp,"no")  ||
	   !strcasecmp(tmp,"off")   ||
	   !strcasecmp(tmp,"false") ||
	   !strcasecmp(tmp,"0")    )
	{  val=false;  return(true);  }
	
	if(!silent)
	{  Error() << " parsing \"" << str << "\" as bool: "
			"invalid argument" << endl;  }
	return(false);
}

bool String_Value_Converter::Str_To_Value(char *str,int &val,bool silent=false)
{
	// trim str: 
	char tmp[strlen(str)+1];
	trim_str(str,tmp);
	if(!simple_find_comment(tmp))
	{  return(Value_Omitted());  }
	
	char *endptr;
	errno=0;
	long v=strtol(tmp,&endptr,/*base=*/0);
	if(*endptr)
	{
		if(!silent)
		{
			Error() << " parsing \"" << tmp << "\" as integer: ";
			if(endptr<=tmp+2)
			{  EStream() << "invalid argument" << endl;  }
			else
			{
				// In this case we could try and use an expression parser. 
				EStream() << "garbage at argument end" << endl;
			}
		}
		return(false);
	}
	
	if(!silent && (errno==ERANGE || v<long(INT_MIN) || v>long(INT_MAX)))
	{
		Warning() << ": Argument \"" << tmp << 
			"\" is out of integer range." << endl;
		//return(false);  well... go on
	}
	
	val=int(v);
	return(true);
}

bool String_Value_Converter::Str_To_Value(char *str,double &val)
{
	// trim str: 
	char tmp[strlen(str)+1];
	trim_str(str,tmp);
	if(!simple_find_comment(tmp))
	{  return(Value_Omitted());  }
	
	char *endptr;
	errno=0;
	double v=strtod(tmp,&endptr);
	if(*endptr)
	{
		Error() << " parsing \"" << tmp << "\" as double: ";
		if(endptr<=tmp+2)
		{  EStream() << "invalid argument" << endl;  }
		else
		{
			// In this case we could try and use an expression parser. 
			EStream() << "garbage at argument end" << endl;
		}
		return(false);
	}
	
	if(errno==ERANGE)
	{
		Warning() << ": Argument \"" << tmp << 
			"\" is out of double range." << endl;
		// return(false);  better go on...
	}
	
	val=v;
	return(true);
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
			Warning() << ": Unterminated string" << endl;
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
					Error() << ": Garbage at end of string" << endl;
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
				"(use \"\" to specify an empty string)" << endl;
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
							"unterminated string" << endl;
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

// for snprintf(): 
#include <stdio.h>

ostream &String_Value_Converter::Print_Value(ostream &os,bool &val)
{
	static const char *onstr="on",*offstr="off";
	os << (val ? onstr : offstr);
	return(os);
}

ostream &String_Value_Converter::Print_Value(ostream &os,int &val)
{
	os << val;
	return(os);
}

ostream &String_Value_Converter::Print_Value(ostream &os,double &val)
{
	char tmp[64];
	// This is great as it writes normal values as you expect it 
	// (e.g. 1.678 or 12398.88099) but large and small values 
	// get written with exponent (starting with e-5 and e+12 here). 
	snprintf(tmp,64,"%.12g",val);
	os << tmp;
	return(os);
}

ostream &String_Value_Converter::Print_Value(ostream &os,std::string &val)
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

ostream &String_Value_Converter::Print_Value(ostream &os,stringlist &val)
{
	if(val.rewind())
	{
		// There is a first element. 
		Print_Value(os,val.current());
		for(;;)
		{
			if(!val.next())  break;
			os << " ";  // strings separated by space
			Print_Value(os,val.current());
		}
	}
	return(os);
}


}  /* end of namespace anitmt */

/*
 * strlist.cpp
 * String list support routines. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser
 * Bugs to wwieser@gmx.de
 * 
 * This is a part of the aniTMT animation project. 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Revision History:
 *   some time in 2001   started writing
 *
 */

#include <string>
#include <iostream>

#ifndef NULL
#define NULL ((void*)0)
#endif


#include <strlist.hpp>
#include <params.hpp>

#include <stdarg.h>


namespace anitmt
{

void stringlist::_addlist(const stringlist &sl)
{
	for(const_iterator i=sl.begin(); i!=sl.end(); i++)
	{  add(*i);  }
}


stringlist::stringlist(const char *str0,...) : // NULL-terminated!!!
	std::list<std::string>() 
{
	if(str0)
	{
		add(str0);
		va_list ap;
		va_start(ap,str0);
		for(;;)
		{
			const char *str=va_arg(ap,const char*);
			if(!str)  break;
			add(str);
		}
		va_end(ap);
	}
}


ostream& operator<<(ostream& os,const stringlist &sl)
{
	#if 0
	for(stringlist::Node *i=sl.first; i; i=i->next)
	{
		os << i->str;
		if(i->next)
			os << " ";
	}
	return(os);
	#else
	return(String_Value_Converter::Print_Value(os,sl));
	#endif
}

}  /* end of namespace anitmt */

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


stringlist::stringlist(const char *str0,...)  // NULL-terminated!!!
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

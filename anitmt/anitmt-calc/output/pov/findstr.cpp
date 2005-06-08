/*
 * findstr.cpp
 * 
 * Find strings in a buffer. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser
 * Bugs to > wwieser -a- gmx -*- de <
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
 *   Apr 2001   started writing
 *
 */


// This includes the config: 
#include "../lib/lproto.hpp"

#include "findstr.hpp"


namespace output_io
{

#if 0
void Find_String::Dump()
{
	for(int i=0; i<256; i++)
	{
		int npr=0;
		for(String_Node *n=hash[i]; n; n=n->next)
		{
			fprintf(stderr,"%s ",n->str);
			++npr;
		}
		if(npr)
		{  fprintf(stderr,"\n");  }
	}
}
#endif

size_t Find_String::Search(char *str,size_t len,RV *rv)
{
	rv->found=NULL;
	rv->id=-1;
	rv->found_len=0;
	rv->hook=NULL;
	rv->ptr=NULL;
	
	char *cend=str+len;
	for(char *c=str; c<cend; c++)
	{
		for(String_Node *sn=hash[_Str2Hash(c)]; sn; sn=sn->next)
		{
			// Compare sn->str against c: 
			char *i=c;  ++i;
			if(sn->len>1)
			{
				for(const char *sns=sn->str+1; i<cend; i++,sns++)
				{
					if(!(*sns))   goto match;
					if(*sns!=*i)  goto nomatch;
				}
				// Oh, the input string is over and we cannot say if f 
				// matches. So, we can not use up all characters now and 
				// have to do things the next time: 
				return(c-str);
			}
			match:;  // Yeah, we found it. 
			rv->found=sn->str;
			rv->found_len=sn->len;
			rv->id=sn->id;
			rv->hook=sn->hook;
			rv->ptr=c;
			return(i-str);
			
			nomatch:;  // not matching, go on
		}
	}
	
	return(len);
}


bool Find_String::Match(char *txt,size_t len,RV *rv)
{
	rv->found=NULL;
	rv->id=-1;
	rv->found_len=0;
	rv->hook=NULL;
	rv->ptr=NULL;
	
	if(!len)  return(false);
	
	for(String_Node *sn=hash[_Str2Hash(txt)]; sn; sn=sn->next)
	{
		if(sn->len>len)  continue;
		for(const char *ca=txt,*cb=sn->str; *cb; ca++,cb++)
		{  if(*ca!=*cb)  goto nomatch;  }
		rv->found=sn->str;
		rv->found_len=sn->len;
		rv->id=sn->id;
		rv->hook=sn->hook;
		rv->ptr=txt+sn->len;
		return(true);
		nomatch:;
	}
	
	return(false);
}


// Call only if n!=top. 
inline void Find_String::_DequeueNotTop(String_Node *n)
{
	if(n->next)  n->next->prev=n->prev;
	if(n->prev)  n->prev->next=n->next;
}

inline void Find_String::_Dequeue(String_Node *n,String_Node **top)
{
	// Dequeue n: 
	if(n==(*top))
	{
		(*top)=n->next;
		if(*top)  (*top)->prev=NULL;
		return;
	}
	_DequeueNotTop(n);
}


// Queue n after prev. 
inline void Find_String::_QueueAfter(String_Node *n,String_Node *prev)
{
	n->next=prev->next;
	n->prev=prev;
	prev->next=n;
	if(n->next)
	{  n->next->prev=n;  }
}


void Find_String::Sort_By_Length()
{
	for(int i=0; i<256; i++)
	{
		String_Node **top=&hash[i];
		if(!(*top))  continue;
		if(!(*top)->next)  continue;
		for(String_Node *_n=(*top)->next; _n; )
		{
			String_Node *n=_n;
			_n=_n->next;
			
			String_Node *i=n->prev;
			if(n->len<=i->len)  continue;
			
			_DequeueNotTop(n);
			for(;;)
			{
				i=i->prev;
				if(!i)
				{
					// Queue n before top. 
					(*top)->prev=n;
					n->next=*top;
					n->prev=NULL;
					*top=n;
					break;
				}
				if(n->len<=i->len)
				{
					// Queue n after i: 
					_QueueAfter(n,i);
					break;
				}
			}
		}
	}
}


bool Find_String::_Add_String(const char *str,int id,const void *hook,
	bool enabled,bool copy)
{
	if(*str=='\0')   return(false);
	String_Node *n=new String_Node(str,id,hook,enabled,copy);
	String_Node **top=&hash[_Str2Hash(str)];
	if(*top)
	{
		// add at the end of *top->next->next...
		#warning could be made faster...
		String_Node *i=*top;
		for(; i->next; i=i->next);  // find the end 
		i->next=n;
		n->prev=i;
	}
	else
	{  *top=n;  }
	return(true);
}


int Find_String::Enable_String(const char *str)
{
	String_Node *n=_Find_Node(str);
	if(!n)  return(1);
	n->enabled=true;
	return(0);
}

int Find_String::Disable_String(const char *str)
{
	String_Node *n=_Find_Node(str);
	if(!n)  return(1);
	n->enabled=false;
	return(0);
}

int Find_String::Delete_String(const char *str)
{
	String_Node *n=_Find_Node(str);
	if(!n)  return(1);
	
	String_Node **top=&hash[_Str2Hash(str)];
	_Dequeue(n,top);
	delete n;
	
	return(0);
}


int Find_String::Enable_ID(int id)
{
	int rv=0;
	for(int i=0; i<256; i++)
	{
		for(String_Node *n=hash[i]; n; n=n->next)
		{
			if(!n->enabled && n->id==id)
			{  n->enabled=true;  ++rv;  }
		}
	}
	return(rv);
}

int Find_String::Disable_ID(int id)
{
	int rv=0;
	for(int i=0; i<256; i++)
	{
		for(String_Node *n=hash[i]; n; n=n->next)
		{
			if(n->enabled && n->id==id)
			{  n->enabled=false;  ++rv;  }
		}
	}
	return(rv);
}


// Find String_Node belonging to string str; 
// Return value is NULL if str is not found. 
Find_String::String_Node *Find_String::_Find_Node(const char *str)
{
	String_Node *first=hash[_Str2Hash(str)];
	if(first)
	{
		// First, compare pointers
		for(String_Node *i=first;;)
		{
			if(i->str==str)  return(i);
			if(!(i=i->next))  break;
		}
		// Next, compare strings bytewise 
		size_t slen=strlen(str);
		for(String_Node *i=first;;)
		{
			if(i->len==slen)
			{
				if(!strcmp(i->str,str))
				{  return(i);  }
			}
			if(!(i=i->next))  break;
		}
	}
	return(NULL);
}


Find_String::String_Node::String_Node(const char *_str,int _id,
	const void *_hook,bool _enabled,bool copy)
{
	prev=NULL;
	next=NULL;
	len=strlen(_str);
	copied=copy;
	if(copy)
	{
		str=new char[len+1];
		strcpy((char*)str,_str);
	}
	else
	{  str=_str;  }
	enabled=_enabled;
	id=_id;
	hook=_hook;
}


Find_String::String_Node::~String_Node()
{
	if(copied && str)
	{  delete[] str;  }
	str=NULL;
}


Find_String::Find_String()
{
	for(int i=0; i<256; i++)
	{  hash[i]=NULL;  }
}


Find_String::~Find_String()
{
	for(int i=0; i<256; i++)
	{
		for(String_Node *n=hash[i]; n; )
		{
			String_Node *tmp=n;
			n=n->next;
			delete tmp;
		}
		hash[i]=NULL;
	}
}


}  // namespace end 


#if 0   /* test program */
#include <stdio.h>

int main()
{
	char *str=
	"Find our little strings in this large one: "
	"#declare is good ; (a >= b) "
	"Donaudampfschiffahrtsgesell";
	
	char *strings[]=
	{ "donau","gesellschaft","lasdh",NULL };
	
	output_io::Find_String find;
	for(int i=0; strings[i]; i++)
	{  find.Add_String(strings[i]);  }
	
	output_io::Find_String::RV rv;
	size_t done=find.Search(str,strlen(str),&rv);
	fprintf(stderr,"done=%d (%d); found \"%s\" at \"%.10s...\" (%.4s)\n",
		done,strlen(str),rv.found,rv.ptr,str+done);
	
	return(0);
}
#endif

#if 0
// Search in str of size len for the first occurence of one of the nf 
// strings passed in f. 
// Don't pass zero-length strings in f. 
// NOTE: The strings are matched in the order they are specified, so 
//       searching for "foo", "foobar" in "void foobar" will return "foo" 
//       as matching. 
// Return value:  number of bytes processed . 
//   = (rv->ptr-str)+strlen(rv->found) if one of the strings was found 
//   < len in case some string matches but the bytes str[len]... are 
//         needed to check if all characters of the sound string match 
//   = len if nothing matches 
size_t Find_String(char *str,size_t len,char **f,int nf,Find_String_RV *rv)
{
	rv->found=NULL;
	rv->ptr=NULL;
	
	unsigned char used[256];
	for(unsigned char *c=used,cend=used+256; c<cend; c++)
	{  *c=0;  }
	
	for(unsigned char **fs=(unsigned char**)f,**fse=&fs[nf]; fs<fse; fs++)
	{  used[**f]=1;  }
	
	char *cend=str+len;
	for(char *c=str; c<cend; c++)
	{
		if(!used[*((unsigned char*)c)])  continue;
		for(char **fs=f,**fse=&fs[nf]; fs<fse; fs++)
		{
			if(**fs!=*c)  continue;
			// Maybe we've found fss=*fs beginning with c. 
			char *fss=*fs,*i;
			for(i=c; i<cend; i++,fss++)
			{
				if(!(*fss))  goto match;
				if(*fss!=c)  goto nomatch;
			}
			// Oh, the input string is over and we cannot say if f 
			// matches. So, we could not use up all characters: 
			return(c-str);
			
			match:;  // Yeah, we found it. 
			rv->found=*fs;
			rv->ptr=c;
			return(i-str);
			
			nomatch:;  // not matching, go on
		}
	}
	
	return(len);
}
#endif

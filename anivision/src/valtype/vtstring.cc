/*
 * valtype/vtstring.cc
 * 
 * String value implementation. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "valtypes.h"
#include <stdarg.h>

#warning "Could clutter this code with assertions if needed..."
// assert(istr->str!=NULL);  // ALWAYS! -> istr may be NULL but not istr->str
// assert(istr->asize<=istr->len);  // even +1 if !=0 (for '\0') 
// assert(strlen(istr->str)==istr->len);  // be careful...


InternalString::InternalString(const char *src1,const char *src2)
{
	int len1=strlen(src1);
	int len2=strlen(src2);
	len=len1+len2;
	asize=len+1;
	str=ALLOC<char>(asize);
	memcpy(str,src1,len1);
	memcpy(str+len1,src2,len2+1);
	refcnt=0;
}


bool String::operator==(const String &b) const
{
	if(istr==b.istr)  return(1);
	bool we_are_null=(!istr || !istr->str);
	bool he_is_null=(!b.istr || !b.istr->str);
	if(we_are_null && he_is_null)  return(1);
	if(we_are_null || he_is_null)  return(0);
	if(istr->len!=b.istr->len)  return(0);
	return(!strcmp(istr->str,b.istr->str));
}


// This replaces the string *this by string b; replacement 
// begins at position idx in *this. Returns 0 on success and 
// 1 if idx is out of range (<0 or >length).
int String::replace_idx(int idx,const String &b)
{
	if(!b.istr || !b.istr->len)  return(0);
	if(!istr)
	{
		if(!idx)
		{  set(b);  return(0);  }
		return(1);
	}
	if(idx<0)  return(1);
	if(idx>=istr->len)
	{
		if(idx==istr->len)
		{  append(b);  return(0);  }
		return(1);
	}
	if(idx+b.istr->len>istr->len)
	{
		int need=idx+b.istr->len+1;
		if(istr->refcnt==1)
		{
			if(istr->asize<need)
			{  istr->realloc(need+(need>=64 ? 32 : need/2));  }
			memcpy(istr->str+idx,b.istr->str,b.istr->len);
			istr->len=idx+b.istr->len;
			istr->str[istr->len]='\0';
		}
		else
		{
			InternalString *ns=new InternalString(need);
			memcpy(ns->str,istr->str,idx);
			memcpy(ns->str+idx,b.istr->str,b.istr->len);
			ns->len=idx+b.istr->len;
			ns->str[ns->len]='\0';
			istr->deref();
			istr=ns;
			istr->aqref();
		}
	}
	else
	{
		if(istr->refcnt==1)
		{  memcpy(istr->str+idx,b.istr->str,b.istr->len);  }
		else
		{
			InternalString *ns=new InternalString(istr->len+1);
			ns->len=istr->len;
			memcpy(ns->str,istr->str,idx);
			memcpy(ns->str+idx,b.istr->str,b.istr->len);
			memcpy(ns->str+idx+b.istr->len,istr->str+idx+b.istr->len,
				istr->len-idx-b.istr->len);
			ns->str[ns->len]='\0';
			istr->deref();
			istr=ns;
			istr->aqref();
		}
	}
	//fprintf(stderr,">>%s<<---------",istr->str);
	return(0);
}


void String::append(const String &b)
{
	if(!b.istr)  return;
	if(!istr)
	{  set(b);  return;  }
	int need=istr->len+b.istr->len+1;
	if(istr->refcnt==1)
	{
		if(istr->asize<need)
		{  istr->realloc(need+need/2);  }
		memcpy(istr->str+istr->len,b.istr->str,b.istr->len+1);
		istr->len+=b.istr->len;
	}
	else
	{
		InternalString *ns=new InternalString(istr->str,b.istr->str);
		istr->deref();
		istr=ns;
		istr->aqref();
	}
}

void String::append(const char *b)
{
	if(!b)  return;
	if(!istr)
	{  set(b);  return;  }
	int blen=strlen(b);
	int need=istr->len+blen+1;
	if(istr->refcnt==1)
	{
		if(istr->asize<need)
		{  istr->realloc(need+need/2);  }
		memcpy(istr->str+istr->len,b,blen+1);
		istr->len+=blen;
	}
	else
	{
		InternalString *ns=new InternalString(need);
		memcpy(ns->str,istr->str,istr->len);
		memcpy(ns->str+istr->len,b,blen+1);
		ns->len=need-1;
		istr->deref();
		istr=ns;
		istr->aqref();
	}
}

void String::append(const char *b,int len)
{
	if(!b)  return;
	int _blen=strlen(b);
	if(len>_blen)
	{  len=_blen;  }
	if(!istr)
	{
		istr=new InternalString(len+1);
		memcpy(istr->str,b,len);
		istr->str[len]='\0';
		istr->len=len;
		istr->aqref();
		return;
	}
	int need=istr->len+len+1;
	if(istr->refcnt==1)
	{
		if(istr->asize<need)
		{  istr->realloc(need+need/2);  }
		memcpy(istr->str+istr->len,b,len+1);
		istr->len+=len;
	}
	else
	{
		InternalString *ns=new InternalString(need);
		memcpy(ns->str,istr->str,istr->len);
		memcpy(ns->str+istr->len,b,len+1);
		ns->len=need-1;
		istr->deref();
		istr=ns;
		istr->aqref();
	}
}


void String::prepend(const String &b)
{
	if(!b.istr)  return;
	if(!istr)
	{  set(b);  return;  }
	int need=istr->len+b.istr->len+1;
	if(istr->refcnt==1)
	{
		if(istr->asize<need)
		{  istr->realloc(need+need/2);  }
		memmove(istr->str+b.istr->len,istr->str,istr->len+1);
		memcpy(istr->str,b.istr->str,b.istr->len);
		istr->len+=b.istr->len;
	}
	else
	{
		InternalString *ns=new InternalString(b.istr->str,istr->str);
		istr->deref();
		istr=ns;
		istr->aqref();
	}
}

void String::prepend(const char *b)
{
	if(!b)  return;
	if(!istr)
	{  set(b);  return;  }
	int blen=strlen(b);
	int need=istr->len+blen+1;
	if(istr->refcnt==1)
	{
		if(istr->asize<need)
		{  istr->realloc(need+need/2);  }
		memmove(istr->str+blen,istr->str,istr->len+1);
		memcpy(istr->str,b,blen);
		istr->len+=blen;
	}
	else
	{
		InternalString *ns=new InternalString(need);
		memcpy(ns->str,b,blen);
		memcpy(ns->str+blen,istr->str,istr->len+1);
		ns->len=need-1;
		istr->deref();
		istr=ns;
		istr->aqref();
	}
}


String operator+(const String &a,const String &b)
{
	if(!a)  return(String(b));
	if(!b)  return(String(a));
	int need=a.len()+b.len()+1;
	String s(need);
	memcpy(s.istr->str,a.istr->str,a.istr->len);
	memcpy(s.istr->str+a.istr->len,b.istr->str,b.istr->len+1);
	s.istr->len=need-1;
	return(s);
}

String operator+(const char *a,const String &b)
{
	if(!a)  return(String(b));
	String tmp=b;
	tmp.prepend(a);
	return(tmp);
}

String operator+(const String &a,const char *b)
{
	if(!b)  return(String(a));
	String tmp=a;
	tmp.append(b);
	return(tmp);
}


void String::trunc(int len)
{
	if(!istr)  return;
	if(istr->len<=len)  return;
	if(istr->refcnt==1)
	{
		istr->len=len;
		istr->str[istr->len]='\0';
		#warning "could re-allocate..."
	}
	else
	{
		InternalString *ns=new InternalString(len+1);
		memcpy(ns->str,istr->str,len);
		ns->str[len]='\0';
		ns->len=len;
		istr->deref();
		istr=ns;
		istr->aqref();
	}
}

void String::skip(int len)
{
	if(!istr)  return;
	if(istr->refcnt==1)
	{
		if(istr->len<=len)
		{
			istr->len=0;
			istr->str[0]='\0';
		}
		else
		{
			istr->len-=len;
			memmove(istr->str,istr->str+len,istr->len+1);
		}
		#warning "could re-allocate..."
	}
	else
	{
		int remain=istr->len-len;
		if(remain<0)  remain=0;
		InternalString *ns=new InternalString(remain+1);
		if(remain)
		{  memcpy(ns->str,istr->str+len,remain+1);  }
		ns->str[remain]='\0';
		ns->len=remain;
		istr->deref();
		istr=ns;
		istr->aqref();
	}
}


void String::sprintf(const char *fmt,...)
{
	// Based on HLib's RefString::sprintf() written by me. 
	
	// Get rid of old stuff: 
	if(istr) istr->deref();
	istr=NULL;
	
	// First, we must get the length of the string in fmt,...
	const int prealloc=64;
	char buf[prealloc];
	va_list ap;
	va_start(ap,fmt);
	int rv=vsnprintf(buf,prealloc,fmt,ap);
	// If this assertion fails, then the vsnprintf() is not C99 conform. 
	str_assert(rv>=0);
	va_end(ap);
	
	size_t dlen=rv;  // (without trailing '\0')
	// The length of the string in fmt,... is dlen. 
	// First, allocate the needed bytes: 
	istr=new InternalString(dlen+1);  // +1 for terminating '\0'
	istr->aqref();
	istr->len=dlen;
	if(rv<prealloc) // Whole string fits into prealloc buf. Simply copy. 
	{  strcpy(istr->str,buf);  }
	else
	{
		// Must re-format the string. 
		va_start(ap,fmt);
		int rv2=vsnprintf(istr->str,istr->len+1,fmt,ap);
		str_assert(rv==rv2);
		va_end(ap);
	}
}

/*
 * refstringw.cc 
 * 
 * Implementation of allocate once & reference string class RefString. 
 * This file contains the string manipulation routines offered by 
 * RefString. 
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include "refstring.h"
#include <string.h>

#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on
#include <assert.h>
#else
#define assert(x) do{}while(0)
#endif


// Re-allocate the string buffer. If this instance is not the only 
// holder of the reference, a new reference (including newlen bytes 
// for data) is allocated (-> detach) and data is copied if needed 
// (see wcopy). If this instance is the only holder, a simple 
// realloc is performed if needed and if the ref is a NULL-ref, 
// a new one is allocated. 
// sflag: Specifies if the size flag is needed (0 or 1). 
//        If a size flag is needed and the current string has none 
//        set, then the string is converted into one with size flag. 
//        Otherways, the size flag value is unchanged. 
// wcopy: 
//   0 -> don't preserve content of string [do not copy on detach]
//  -1 -> preserve content of string and keep it at the beginning
//        (allow to append data)
//  +1 -> just as -1, but keep string at the end (to allow to 
//        prepend data)
// '\0'-terminated strings stay '\0'-terminated. 
// Return value: 
//  0 -> success
// -1 -> allocation failed (ref now NULL)
int RefString::_realloc(size_t newlen,int sflag,int wcopy)
{
	// Make sure we're the only user of this reference: 
	// OLD: if(_detach())
	// OLD: {  return(-1);  }
	// For more performance, a manual detach is done: 
	int deref_again_after=0;
	if(!ref)  // NULL reference
	{
		// Allocate it new: 
		ref=_alloc(newlen,sflag);
		if(!ref)  return(-1);
	}
	else if(*ref<=3)  // We're the only user. 
	{
		// Compute current data length: 
		size_t curdlen=(_sflag() ? _rlength() : strlen(_str())+1);
		if(_sflag()==sflag && newlen==curdlen)
		{  return(0);  }  // nothing to do 
		// Obviously, we have to re-allocate the ref: 
		sflag=(sflag || _sflag());
		// Fast path: 
		if(sflag==_sflag() && wcopy<=0)
		{
			char *dataptr=(char*)ref;
			if(sflag)  dataptr-=sizeof(size_t);
			dataptr=(char*)LRealloc(dataptr,
				newlen+sizeof(*ref)+(sflag ? sizeof(size_t) : 0));
			if(!dataptr)
			{
				// Allocation failed. As the we must then set the reference 
				// to NULL, we have to free it: 
				_deref();
				return(-1);
			}
			if(sflag)
			{
				*((size_t*)dataptr)=newlen;
				dataptr+=sizeof(size_t);
			}
			else if(newlen<curdlen)
			{  // make sure new string is still '\0'-terminated
				assert(newlen);
				*(dataptr+sizeof(*ref)+newlen-1)='\0';
			}
			// *ref stays const (2 or 3). 
			ref=(int*)dataptr;
		}
		else // slow path
		{
			// Allocate new ref, copy stuff, free old one. 
			// This is a bit ugly, as I simply add a reference, 
			// go into the next brach and then delete that ref 
			// again. 
			_addref();
			deref_again_after=1;
			goto alloc_and_copy;
		}
	}
	else  // ref used by others, too. 
	{
		alloc_and_copy:;
		// First, we have to dereference: 
		sflag=(sflag || _sflag());
		char *olddata=_str();
		size_t oldlen=(_sflag() ? _rlength() : (strlen(olddata)+1));
		_deref();   // ref now NULL. 
		// olddata stays valid as we're not the only holder of the ref. 
		// THIS IS A RACE CONDITION IN CASE OF MULTITHREADING. 
		
		// Then, we have to allocate memory and copy the data 
		// if needed. 
		ref=_alloc(newlen,sflag);
		if(!ref)
		{
			if(deref_again_after)  _deref();
			return(-1);
		}
		// ref counter is now set up to 1 ref (2 or 3 depending 
		// on sflag). 
		if(wcopy)   // must copy content
		{
			size_t copylen,delta;
			if(oldlen<newlen)
			{  copylen=oldlen;  delta=newlen-oldlen;  }
			else
			{  copylen=newlen;  delta=oldlen-newlen;  }
			if(wcopy<0)  // keep content at beginning 
			{
				memcpy(_str(),olddata,copylen);
				if(!sflag && oldlen>newlen)  // make sure the string is terminated
				{
					assert(newlen);
					*(_str()+newlen-1)='\0';
				}
			}
			else  // wcopy>0
			{
				memcpy(
					_str()  + ((oldlen<newlen) ? delta : 0),
					olddata + ((oldlen>newlen) ? delta : 0),
					copylen);
			}
		}
		// Data now copied. 
		if(deref_again_after)  _deref();
	}
	return(0);
}


int RefString::chtype(int sflag)
{
	if(!ref)
	{  return(0);  }
	if(sflag)
	{
		// Convert to data string with size field. 
		if(_sflag())
		{  return(0);  }
		// '\0'-termination gets removed: 
		return(_realloc(strlen(_str()),1,-1));
	}
	else
	{
		// Convert to normal '\0'-terminated string. 
		if(!_sflag())
		{  return(0);  }
		size_t rlen=_rlength();
		char *c=_str();
		// Last char may be '\0'. 
		for(char *cend=(c+rlen)-1; c<cend; c++)
		{
			if(*c=='\0')
			{  return(-2);  }
		}
		// Simply calling _realloc() won't work as _realloc() does 
		// not convert a sflag string into a !sflag one. 
		int no_nul=((*c=='\0') ? 0 : 1);
		int *newref=_alloc(rlen+no_nul,0);
		if(!newref)
		{  _deref();  return(-1);  }
		memcpy(newref+1,_str(),rlen);
		_deref();
		ref=newref;
		// Append '\0'-char if not already there: 
		if(no_nul)
		{  *(_str()+rlen)='\0';  }
	}
	return(0);
}


// Check if sflag error has to be returned by function call 
// with data length spec. 
//inline int RefString::_check_sflag()
//{  return(!ref ? 0 : (_sflag() ? 0 : 1));  }


// Prepend/append string: 
// pre_app=-1 -> append
// pre_app=+1 -> prepend
int RefString::_pre_app_end(const char *str,size_t len,int pre_app)
{
	if(!str)  return(0);
	
	if(!ref)
	{  return(_copy(str,&len));  }
	if(!_sflag())
	{  return(-2);  }
	size_t curlen=_rlength();
	if(_realloc(curlen+len,1,pre_app))
	{  return(-1);  }
	memcpy(_str()+((pre_app<0) ? curlen : 0),str,len);
	return(0);
}

int RefString::_pre_app_end(const char *str,int pre_app)
{
	if(!str)  return(0);
	
	if(!ref)
	{  return(_copy(str,NULL));  }
	size_t addlen=strlen(str);
	if(_sflag())  // Do not copy terminating '\0'-char. 
	{  return(_pre_app_end(str,addlen,pre_app));  }
	size_t curlen=strlen(_str());
	if(_realloc(curlen+addlen+1,0,pre_app))
	{  return(-1);  }
	memcpy(_str()+((pre_app<0) ? curlen : 0),str,addlen);
	_str()[curlen+addlen]='\0';
	return(0);
}


// Trunc/skip: tr_sk=-1 -> trunc; tr_sk=+1 -> skip
int RefString::_trunc_skip(size_t len,int tr_sk)
{
	if(!ref)  return(0);
	
	if(_sflag())
	{
		size_t curlen=_rlength();
		if((tr_sk<0 && curlen<=len) ||   // trunc
		   (tr_sk>0 && !len))   // skip
		{  return(0);  }  // nothing to do 
		size_t newlen = (tr_sk<0) ? len : (len>=curlen ? 0 : curlen-len);
		if(_realloc(newlen,1,tr_sk))
		{  return(-1);  }
	}
	else
	{
		size_t curlen=strlen(_str()),newlen;
		if(tr_sk<0)  // trunc
		{
			if(curlen<=len)  // nothing to do
			{  return(0);  }
			newlen=len;
		}
		else  // skip
		{
			if(!len)  // nothing to do
			{  return(0);  }
			newlen = (curlen<=len) ? 0 : (curlen-len);
		}
		if(_realloc(newlen+1,0,tr_sk))
		{  return(-1);  }
	}
	return(0);
}

/*
 * valuehandler.h
 * 
 * Header file containing value handler abstraction. 
 * This is the file to be included by the user. 
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

// This is the file to be included by the user. 

#ifndef _INC_PAR_ValueHandlers_H_
#define _INC_PAR_ValueHandlers_H_ 1
// ### HLIB notes: 
// ### dont use ValueHandlerT (op new...)

#include "valhdl.h"


namespace par
{

#ifdef ParNeed_ValueHandlerT
// Description: see struct ValueHandler. 
// Use the capitalized versions instead of the ones inherited 
// by ValueHandler. 
template<class T> class ValueHandlerT : ValueHandler
{
	public:
		ValueHandlerT() { }
		virtual ~ValueHandlerT() { }
		
		virtual T *Alloc(ParamInfo *)
			{  return(new T);  }
		virtual void Free(ParamInfo *,T *val)
			{  if(val)  delete val;  }
		virtual int Copy(ParamInfo *,T *dest,const T *src,int op)
			{  if(op!=SOPCopy) return(-2); *dest = *src;  return(0);  }
		virtual	ParParseState Parse(ParamInfo *pi,T *val,ParamArg *pa)=0;
		virtual char *Print(ParamInfo *pi,const T *val,char *dest,size_t len)=0;
		
	private:
		// All these wrappers...
		virtual void *alloc(ParamInfo *pi)
			{  return(Alloc(pi));  }
		virtual void free(ParamInfo *pi,void *val)
			{  Free(pi,(T*)val);  }
		virtual int copy(ParamInfo *pi,void *dest,const void *src,int op)
			{  return(Copy(pi,(T*)dest,(T*)src,op));  }
		virtual	ParParseState parse(ParamInfo *pi,void *val,ParamArg *pa)
			{  return(Parse(pi,(T*)val,pa));  }
		virtual	char *print(ParamInfo *pi,const void *val,char *dest,size_t len)
			{  return(Print(pi,(T*)val,dest,len));  }
};
#endif  /* ParNeed_ValueHandlerT */


namespace internal
{

template<class T> PAR::ParParseState simple_parse(
	PAR::ParamInfo *pi,T *valptr,ParamArg *arg);
// Specialisation of the function above for type bool (used for switch args): 
extern PAR::ParParseState simple_parse(
	PAR::ParamInfo *,bool *valptr,ParamArg *arg);

extern char *templ_val2str(long val,char *dest,size_t len);
extern char *templ_val2str(int val,char *dest,size_t len);
extern char *templ_val2str(unsigned long val,char *dest,size_t len);
extern char *templ_val2str(unsigned int val,char *dest,size_t len);
extern char *templ_val2str(float val,char *dest,size_t len);
extern char *templ_val2str(double val,char *dest,size_t len);
extern char *templ_val2str(bool val,char *dest,size_t len);

extern bool ValueInNextArg(ParamArg *arg);

}  // end of namespace internal


// Simple value handlers for simple types like int, long, float, double. 
template<class T>struct SimpleValueHandler : ValueHandler
{
	SimpleValueHandler(int *failflag=NULL) : ValueHandler(failflag)  { }
	~SimpleValueHandler()  { }
	
	size_t cpsize()            // May be copied via memcpy(); does not 
	{  return(sizeof(T));  }   // need construction or destruction. 
	
	ParParseState parse(ParamInfo *pi,void *val,ParamArg *arg)
	{  return(internal::simple_parse(pi,(T*)val,arg));  }
	
	char *print(ParamInfo *,const void *val,char *dest,size_t len)
	{  return(internal::templ_val2str(*(T*)(val),dest,len));  }
};


// Value handler for options (this is mostly a do-nothing wrapper...)
struct OptionValueHandler : ValueHandler
{
	OptionValueHandler(int *failflag=NULL) : ValueHandler(failflag)  { }
	~OptionValueHandler()  { }
	
	size_t cpsize()             // May be copied via memcpy(); does not 
	{  return(sizeof(int));  }  // need construction or destruction. 
	
	ParParseState parse(ParamInfo *,void *val,ParamArg *pa);
	
	char *print(ParamInfo *pi,const void *val,char *dest,size_t len);
};


// Value handler used to convert strings to integers, e.g. 
// ignore -> 0; warning -> 1; error -> 2. 
// The strings to convert are stored in an array of type MapEntry. 
// NOTE: The last element of that array MUST contain str=NULL. 
struct EnumValueHandler : ValueHandler
{
	struct MapEntry
	{
		int val;
		const char *str;
	};
	
	// map: array of string - value pairs; last element with str=NULL. 
	EnumValueHandler(const MapEntry *_map,int *failflag=NULL) : 
		ValueHandler(failflag)  {  map=_map;  }
	~EnumValueHandler()
		{  map=NULL;  }
	
	size_t cpsize()              // May be copied via memcpy(); does not 
	{  return(sizeof(int));  }   // need construction or destruction. 
	
	ParParseState parse(ParamInfo *pi,void *val,ParamArg *arg);
	char *print(ParamInfo *,const void *val,char *dest,size_t len);
	
	private:
		const MapEntry *map;  // NULL or array; last elem with str=NULL. 
};


// Using RefString as underlaying string implementation. 
struct StringValueHandler : ValueHandler
{
	StringValueHandler(int *failflag=NULL) : ValueHandler(failflag) {}
	~StringValueHandler() {}
	
	void *alloc(ParamInfo *pi);
	void free(ParamInfo *pi,void *val);
	
	// Note: SOPCopy -> copy string
	//       SOPAdd ->  append string
	//       SOPSub ->  prepend string
	int copy(ParamInfo *,void *dest,const void *src,int operation = PAR::SOPCopy);
	
	ParParseState parse(ParamInfo *pi,void *val,ParamArg *pa);
	char *print(ParamInfo *pi,const void *val,char *dest,size_t len);
};


// Using RefStrList as underlaying string list implementation. 
struct StringListValueHandler : ValueHandler
{
	StringListValueHandler(int *failflag=NULL) : ValueHandler(failflag) {}
	~StringListValueHandler() {}
	
	void *alloc(ParamInfo *pi);
	void free(ParamInfo *pi,void *val);
	
	// Note: SOPCopy -> copy list
	//       SOPAdd ->  append list
	//       SOPSub ->  remove entries from list
	int copy(ParamInfo *,void *dest,const void *src,int operation = PAR::SOPCopy);
	
	ParParseState parse(ParamInfo *pi,void *val,ParamArg *pa);
	char *print(ParamInfo *pi,const void *val,char *dest,size_t len);
};




// These value handlers already exist (pointers to static structs): 
// They are used e.g. by ParameterConsumer_Overloaded and are the 
// default value handlers. 
extern ValueHandler *default_int_handler;
extern ValueHandler *default_uint_handler;
extern ValueHandler *default_long_handler;
extern ValueHandler *default_ulong_handler;
extern ValueHandler *default_float_handler;
extern ValueHandler *default_double_handler;
extern ValueHandler *default_switch_handler;
extern ValueHandler *default_option_handler;
extern ValueHandler *default_string_handler;
extern ValueHandler *default_stringlist_handler;

}  // end of namespace par

#endif  /* _INC_PAR_ValueHandlers_H_ */

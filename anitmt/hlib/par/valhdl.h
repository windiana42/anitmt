/*
 * valhdl.h
 * 
 * Value handler abstraction - internally used header. 
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

#ifndef _INC_PAR_ValHandler_H_
#define _INC_PAR_ValHandler_H_ 1

#include <hlib/prototypes.h>
#include <hlib/linkedlist.h>

#include "cmdline.h"

#include <stddef.h>

#include "parintrnl.h"

namespace par
{

// Value handler (pure C++ fans should have a look at 
// the ValueHandlerT template.)
// One value handler is needed for each different value type. 
// So, you will normally need only one ValueHandler for double, 
// one for unsigned long, etc. 
struct ValueHandler : public PAR
{  _CPP_OPERATORS_FF
	ValueHandler(int * /*failflag*/=NULL) { }
	virtual ~ValueHandler() { }
	
	// Allocate and construct a new value returning NULL 
	// if allocation or construction failed: 
	// Default implementation: LMalloc(cpsize()). 
	virtual void *alloc(ParamInfo *);
	
	// Just the opposite of alloc() above; used to properly 
	// destroy and free the passed value: 
	// Default implementation: LFree(). 
	virtual void free(ParamInfo *,void * /*val*/);
	
	// dest and src both point to variables; the function must 
	// modify dest with respect to src and operation. 
	// Operation may have different values, the most basic one is 
	// * SOPCopy which means ``Copy src to dest'' 
	//   (i.e. *(T*)dest=*(T*)src). 
	//   This operation MUST be implemented. 
	//   The copy routine should overwrite the value in dest. 
	// * Other operations may be implemented. 
	// Return value: 
	//   0 -> OK; !=0 -> failed. 
	//  -1 -> allocation failure 
	//  -2 -> operation (!=PAR::SOPCopy) not supported
	// else -> something else failed (your implementation of 
	//         ParameterSource::ParamCopyError() should be 
	//         able to deal with it; use negative values). 
	// Default implementation: 
	//    if(operation!=PAR::SOPCopy)  return(-2);
	//    if(!cpsize())  return(1)  
	//    memcpy(dest,src,cpsize());  return(0). 
	virtual int copy(ParamInfo *,void * /*dest*/,void * /*src*/,
		int /*operation*/ = PAR::SOPCopy);
	
	// In case this handler handles simple values like int/double/etc 
	// which can be savely copied via memcpy(), this returns the size 
	// of the object. If the handler handles values which have to be 
	// constructed instead of copied, cpsize() must return 0 so that 
	// alloc()/copy()/free() (see above) must be used. 
	// Default implementation: return(0). 
	virtual size_t cpsize();
	
	// Parse in the passed argument and store it in *val. 
	// This function has to parse in the argument passed as 
	// ParamArg* and store it under val. val points to some 
	// allocated memory which is large enough to keep the 
	// object of the type this ValueHandler is dealing with. 
	// In case cpsize() returns !=0, the opject referred to 
	// by val is simply allocated via malloc, if it returns 0, 
	// the object is allocated via ValueHandler::alloc() 
	// and thus set up correctly. 
	// The parsing function must take care of the assignment 
	// operator in ParamArg::assmode and return PPSIllegalAssMode 
	// if it does not support that mode. 
	// (assmode='\0' -> `='; assmode='+' -> `+='; etc.) 
	// The parsing function should noch change anything in 
	// the ParamArg (not even pdone). 
	// The ParamInfo* passes the info structure associated with 
	// the passed ParamArg in case the parsing function needs 
	// this info. 
	// Return value: 
	//  <0 -> warning 
	//   0 -> success
	//  >0 -> error 
	// See enum ParParseState for a list of possible error 
	// codes. You may also return custom error codes as longe 
	// as your custom handlers can deal with them. 
	// Default implementation: -pure virtual-
	virtual	ParParseState parse(ParamInfo *,void * /*val*/,
		ParamArg *) HL_PureVirt(PPSIllegalArg)
	
	// Helper for print(): 
	static char *stralloc(size_t len);
	static void strfree(char *str);
	
	// Used to write the value of the passed val to a string. 
	// In case the string representation of the value fits into 
	// dest of size len, it is stored there ('\0'-terminated) 
	// and dest is returned. 
	// In case the string representation is longer than len, 
	// a new one is allocated via ValueHandler::stralloc() and 
	// returned. In case stralloc() fails, NULL is returned. 
	// Default implementation: -pure virtual-
	virtual char *print(ParamInfo *,void * /*val*/,
		char * /*dest*/,size_t /*len*/) HL_PureVirt(NULL)
};

}  // namespace end

#endif  /* _INC_PAR_ValHandler_H_ */

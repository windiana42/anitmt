/*
 * ilfcall.hpp
 * 
 * Function caller class for input lib / exparse. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _NS_ExParse_FunctionCaller_HPP_
#define _NS_ExParse_FunctionCaller_HPP_ 1

#include "function.hpp"

namespace exparse
{

// Use this class to call functions like sin, cos, ...
class FunctionCaller
{
	private:
		FunctionArgs fa;
		
		inline void _SetFunction(const FunctionDesc *fd);
	public:
		FunctionCaller();
		FunctionCaller(const FunctionDesc *fdesc,Value *result);
		~FunctionCaller();
		
		//! These functions feturn values set by one of the SetXX() 
		//! functions below. 
		Value *result()  {  return(fa.result);  }
		//! Function name and description (or NULL): 
		const char *name()  {  return(fa.fdesc ? fa.fdesc->name : NULL);  }
		const FunctionDesc *desc()  {  return(fa.fdesc);  }
		//! How many args are currently set: 
		int nargs()  {  return(fa.nargs);  }
		
		//! Best way to call a function: 
		//! SetFunction() -> SetResult() -> AddArg() -> Compute()
		//! (The first two can be done via constructor.) 
		//! NOTE that SetFunction() resets all values (the result 
		//!      and the args), so you have to call SetResult()/AddArg() 
		//!      each time you SetFunction(). 
		//! This class never allocates or deletes values. That's up to you. 
		
		//! Set the function to be called. Either you pass the 
		//! description directly or a name. The name will get 
		//! looked up (binary search); the function returns 0 on 
		//! success and 1 on failure (function not found). 
		int SetFunction(const char *name);
		void SetFunction(const FunctionDesc *fd);
		
		//! Specify the value where to store the result: 
		void SetResult(Value *val)
			{  fa.result=val;  }
		
		//! Add an argument for the function. They are passed down 
		//! to the function in the order they are added here. 
		//! Return value: 
		//!  FSSuccess     -> OK
		//!  FSTooManyArgs -> Attempt to add too many args; function 
		//!                 does not support that many. 
		//!  FSNoFunction  -> No function set (call SetFunction()). 
		FunctionState AddArg(const Value *arg);
		
		//! Finally compute the function storing the result in the 
		//! passed pointer (if successful). 
		//! See FunctionState for possible return values. 
		FunctionState Compute()
			{  return(CallFunction(&fa));  }
		
		//! Calls operator delete on all specified args (not result). 
		void FreeArgs();
};

}  // namespace end 

#endif  /* _NS_ExParse_FunctionCaller_HPP_ */

/*
 * ani-parser/opfunc.h
 * 
 * Simple operator function - specific header. 
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

#ifndef _ANIVISION_ANI_OPFUNC_H_
#define _ANIVISION_ANI_OPFUNC_H_ 1

#include "treenode.h"
#include "exprvalue.h"


namespace ANI
{

class ExprValueType;
class ExprValue;

// Argument struct for operator function: 
struct OpFunc_Args
{
	// Number of arguments
	int nargs;
	// Array [nargs] of arguments: 
	const ExprValue *_args;
	// Store result here: 
	ExprValue *res;
	
	// Recommended access function (allows for checks if needed): 
	const ExprValue *arg(int i)
		{  return(&_args[i]);  }
	// Shortcut for POD rvalues (RVALUES only). 
	inline const Value &pod3(int i);   // NOTE: i=0,1,2 ONLY. 
	inline const Value &podU(int i,int use);  // NOTE: use=0,1,2 ONLY. 
};


// Init and cleanup some static vars: 
extern void OpFunc_InitStaticVals();
extern void OpFunc_CleanupStaticVals();

// Pointer to operator function. 
// Return value: 
//     0 -> OK
//  else -> error
typedef int (*OpFunc_Ptr)(OpFunc_Args *);


// Primary function: 
// Returns operation function for passed operator (OperatorFuncID) 
// which takes nargs arguments of type argtypes[nargs]. 
// If such a function exists, non-NULL is returned and the result 
//   type is stored in result_type. 
// Otherwise NULL is returned -- do not assume anything about 
//   result_type in that case. 
extern OpFunc_Ptr LookupOperatorFunction(OperatorFuncID of_id,
	int nargs,const ExprValueType *argtypes,
	ExprValueType *result_type);

}  // end of namespace ANI

#endif  /* _ANIVISION_ANI_OPFUNC_H_ */

/*
 * function.hpp
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

#ifndef _NS_ExParse_Function_HPP_
#define _NS_ExParse_Function_HPP_ 1

#include "value.hpp"

namespace exparse
{

struct FunctionDesc;

struct FunctionArgs /*: Error_Reporter*/
{
	Value *result;  //! where to store the result
	int nargs;      //! number of args 
	const Value **args;   //! args to the function (Value *args[nargs])
	const FunctionDesc *fdesc;  //! corresponding function entry 
};

enum FunctionState
{
	FSSuccess=0,    //! success
	FSNoFunction,   //! no function was set 
	FSNoResult,     //! result pointer is NULL (call SetResult())
	FSTooManyArgs,  //! too many args
	FSWrongArgNo,   //! wrong number of args
	FSIllegalArg,   //! at least one argument was of wrong type
	FSOutOfRange    //! at least one arg was out of range for the op. 
};

struct FunctionDesc
{
	const char *name;           //! name of function ("sin",...)
	int (*check_nargs)(int n);  //! checks if it is okay to pass n 
	                            //! args to the function (see above) 
	FunctionState (*func)(FunctionArgs *fa);  //! computes the func 
};

enum OperatorType
{
	// Don't forget to add an entry for Operator2String(). 
	OTValue=0,           // \    (starting with value 0)
	OTFunction,          //  >- special ones _______ 
	OTBrOp,OTBrCl,       // /  (, )    _____/ The rest is sorted by precedence: 
	OTDot,               // a.b
	OTNot,OTPos,OTNeg,   // !a, +a, -a
	OTPow,               // a^b
	OTMul,OTDiv,         // a*b, a/b
	OTAdd,OTSub,         // a+b, a-b
	OTLE,OTGE,OTL,OTG,   // <=, >=, <, >
	OTEq,OTNEq,          // ==, !=
	OTAnd,               // &&
	OTOr,                // ||
	OTCondQ,OTCondC,     // a?b:c  (`?' -> query; `:' -> choose)
	//OTAssign
	OTSep,               // ,
	_OTLast              // MUST BE THE LAST ELEMENT
};

struct OperatorDesc
{
	const char *name;   //! string representation
	int precedence;     //! operator precedence (>=0; larger -> higher)
	int noperands;      //! number of operands 
	int binddir;        //! +1 -> binds left-to-right; 0 -> irrelevant; 
	                    //! -1 -> binds right-to-left
	const FunctionDesc *fdesc;  //! corresponding function desc 
};

//! Looks up function using binary search and returns NULL 
//! if not found. 
//! Note: Binary search is about twice as fast as stupid linear 
//!       search when I look up all 40 functions and 5 unknown 
//!       ones. You don't gain too much. 
extern const FunctionDesc *LookupFunction(const char *name);

//! Look up internal functions as used by operators. 
//! This is a linear search and only performed once. 
extern const FunctionDesc *LookupInternalFunction(const char *name);

//! Returns the operator description correspondig to the operator. 
//! This is a simple array lookup. 
extern const OperatorDesc *OperatorDescription(OperatorType otype);

//! This must be called before the first use of any exparse function: 
//! It initializes the following values and the operator description. 
extern void InitData();
//! Don't forget to also clean up at the end. 
extern void CleanupData();

//! These values describe some needed sizes: 
//! Don't modify them. 
extern int n_functions;
extern int max_precedence_val;
extern int cond_precedence_val;
extern int *binddir_of_precedence;  // binddir[precedence]
extern const FunctionDesc *gen_vec_fdesc;  // as used by <>. 

//! Actually call & compute a function pasing the arguments in 
//! FunctionArgs::args and storing the result under FunctionArgs::result 
//! if successful. 
//! YOU WILL NORMALLY use the interface class FunctionCaller (ilfcall.hpp). 
//! See FunctionState for possible return values. 
extern FunctionState CallFunction(FunctionArgs *fa);

}  // namespace end 

#endif  /* _NS_ExParse_Function_HPP_ */

/*
 * ani-parser/assfunc.h
 * 
 * Assignment / implicit casting - specific header. 
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

#ifndef _ANIVISION_ANI_ASSFUNC_H_
#define _ANIVISION_ANI_ASSFUNC_H_ 1

#include <hlib/cplusplus.h>
#include <ani-parser/treenode.h>


namespace ANI
{

class ExprValueType;
class ExprValue;


// Assignment function handler: 
// Derived classes hidden in assfunc.cc. 
struct AssignmentFuncBase
{
	// Counts number of created assignment functions: 
	static int ass_funcs_created;
	
	_CPP_OPERATORS
	AssignmentFuncBase(int /*_need_rvalue*/)
		{  ++ass_funcs_created;  }
	virtual ~AssignmentFuncBase();
	// The lval is a true non-const C++ reference. 
	virtual void Assign(ExprValue &lval,const ExprValue &rval);
}__attribute__((__packed__));

// Primary function: 
// Assignment function stored in *ret_assfunc if non-NULL. 
// Must be deleted by called (using operator delete) if no 
// longer needed. 
// Returns same value as ExprValueType::CanAssignType(). 
extern int LookupAssignmentFunction(
	const ExprValueType &lval,const ExprValueType &rval,
	AssignmentFuncBase **ret_assfunc,AssignmentOpID ass_op=AO_Set);


/******************************************************************************/

// Increment/decrement "assignment" functions: 
struct IncDecAssFuncBase
{
	// Counts number of created assignment functions: 
	static int incdec_funcs_created;
	
	_CPP_OPERATORS
	IncDecAssFuncBase()  { ++incdec_funcs_created; }
	virtual ~IncDecAssFuncBase();
	// The lval is a true non-const C++ reference (though 
	// not actually needed here). 
	virtual void IncDec(ExprValue &val);
}__attribute__((__packed__));

// Primary inc/dec query function. 
// A value of specified type shall be incremented (inc_or_dec=+1) 
// or decremented (inc_or_dec=-1). 
// IncDecAssFunc stored in *ret_incdecfunc if non-NULL. 
// Must be deleted by called (using operator delete) if no 
// longer needed. 
// Return value: 1 -> OK, 0 -> cannot inc/dec passed type. 
extern int LookupAssIncDecFunction(
	const ExprValueType &ltype,
	IncDecAssFuncBase **ret_incdecfunc,int inc_or_dec);

}  // end of namespace ANI

#endif  /* _ANIVISION_ANI_ASSFUNC_H_ */

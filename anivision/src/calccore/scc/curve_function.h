/*
 * calccore/scc/curve_function.h
 * 
 * Function curve class. This is a very specialized class which depends 
 * more or less on the user-supplied function handling in AniVision and 
 * the used calc core. This is the reason why this class is not in the 
 * objdesc/ directory. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _ANIVISION_CALCCORE_SCC_CURVE_FUNCTION_H_
#define _ANIVISION_CALCCORE_SCC_CURVE_FUNCTION_H_

#include <objdesc/curve.h>
#include <numerics/spline/sl_base.h>

#include <ani-parser/tree.h>


namespace NUM
{

class CurveFunction_Function : public CurveFunctionBase
{
	public:
		
	protected:
		// This accumulates the user-specified stuff until 
		// Create() gets called: (Is NULL after Create().)
		struct UData
		{
			Range t_range;
			CC::ADTNScopeEntry *expr_entry;
			
			_CPP_OPERATORS
			UData();
			~UData();
		};
		
		UData *udata;
		
		// Attach/Detach t value to hook before calling the eval function. 
		void _AttachTValue(double t,ANI::ExprValue *save_old) const;
		void _DetachTValue(const ANI::ExprValue &restore_old) const;
		
		// Must be used as derived class. 
	public:
		CurveFunction_Function();
		~CurveFunction_Function();
		
		//**** API for user parameters: ****
		
		// Set range of $t parameter for the function. 
		// Default is 0..1, default length is 1. 
		// Return value: 
		//   0 -> OK
		//.  1 -> nothing done; t already set
		int Set_T(const Range &t);
		
		// Set the function expression tree. 
		// Actually, a ADTNScopeEntry with last child being a TNExpression 
		// must be passed (because the ADTNScopeEntry_CoreHook is needed). 
		// Return value: 
		//   0 -> OK
		//.  1 -> nothing done; expr already set
		int Set_Expr(ANI::TreeNode *expr_entry);
		
		// [overriding virtuals:]
		int Create(EvalContext *);
		int Eval(double t,double *result,EvalContext *) const;
		int EvalD(double t,double *result,EvalContext *) const;
		int EvalDD(double t,double *result,EvalContext *) const;
};

}  // end of namespace NUM

#endif  /* _ANIVISION_CALCCORE_SCC_CURVE_FUNCTION_H_ */

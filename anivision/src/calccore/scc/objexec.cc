/*
 * calccore/scc/objexec.cc
 * 
 * Simple calc core object ADB thread execution ("eval") routines. 
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

#include "ccif_simple.h"

#include <objdesc/omatrix.h>

#include <objdesc/curve_fixed.h>
#include <objdesc/curve_lspline.h>
#include <objdesc/curve_cspline.h>
#include "curve_function.h"

#include <objdesc/curvetmap_none.h>
#include <objdesc/curvetmap_linear.h>

#include <objdesc/frontspec_speed.h>

#include <objdesc/upspec_gravity.h>


namespace CC { namespace S
{

// Symbolic names for the slot_id parameter. 
enum _AniDescSlotID
{
	SID_MoveBase,
	SID_CurveBase,
	SID_CurvePosFixedBase,
	SID_CurvePosLsplineBase,
	SID_CurvePosCsplineBase,
	SID_CurvePosFunctionBase,
	SID_CurveTMapNoneBase,
	SID_CurveTMapConstBase,
	SID_FrontSpeedBase,
	SID_UpGravityBase,
};


template<typename T,typename B> inline int CCObject::_InterpreteScope_Template(
	const char *scope_name,int slot_id,
	ADTNTypedScope *tsc,ExecThread *et,B **_dptr)
{
	// This will create a new one if there is none on the stack. 
	T *dptr = _PopCPtr<T>(et);
	
	int rv=InterpreteScopeEntries(tsc,et, scope_name, dptr,slot_id);
	if(rv>0)  // Context switch. 
	{
		_PushCPtr(et,dptr);
		return(1);
	}
	assert(rv==0);
	
	*_dptr=dptr;
	return(0);
}


// The tsc MUST be a "fixed { ... }" scope, i.e. 
// tsc->scope_type->name must be "fixed". 
int CCObject::_InterpreteCurvePosFixed(ADTNTypedScope *tsc,ExecThread *et,
	NUM::CurveFunctionBase **_curvefunc)
{
	return(_InterpreteScope_Template<NUM::CurveFunction_Fixed>(
		"(curve pos) fixed", /*slot_id=*/SID_CurvePosFixedBase,
		tsc,et,/*dptr=*/_curvefunc));
}

int CCObject::_InterpreteCurvePosFixed_Slot(ExecThread *et,
	InterpreteScopeEntriesInfo *info)
{
	NUM::CurveFunction_Fixed *curvefunc=
		(NUM::CurveFunction_Fixed*)info->dptr;
	
	switch(info->tok_id)
	{
		case TID_pos:
		{
			double tmp[3];
			int rv=_GetVector3Value(tmp,info->value,et);
			if(rv<0)  {  ++n_errors;  return(0);  }
			if(rv)  return(rv);  // >0 here
			switch(curvefunc->SetPos(tmp,/*dim=*/3))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				default: assert(0);
			}
		} break;
		default: assert(0);
	}
	
	return(ISS_Cont);
}


// The tsc MUST be a "lspline { ... }" scope, i.e. 
// tsc->scope_type->name must be "lspline". 
int CCObject::_InterpreteCurvePosLspline(ADTNTypedScope *tsc,ExecThread *et,
	NUM::CurveFunctionBase **_curvefunc)
{
	return(_InterpreteScope_Template<NUM::CurveFunction_LSpline>(
		"(curve pos) lspline", /*slot_id=*/SID_CurvePosLsplineBase,
		tsc,et,/*dptr=*/_curvefunc));
}

// The tsc MUST be a "cspline { ... }" scope, i.e. 
// tsc->scope_type->name must be "cspline". 
int CCObject::_InterpreteCurvePosCspline(ADTNTypedScope *tsc,ExecThread *et,
	NUM::CurveFunctionBase **_curvefunc)
{
	return(_InterpreteScope_Template<NUM::CurveFunction_CSpline>(
		"(curve pos) cspline", /*slot_id=*/SID_CurvePosCsplineBase,
		tsc,et,/*dptr=*/_curvefunc));
}

int CCObject::_InterpreteCurvePosLCspline_Slot(ExecThread *et,
	InterpreteScopeEntriesInfo *info,int spline_type)
{
	// NOTE: This function processes LINEAR and CUBIC splines. 
	//       See spline_order. 
	
	NUM::CurveFunction_Spline_Base *curvefunc=
		(NUM::CurveFunction_Spline_Base*)info->dptr;
	
	switch(spline_type)
	{
		case 1:  // ok
		case 3:  break;
		default:  assert(0);  // currently, no more categories accepted
	}
	
	switch(info->tok_id)
	{
		case TID_pts:  // fall through
		case TID_pts_t:
		{
			bool is_pts_t=(info->tok_id==TID_pts_t);
			NUM::VectorArray<double> pts;
			int rv=_GetVectorArray(pts,info->value,et,is_pts_t ? 4 : 3);
			if(rv<0)  {  ++n_errors;  return(0);  }
			if(rv)  return(rv);  // >0 here
			// Got the point array. 
			// This will remove the data from pts: 
			switch(curvefunc->SetPoints(pts,is_pts_t))
			{
				case 0:  return(ISS_Cont);
				case 1:
					Error(info->value->GetLocationRange(),
						"spline control points were already set\n");
					++n_errors;
					return(ISS_Cont);
				case 2:
					Error(info->value->GetLocationRange(),
						"need at least 2 points for spline\n");
					++n_errors;
					return(ISS_Cont);
				case 3:
					Error(info->value->GetLocationRange(),
						"incompatible \"tvals\" spec\n");
					++n_errors;
					return(ISS_Cont);
				default: assert(0);
			}
		} break;
		case TID_addpts:
		{
			String str;
			int rv=_GetStringValue(str,info->value,et);
			if(rv<0)  {  ++n_errors;  return(0);  }
			if(rv)  return(rv);  // >0 here
			switch(curvefunc->SetAddPoints(str.str()))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				case 2:  return(ISS_IllegalValue);
				default: assert(0);
			}
		} break;
		case TID_tvals:
		{
			String str;
			int rv=_GetStringValue(str,info->value,et);
			if(rv<0)  {  ++n_errors;  return(0);  }
			if(rv)  return(rv);  // >0 here
			switch(curvefunc->SetTValueMode(str.str()))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				case 2:  return(ISS_IllegalValue);
				case 3:
					Error(info->value->GetLocationRange(),
						"tvals spec incompatible with pts_t\n");
					++n_errors;
					return(ISS_Cont);
				default: assert(0);
			}
		} break;
		default: assert(0);
	}
	
	return(ISS_Cont);
}
	

// The tsc MUST be a "function { ... }" scope, i.e. 
// tsc->scope_type->name must be "function". 
int CCObject::_InterpreteCurvePosFunction(ADTNTypedScope *tsc,ExecThread *et,
	NUM::CurveFunctionBase **_curvefunc)
{
	return(_InterpreteScope_Template<NUM::CurveFunction_Function>(
		"(curve pos) function", /*slot_id=*/SID_CurvePosFunctionBase,
		tsc,et,/*dptr=*/_curvefunc));
}

int CCObject::_InterpreteCurvePosFunction_Slot(ExecThread *et,
	InterpreteScopeEntriesInfo *info)
{
	NUM::CurveFunction_Function *curvefunc=
		(NUM::CurveFunction_Function*)info->dptr;
	
	switch(info->tok_id)
	{
		case TID_pos:
		{
			// Okay... this is nonstandard because this is not a subtree 
			// which we will evaluate NOW but instead this is a function 
			// to be evaluated lateron. 
			// info->value is the ANI::TreeNode root of the expression. 
			//info->value->DumpTree(stderr);
			
Warning(info->value->GetLocationRange(),
	"implement function pos entry! [todo warning]\n");
ANI::CheckIfEvalInfo cinfo;
// NOTE: FIXME: This check should be done after the TF step and not here. 
//       Store the result in the core hook. 
info->value->CheckIfEvaluable(&cinfo);
fprintf(stderr,"CheckIfEvaluable={refs_to: this: %d, stack: %d, "
	"val: %d, static: %d}\n",
	cinfo.nrefs_to_this_ptr,cinfo.nrefs_to_local_stack,
		cinfo.number_of_values,cinfo.nrefs_to_static);
if(cinfo.nrefs_to_local_stack)
{
	// Cannot use general purpose eval thread; need to clone the 
	// current thread or block it. 
	// ...unless there is interpolate specified in which case 
	// we can do all the necessary evaluations right now. 
	Error(info->value->GetLocationRange(),
		"cannot use local stack vars in function{} blocks\n");
	++n_errors;
	return(ISS_Cont);
}
			
			// We need to remember the function expression tree. 
			switch(curvefunc->Set_Expr(info->value->parent()))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				default: assert(0);
			}
		} break;
		case TID_t:
		{
			Range t;
			int rv=_GetRangeValue(t,info->value,et);
			if(rv<0)  {  ++n_errors;  return(0);  }
			if(rv)  return(rv);  // >0 here
			switch(curvefunc->Set_T(t))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				default: assert(0);
			}
		} break;
		default: assert(0);
	}
	
	return(ISS_Cont);
}


// The tsc MUST be a "curve { ... }" scope, i.e. 
// tsc->scope_type->name must be "curve". 
int CCObject::_InterpreteCurvePosCurve(ADTNTypedScope *tsc,ExecThread *et,
	NUM::CurveFunctionBase **_curvefunc)
{
	assert(!"!implemented");
	return(0);
}


// The tsc MUST be a "join { ... }" scope, i.e. 
// tsc->scope_type->name must be "join". 
int CCObject::_InterpreteCurvePosJoin(ADTNTypedScope *tsc,ExecThread *et,
	NUM::CurveFunctionBase **_curvefunc)
{
	assert(!"!implemented");
	return(0);
}


int CCObject::_InterpreteCurvePos(InterpreteScopeEntriesInfo *info,
	ExecThread *et,NUM::CurveFunctionBase **_posfunc)
{
	do {
		if(!info->tsc)  break;
		
		int rv=-1;
		switch(info->tsc_tok_id)
		{
			case TID_fixed:   rv=_InterpreteCurvePosFixed(info->tsc,et,_posfunc);   break;
			case TID_lspline: rv=_InterpreteCurvePosLspline(info->tsc,et,_posfunc); break;
			case TID_cspline: rv=_InterpreteCurvePosCspline(info->tsc,et,_posfunc); break;
			case TID_function:rv=_InterpreteCurvePosFunction(info->tsc,et,_posfunc);break;
			case TID_curve:   rv=_InterpreteCurvePosCurve(info->tsc,et,_posfunc);   break;
			case TID_join:    rv=_InterpreteCurvePosJoin(info->tsc,et,_posfunc);    break;
			default: goto breakout;
		}
		
		assert(rv>=0);
		return(rv);
	} while(0); breakout:;
	
	Error(info->value->GetLocationRange(),
		"curve \"pos\" property requires a const/lspline/cspline/"
		"function/curve/join block as value\n");
	++n_errors;
	return(0);
}


int CCObject::_InterpreteCurveTMapNone(ADTNTypedScope *tsc,ExecThread *et,
	NUM::CurveTMapFunction **_tmapfunc)
{
	return(_InterpreteScope_Template<NUM::CurveTMapFunction_None>(
		"(curve tmap) none", /*slot_id=*/SID_CurveTMapNoneBase,
		tsc,et,/*dptr=*/_tmapfunc));
}

int CCObject::_InterpreteCurveTMapNone_Slot(ExecThread *et,
	InterpreteScopeEntriesInfo *info)
{
	// Currently no tokens recognized, hence we may not be here. 
	assert(0);
	return(-1);
}


int CCObject::_InterpreteCurveTMapConst(ADTNTypedScope *tsc,ExecThread *et,
	NUM::CurveTMapFunction **_tmapfunc)
{
	return(_InterpreteScope_Template<NUM::CurveTMapFunction_Linear>(
		"(curve tmap) constant", /*slot_id=*/SID_CurveTMapConstBase,
		tsc,et,/*dptr=*/_tmapfunc));
}

int CCObject::_InterpreteCurveTMapConst_Slot(ExecThread *et,
	InterpreteScopeEntriesInfo *info)
{
	// Currently no tokens recognized, hence we may not be here. 
	// In the future there will be tokens recognized. 
	assert(0);
	return(-1);
}


int CCObject::_InterpreteCurveTMapCspline(ADTNTypedScope *tsc,ExecThread *et,
	NUM::CurveTMapFunction **_tmapfunc)
{
	assert(!"!implemented");
}


int CCObject::_InterpreteCurveTMap(InterpreteScopeEntriesInfo *info,
	ExecThread *et,NUM::CurveTMapFunction **_tmapfunc,bool is_speed)
{
	do {
		if(!info->tsc)  break;
		
		int rv=-1;
		switch(info->tsc_tok_id)
		{
			case TID_none:
				rv=_InterpreteCurveTMapNone(info->tsc,et,_tmapfunc);    break;
			case TID_constant:
				if(!is_speed)  goto breakout;
				rv=_InterpreteCurveTMapConst(info->tsc,et,_tmapfunc);   break;
			case TID_cspline:
				rv=_InterpreteCurveTMapCspline(info->tsc,et,_tmapfunc); break;
			default:  goto breakout;
		}
		
		assert(rv>=0);
		return(rv);
	} while(0); breakout:;
	
	Error(info->value->GetLocationRange(),
		"curve \"%s\" property requires a none/%scspline"
		"block as value\n",
		is_speed ? "speed" : "length",is_speed ? "constant/" : "");
	++n_errors;
	return(0);
}


// Interprete curve. The tsc MUST be a "curve { ... }" scope, i.e. 
// tsc->scope_type->name must be "curve". 
int CCObject::_InterpreteCurve(ADTNTypedScope *tsc,ExecThread *et,
	NUM::OCurve **_ocurve)
{
	return(_InterpreteScope_Template<NUM::OCurve>(
		"curve", /*slot_id=*/SID_CurveBase,
		tsc,et,/*dptr=*/_ocurve));
}

int CCObject::_InterpreteCurve_Slot(ExecThread *et,
	InterpreteScopeEntriesInfo *info)
{
	NUM::OCurve *ocurve=(NUM::OCurve*)info->dptr;
	
	switch(info->tok_id)
	{
		case TID_pos:
		{
			NUM::CurveFunctionBase *posfunc=NULL;
			int rv=_InterpreteCurvePos(info,et,&posfunc);
			if(rv) return(rv);  // never <0
			// Got the curve pos func. 
			switch(ocurve->SetPosCurve(posfunc))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				default: assert(0);
			}
		} break;
		case TID_dt:
		{
			double dt;
			int rv=_GetScalarValue(dt,info->value,et);
			if(rv<0)  {  ++n_errors;  return(0);  }
			if(rv)  return(rv);  // >0 here
			switch(ocurve->Set_DT(dt))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				case 2:  return(ISS_TimeSpecConflict);
				default: assert(0);
			}
		} break;
		case TID_t:
		{
			Range t;
			int rv=_GetRangeValue(t,info->value,et);
			if(rv<0)  {  ++n_errors;  return(0);  }
			if(rv)  return(rv);  // >0 here
			switch(ocurve->Set_T(t))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				case 2:  return(ISS_TimeSpecConflict);
				default: assert(0);
			}
		} break;
		case TID_speed:
		case TID_length:
		{
			NUM::CurveTMapFunction *tmapfunc=NULL;
			int rv=_InterpreteCurveTMap(info,et,&tmapfunc,
				info->tok_id==TID_speed);
			if(rv) return(rv);  // never <0
			// Got the t mapping func. 
			switch(ocurve->SetTMapFunction(tmapfunc))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				/*case 2:
					Error(info->value->GetLocationRange(),
						"cannot combine \"speed\" and \"length\"\n");
					++n_errors;
					return(ISS_Cont);*/
				default: assert(0);
			}
		} break;
		default: assert(0);
	}
	
	return(ISS_Cont);
}


int CCObject::_InterpreteFrontSpeed(ADTNTypedScope *tsc,ExecThread *et,
	NUM::OFrontSpec **_ofront)
{
	return(_InterpreteScope_Template<NUM::OFrontSpec_Speed>(
		"(front) speed", /*slot_id=*/SID_FrontSpeedBase,
		tsc,et,/*dptr=*/_ofront));
}

int CCObject::_InterpreteFrontSpeed_Slot(ExecThread *et,
	InterpreteScopeEntriesInfo *info)
{
	// Currently no tokens recognized, hence we may not be here. 
	// In the future there will be tokens recognized. 
	assert(0);
	return(-1);
}


int CCObject::_InterpreteUpGravity(ADTNTypedScope *tsc,ExecThread *et,
	NUM::OUpSpec **_oup)
{
	return(_InterpreteScope_Template<NUM::OUpSpec_Gravity>(
		"(up) gravity", /*slot_id=*/SID_UpGravityBase,
		tsc,et,/*dptr=*/_oup));
}

int CCObject::_InterpreteUpGravity_Slot(ExecThread *et,
	InterpreteScopeEntriesInfo *info)
{
	NUM::OUpSpec_Gravity *oup=(NUM::OUpSpec_Gravity*)info->dptr;
	
	switch(info->tok_id)
	{
		case TID_force_center:  // fall through
		case TID_force_up:
		{
			bool is_center=(info->tok_id==TID_force_center);
			double tmp[3];
			int rv=_GetVector3Value(tmp,info->value,et);
			if(rv<0)  {  ++n_errors;  return(0);  }
			if(rv)  return(rv);  // >0 here
			switch(oup->SetForce(tmp,is_center))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				case 3:
					Error(info->value->GetLocationRange(),
						"cannot use force_up and force_center simultaniously\n");
					++n_errors;
					return(ISS_Cont);
				default: assert(0);
			}
		} break;
		case TID_accel_scale:
		{
			double tmp;
			int rv=_GetScalarValue(tmp,info->value,et);
			if(rv<0)  {  ++n_errors;  return(0);  }
			if(rv)  return(rv);  // >0 here
			switch(oup->SetAccelScale(tmp))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				default: assert(0);
			}
		} break;
		default: assert(0);
	}
	
	return(ISS_Cont);
}


int CCObject::_InterpreteMovePos(InterpreteScopeEntriesInfo *info,
	ExecThread *et,NUM::OCurve **_ocurve)
{
	do {
		if(!info->tsc)  break;
		
		int rv=-1;
		switch(info->tsc_tok_id)
		{
			case TID_curve:  rv=_InterpreteCurve(info->tsc,et,_ocurve);  break;
			default:  goto breakout;
		}
		
		assert(rv>=0);
		return(rv);
	} while(0); breakout:;
	
	Error(info->value->GetLocationRange(),
		"move \"pos\" property requires a curve block as value\n");
	++n_errors;
	return(0);
}


int CCObject::_InterpreteMoveFront(InterpreteScopeEntriesInfo *info,
	ExecThread *et,NUM::OFrontSpec **_ofront)
{
	do {
		if(!info->tsc)  break;
		
		int rv=-1;
		switch(info->tsc_tok_id)
		{
			case TID_speed:    rv=_InterpreteFrontSpeed(info->tsc,et,_ofront);  break;
			//case TID_curve:  rv=_InterpreteCurve(info->tsc,et,_ocurve);  break;
			default:  goto breakout;
		}
		
		assert(rv>=0);
		return(rv);
	} while(0); breakout:;
	
	Error(info->value->GetLocationRange(),
		"move \"front\" property requires a curve block as value\n");
	++n_errors;
	return(0);
}	


int CCObject::_InterpreteMoveUp(InterpreteScopeEntriesInfo *info,
	ExecThread *et,NUM::OUpSpec **_oup)
{
	do {
		if(!info->tsc)  break;
		
		int rv=-1;
		switch(info->tsc_tok_id)
		{
			case TID_gravity:  rv=_InterpreteUpGravity(info->tsc,et,_oup);  break;
			//case TID_curve:  rv=_InterpreteCurve(info->tsc,et,_ocurve);  break;
			default:  goto breakout;
		}
		
		assert(rv>=0);
		return(rv);
	} while(0); breakout:;
	
	Error(info->value->GetLocationRange(),
		"move \"front\" property requires a curve block as value\n");
	++n_errors;
	return(0);
}	


int CCObject::_InterpreteMoveScope(ADTNTypedScope *tsc,ExecThread *et,
	NUM::OMovement **_movement)
{
	return(_InterpreteScope_Template<NUM::OMovement>(
		"move ADB", /*slot_id=*/SID_MoveBase,
		tsc,et,/*dptr=*/_movement));
}

int CCObject::_InterpreteMoveScope_Slot(ExecThread *et,
	InterpreteScopeEntriesInfo *info)
{
	NUM::OMovement *movement=(NUM::OMovement*)info->dptr;
	
	switch(info->tok_id)
	{
		case TID_pos:
		{
			NUM::OCurve *ocurve=NULL;
			int rv=_InterpreteMovePos(info,et,&ocurve);
			if(rv)  return(rv);  // only >0 here
			switch(movement->SetPosCurve(ocurve))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				default: assert(0);
			}
		} break;
		case TID_front:
		{
			NUM::OFrontSpec *ofront=NULL;
			int rv=_InterpreteMoveFront(info,et,&ofront);
			if(rv)  return(rv);  // only >0 here
			switch(movement->SetFrontSpec(ofront))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				default: assert(0);
			}
		} break;
		case TID_up:
		{
			NUM::OUpSpec *oup=NULL;
			int rv=_InterpreteMoveUp(info,et,&oup);
			if(rv)  return(rv);  // only >0 here
			switch(movement->SetUpSpec(oup))
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);
				default: assert(0);
			}
		} break;
		case TID_front_mode:  // fall through
		case TID_up_mode:
		{
			String str;
			int rv=_GetStringValue(str,info->value,et);
			if(rv<0)  {  ++n_errors;  return(ISS_Cont);  }
			if(rv)  return(rv);  // >0 here
			switch(info->tok_id==TID_front_mode ? 
				movement->SetFrontMode(str.str()) : 
				movement->SetUpMode(str.str()) )
			{
				case 0:  return(ISS_Cont);
				case 1:  return(ISS_AlreadySet);  // <-- Will not happen: new settings override old ones. 
				case 2:  return(ISS_IllegalValue);
				default: assert(0);
			}
		} break;
		default: assert(0);
	}
	
	return(ISS_Cont);
}


int CCObject::InterpreteScopeSlot(int slot_id,ExecThread *et,
	InterpreteScopeEntriesInfo *info)
{
	fprintf(stderr,"  slot[%d] -> \"%s\"\n",
		slot_id,SCCTokenTranslator::Tok2Str(info->tok_id));
	switch(slot_id)
	{
		case SID_MoveBase:               return(_InterpreteMoveScope_Slot(et,info));
		case SID_CurveBase:              return(_InterpreteCurve_Slot(et,info));
		case SID_CurvePosFixedBase:      return(_InterpreteCurvePosFixed_Slot(et,info));
		case SID_CurvePosLsplineBase:    return(_InterpreteCurvePosLCspline_Slot(et,info,/*spline_type=*/1));
		case SID_CurvePosCsplineBase:    return(_InterpreteCurvePosLCspline_Slot(et,info,/*spline_type=*/3));
		case SID_CurvePosFunctionBase:   return(_InterpreteCurvePosFunction_Slot(et,info));
		case SID_CurveTMapNoneBase:      return(_InterpreteCurveTMapNone_Slot(et,info));
		case SID_FrontSpeedBase:         return(_InterpreteFrontSpeed_Slot(et,info));
		case SID_UpGravityBase:          return(_InterpreteUpGravity_Slot(et,info));
		default: assert(0);
	}
	return((InterpreteScopeSlot_State)-1);  // not reached
}


// Exclusively used by EvalAniDescBlock(). 
int CCObject::_DoEvalAniDescBlock(ADTNAniDescBlock *adb,ExecThread *et)
{
	EvalContext_SCC _ectx(et,this);
	EvalContext_SCC *ectx=&_ectx;
	
	for(ANI::TreeNode *_tn=et->ess->popTN(adb->down.first()); 
		_tn; _tn=_tn->next)
	{
		if(_tn->NType()!=ANI::TN_AniDesc) assert(0);
		if(((ADTreeNode*)_tn)->ADNType()!=ADTN_ScopeEntry) assert(0);
		
		ADTNScopeEntry *scent=(ADTNScopeEntry*)_tn;
		//ANI::TNIdentifier *name = (scent->down.first()==scent->down.last() ? 
		//	NULL : (ANI::TNIdentifier*)scent->down.first());
		ANI::TreeNode *value=scent->down.last();
		
		// The core hook is attached during registration step. 
		ADTNScopeEntry_CoreHook *scent_ch=
				(ADTNScopeEntry_CoreHook*)scent->ent_core_hook;
		assert(scent_ch);
		
		// This is checked in ADTN's DoExprTF():  
		assert(!scent_ch->entry_name_tok);  // 0 -> not set
		
		if(value->NType()==ANI::TN_AniDesc && 
			((ADTreeNode*)value)->ADNType()==ADTN_TypedScope)
		{
			ADTNTypedScope *tsc=(ADTNTypedScope*)value;
			
			//RefString tsc_type(tsc->scope_type->name);
			switch(tsc->scope_type_tok)
			{
				case TID_move:
				{
					NUM::OMovement *movement=NULL;
					if(_InterpreteMoveScope(tsc,et,&movement))
					{
						// Context switch. 
						et->ess->pushTN(_tn);
						return(1);
					}
					
					// Got a new movement; add to list. 
					movements.append(movement);
					
					// Finally create the movement: 
					// This will generate all the internal data and also 
					// try to achieve continuacy,... 
					int rv=movement->Create(setting->CurrentSettingTime(),ectx);
					if(rv)
					{  ++n_errors;  }
					
					// FIXME: Need to check if successive movements overlap 
					// in time (using movement->{T0,T1}(). 
				}  break;
				default:
					// This should have been caught during ADTN's DoExprTF(). 
					assert(0);
					++n_errors;
			}
		}
		else
		{
			// This should have been caught during ADTN's DoExprTF(). 
			assert(0);
			++n_errors;  continue;
		}
	}
	
	return(0);
}


int CCObject::EvalAniDescBlock(ADTNAniDescBlock *adb,ExecThread *et)
{
fprintf(stderr,"Why the hell?!\n");  // -> check n_errors and the like
// We must make sure that only one thread at a time executes an 
// ani desc block. 
if(AquireADBLock(adb,et))
{  return(0);  }  // <-- No context switch (could use 1 for waiting, too). 
	
	fprintf(stdout,"SCC: Interpreting ADB @%s\n",
		adb->GetLocationRange().PosRangeString().str());
	
	int rv=_DoEvalAniDescBlock(adb,et);
	
	if(rv)
	{
		// Context switch. 
		if(et->schedule_status==ExecThread::SSRequest)
		{
			Error(adb->GetLocationRange(),
				"OOPS: Cloning from inside ADB forbidden\n");
			abort();
		}
		
		return(1);
	}
	
	// If we come here, there is regular exit, i.e. no context switch. 
LoseADBLock();  // Does NOT clear n_errors. 
	return(n_errors ? -1 : 0);
}

}}  // end of namespace CC::S

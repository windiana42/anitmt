/*
 * calccore/scc/curve_function.cc
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

#include "curve_function.h"
#include "ccif_simple.h"
#include <calccore/adtree.h>
#include <core/anicontext.h>
#include <core/aniinstance.h>


namespace NUM
{

CurveFunction_Function::UData::UData() : 
	t_range()
{
	expr_entry=NULL;
}

CurveFunction_Function::UData::~UData()
{
	expr_entry=NULL;
}

//------------------------------------------------------------------------------

void CurveFunction_Function::_AttachTValue(double t,
	ANI::ExprValue *save_old) const
{
	assert(udata->expr_entry);
	
	CC::S::ADTNScopeEntry_CoreHook *ch=
		(CC::S::ADTNScopeEntry_CoreHook*)udata->expr_entry->ent_core_hook;
	// Increase recursion depth: 
	if(++ch->func_val_t->recursion_depth>64)
	{
		Error(udata->expr_entry->GetLocationRange(),
			"OOPS: Function recursion limit (%d) exceeded\n",
			ch->func_val_t->recursion_depth-1);
		abort();
	}
	// Copy (internal reference) old value: 
	*save_old=ch->func_val_t->t_val;
	// Set new value: 
	ch->func_val_t->t_val.assign(t);
}

void CurveFunction_Function::_DetachTValue(
	const ANI::ExprValue &restore_old) const
{
	CC::S::ADTNScopeEntry_CoreHook *ch=
		(CC::S::ADTNScopeEntry_CoreHook*)udata->expr_entry->ent_core_hook;
	// Restore old value: 
	ch->func_val_t->t_val=restore_old;
	// Decrease recursion depth: 
	--ch->func_val_t->recursion_depth;
}


int CurveFunction_Function::Eval(double t,double *result,
	EvalContext *_ectx) const
{
	if(!udata->expr_entry)
	{
		// No function expression specified. 
		for(int i=0; i<dim; i++)
			result[i]=0.0;
		return(0);
	}
	
	ANI::ExprValue tmp;
	_AttachTValue(t,&tmp);
	
	// The actual evaluation may not have context switches. 
	// We simply loop over several switches and bail out if there 
	// are too many. (This loop is needed because of the context 
	// switch hack which is (only) enabled for stress testing of 
	// the tread system.) 
	
	ANI::TNExpression *expr=(ANI::TNExpression*)udata->expr_entry->down.last();
	
	// Okay, so we can access the current ExecThread using: 
	//CC::S::EvalContext_SCC *ectx=(CC::S::EvalContext_SCC*)_ectx;
	//ExecThread = ectx->et.
	// ...but we need to evaluate the function without animation context. 
	// In case there is no access to the local stack, we can use the 
	// eval exec thread which is also used by the POV output; this is 
	// accessible via the animation context. 
	
	CC::S::EvalContext_SCC *ectx=(CC::S::EvalContext_SCC*)_ectx;
	ExecThread *eval_thread=ectx->et->Context()->eval_thread;
	if(ectx->ccobj)  // <-- there is a this pointer
	{
		// We need to supply the correct this pointer: 
		AniObjectInstance *obj_inst=ectx->ccobj->GetAniObjectInstance();
		assert(obj_inst);
		eval_thread->stack->AllocateEmptyFrames2(
			ANI::ScopeInstance(ANI::ScopeInstance::InternalCreate,obj_inst));
	}
	
	ANI::ExprValue val;
	int n_switches=0;
	bool fail=0;
	for(;;)
	{
		if(!expr->Eval(val,/*ExecThread=*/ectx->et))  break;
		if((++n_switches)>16)  {  fail=1;  break;  }
	}
	if(n_switches || fail)
	{
		Error(udata->expr_entry->GetLocationRange(),
			"OOPS: %s%d context switch attempts during "
			"function {} evaluation\n",
			fail ? "more than " : "",
			fail ? (n_switches-1) : n_switches);
		if(fail) abort();
	}
	
	if(ectx->ccobj)
	{
		// Deallocate the stack frames again: 
		eval_thread->stack->FreeEmptyFrames2();
	}
	
	// Extract value: 
	Value tmpval;
	val.GetPOD(&tmpval);
	const Vector *vect=tmpval.GetPtr<Vector>();
	assert(vect->dim()==dim);
	for(int i=0; i<dim; i++)
		result[i]=vect->operator[](i);
	
	_DetachTValue(tmp);
	
	return(0);
}

int CurveFunction_Function::EvalD(double t,double *result,
	EvalContext *_ectx) const
{
	if(!udata->expr_entry)
	{
		// No function expression specified. 
		for(int i=0; i<dim; i++)
			result[i]=0.0;
		return(0);
	}
	
fprintf(stderr,"IMPLEMENT CurveFunction_Function::EvalD!\n");
//assert(!"!implemented");
	
	// So we need to do some numerical differentiation...
	// For now, use this hack: 
	double resA[dim],resB[dim];
	double h=5e-5;
	Eval(t-h,resA,_ectx);
	Eval(t+h,resB,_ectx);
	h+=h;
	for(int i=0; i<dim; i++)
		result[i]=(resB[i]-resA[i])/h;
	return(0);
}

int CurveFunction_Function::EvalDD(double /*t*/,double *result,
	EvalContext *_ectx) const
{
	if(!udata->expr_entry)
	{
		// No function expression specified. 
		for(int i=0; i<dim; i++)
			result[i]=0.0;
		return(0);
	}
	
	assert(!"!implemented");
	return(0);
}


int CurveFunction_Function::Set_T(const Range &t)
{
	if(udata->t_range.Valid())  return(1);
	udata->t_range=t;
	return(0);
}


int CurveFunction_Function::Set_Expr(ANI::TreeNode *expr_entry)
{
	using namespace CC;
	
	// Sanity checks: 
	bool fail=1;
	do {
		if(!expr_entry)  break;
		if(expr_entry->NType()!=ANI::TN_AniDesc)  break;
		if(((ADTreeNode*)expr_entry)->ADNType()!=ADTN_ScopeEntry)  break;
		ADTNScopeEntry *scent=(ADTNScopeEntry*)expr_entry;
		if(!scent->down.last())  break;
		if(scent->down.last()->NType()!=ANI::TN_Expression)  break;
		if(!scent->ent_core_hook)  break;
		if(!((S::ADTNScopeEntry_CoreHook*)scent->ent_core_hook)->func_val_t)
			break;
		fail=0;
	} while(0);
	assert(!fail);
	
	if(udata->expr_entry)  return(1);
	udata->expr_entry=(ADTNScopeEntry*)expr_entry;
	return(0);
}


int CurveFunction_Function::Create(EvalContext *)
{
	assert(udata);
	
	if(dim<0)
	{
		// In this case the position was not set. 
		// FIXME: This is just a hack for testing. 
		fprintf(stderr,"OOPS!! dim not yet set! (curve_function.cc)\n");
		dim=3;
	}
	
	assert(dim>=0);
	
	switch(udata->t_range.Valid())
	{
		case Value::RValidA|Value::RValidB:  break;
		case Value::RValidA:
			udata->t_range.set_b(udata->t_range.val_a()+1.0);  break;
		case Value::RValidB:
			udata->t_range.set_a(udata->t_range.val_b()-1.0);  break;
		case 0:  udata->t_range.set(0.0,1.0);  break;
	}
	
	return(0);
}


CurveFunction_Function::CurveFunction_Function()
{
	udata=new UData();
}

CurveFunction_Function::~CurveFunction_Function()
{
	DELETE(udata);
}

}  // end of namespace NUM

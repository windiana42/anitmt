/*
 * ani-parser/expreval.cc
 * 
 * Main expression evaluation code. 
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

#include "tree.h"
#include "coreglue.h"
#include "opfunc.h"
#include "threadinfo.h"

#include <stdio.h>

#warning "**************************************************"
#warning "*******     !! CHECK ALL the FIXMEs !!     *******"
#warning "**************************************************"

// This is the actual expression evaluation code. 
// Actually doesn't look like that, right?
// Am, yeah -- because the _real_ work is done in the functions 
// called from here. 

// How it works: 
// If the info pointer is NULL, do everything we can do and 
// return ExprValue::None otherwise. This is used in 
// the most easy step of subexpression evaluation (i.e. 
// only immediate values get folded, no vals declared const 
// somewhere else). 
// **NOTE** Eval() no longer _returns_ the value but instead 
//          stores if in the passed EvprValue &retval. 

// NOTE: 
//   THERE ARE EVAL CHECK FUNCTIONS IN checkifeval.cc. 
//   MAKE SURE TO KEEP THEM IN SYNC WITH THE Eval() FUNCTIONS 
//   IN THIS FILE. 

#warning "Implement the following speed optimization:"
// FIXME: Should optimize code like "retval=ExprValue(xyz)" to 
//        avoid temporary and use retval->set(xyz) instead. 

namespace ANI
{

int TreeNode::Eval(ExprValue &retval,ExecThreadInfo *info)
{
	// Must be overridden when ever used. 
	// Note that not all tree nodes need to override that but namely 
	// TNIdentifier and all TNExpression-derived nodes. 
	if(!info)  // <-- special case
	{  retval=ExprValue::None;  return(0);  }
	
	fprintf(stderr,"OOPS: Eval() called for ntype=%d.\n",NType());
	assert(0);
	retval=ExprValue::None;
	return(-1);
}

/*----------------------------------------------------------------------------*/

ExprValue AniGlue_ScopeBase::EvalAddressationFunc::Eval(
	TNIdentifier * /*tn*/,ExecThreadInfo * /*info*/)
{
	// Must be overridden if used. 
	assert(0);
}

ExprValue AniGlue_ScopeBase::EvalAddressationFunc::Eval(
	TNIdentifier * /*tn*/,const ExprValue &/*base*/,ANI::ExecThreadInfo *)
{
	// Must be overridden if used. 
	assert(0);
}

ExprValue &AniGlue_ScopeBase::EvalAddressationFunc::GetRef(
	TNIdentifier * /*idf*/,ExecThreadInfo * /*info*/)
{
	// Must be overridden if used. (Only used in declarations.) 
	assert(0);
}

int AniGlue_ScopeBase::EvalAddressationFunc::CheckIfEvaluable(CheckIfEvalInfo *)
{
	// Must be overridden. 
	assert(0);
}

AniGlue_ScopeBase::EvalAddressationFunc::~EvalAddressationFunc()
{
	// Not inline because virtual. 
	/*empty*/
}

/*----------------------------------------------------------------------------*/

// Special function... as called in TF step: 
ExprValue TreeNode::DoTryIEval(TreeNode **update_tn)
{
	// For NO immediate subexpression evaluation / trivial const folding, 
	// simply return ExprValue::None. 
	//return(ExprValue::None);
	
	#define DUMP_FOLDING_EVAL 0
	
	#if DUMP_FOLDING_EVAL
	if(!down.is_empty())
	{
		fprintf(stdout,"Evaluating: ");
		DumpTree(stdout);
	}
	#endif
	
	ExprValue tmp;
	Eval(/*retval=*/tmp,/*info=*/NULL/*OK, special purpose*/);
	
	#if DUMP_FOLDING_EVAL
	if(!down.is_empty())
	{  fprintf(stdout,"Result: %s\n",tmp.ToString().str());  }
	#endif
	
	if(!(tmp==ExprValue::None) && !down.is_empty())
	{
		// Subexpression evaluation was successful and we're not a 
		// tree leaf. (Tree leaf -> We should not "replace" TNValue 
		// with TNValue here...)
		TNValue *tn_val=new TNValue(GetLocationRange(),tmp);
		if(update_tn)  *update_tn=tn_val;
		
		ReplaceBy(tn_val);
	}
	return(tmp);
}


/*----------------------------------------------------------------------------*/

// NOTE: 0 -> OFF; 2,3,4... -> probability 1:<val> for context switch. 
#define DO_CONTEXT_SWITCH_TEST 4

int TNIdentifier::Eval(ExprValue &retval,ExecThreadInfo *info)
{
	if(!info)
	{  retval=ExprValue::None;  return(0);  }
	
	if(info->switch_context_now)
	{
		info->switch_context_now=0;
		info->ess->StartingContextSwitch();
		return(1);
	}
	
	#if DO_CONTEXT_SWITCH_TEST
	#warning "*********** Context switch test hack enabled. ***********"
	static bool init_done=0;
	if(!init_done)
	{
		int initval=getpid();
		//fprintf(stderr,"VM: Context switch hack: USING CONSTANT SEED!!!!!!\n");
		srand(initval);
		init_done=1;
		fprintf(stderr,"VM: Context switch hack: SRAND(%d) INITIALIZED.\n",
			initval);
	}
	if(!(rand()%DO_CONTEXT_SWITCH_TEST))
	{
		info->ess->StartingContextSwitch();
		return(1);
	}
	#endif
	
	assert(evaladrfunc);
	// Use the non-member_select version of the addressation function: 
	retval=evaladrfunc->Eval(this,info);
	
	// In case there are variable notifiers installed, call them: 
	info->CallVarNotifiers(this,retval);
	
	return(0);
}


int TNExpression::Eval(ExprValue &retval,ExecThreadInfo *info)
{
	// Special case as usual :)
	// I mean: TNExpression is a base class for different expressions 
	// the only special case of no derived class is an expression 
	// holding an identifier. 
	if(!down.first() || down.first()->NType()!=TN_Identifier || 
		down.first()!=down.last())
	{  assert(0);  }
	
	return(down.first()->Eval(retval,info));
}


int TNExpressionList::Eval(ExprValue &retval,ExecThreadInfo *info)
{
	for(TreeNode *i=info ? info->ess->popTN(down.first()) : down.first(); 
		i; i=i->next)
	{
		if(i->Eval(retval,info))
		{  info->ess->pushTN(i); return(1);  }
		if(!info && retval==ExprValue::None)
		{  /*retval is set.*/  return(0);  }
	}
	
	// Expression list returns void. 
	retval=ExprValue::CVoidType;
	return(0);
}


int TNValue::Eval(ExprValue &retval,ExecThreadInfo *info)
{
	if(info->switch_context_now)
	{
		info->switch_context_now=0;
		info->ess->StartingContextSwitch();
		return(1);
	}
	
	#if DO_CONTEXT_SWITCH_TEST
	#warning "*********** Context switch test hack enabled. ***********"
	if(info && !(rand()%DO_CONTEXT_SWITCH_TEST))
	{
		info->ess->StartingContextSwitch();
		return(1);
	}
	#endif
	
	// This is the easy part... Constant value. 
	retval=val;
	return(0);
}


int TNArray::Eval(ExprValue &retval,ExecThreadInfo *info)
{
	EvalStateStack *ess=info ? info->ess : NULL;
	
	ssize_t elem_idx=ess->popL(-1);
	
	// Create array: 
	ExprValueArray val;
	if(elem_idx<0)
	{
		val=ExprValueArray(nelem,elem_type,/*set_prototypes=*/0);
		// Could probably make array an lvalue when not copying here?!
		retval=ExprValue(ExprValue::Copy,val);
		elem_idx=0;
	}
	else
	{
		ess->popEV(&retval);
		retval.GetArray(&val);
	}
		
	#warning "Comma at end/beginning ignored."
	
	// Compute elements:
	for(TreeNode *i=ess->popTN(down.first()); i; i=i->next,elem_idx++)
	{
		// May need implicit type conversion (done by assfunc). 
		ExprValue tmp;
		if(i->Eval(tmp,info))
		{
			ess->pushTN(i);
			ess->pushEV(retval);
			ess->pushL(elem_idx);
			return(1);
		}
		if(!info && tmp==ExprValue::None)
		{  retval=ExprValue::None;  return(0);  }
		
		int rv;
		if(assfunc[elem_idx])
		{
			ExprValue tmp2;
			assfunc[elem_idx]->Assign(tmp2,tmp);
			rv=val.IndexSet(elem_idx,tmp2);
		}
		else
		{  rv=val.IndexSet(elem_idx,tmp);  }
		assert(rv==0);
	}
	
	return(0);
}


int TNFCallArgument::Eval(ExprValue &retval,ExecThreadInfo *info)
{
	// This is simple as the identifer mapping was already done. 
	// Simply evaluate the expresion: 
	return(down.last()->Eval(retval,info));
}


int TNAssignment::Eval(ExprValue &retval,ExecThreadInfo *info)
{
	EvalStateStack *ess=info ? info->ess : NULL;
	
	assert(assfunc);
	
	// Evaluate the right and left part of the assignment: 
	ExprValue rvalue;
	ExprValue lvalue;
	switch(ess->popS())  // Fall through switch. 
	{
		case 0:
			if(down.last()->Eval(rvalue,info))
			{
				ess->pushS(0);
				return(1);
			}
			goto skippop;
		case 1:
			ess->popEV(&rvalue);
		skippop:
			if(down.first()->Eval(lvalue,info))
			{
				ess->pushEV(rvalue);
				ess->pushS(1);
				return(1);
			}
			break;
		default: assert(0);
	}
	
	if(!info && (rvalue==ExprValue::None || lvalue==ExprValue::None))
	{  retval=ExprValue::None;  return(0);  }
	
	// Actually assign the value: 
	assfunc->Assign(lvalue,rvalue);
	
	// Assignment returns void. 
	retval=ExprValue(ExprValue::CVoidType);
	return(0);
}


int TNOperatorFunction::Eval(ExprValue &retval,ExecThreadInfo *info)
{
	// Finally the hard part: 
	// Okay okay, doesn't really look like something complicated HERE 
	// because the actual work is hidden in evalfunc. 
	assert(evalfunc);
	return(evalfunc->Eval(retval,this,info));
}


int TNDeclarator::Eval(ExprValue &retval,ExecThreadInfo *info)
{
	// Declarator is not trivially constant-folded; hence info 
	// may not be NULL. 
	assert(info);  // otherwise: if(!info) { retval=ExprValue::None); return(0); }
	
	retval=ExprValue::CVoidType;  // Declarator returns void. 
	
	assert(evalfunc);
	return(evalfunc->Eval(this,info));
}


int TNNewArraySpecifier::Eval(ExprValue &retval,ExecThreadInfo *info)
{
	assert(info);  // cannot be constant folded
	
	assert(evalfunc);
	return(evalfunc->Eval(retval,(TNOperatorFunction*)this,info));
}


int TNFuncDefArgDecl::Eval(ExprValue &retval,ExecThreadInfo *info)
{
	// Simply evaluate the declarator: 
	// NOTE that this will return void because the declarator does so. 
	return(down.first()->Eval(retval,info));
}


//==============================================================================

int TNOperatorFunction::EvalFunc::Eval(ExprValue &retval,
	TNOperatorFunction * /*tn*/,ExecThreadInfo * /*info*/)
{
	// Must be overridden. 
	assert(0);
	retval=ExprValue::None;
	return(0);
}

int TNOperatorFunction::EvalFunc::DelayedEval(ExprValue & /*val*/,
	TNOperatorFunction * /*tn*/,ExecThreadInfo * /*info*/)
{
	// Must be overridden if used. 
	assert(0);
	return(0);
}

TNOperatorFunction::EvalFunc::~EvalFunc()
{
	// Not inline because virtual. 
	/*empty*/
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_DefaultOpFunc::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	EvalStateStack *ess=info ? info->ess : NULL;
	
	assert(fptr);
	
	ExprValue val[nargs];
	int idx=0;
	TreeNode *i=tn->down.first();
	TreeNode *stn=ess->popTN(i);
	for(; i!=stn; i=i->next,idx++)
	{  ess->popEV(&val[idx]);  }
	for(; i; i=i->next,idx++)
	{
		if(i->Eval(val[idx],info))
		{
			ess->pushTN(i);
			for(--idx,i=i->prev; i; i=i->prev,idx--)
			{  ess->pushEV(val[idx]);  }
			return(1);
		}
		if(!info && val[idx]==ExprValue::None)
		{  retval=ExprValue::None;  return(0);  }
	}
	//assert(idx==nargs);  // can be left away
	
	OpFunc_Args ofa;
	ofa.nargs=nargs;
	ofa._args=val;
	ofa.res=&retval;
	
	// Actually call the operator function: 
	int rv=(*fptr)(&ofa);
	assert(rv==0);  // FIXME: What to do in case of failure? 
	
	return(0);
}

TNOperatorFunction::EvalFunc_DefaultOpFunc::~EvalFunc_DefaultOpFunc()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_PODCast::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	assert(fptr);
	//assert(!!proto_type);
	
	ExprValue val[2];
	// (Note the order: the destination prototype is val[1].) 
	if(tn->down.last()->Eval(val[0],info))
	{  return(1);  }
	val[1]=proto_type;
	
	if(!info && val[0]==ExprValue::None)
	{  retval=ExprValue::None;  return(0);  }
	
	OpFunc_Args ofa;
	ofa.nargs=2;
	ofa._args=val;
	ofa.res=&retval;
	
	// Actually call the operator function: 
	int rv=(*fptr)(&ofa);
	assert(rv==0);  // FIXME: What to do in case of failure? 
	
	return(0);
}

TNOperatorFunction::EvalFunc_PODCast::~EvalFunc_PODCast()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_SubscriptArray::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	EvalStateStack *ess=info ? info->ess : NULL;
	
	ssize_t idx;
	switch(ess->popS())  // Fallthrough switch. 
	{
		case 0:
		{
			// Eval index: 
			ExprValue idx_val;
			if(tn->down.last()->Eval(idx_val,info))
			{
				ess->pushS(0);
				return(1);
			}
			if(!info && idx_val==ExprValue::None)
			{  retval=ExprValue::None;  return(0);  }
			idx=idx_val.GetPODInt();
			// Especially note that idx may be negative or out of range here. 
			goto skippop;
		}
		case 1:
			idx=ess->popL(-222);
		skippop:
		{
			// Get array to index: 
			ExprValue arr;
			if(tn->down.first()->Eval(arr,info))
			{
				ess->pushL(idx);
				ess->pushS(1);
				return(1);
			}
			if(!info && arr==ExprValue::None)
			{  retval=ExprValue::None;  return(0);  }
			
			//fprintf(stderr,"expreval.cc: Array subscription %s at %d\n",
			//	arr.ToString().str(),idx);
			
			// This is a special version which only works for arrays. 
			retval=ExprValue(ExprValue::IndexArray,arr,idx);
		}  break;
		default: assert(0);
	}
	
	return(0);
}

TNOperatorFunction::EvalFunc_SubscriptArray::~EvalFunc_SubscriptArray()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_SubscriptPOD::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	EvalStateStack *ess=info ? info->ess : NULL;
	
	ssize_t idx;
	switch(ess->popS())  // Fallthrough switch. 
	{
		case 0:
			// Eval index: 
			if(fixed_idx<0)
			{
				ExprValue idx_val;
				if(tn->down.last()->Eval(idx_val,info))
				{
					ess->pushS(0);
					return(1);
				}
				if(!info && idx_val==ExprValue::None)
				{  retval=ExprValue::None;  return(0);  }
				idx=idx_val.GetPODInt();
			}
			else
			{  idx=fixed_idx;  }
			goto skippop;
		case 1:
			idx=ess->popL(-223);
		skippop:
		{	// Especially note that idx may be negative or out of range here. 
			
			// Get POD type to index: 
			ExprValue pod;
			if(tn->down.first()->Eval(pod,info))
			{
				if(fixed_idx<0)
				{
					ess->pushL(idx);
					ess->pushS(1);
				}
				else
				{  ess->pushS(0);  }
				return(1);
			}
			if(!info && pod==ExprValue::None)
			{  retval=ExprValue::None;  return(0);  }
			
			// This is a special version for PODs...
			retval=ExprValue(index_type,pod,idx);
		}	break;
		default: assert(0);
	}
	
	return(0);
}

TNOperatorFunction::EvalFunc_SubscriptPOD::~EvalFunc_SubscriptPOD()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_IfElse::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	EvalStateStack *ess=info ? info->ess : NULL;
	
	short int cond=ess->popS(100);
	switch(cond)  // Fallthrough switch. 
	{
		case 100:
			// Evaluate the condition: 
			if(tn->down.first()->Eval(retval,info))
			{
				ess->pushS(100);
				return(1);
			}
			
			if(!info && retval==ExprValue::None)
			{  retval=ExprValue::None;  return(0);  }
			
			cond=retval.is_null();
		case 0:
		case 1:
		case 2:
		case -1:
			if(cond ? 
				// Condition false. Evaluate "false" branch: 
				tn->down.last()->Eval(retval,info) : 
				// Condition true. Eval "true" branch: 
				tn->down.first()->next->Eval(retval,info) )
			{
				assert(cond>=-1 && cond<=2);
				ess->pushS(cond);
				return(1);
			}
		break;
		default: assert(0);
	}
	
	return(0);
}

TNOperatorFunction::EvalFunc_IfElse::~EvalFunc_IfElse()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_IfElse2::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	EvalStateStack *ess=info ? info->ess : NULL;
	
	switch(ess->popS())  // Fallthrough switch. 
	{
		case 0:
			// Evaluate the condition: 
			if(tn->down.first()->Eval(retval,info))
			{
				ess->pushS(0);
				return(1);
			}
			
			if(!info && retval==ExprValue::None)
			{  /*retvsl is set*/  return(0);  }
			
			if(!retval.is_null())
			{
				// Condition true. Return condition itself. 
				// Nothing to do. 
				break;
			}// else-> false, see below. 
		case 1:
			// Condition false. Evaluate "false" branch: 
			if(tn->down.last()->Eval(retval,info))
			{
				ess->pushS(1);
				return(1);
			}
			break;
		default: assert(0);
	}
	
	return(0);
}

TNOperatorFunction::EvalFunc_IfElse2::~EvalFunc_IfElse2()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_LogicalAnd::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	#warning "For speedup, could have pre-declared >true< and >false< ExprValues."
	
	EvalStateStack *ess=info ? info->ess : NULL;
	
	ExprValue op;
	switch(ess->popS())  // Fallthrough switch. 
	{
		case 0:
			// First, eval first opernand: 
			if(tn->down.first()->Eval(op,info))
			{
				ess->pushS(0);
				return(1);
			}
			
			if(!info && op==ExprValue::None)
			{  retval=ExprValue::None;  return(0);  }
			
			if(op.is_null())
			{
				// First operator is null, so do not eval second one. 
				retval=ExprValue(ExprValue::Copy,(int)0);
				break;
			}
		case 1:
			// Next, eval second operand: 
			if(tn->down.last()->Eval(op,info))
			{
				ess->pushS(1);
				return(1);
			}
			
			if(!info && op==ExprValue::None)
			{  retval=ExprValue::None;  return(0);  }
			
			retval=ExprValue(ExprValue::Copy,op.is_null() ? 0 : 1);
			break;
		default: assert(0);
	}
	
	return(0);
}

TNOperatorFunction::EvalFunc_LogicalAnd::~EvalFunc_LogicalAnd()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_LogicalOr::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	#warning "For speedup, could have pre-declared >true< and >false< ExprValues."
	
	EvalStateStack *ess=info ? info->ess : NULL;
	
	ExprValue op;
	switch(ess->popS())  // Fallthrough switch. 
	{
		case 0:
			// First, eval first opernand: 
			if(tn->down.first()->Eval(op,info))
			{
				ess->pushS(0);
				return(1);
			}
			
			if(!info && op==ExprValue::None)
			{  retval=ExprValue::None;  return(0);  }
			
			if(!op.is_null())
			{
				// First operator is not null, so do not eval second one. 
				retval=ExprValue(ExprValue::Copy,1);
				break;
			}
		case 1:
			// Next, eval second operand: 
			if(tn->down.last()->Eval(op,info))
			{
				ess->pushS(1);
				return(1);
			}
			
			if(!info && op==ExprValue::None)
			{  retval=ExprValue::None;  return(0);  }
			
			retval=ExprValue(ExprValue::Copy,op.is_null() ? 0 : 1);
			break;
		default: assert(0);
	}
	
	return(0);
}

TNOperatorFunction::EvalFunc_LogicalOr::~EvalFunc_LogicalOr()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_MemberSelect::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	// Eval base: 
	ExprValue base;
	if(tn->down.first()->Eval(base,info))
	{  return(1);  }
	if(!info && base==ExprValue::None)
	{  retval=ExprValue::None;  return(0);  }
	
	// Get member element: 
	TNIdentifier *memb=(TNIdentifier*)tn->down.last();
	assert(memb->NType()==TN_Identifier);
	assert(memb->evaladrfunc);
	
	//fprintf(stderr,"Member select: <%s>\n",memb->CompleteStr().str());
	
	// Member select version of TNIdentifier's eval func: 
	retval=memb->evaladrfunc->Eval(memb,base,info);
	
	// In case there are variable notifiers installed, call them: 
	info->CallVarNotifiers(memb,retval);
	
	return(0);
}

TNOperatorFunction::EvalFunc_MemberSelect::~EvalFunc_MemberSelect()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_PODLength::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	// Eval "base": 
	ExprValue base;
	if(tn->down.first()->Eval(base,info))
	{  return(1);  }
	if(!info && base==ExprValue::None)
	{  retval=ExprValue::None;  return(0);  }
	
	Value tmp;
	base.GetPOD(&tmp);
	
	switch(ptype)
	{
		case Value::VTVector:
			retval.assign((double)tmp.GetPtr<Vector>()->abs());
			break;
		case Value::VTString:
			retval.assign((int)tmp.GetPtr<String>()->len());
			break;
		case Value::VTRange:
			retval.assign((int)tmp.GetPtr<Range>()->length());
			break;
		default: assert(0);
	}
	
	return(0);
}

TNOperatorFunction::EvalFunc_PODLength::~EvalFunc_PODLength()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_PODSize::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	// Eval "base": 
	ExprValue base;
	if(tn->down.first()->Eval(base,info))
	{  return(1);  }
	if(!info && base==ExprValue::None)
	{  retval=ExprValue::None;  return(0);  }
	
	Value tmp;
	base.GetPOD(&tmp);
	
	switch(magic)
	{
		case 'd':
			retval.assign((int)tmp.GetPtr<Vector>()->dim());
			break;
		case 'r':
			retval.assign((int)tmp.GetPtr<Matrix>()->rows());
			break;
		case 'c':
			retval.assign((int)tmp.GetPtr<Matrix>()->cols());
			break;
		default: assert(0);
	}
	
	return(0);
}

TNOperatorFunction::EvalFunc_PODSize::~EvalFunc_PODSize()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_ArraySize::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	// Eval "base": 
	ExprValue base;
	if(tn->down.first()->Eval(base,info))
	{  return(1);  }
	if(!info && base==ExprValue::None)
	{  retval=ExprValue::None;  return(0);  }
	
	ExprValueArray arr;
	base.GetArray(&arr);
	
	retval.assign((int)arr.GetSize());
	return(0);
}

TNOperatorFunction::EvalFunc_ArraySize::~EvalFunc_ArraySize()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_NewArray::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	// down.first() is the type specifier, next is the highest 
	// array type last is the lowest. 
	// Work done by the NewArraySpecifiers. 
	return(tn->down.first()->next->Eval(retval,info));
}

TNOperatorFunction::EvalFunc_NewArray::~EvalFunc_NewArray()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
// This is actually attached to TNNewArraySpecifier: 
int TNOperatorFunction::EvalFunc_NewArrayBottom::Eval(ExprValue &retval,
	TNOperatorFunction *new_arr_spec,ExecThreadInfo *info)
{
	// NOTE: new_arr_spec is of type TNNewArraySpecifier. 
	
	// This is the bottommost level. Create array of kown type. 
	// First, get size: 
	assert(new_arr_spec->down.first());
	ExprValue _arrsize;
	if(new_arr_spec->down.first()->Eval(_arrsize,info))
	{  return(1);  }
	ssize_t arrsize=_arrsize.GetPODInt();
	
	if(arrsize<0)
	{
		Error(new_arr_spec->GetLocationRange(),
			"OOPS: creating array of negative size %d\n",arrsize);
		abort();
	}
	
	// This will set prototype values. 
	ExprValueArray arr(arrsize,elem_type,/*set_prototype=*/1);
	
	retval=ExprValue(ExprValue::Copy,arr);
	return(0);
}

TNOperatorFunction::EvalFunc_NewArrayBottom::~EvalFunc_NewArrayBottom()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
// This is actually attached to TNNewArraySpecifier: 
int TNOperatorFunction::EvalFunc_NewArrayArray::Eval(ExprValue &retval,
	TNOperatorFunction *new_arr_spec,ExecThreadInfo *info)
{
	// NOTE: new_arr_spec is of type TNNewArraySpecifier. 
	
	EvalStateStack *ess=info ? info->ess : NULL;
	
	ExprValueArray arr;
	ssize_t doneelems=ess->popL(-1);
	switch(doneelems)  // Falltrough switch. 
	{
		case -1:
		{
			// First, get size: 
			assert(new_arr_spec->down.first());
			ExprValue _arrsize;
			if(new_arr_spec->down.first()->Eval(_arrsize,info))
			{
				ess->pushL(-1);
				return(1);
			}
			ssize_t arrsize=_arrsize.GetPODInt();
			
			if(arrsize<0)
			{
				Error(new_arr_spec->GetLocationRange(),
					"OOPS: creating array of negative size %d\n",arrsize);
				abort();
			}
			
			// Create the array...
			arr=ExprValueArray(arrsize,elem_type,/*set_prototype=*/0);
			retval=ExprValue(ExprValue::Copy,arr);
			
			doneelems=0;
		}	goto skippop;
		default:
			ess->popEV(&retval);
			retval.GetArray(&arr);
		skippop:
		{
			// ...and fill in the elements: 
			for(ssize_t i=doneelems,arrsize=arr.GetSize(); i<arrsize; i++)
			{
				// NOTE that we evaluate the array size for each element separately. 
				//      Hence,  new arr[10][++i]; will create an array of 10 integer
				//      arrays where these these 10 arrays have increasing size. 
				ExprValue tmp;
				if(new_arr_spec->next->Eval(tmp,info))
				{
					ess->pushL(i);  // doneelems
					ess->pushEV(retval);
					return(1);
				}
				int rv=arr.IndexSet(i,tmp);
				assert(rv==0);
			}
		}
	}
	return(0);
}

TNOperatorFunction::EvalFunc_NewArrayArray::~EvalFunc_NewArrayArray()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_PreIncDec::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	// Pre-increment/decrement is easy. 
	if(tn->down.first()->Eval(retval,info))
	{  return(1);  }
	
	// Actually increment/decrement: 
	assert(ida_func);
	ida_func->IncDec(retval);
	return(0);
}

TNOperatorFunction::EvalFunc_PreIncDec::~EvalFunc_PreIncDec()
{
	// Not inline because virtual. 
	DELETE(ida_func);
}

//------------------------------------------------------------------------------
int TNOperatorFunction::EvalFunc_PostIncDec::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	// Post-increment/decrement is more complicated than pre-inc/dec. 
	// The actual inc/dec operation has to be done lateron. 
	if(tn->down.first()->Eval(retval,info))
	{  return(1);  }
	
	// Queue the inc/dec. 
	info->QueueForDelayedEval(tn,retval);
	return(0);
}

int TNOperatorFunction::EvalFunc_PostIncDec::DelayedEval(ExprValue &val,
	TNOperatorFunction * /*tn*/,ExecThreadInfo * /*info*/)
{
	// Actually increment/decrement: 
	assert(ida_func);
	ida_func->IncDec(val);
	return(0);
}

TNOperatorFunction::EvalFunc_PostIncDec::~EvalFunc_PostIncDec()
{
	// Not inline because virtual. 
	DELETE(ida_func);
}

//==============================================================================

int TNDeclarator::EvalFunc::Eval(TNDeclarator * /*tn*/,
	ExecThreadInfo * /*info*/)
{
	// Must be overridden. 
	assert(0);
	return(0);
}

TNDeclarator::EvalFunc::~EvalFunc()
{
	// Not inline because virtual. 
}


void TNDeclarator::_RemoveEvalFunc()
{
	if(evalfunc)
	{
		if(evalfunc!=evalfunc_noop || --evalfunc_noop->refcnt<=0)
		{  DELETE(evalfunc);  }
		else
		{  evalfunc=NULL;  }
	}
}

TNDeclarator::EvalFunc *TNDeclarator::GetNoOpEvalFunc()
{
	if(!evalfunc_noop)
	{  new EvalFunc_NoOp();  }  // Constructor will set evalfunc_noop. 
	assert(evalfunc_noop);
	++evalfunc_noop->refcnt;
	return(evalfunc_noop);
}

//------------------------------------------------------------------------------
TNDeclarator::EvalFunc_NoOp *TNDeclarator::evalfunc_noop=NULL;

int TNDeclarator::EvalFunc_NoOp::Eval(
	TNDeclarator * /*tn*/,ExecThreadInfo * /*info*/)
{
	// - What's this?
	// - It's the no-op eval func. 
	// - And... what does it do?
	// - Nothing. 
	return(0);
}

TNDeclarator::EvalFunc_NoOp::EvalFunc_NoOp() : TNDeclarator::EvalFunc()
{
	refcnt=0;
	assert(!TNDeclarator::evalfunc_noop);
	TNDeclarator::evalfunc_noop=this;
}

TNDeclarator::EvalFunc_NoOp::~EvalFunc_NoOp()
{
	// Not inline because virtual. 
	assert(!refcnt);
	assert(this==TNDeclarator::evalfunc_noop);
	TNDeclarator::evalfunc_noop=NULL;
}

//------------------------------------------------------------------------------
static inline int _DoDeclAssignment(TNIdentifier *name,
	TNDeclarator *tn,ExecThreadInfo *info)
{
	assert(tn->GetDeclType()==TNDeclarator::DT_Initialize);
	
	//-OLD VERSION: (Now unusable due to possibility of context switches). 
	//- ExprValue initval=tn->down.last()->Eval(info);
	//- ExprValue lvalue=name->Eval(info);
	//- lvalue.assign(initval);
	
	// Fast version using a real C++ reference. 
	// No need for var set up with prototype value any more. 
	// However, we now need to set prototype values for all 
	// decls without initializer. 
	
	ExprValue initval;
	if(tn->down.last()->Eval(initval,info))
	{  return(1);  }
	assert(name->evaladrfunc);
	ExprValue &lvalue_ref=name->evaladrfunc->GetRef(name,info);
	// We expect the lvalue to have ExprValue::iev=NULL. 
	assert(!lvalue_ref);
	//lvalue_ref.assign(initval);  // Do NOT use "=" instead of assign(). 
	// NOW using assignment function instead: 
	tn->assfunc->Assign(lvalue_ref,initval);
	
	// Make sure that we actually set the value: This could be removed. 
	//assert(!!name->evalfunc->GetRef(name,info));
	
	// Debug message: 
	//fprintf(stderr,"  expr. initialized var %s with %s.\n",
	//	((AniScopeBase*)name->ani_scope)->CompleteName().str(),
	//	name->evalfunc->GetRef(name,info).ToString().str());
	
	// In case there are variable notifiers installed, call them: 
	info->CallVarNotifiers(name,lvalue_ref);
	
	return(0);
}

static inline void _DoDeclProto(TNIdentifier *name,const ExprValueType &type,
	TNDeclarator *tn,ExecThreadInfo *info)
{
	// Yes... it's !=DT_Initialize here -- we're here for a prototybe 
	// because of a missing initialisation. 
	assert(tn->GetDeclType()!=TNDeclarator::DT_Initialize);
	
	assert(name->evaladrfunc);
	ExprValue &lvalue_ref=name->evaladrfunc->GetRef(name,info);
	// We expect the lvalue to have ExprValue::iev=NULL. 
	assert(!lvalue_ref);
	// Could use "assign()" here instead of "=" as well...
	lvalue_ref=ExprValue(ExprValue::Prototype,type);
	
	// Make sure that we actually set the value: This could be removed. 
	//assert(!!name->evalfunc->GetRef(name,info));
	
	// Debug message: 
	//fprintf(stderr,"  proto initialized var %s with %s.\n",
	//	((AniScopeBase*)name->ani_scope)->CompleteName().str(),
	//	name->evalfunc->GetRef(name,info).ToString().str());
	
	// In case there are variable notifiers installed, call them: 
	info->CallVarNotifiers(name,lvalue_ref);
}

//------------------------------------------------------------------------------
int TNDeclarator::EvalFunc_Static::Eval(
	TNDeclarator *tn,ExecThreadInfo *info)
{
	if(_DoDeclAssignment(name,tn,info))
	{  return(1);  }
	
	// Store NoOp eval func because static variable has 
	// been initialized: 
	tn->_RemoveEvalFunc();
	tn->evalfunc=tn->GetNoOpEvalFunc();
	
	return(0);
}

TNDeclarator::EvalFunc_Static::~EvalFunc_Static()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNDeclarator::EvalFunc_Normal::Eval(
	TNDeclarator *tn,ExecThreadInfo *info)
{
	return(_DoDeclAssignment(name,tn,info));
}

TNDeclarator::EvalFunc_Normal::~EvalFunc_Normal()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNDeclarator::EvalFunc_ProtoStatic::Eval(
	TNDeclarator *tn,ExecThreadInfo *info)
{
	_DoDeclProto(name,type,tn,info);
	
	// Store NoOp eval func because static variable has 
	// been initialized: 
	tn->_RemoveEvalFunc();
	tn->evalfunc=tn->GetNoOpEvalFunc();
	
	return(0);
}

TNDeclarator::EvalFunc_ProtoStatic::~EvalFunc_ProtoStatic()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------
int TNDeclarator::EvalFunc_ProtoNormal::Eval(
	TNDeclarator *tn,ExecThreadInfo *info)
{
	_DoDeclProto(name,type,tn,info);
	return(0);
}

TNDeclarator::EvalFunc_ProtoNormal::~EvalFunc_ProtoNormal()
{
	// Not inline because virtual. 
}

//==============================================================================

int TNJumpStmt::EvalFunc::Eval(TNJumpStmt * /*tn*/,ExecThreadInfo * /*info*/)
{
	// Must be overridden. 
	assert(0);
	return(0);
}

TNJumpStmt::EvalFunc::~EvalFunc()
{
	// Not inline because virtual. 
}

}  // end of namespace func

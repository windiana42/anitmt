/*
 * ani-parser/exprtf.cc
 * 
 * AniVision programming language expression type-fix and constant 
 * folding / subexpression evaluation code. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include "exprtf.h"
#include "coreglue.h"
#include "opfunc.h"

#include <stdio.h>


#warning "**************************************************"
#warning "*******     !! CHECK ALL the FIXMEs !!     *******"
#warning "**************************************************"

// NOTE: is_const is actually unused except for detection of assignment 
//       and increment/decrement to const lvalues. Thus, is_const needs 
//       only be valid as long as is_lvalue is set. The rest is done 
//       lateron during subexpression evaluation. 
// FIXME: const flag for functions currently ignored. 

namespace ANI
{

// Not inline because rarely called. 
TFInfo::TFInfo() : 
	tp()
{
	n_nodes_visited=0;
	in_init=0;
	anidescstmt=NULL;
}

TFInfo::~TFInfo()
{
	// Nothing to do currently. 
}


int TreeNode::DoExprTF(TFInfo * /*ti*/,TreeNode ** /*update_tn*/)
{
	// TN_None may not exist at this stage...
	if(ntype==TN_None) assert(0);
	
	// This function must be overridden. 
	assert(0);
	return(1);
}


/*----------------------------------------------------------------------------*/

int TNIdentifierSimple::DoExprTF(TFInfo * /*ti*/,TreeNode ** /*update_tn*/)
{
	// We must stop at TNIdentifier and not go to 
	// TNIdentifierSimple. 
	assert(0);
}


int TNIdentifier::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	// NO NEED TO visit children. 
	
	int fail=0;
	
	// This must have been assigned by the registration step. 
	// (If not: either error happend and we should not be here or 
	// this is an internal error [left out identifier]). 
	assert(ani_scope);
	
	// fprintf(stderr,"IDENTIFIER: %s: <<%s>>=%s\n",
	//	GetLocationRange().PosRangeString().str(),
	//	CompleteStr().str(),
	//	ani_scope->AniScopeTypeStr());
	switch(ani_scope->CG_ASType())
	{
		case AniGlue_ScopeBase::AST_Incomplete:
		{
			// Look up incomplete type. 
			// If the name is of a variable, it must be unique (no type 
			//   or function may have that name). It is then unambiguously 
			//   resolvable and we return the AniVariable (also for member 
			//   select). 
			// If the name is a function, it may be ambiguous but may not 
			//   match a var or type name. In this case we return the 
			//   AniIncomplete. 
			// Higher level which does not expect AniIncomplete should 
			// check for that. 
			AniGlue_ScopeBase::LookupIdentifierInfo lii;
			ani_scope->CG_LookupIdentifier(&lii);
			if(lii.ani_scope)
			{
				// Lookup successful. The this->ani_scope was modified 
				// by the call updating it with the returned ani_scope 
				// and the old AniIncomplete was deleted. 
				// OR, for a function, ani_scope was left untouched. 
				assert(ani_scope==lii.ani_scope);

				// Store for eval: This need not be set. 
				if(lii.evalfunc)
				{
					assert(!evaladrfunc);
					evaladrfunc=lii.evalfunc;
				}
			}
			else
			{
				// Not successful. 
				// Error already written. 
				++fail;
			}
		}  break;
		case AniGlue_ScopeBase::AST_UserVar:
		{
			// This happens for declarations. We already assigned the 
			// final (non-incomplete) ani scope, so no lookup is 
			// required. 
			// In that case we still need the evalfunc. 
			assert(!evaladrfunc);
			evaladrfunc=ani_scope->CG_GetVarEvalAdrFuncFunc(
				/*idf_for_debug=*/this);
			if(!evaladrfunc)
			{  ++fail;  }
		}  break;
		case AniGlue_ScopeBase::AST_InternalVar:
		{
			// This happens for "$t in ADB function{}" for example. 
			assert(!evaladrfunc);
			evaladrfunc=ani_scope->CG_GetVarEvalAdrFuncFunc(
				/*idf_for_debug=*/this);
			if(!evaladrfunc)
			{  ++fail;  }
		}	break;
		case AniGlue_ScopeBase::AST_UserFunc:      // fall
		case AniGlue_ScopeBase::AST_InternalFunc:  //      through
		case AniGlue_ScopeBase::AST_Object:        //            ...
		case AniGlue_ScopeBase::AST_Setting:       //             here
			break;
		// No identifier name can refer to an animation or 
		// an anon scope. Grammar does not allow animation 
		// (although we could allow that) and AnonScope is 
		// ANONYMOUS. 
		case AniGlue_ScopeBase::AST_Animation:   // fall through
		case AniGlue_ScopeBase::AST_AnonScope:   assert(0);  break;
		default:  assert(0);
	}
	
	if(!fail)
	{
		// Store expression value type. 
		// (NOTE: This switch() cannot be merged with the switch() 
		// above because the ani_scope is likely to be changed by 
		// CG_LookupIdentifier().)
		switch(ani_scope->CG_ASType())
		{
			case AniGlue_ScopeBase::AST_Incomplete:
				ti->tp.sevt.SetAniScope(ani_scope);
				ti->tp.is_lvalue=0;
				ti->tp.is_const=0;
				ti->tp.has_effect=0;
				ti->tp.try_ieval=0;
				break;
			case AniGlue_ScopeBase::AST_UserVar:
			case AniGlue_ScopeBase::AST_InternalVar:
			{
				AniGlue_ScopeBase::VariableTypeInfo vti;
				vti.store_here=&ti->tp.sevt;
				ani_scope->CG_GetVariableType(&vti);
				ti->tp.is_lvalue=vti.is_lvalue;
				ti->tp.is_const=vti.is_const;
				ti->tp.has_effect=0;
				ti->tp.try_ieval=vti.may_ieval;
			}	break;
			case AniGlue_ScopeBase::AST_Animation:
			case AniGlue_ScopeBase::AST_Setting:
			case AniGlue_ScopeBase::AST_Object:
				ti->tp.sevt.SetAniScope(ani_scope);
				ti->tp.is_lvalue=0;
				ti->tp.is_const=0;
				ti->tp.has_effect=0;
				ti->tp.try_ieval=0;
				break;
			default: assert(0);
		}
	}
	
	return(fail);
}


int TNExpression::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	// This has to be overridden. 
	assert(expr_type==TNE_Expression);  // Otherwise may may not be here. 
	assert(down.first()==down.last());
	
	++ti->n_nodes_visited;
	// First, process children: 
	int fail=0;
	bool has_effect=0;
	bool try_ieval=1;
	for(TreeNode *i=down.first(); i; i=i->next)
	{
		int thisfail=i->DoExprTF(ti);
		fail+=thisfail;
		if(thisfail) continue;
		if(ti->tp.has_effect)
		{  has_effect=1;  }
		if(!ti->tp.try_ieval)
		{  try_ieval=0;  }
	}
	ti->tp.has_effect=has_effect;
	ti->tp.try_ieval=try_ieval;
	
	// Currently, this is always false here...
	assert(fail || try_ieval==0);
	
	return(fail);
}


int TNExpressionList::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	
	bool is_const=1;
	bool has_effect=0;
	
	// First, process children: 
	int fail=0;
	for(TreeNode *_i=down.first(); _i; )
	{
		TreeNode *i=_i;  // Needed because of the DoTryIEval(). 
		_i=_i->next;
		
		int thisfail=i->DoExprTF(ti);
		fail+=thisfail;
		
		if(!ti->tp.is_const)  is_const=0;
		if(ti->tp.has_effect)  has_effect=1;
		
		if(thisfail) continue;
		
		if(ti->tp.try_ieval)
		{  i->DoTryIEval();  }  // *i may be invalid below here!
	}
	
	ti->tp.is_lvalue=0;
	ti->tp.is_const=is_const;
	ti->tp.has_effect=has_effect;
	ti->tp.try_ieval=0;  // no expression any more...
	ti->tp.sevt.SetUnknown();
	
	return(fail);
}


int TNValue::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	
	// Store expression value type. 
	val.GetExprValueType(&ti->tp.sevt);
	ti->tp.is_lvalue=0;
	ti->tp.is_const=1;
	ti->tp.has_effect=0;
	ti->tp.try_ieval=1;
	
	return(0);
}


int TNArray::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	
	// Need array of child types: 
	int _dcount=down.count();
	ExprValueType *child_types=_dcount ? new ExprValueType[_dcount] : NULL;
	
	// First, process (and count) children: 
	int sumfail=0;
	ssize_t nchld=0;
	ExprValueType entry_type(ExprValueType::CUnknownType);
	bool is_const=1;
	bool has_effect=0;
	bool try_ieval=1;
	for(TreeNode *_i=down.first(); _i; nchld++)
	{
		TreeNode *i=_i;
		_i=_i->next;
		
		int fail=i->DoExprTF(ti);
		sumfail+=fail;
		if(fail) continue;
		
		// Store for use below: 
		child_types[nchld]=ti->tp.sevt;
		
		if(!ti->tp.is_const)  is_const=0;
		if(ti->tp.has_effect)  has_effect=1;
		if(ti->tp.try_ieval)
		{
			ExprValue tmp=i->DoTryIEval();  // i may be invalid below here. 
			// Get valid "i" again: 
			i=_i ? _i->prev : down.last();
			if(tmp==ExprValue::None)
			{  try_ieval=0;  }
		}
		else
		{  try_ieval=0;  }
		
		if(!nchld)
		{
			// First entry determines entry type for now. 
			// Widening can be performed in else branch. 
			entry_type=ti->tp.sevt;
		}
		else
		{
			// See if we can assign the it. This may require 
			// type conversion (WIDENING only). 
			#warning "in_assignment..."
			int rv=entry_type.CanAssignType(ti->tp.sevt,NULL);
			assert(rv>=0);  // If failure -> write user error! 
			if(rv==2)  continue;
			if(rv==1)  // Can apply widening on new entry. 
			{  continue;  }
			
			// See if we can do widening the other way round: 
			// Widen all the previous array elements: 
			rv=ti->tp.sevt.CanAssignType(entry_type,NULL);
			assert(rv>=0);  // If failure -> write user error! 
			if(rv==2)  assert(0);  // should habe been caught above
			if(rv==1)
			{
				// Need widening of prev elems: 
				entry_type=ti->tp.sevt;
				continue;
			}
			
			// NOTE: This means that we need a complete widening 
			//       chain because this may already be the second widening 
			//       attempt (e.g. scalar -> vector) and we must make sure 
			//       that also the first entry (e.g. integer) can be 
			//       widened into vector (although that is not checked here)!
			// NOTE: Arrays are types in AniVision. 
			// So, {1,2,3} is an int[]
			//     {1,1.2,4} is a scalar[]
			// etc. 
			Error(i->GetLocationRange(),
				"type mismatch in array elements: %s <-> %s\n",
				entry_type.TypeString().str(),
				ti->tp.sevt.TypeString().str());
			++sumfail;
			continue;
		}
	}
	
	if(!sumfail)
	{
		// Store for later use: 
		this->nelem=nchld;
		this->elem_type=entry_type;
		
		// Okay, need to set up the assignment functions: 
		this->assfunc=(AssignmentFuncBase**)
			LMalloc(nelem*sizeof(AssignmentFuncBase*));
		nchld=0;
		for(TreeNode *i=down.first(); i; i=i->next,nchld++)
		{
			AssignmentFuncBase *af=NULL;
			int rv=elem_type.CanAssignType(child_types[nchld],&af);
			if(rv==2)
			{
				// Direct assignment: 
				assfunc[nchld]=NULL;
				DELETE(af);
			}
			else if(rv==1)
			{
				// Need conversion: 
				assfunc[nchld]=af;
				assert(af);
			}
			// If this assert fails, then there seems to be a 
			// non-"closed" widening chain or a bug in the 
			// code above. 
			else assert(0);
		}
		
		// Okay, so this is essentially an array of type entry_type. 
		// NOTE THAT entry_type is unknown for empty arrays. 
		// (FIXME: consequences of that)
		ti->tp.sevt.SetArray(entry_type);
		ti->tp.is_lvalue=0;
		ti->tp.is_const=is_const;
		ti->tp.has_effect=has_effect;
		ti->tp.try_ieval=try_ieval;
		#warning "implement new [array]"
	}
	
	if(child_types)
	{  delete[] child_types;  }
	
	return(sumfail);
}

void TNArray::_DeleteAssFunc()
{
	if(assfunc)
	{
		for(ssize_t i=0; i<nelem; i++)
		{  DELETE(assfunc[i]);  }
		assfunc=(AssignmentFuncBase**)LFree(assfunc);
	}
}


int TNTypeSpecifier::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	
	fprintf(stderr,"Why do we need type specifier here?\n");
	// If we do, implement a GetExprValueType() function which then 
	// also has to be used by the core (registertn.cc). 
	assert(0);
	
	return(1);
}


int TNOperatorFunction::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	int sumfail=0;
	
	int handeled=0;
	bool _old_init_val=ti->in_init;
	ti->in_init=0;
	
	// Okay, let's come to the hard part...
	// This switch selects all operator functions which need 
	// special treatment (including the inc/decrement functions). 
	switch(func_id)
	{
		case OF_FCall:
		{
			// The first child is NOT an identifier. 
			// We need to get the type of the first child. It is most likely 
			// an ani scope type which is currently attached to an AniIncomplete. 
			// Let's see if the first child IS a function at all -- if not we 
			// can give up. 
			sumfail+=down.first()->DoExprTF(ti);
			if(sumfail)  break;
			const AniGlue_ScopeBase *ascope=ti->tp.sevt.AniScope();
			ExprValueType ft_for_error(ti->tp.sevt);
			
			// Need to get the type of all function arguments first, 
			// then resolve the AniIncomplete to a function using 
			// overloading rules...
			int n_args=down.count()-1;  // -1 for the identifier
			ExprValueType arg_type[n_args];
			TNIdentifier *arg_identif[n_args];  // Identifier for "name=val" style
			int i=0;
			bool try_ieval_all=1;
			bool any_has_effect=0;
			for(TreeNode *tn=down.first()->next; tn; tn=tn->next,i++)
			{
				int fail=tn->DoExprTF(ti);
				sumfail+=fail;
				if(fail)  continue;
				arg_type[i]=ti->tp.sevt;
				arg_identif[i] = (tn->down.first()==tn->down.last()) ? 
					NULL : (TNIdentifier*)tn->down.first();
				// NOTE: ti->tp.try_ieval + DoTryIEval() need not be done 
				//       here because that is done by the children which 
				//       are of type TNFCallArgument. 
				if(!ti->tp.try_ieval)
				{  try_ieval_all=0;  }
				if(ti->tp.has_effect)
				{  any_has_effect=1;  }
			}
			if(sumfail)
			{  break;  }
			
			// Note: ascope will be an AniIncomplete here in the normal 
			//       case. In error case or for special functions, it 
			//       can be something else. An apropriate error is 
			//       written by CG_LookupFunction()'s default 
			//       implementation. 
			// Need to resolve the function. 
			AniGlue_ScopeBase::LookupFunctionInfo lfi;
			lfi.n_args=n_args;
			lfi.arg_name=arg_identif;
			lfi.arg_type=arg_type;
			lfi.opfunc=this;
			if(!ascope || // (should not happen)
				((AniGlue_ScopeBase*)ascope)->CG_LookupFunction(&lfi)<0)
			{
				Error(down.first()->GetLocationRange(),
					"cannot call %s as a function\n",
					ft_for_error.TypeString().str());
				++sumfail;
				break;
			}
			if(!lfi.ani_function)
			{
				// Lookup failed; error written. 
				++sumfail;
				break;
			}
			// The function above already replaced the AniIncomplete 
			// in the TNIdentifier tree node with the correct ani function 
			// (although probably nobody will ever look at it again -- :p). 
			// NOTE THAT the AniIncomplete DELETED ITSELF. We can no longer 
			// use it. 
			ascope=NULL;  // Do not use any more. 
			
			// Okay, the type of the expression is now the return type of the 
			// function: 
			ti->tp.sevt=lfi.ret_evt;
			ti->tp.is_lvalue=0;
			#warning "Should see if function call is >const<, i.e. is subexpr-evaluable."
			ti->tp.is_const=0;
			// The has_effect code makes sure that a function has no
			// effect if we can const-fold it (like sin(),cos(),...). 
			// (It is then treated just like the constant and a statement 
			// "sin(2);" gets a warning "statement without effect".) 
			ti->tp.has_effect=(!lfi.may_try_ieval || any_has_effect);
			ti->tp.try_ieval=(lfi.may_try_ieval && try_ieval_all);
			
			// Store all the needed info for the evaluation: 
			// Fortunately, the core does the work for us ;)
			this->evalfunc=(TNOperatorFunction::EvalFunc*)lfi.evalfunc;
			assert(this->evalfunc);
			
			handeled=1;
		}	break;
		case OF_PODCast:
		{
			// POD type cast is just like the POD operators below. 
			// Just needs special treatment for the second arg. 
			// Expected args are: 
			//  0 -> argument value; 1 -> destination type
			
			// Variables lose their lvalue status after POD type cast. 
			
			assert(down.count()==2);
			ExprValueType arg_type[2];
			
			// Get destination type as stored in new_type: 
			arg_type[1]=new_type;  // Grammar only allows POD here. 
			
			sumfail+=down.last()->DoExprTF(ti);
			if(sumfail) break;
			arg_type[0]=ti->tp.sevt;
			
			ExprValueType res_type;
			OpFunc_Ptr fptr=LookupOperatorFunction(func_id,
				/*n_args=*/2,arg_type,&res_type);
			if(fptr)
			{
				// Successful cast must result in desired type. 
				assert(res_type.IsEqualTo(arg_type[1]));
				
				// Store result. 
				ti->tp.sevt=res_type;
				ti->tp.is_lvalue=0;
				//ti->tp.is_const= <unmodified>;
				//ti->tp.has_effect= <unmodified>;
				//ti->tp.try_ieval= <unmodified>;  executed lateron
				
				// Store for eval: 
				ExprValue proto_type(ExprValue::Prototype,new_type);
				this->evalfunc=new EvalFunc_PODCast(proto_type,fptr);
				
				handeled=1;
				break;
			}
			
			Error(down.first()->GetLocationRange(),
				"cannot type cast %s to %s\n",
				ti->tp.sevt.TypeString().str(),
				new_type.TypeString().str());
			++sumfail;  break;
			
		}	break;
		case OF_MembSel:  // fall through
		case OF_Mapping:
		{
			// FIXME: No difference between mapping and member select, 
			//        currently. 
			//        You can do things like "vector a; a->x=1;" 
			#warning "Mapping..."
			if(func_id==OF_Mapping)
			{
				Error(loc0,
					"mapping \"->\" not (yet) supported\n");
				++sumfail;  break;
			}
			
			// If successful, OF_MembSel and OF_Mapping both return 
			// some ani scope. If e.g. in the middle of an member select 
			// chain, an ani scope is returned; for functions, an 
			// AniIncomplete (which is in the correct position in the 
			// ani tree). 
			// Let's say we have: 
			//   object A { foo(){} }
			//   object B { bar() { A a=...;  a.foo();  }  }
			// Then the "a.foo" returns an AniIncomplete in scope "::A". 
			// This AniIncomplete is then resolved into a function based 
			// on the passed arguments (none in this case). 
			// A lot of this work is done by TNIdentifier. 
			assert(down.count()==2);
			
			// Get type of left operand: 
			sumfail+=down.first()->DoExprTF(ti);
			if(sumfail)  break;
			bool is_const=ti->tp.is_const;
			bool has_effect=ti->tp.has_effect;
			
			// Okay, the right operand must be a name. 
			// Yeah... and nothing else! Just got rid of some trouble :)
			TNIdentifier *rname=(TNIdentifier*)down.last();
			assert(rname->NType()==TN_Identifier);
			
			// Left type: 
			TFInfo::Type ltp=ti->tp;
			bool fail=1;
			do {
				AniGlue_ScopeBase *lasb=ltp.sevt.AniScope();
				if(lasb)
				{
					AniGlue_ScopeBase::AniScopeType as_type=lasb->CG_ASType();
					if(as_type!=AniGlue_ScopeBase::AST_Animation && 
					   as_type!=AniGlue_ScopeBase::AST_Setting && 
					   as_type!=AniGlue_ScopeBase::AST_Object )
					{
						if(as_type==AniGlue_ScopeBase::AST_UserVar || 
						   as_type==AniGlue_ScopeBase::AST_InternalVar )
						{  assert(0);  }  // no pointers allowed...
						// AST_*Var is INVALID below! Dealt with above. 
						if(as_type==AniGlue_ScopeBase::AST_Incomplete)
						{
							Error(down.first()->GetLocationRange(),
								"cannot use %s as base for member select\n",
								ltp.sevt.TypeString().str());
							++sumfail;  fail=0;
						}
						if(as_type==AniGlue_ScopeBase::AST_AnonScope)
						{  assert(0);  }
						break;
					}
					
					// If the identifier on the right hand side is not 
					// an incomplete place holder, then it has already 
					// been "looked up" and the member select is invalid. 
					// (This can happen for internal vars.) 
					if(rname->ani_scope->CG_ASType()!=
						AniGlue_ScopeBase::AST_Incomplete)
					{  break;  }  // fail stays 1. 
					
					// Exchange the AniIncomplete in the identifier rname. 
					// The new AniIncomplete must be in the correct position 
					// in the ani tree. 
					// Set the member select quality to 2, i.e. "explicit". 
					int ms_quality=2;
					// And the member select source to the left ASB: 
					AniGlue_ScopeBase *ms_from=lasb;
					
					// NOTE: lasb==rname->ani_scope may probably happen for 
					//       "this.this" (at least some time ago...)
					lasb->CG_ReplaceAniIncomplete(
						/*old_location=*/rname->ani_scope,
						/*member select info:*/ ms_from,ms_quality);
					
					// Do the lookup and assign the return type: 
					sumfail+=down.last()->DoExprTF(ti);
					if(!is_const)  ti->tp.is_const=0;
					if(has_effect)  ti->tp.has_effect=1;
					ti->tp.try_ieval=0;  // member select/mapping -> identifier
					
					// Store for eval: 
					this->evalfunc=new EvalFunc_MemberSelect();
					
					handeled=1;  fail=0;  break;
				}
				const Value::CompleteType *pod=ltp.sevt.PODType();
				if(pod)
				{
					if(pod->type!=Value::VTMatrix && 
					   pod->type!=Value::VTVector && 
					   pod->type!=Value::VTRange && 
					   pod->type!=Value::VTString )
					{  break;  }
					
					// For matrix, vector and range, there are special 
					// built-in "members". 
					if(rname->down.first()!=rname->down.last())
					{  break;  }
					const RefString &rstr=
						((TNIdentifierSimple*)rname->down.first())->name;
					int idx=-10;  // -1,-2,... special
					     if(rstr=="x")       idx=0;
					else if(rstr=="y")       idx=1;
					else if(rstr=="z")       idx=2;
					else if(rstr=="a")       idx=-1;
					else if(rstr=="b")       idx=-2;
					else if(rstr=="length")  idx=-3;
					else if(rstr=="dim")     idx=-4;
					else if(rstr=="rows")    idx=-5;
					else if(rstr=="cols")    idx=-6;
					// FIXME: maybe .x0, .x1, .x2... for vector. 
					else break;
					
					ExprValue::_IndexPOD index_type=
						(ExprValue::_IndexPOD)(-1234);
					Value::CompleteType res;
					bool can_be_lvalue=1;
					if(idx==-3)  // "length"
					{
						if(pod->type==Value::VTVector)
						{  res.type=Value::VTScalar;  }
						else if(pod->type==Value::VTString)
						{  res.type=Value::VTInteger;  }
						else if(pod->type==Value::VTRange)
						{  res.type=Value::VTScalar;  }
						else break;  // <-- Important!
						this->evalfunc=new EvalFunc_PODLength(pod->type);
						can_be_lvalue=0;
					}
					else if(idx==-4 && pod->type==Value::VTVector)
					{
						res.type=Value::VTInteger;
						this->evalfunc=new EvalFunc_PODSize('d');
						can_be_lvalue=0;
					}
					else if((idx==-5 || idx==-6) && pod->type==Value::VTMatrix)
					{
						res.type=Value::VTInteger;
						this->evalfunc=new EvalFunc_PODSize(
							(idx==-5) ? 'r' : 'c');
						can_be_lvalue=0;
					}
					else if((idx==-1 || idx==-2) && pod->type==Value::VTRange)
					{
						res.type=Value::VTScalar;
						index_type=ExprValue::IndexPODRange;
					}
					else if(idx>=0 && pod->type==Value::VTVector && 
						pod->n>idx)
					{
						res.type=Value::VTScalar;
						index_type=ExprValue::IndexPODVector;
					}
					else if(idx>=0 && pod->type!=Value::VTMatrix && 
						int(pod->r)>idx)
					{
						res.type=Value::VTVector;
						res.n=int(pod->c);
						index_type=ExprValue::IndexPODMatrix;
					}
					else break;  // <-- Important!
					
					ti->tp.sevt.SetPOD(res);
					if(!can_be_lvalue)  ti->tp.is_lvalue=0;
					//else: ti->tp.is_lvalue <unmodified>
					// ti->tp.is_const <unmodified>
					// ti->tp.has_effect <unmodified>
					// ti->tp.try_ieval <unmodified> and executes lateron
					
					// Store for eval: 
					if(!this->evalfunc)
					{
						assert(index_type!=-1234);
						this->evalfunc=new EvalFunc_SubscriptPOD(index_type,
							/*fixed_idx=*/(idx<0 ? -(idx+1) : idx) );
					}
					this->ms_is_subscript=1;
					
					handeled=1; fail=0;  break;
				}
				ExprValueType arrtype=ltp.sevt.ArrayType();
				if(!!arrtype)
				{
					if(rname->down.first()!=rname->down.last())
					{  break;  }
					const RefString &rstr=
						((TNIdentifierSimple*)rname->down.first())->name;
					if(rstr!="size")  break;
					
					Value::CompleteType res;
					res.type=Value::VTInteger;
					
					ti->tp.sevt.SetPOD(res);
					ti->tp.is_lvalue=0;
					// ti->tp.is_const <unmodified>
					// ti->tp.has_effect <unmodified>
					// ti->tp.try_ieval <unmodified> and executes lateron
					
					// Store for eval: 
					this->evalfunc=new EvalFunc_ArraySize();
					this->ms_is_subscript=1;
					
					handeled=1; fail=0;  break;
				}
			} while(0);
			
			if(fail)
			{
				Error(loc0,
					"invalid request for member %s inside type %s\n",
					rname->CompleteStr().str(),
					ltp.sevt.TypeString().str());
				++sumfail;  break;
			}
			
		}	break;
		case OF_Subscript:
		{
			// Subscript has similarities with member select. 
			assert(down.count()==2);
			
			// Get type of subscript expression operand: 
			sumfail+=down.last()->DoExprTF(ti);
			if(sumfail)  break;
			bool is_const=ti->tp.is_const;
			bool has_effect=ti->tp.has_effect;
			
			const Value::CompleteType *pod=ti->tp.sevt.PODType();
			if(!pod || pod->type!=Value::VTInteger)
			{
				Error(down.last()->GetLocationRange(),
					"unsuitable index type (%s) for subscript\n",
					ti->tp.sevt.TypeString().str());
				++sumfail;  break;
			}
			
			if(ti->tp.try_ieval)
			{
				ExprValue tmp=down.last()->DoTryIEval();
				ti->tp.try_ieval=!(tmp==ExprValue::None);
			}
			bool try_ieval=ti->tp.try_ieval;
			
			// Get the type of the operand which we like to 
			// use subscript on: 
			sumfail+=down.first()->DoExprTF(ti);
			if(sumfail)  break;
			
			// Uncomment this to test multiple indices during 
			// immediate constant folding: 
			//#warning ENABLE THIS!!!
			//fprintf(stderr,"exprtf.cc:%d: ENABLE THIS*******************\n",__LINE__);
			if(ti->tp.try_ieval)
			{
				ExprValue tmp=down.first()->DoTryIEval();
				ti->tp.try_ieval=!(tmp==ExprValue::None);
			}
			
			// We need to see if that is an array, etc. For that, we need 
			// the type. 
			do {
				ExprValueType arr_type=ti->tp.sevt.ArrayType();
				if(!!arr_type)
				{
					// Okay... and the resulting type is: arr_type. 
					ti->tp.sevt=arr_type;
					// If the index is non-const, the result is non-const, too. 
					if(!is_const)  ti->tp.is_const=0;
					if(has_effect)  ti->tp.has_effect=1;
					// The result type is an lvalue if it was an lvalue before: 
					//  {1,2,3}[1]  yields to NO lvalue
					//  array[1]  yields to an lvalue (with array being a variable)
					// ti->tp.is_lvalue <unmodified>
					if(!try_ieval)  ti->tp.try_ieval=0;
					
					// Store for eval: 
					this->evalfunc=new EvalFunc_SubscriptArray();
					
					handeled=1;
					break;
				}
				
				const Value::CompleteType *pod=ti->tp.sevt.PODType();
				if(pod)
				{
					// Subscript on vectors results in scalars; subscript 
					// on matrices in vectors. 
					bool pod_ok=0;
					Value::CompleteType res;
					if(pod->type==Value::VTMatrix)
					{
						// Store for eval: 
						this->evalfunc=new EvalFunc_SubscriptPOD(
							ExprValue::IndexPODMatrix);
						
						res.type=Value::VTVector;
						res.n=int(pod->c);
						pod_ok=1;
					}
					else if(pod->type==Value::VTVector)
					{
						// Store for eval: 
						this->evalfunc=new EvalFunc_SubscriptPOD(
							ExprValue::IndexPODVector);
						
						res.type=Value::VTScalar;
						pod_ok=1;
					}
					// Subscript for String returns character (as string): 
					else if(pod->type==Value::VTString)
					{
						// Store for eval: 
						this->evalfunc=new EvalFunc_SubscriptPOD(
							ExprValue::IndexPODString);
						
						res.type=Value::VTString;
						pod_ok=1;
					}
					
					if(pod_ok)
					{
						ti->tp.sevt.SetPOD(res);
						if(!is_const)  ti->tp.is_const=0;
						if(has_effect)  ti->tp.has_effect=1;
						// ti->tp.is_lvalue <unmodified>
						if(!try_ieval)  ti->tp.try_ieval=0;
						
						handeled=1;
						break;
					}
				}
				
				Error(down.first()->GetLocationRange(),
					"subscript attempt on non-array type %s\n",
					ti->tp.sevt.TypeString().str());
				++sumfail;  break;
			} while(0);
			
		}	break;
		case OF_New:
		{
			// The first child IS an identifier. 
			sumfail+=down.first()->DoExprTF(ti);
			if(sumfail)  break;
			AniGlue_ScopeBase *ascope=ti->tp.sevt.AniScope();
			if(!ascope || 
			    (ascope->CG_ASType()!=AniGlue_ScopeBase::AST_Object))
			{
				Error(down.first()->GetLocationRange(),
					"cannot use operator new on non-object type %s\n",
					ti->tp.sevt.TypeString().str());
				++sumfail;
				break;
			}
			
			// Need to get the type of all constructor arguments first, 
			// then resolve the constructor using overloading rules...
			int n_args=down.count()-1;  // -1 for the identifier
			ExprValueType arg_type[n_args];
			TNIdentifier *arg_identif[n_args];  // Identifier for "name=val" style
			int i=0;
			for(TreeNode *tn=down.first()->next; tn; tn=tn->next,i++)
			{
				int fail=tn->DoExprTF(ti);
				sumfail+=fail;
				if(fail)  continue;
				arg_type[i]=ti->tp.sevt;
				arg_identif[i] = (tn->down.first()==tn->down.last()) ? 
					NULL : (TNIdentifier*)tn->down.first();
			}
			if(sumfail)
			{  break;  }
			
			// Okay... now try to resolve it: 
			// ascope will be an AniObject here. 
			if(ascope->CG_ASType()!=AniGlue_ScopeBase::AST_Object)
			{  assert(0);  /* That is not supposed to happen. */  }
			
			// Need to resolve the constructor function. 
			AniGlue_ScopeBase::LookupFunctionInfo lfi;
			lfi.n_args=n_args;
			lfi.arg_name=arg_identif;
			lfi.arg_type=arg_type;
			// I do NOT set call_loc here...
			//lfi.call_loc=NULL;
			// ...but name_idf. 
			lfi.name_idf=(TNIdentifier*)down.first();
			((AniGlue_ScopeBase*)ascope)->CG_LookupConstructor(&lfi);
			if(!lfi.ani_function)
			{
				// Lookup failed; error written. 
				++sumfail;
				break;
			}
			
			// Okay, the type of the expression is now the return type 
			// of the operator new: 
			ti->tp.sevt.SetAniScope(ascope);
			ti->tp.is_lvalue=0;
			#warning "Should see if constructor call returns >const<??, i.e. is subexpr-evaluable."
			ti->tp.is_const=0;
			ti->tp.has_effect=1;
			ti->tp.try_ieval=0;  // operator new...
			
			// Store all the needed info for the evaluation: 
			// Fortunately, the core does the work for us ;)
			// 
			// The identifier gets evaulated to return the "this" pointer 
			// of the newly created object: 
			TNIdentifier *our_name=(TNIdentifier*)down.first();
			// The identifier may NOT have an eval func assigned. 
			// If he has, it is the wrong one and then it is a BUG!
			assert(!our_name->evaladrfunc);
			assert(lfi.allocfunc);
			our_name->evaladrfunc=(AniGlue_ScopeBase::EvalAddressationFunc*)
				lfi.allocfunc;
			// EvalFunc_OpNew will only allocate the object but not 
			// call the constructor which is done here: 
			assert(lfi.evalfunc);
			this->evalfunc=(TNOperatorFunction::EvalFunc*)lfi.evalfunc;
			
			handeled=1;
		}	break;
		case OF_NewArray:
		{
			bool is_const=1;
			
			// See if we may build arrays with this type: 
			int arr_fail=1;
			do {
				const Value::CompleteType *pod=new_type.PODType();
				if(pod)
				{
					switch(pod->type)
					{
						// Fall through switch. 
						case Value::VTInteger:
						case Value::VTScalar:
						case Value::VTRange:
						case Value::VTVector:
						case Value::VTMatrix:
						case Value::VTString:
							arr_fail=0;
							break;
					}
					break;
				}
				AniGlue_ScopeBase *asb=new_type.AniScope();
				if(asb)
				{
					switch(asb->CG_ASType())
					{
						case AniGlue_ScopeBase::AST_Object:
						{
							// FIXME:!!!
							// set is_const=0 if the lfi.ani_function 
							// is non-const. Urmha... the is_const thingy is flawed...
							is_const=0;  // FIXME!!!! (see above)
							
							arr_fail=0;
						} break;
					}
				}
			} while(0);
			if(arr_fail)
			{
				Error(down.first()->GetLocationRange(),
					"cannot build array of type %s\n",
					new_type.TypeString().str());
				++sumfail;  break;
			}
			
			// First. get the type of the array: 
			// No need to look at the type specifier; it has 
			// already been done. 
			ExprValueType arr_evt=new_type;
			
			bool seen_expr=0;
			TreeNode *bottommost=NULL;
			for(TreeNode *_tn=down.last(); _tn->prev; _tn=_tn->prev)
			{
				int fail=_tn->DoExprTF(ti);
				sumfail+=fail;
				if(fail) continue;
				if(!ti->tp.is_const)
				{  is_const=0;  }
				// ti->tp.sevt must be integer or void. 
				if(ti->tp.sevt.TypeID()==EVT_POD && 
					ti->tp.sevt.PODType()->type==Value::VTInteger )
				{
					seen_expr=1;
					if(!bottommost)  bottommost=_tn;
				}
				else if(ti->tp.sevt.TypeID()==EVT_Void)
				{
					if(seen_expr)
					{
						TNLocation loc=_tn->next->GetLocationRange();
						Error(loc,
							"missing array size spec in previous level @%s\n",
							_tn->GetLocationRange().
								PosRangeStringRelative(loc).str());
						++sumfail;  break;
					}
				}
				else
				{  assert(0);  }
				
				ExprValueType tmp=arr_evt;
				arr_evt.SetArray(tmp);
			}
			if(sumfail)  break;
			
			// *bottommost stores the bottom most node with a size 
			// specifier. 
			assert(bottommost);
			
			ti->tp.sevt=arr_evt;
			ti->tp.is_const=is_const;
			ti->tp.is_lvalue=0;
			ti->tp.has_effect=1;
			ti->tp.try_ieval=0;  // operator new (array)
			
			// Store for eval: 
			this->evalfunc=new EvalFunc_NewArray();
			
			ExprValueType tmptype(arr_evt);
			for(TreeNode *tn=down.first()->next; tn; tn=tn->next)
			{
				tmptype=tmptype.ArrayType();
				assert(!!tmptype);
				
				TNNewArraySpecifier *nas=(TNNewArraySpecifier*)tn;
				assert(nas->NType()==TN_NewArraySpecifier);
				assert(!nas->down.is_empty());
				assert(!nas->evalfunc);
				
				if(tn==bottommost)
				{
					assert(!nas->next || nas->next->down.is_empty());
					nas->evalfunc=new EvalFunc_NewArrayBottom(tmptype);
					break;
				}
				nas->evalfunc=new EvalFunc_NewArrayArray(tmptype);
			}
			
			handeled=1;
		}	break;
		case OF_Delete:
		{
			// See what we want to delete: 
			sumfail+=down.first()->DoExprTF(ti);
			if(sumfail)  break;
			
			bool can_delete=0;
			do {
				if(ti->tp.sevt.TypeID()!=EVT_Array)
				{  can_delete=1;  break;  }
				if(ti->tp.sevt.TypeID()==EVT_AniScope &&
					ti->tp.sevt.AniScope()->CG_ASType()==
						AniGlue_ScopeBase::AST_Object)
				{  can_delete=1;  break;  }
			} while(0);
			if(!can_delete)
			{
				Error(down.first()->GetLocationRange(),
					"cannot use delete on type %s\n",
					ti->tp.sevt.TypeString().str());
				++sumfail;  break;
			}
			
			ti->tp.sevt.SetVoid();
			//ti->tp.is_const=<unmodified>
			ti->tp.is_lvalue=0;
			ti->tp.has_effect=1;
			ti->tp.try_ieval=0;  // operator delete
			
			// Store for eval: 
			// FIXME: MISSING. 
			this->evalfunc=NULL;
			
			handeled=1;
		}	break;
		case OF_PostIncrement:  // fall...
		case OF_PostDecrement:  // ...through...
		case OF_PreIncrement:   // ...to...
		case OF_PreDecrement:   // ...here: 
		{	
			// Check the operand: 
			assert(down.first());
			sumfail+=down.first()->DoExprTF(ti);
			if(sumfail)  break;
			
			if(!ti->tp.is_lvalue)
			{
				Error(down.first()->GetLocationRange(),
					"cannot %screment non-lvalue\n",
					(func_id==OF_PostIncrement || func_id==OF_PreIncrement) ? 
						"in" : "de");
				++sumfail;  break;
			}
			
			// FIXME: correct? "I mean -- use of is_const flag..."
			//        (also for assignment!)
			if(ti->tp.is_const)
			{
				Error(down.first()->GetLocationRange(),
					"cannot %screment const lvalue\n",
					(func_id==OF_PostIncrement || func_id==OF_PreIncrement) ? 
						"in" : "de");
				++sumfail;  break;
			}
			
			int inc_dec=0;
			if(func_id==OF_PreIncrement || func_id==OF_PostIncrement)
			{  inc_dec=+1;  }
			else if(func_id==OF_PreDecrement || func_id==OF_PostDecrement)
			{  inc_dec=-1;  }
			assert(inc_dec!=0);
			
			IncDecAssFuncBase *assfunc=NULL;
			if(!ti->tp.sevt.CanIncDec(&assfunc,inc_dec))
			{
				Error(down.first()->GetLocationRange(),
					"cannot %screment lvalue of type %s\n",
					inc_dec>0 ? "in" : "de",
					ti->tp.sevt.TypeString().str());
				++sumfail;  break;
			}
			
			// Type stays unmodified. Even lvalue status. 
			// Just that it now has an effect. 
			ti->tp.has_effect=1;
			ti->tp.try_ieval=0;  // inc/dec !
			
			// Store for eval: 
			if(func_id==OF_PreIncrement || func_id==OF_PreDecrement)
			{  this->evalfunc=new EvalFunc_PreIncDec(assfunc);  }
			else if(func_id==OF_PostIncrement || func_id==OF_PostDecrement)
			{  this->evalfunc=new EvalFunc_PostIncDec(assfunc);  }
			else assert(0);
			
			handeled=1;
		}	break;
			
		default:
		{
			// All the other operators. 
			// Variables lose their lvalue status when these 
			// operators are applied to them. 
			
			// Okay, try standard POD operators; first check the 
			// argument types. 
			int n_args=down.count();
			ExprValueType arg_type[n_args];
			int idx_cnt=0;
			int n_const=0;
			bool has_effect=0;
			int try_ieval=0;  // counter
			int first_try_ieval=-1;  // index
			bool is_lvalue3[3]={0,0,0};  // only the first 3
			for(TreeNode *_tn=down.first(); _tn; idx_cnt++)
			{
				TreeNode *tn=_tn;
				_tn=_tn->next;
				
				int fail=tn->DoExprTF(ti);
				sumfail+=fail;
				if(fail)  continue;
				arg_type[idx_cnt]=ti->tp.sevt;
				// Count constant args: 
				if(ti->tp.is_const)  ++n_const;
				if(ti->tp.has_effect)  has_effect=1;
				// lvalue "statistics"
				if(ti->tp.is_lvalue && idx_cnt<3)
				{  is_lvalue3[idx_cnt]=1;  }
				
				if(ti->tp.try_ieval)
				{
					ExprValue tmp=tn->DoTryIEval();  // tn may be invalid below here
					if(!(tmp==ExprValue::None))
					{
						++try_ieval;
						if(first_try_ieval<0)
						{  first_try_ieval=idx_cnt;  }
					}
				}
			}
			if(sumfail)
			{  break;  }
			
			// Special functions: 
			if(func_id==OF_IfElse || func_id==OF_IfElse2)
			{
				if( (func_id==OF_IfElse) ? 
						!arg_type[1].IsEqualTo(arg_type[2]) : 
						!arg_type[0].IsEqualTo(arg_type[1]) )
				{
					Error(loc0,
						"type mismatch %s <-> %s for operator `?:'\n",
						arg_type[func_id==OF_IfElse ? 1 : 0].TypeString().str(),
						arg_type[func_id==OF_IfElse ? 2 : 1].TypeString().str());
					++sumfail;  break;
				}
				
				// Store result. 
				//ti->tp.sevt= <set below>;
				//ti->tp.is_lvalue= <set below>;
				ti->tp.is_const=(n_const==n_args);
				ti->tp.has_effect=has_effect;
				ti->tp.try_ieval=(first_try_ieval==0 ? 1 : 0);
				
				if(func_id==OF_IfElse)
				{
					assert(n_args==3);
					
					ti->tp.is_lvalue=(is_lvalue3[1] && is_lvalue3[2]) ? 1 : 0;
					ti->tp.sevt=arg_type[1];
					
					// Store info for eval: 
					this->evalfunc=new EvalFunc_IfElse();
				}
				else if(func_id==OF_IfElse2)
				{
					assert(n_args==2);
					
					ti->tp.is_lvalue=(is_lvalue3[0] && is_lvalue3[1]) ? 1 : 0;
					ti->tp.sevt=arg_type[0];
					
					// Store info for eval: 
					this->evalfunc=new EvalFunc_IfElse2();
				}
				else assert(0);
				
				handeled=1;
				break;
			}
			else if(func_id==OF_LogicalAnd || func_id==OF_LogicalOr)
			{
				// Accept any non-void type: 
				assert(n_args==2);
				bool argfail=0;
				for(int i=0; i<n_args; i++)
				{
					switch(arg_type[i].TypeID())
					{
						// Fall through switch: 
						case EVT_POD:
						case EVT_AniScope:
						case EVT_Array:
							break;
						default:  argfail=1;  goto breakfor;
					}
				}
				breakfor:;
				if(argfail)  break;  // -> sumfail=0 -> error written below
				
				// Store result. 
				Value::CompleteType ctype;
				ctype.type=Value::VTInteger;
				ti->tp.sevt.SetPOD(ctype);
				ti->tp.is_lvalue=0;
				ti->tp.is_const=(n_const==n_args);
				ti->tp.has_effect=has_effect;
				ti->tp.try_ieval=(first_try_ieval==0 ? 1 : 0);
				
				// Store info for eval: 
				if(func_id==OF_LogicalAnd)
				{  this->evalfunc=new EvalFunc_LogicalAnd();  }
				else if(func_id==OF_LogicalOr)
				{  this->evalfunc=new EvalFunc_LogicalOr();  }
				else assert(0);
				
				handeled=1;
				break;
			}
			else
			{
				// Try operator functions: 
				ExprValueType res_type;
				OpFunc_Ptr fptr=LookupOperatorFunction(func_id,
					n_args,arg_type,&res_type);
				if(fptr)
				{
					// Store result. 
					ti->tp.sevt=res_type;
					ti->tp.is_lvalue=0;
					ti->tp.is_const=(n_const==n_args);
					ti->tp.has_effect=has_effect;
					ti->tp.try_ieval = (try_ieval==idx_cnt ? 1 : 0);

					// Store info for eval: 
					assert(!this->evalfunc);
					this->evalfunc=new EvalFunc_DefaultOpFunc(n_args,fptr);

					handeled=1;
					break;
				}
			}
			
			assert(!handeled && !sumfail);
			if(n_args==1)
			{
				Error(loc0,
					"no match for operator %s %s\n",
					OperatorFuncIDString(func_id),
					arg_type[0].TypeString().str());
				++sumfail;
			}
			else if(n_args==2)
			{
				Error(loc0,
					"no match for operator %s %s %s\n",
					arg_type[0].TypeString().str(),
					OperatorFuncIDString(func_id),
					arg_type[1].TypeString().str());
				++sumfail;
			}
			else
			{
				// FIXME: improve error message: write types. 
				Error(loc0,
					"type mismatch for operator %s\n",
					OperatorFuncIDString(func_id));
				++sumfail;
			}
			
		}	break;
	}
	
	assert(sumfail || handeled);
	
	ti->in_init=_old_init_val;
	return(sumfail);
}


int TNFCallArgument::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	
	// First, look at the expression which is the function argument. 
	assert(!ti->in_init);
	ti->in_init=1;
	int fail=down.last()->DoExprTF(ti);
	ti->in_init=0;
	// Okay, ti now stores the type of the function arg. If an 
	// identifier was given ("name=value" style), it is in 
	// down.first() [if != down.last()]. We don't care -- the 
	// TNOperatorFunction OF_FCall deals with that. 
	
	// This is important because we need to know the definitive 
	// eval status of all args for function eval. 
	if(!fail && ti->tp.try_ieval)
	{
		ExprValue tmp=down.last()->DoTryIEval();
		ti->tp.try_ieval=!(tmp==ExprValue::None);
	}
	
	return(fail);
}


int TNAssignment::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	
	assert(down.count()==2);
	
	int fail=0;
	do {
		// First, look at the expression to be assigned. 
		fail+=down.last()->DoExprTF(ti);
		if(fail)  break;
		TFInfo::Type rtp=ti->tp;
		
		if(rtp.try_ieval)
		{  down.last()->DoTryIEval();  }
		
		// Then, check the lvalue: 
		fail+=down.first()->DoExprTF(ti);
		if(fail)  break;
		
		if(!ti->tp.is_lvalue)
		{
			Error(down.first()->GetLocationRange(),
				"not an lvalue in assignment\n");
			++fail;  break;
		}
		if(ti->tp.is_const)
		{
			Error(down.first()->GetLocationRange(),
				"assignment to constant expression\n");
			++fail;  break;
		}
		
		// Check type match: 
		assert(!ti->in_init);
		int rv=ti->tp.sevt.CanAssignType(rtp.sevt,&assfunc,ass_op);
		assert(rv>=0);  // Write user-error on failure?
		if(rv==0)
		{
			Error(loc,
				"cannot assign type %s to value of type %s%s%s\n",
				rtp.sevt.TypeString().str(),
				ti->tp.sevt.TypeString().str(),
				ass_op==AO_Set ? "" : " using ",
				ass_op==AO_Set ? "" : AssignmentOpIDString(ass_op));
			++fail;  break;
		}
		
		// Okay...
		// Assignment returns void. (For lvalue type, uncomment next line). 
		//ti->tp.sevt= <unmodified>
		ti->tp.sevt.SetVoid();
		ti->tp.is_lvalue=0;
		ti->tp.is_const=rtp.is_const;
		ti->tp.has_effect=1;
		ti->tp.try_ieval=0;
	} while(0);
	
	return(fail);
}


int TNStatement::DoExprTF(TFInfo * /*ti*/,TreeNode ** /*update_tn*/)
{
	// This has to be overridden. 
	assert(0);
	return(1);
}


int TNExpressionStmt::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	// Max 1 child. 
	int sumfail=0;
	while(down.first())
	{
		// If the complete stmt is just a post in/decrement, 
		// convert it into a pre in/decrement because that is 
		// easier to execute. 
		// THIS MUST BE DONE HERE BEFORE CALLING DoExprTF(). 
		// Huh... that's what I call an expensive micro-optimization...
		do {
			if(down.first()->NType()!=TN_Expression || 
			   ((TNExpression*)down.first())->ExprType()!=TNE_OperatorFunction)
			{  break;  }
			TNOperatorFunction *of=(TNOperatorFunction*)down.first();
			if(of->GetFuncID()!=OF_PostIncrement && 
			   of->GetFuncID()!=OF_PostDecrement )  break;
			if(of->down.count()!=1)  break;
			TreeNode *wlk=of->down.first();
			if(wlk->NType()!=TN_Expression)  break;
			if(wlk->down.count()!=1)  break;
			wlk=wlk->down.first();
			if(wlk->NType()!=TN_Identifier)  break;
			switch(of->GetFuncID())
			{
				case OF_PostIncrement:  of->func_id=OF_PreIncrement;  break;
				case OF_PostDecrement:  of->func_id=OF_PreDecrement;  break;
				default: assert(0);  // See if() above. 
			}
		} while(0);
		
		int fail=down.first()->DoExprTF(ti);
		sumfail+=fail;
		if(fail)  break;
		if(!ti->tp.has_effect)
		{
			Warning(down.first()->GetLocationRange(),
				"statement without effect\n");
		}
		if(ti->tp.try_ieval)
		{
			ExprValue tmp=down.first()->DoTryIEval();
			//ti->tp.try_ieval=!(tmp==ExprValue::CUnknownValue);
		}
		
		break;
	}
	ti->tp.try_ieval=0;  // no expression but statement
	return(sumfail);
}


int TNIterationStmt::DoExprTF(TFInfo *ti,TreeNode **update_tn)
{
	++ti->n_nodes_visited;
	int fail=0;
	
	// First, look at the loop condition: 
	bool is_const=1;
	int can_optimize=0;
	while(loop_cond)
	{
		fail+=loop_cond->DoExprTF(ti);
		if(fail)  break;
		if(!ti->tp.is_const)  is_const=0;
		
		// Loop condition must be bool-convertable to "int". 
		if(!ti->tp.sevt.CanBoolConvert())
		{
			Error(loop_cond->GetLocationRange(),
				"cannot bool-convert expression of type %s\n",
				ti->tp.sevt.TypeString().str());
			++fail;  break;
		}
		
		if(ti->tp.try_ieval)
		{
			ExprValue tmp=loop_cond->DoTryIEval((TreeNode**)&loop_cond);
			// MAY have changed loop_cond. 
			if(!(tmp==ExprValue::None))
			{  can_optimize=(tmp.is_null() ? (-1) : (+1));  }
		}
		
		break;
	};
	
	// Check the "for()" statements: 
	if(for_init_stmt)
	{
		fail+=for_init_stmt->DoExprTF(ti,(TreeNode**)&for_init_stmt);
		if(!ti->tp.is_const)  is_const=0;
	}
	if(for_inc_stmt)
	{
		fail+=for_inc_stmt->DoExprTF(ti,(TreeNode**)&for_inc_stmt);
		if(!ti->tp.is_const)  is_const=0;
	}
	
	// Finally, look at the loop body: 
	fail+=loop_stmt->DoExprTF(ti,(TreeNode**)&loop_stmt);
	
	if(!is_const)
	{  ti->tp.is_const=0;  }
	ti->tp.is_lvalue=0;
	ti->tp.sevt.SetUnknown();
	ti->tp.has_effect=1;
	ti->tp.try_ieval=0;  // no expression but stmt
	
	// We may only use ReplaceBy() if update_tn is set (hence the 
	// caller is aware of the possibility). 
	assert(update_tn && *update_tn==this);
	// Do some optimization if we can...
	if(can_optimize<0 && itertype!=IT_DoWhile)
	{
		// Loop condition is 0 and we're not a "do..while" loop. 
		// Remove the loop body. 
		if(for_init_stmt)
		{  *update_tn=RemoveChild(for_init_stmt);  }
		else
		{  *update_tn=new TNExpressionStmt(GetLocationRange());  }
		ReplaceBy(*update_tn);
	}
	
	return(fail);
}


int TNSelectionStmt::DoExprTF(TFInfo *ti,TreeNode **update_tn)
{
	++ti->n_nodes_visited;
	int fail=0;
	
	bool is_const=1;
	int can_optimize=0;
	while(cond_expr)
	{
		fail+=cond_expr->DoExprTF(ti);
		if(fail)  break;
		if(!ti->tp.is_const)  is_const=0;
		
		// Condition must be bool-convertable to "int". 
		if(!ti->tp.sevt.CanBoolConvert())
		{
			Error(cond_expr->GetLocationRange(),
				"cannot bool-convert expression of type %s\n",
				ti->tp.sevt.TypeString().str());
			++fail;  break;
		}
		
		if(ti->tp.try_ieval)
		{
			ExprValue tmp=cond_expr->DoTryIEval(((TreeNode**)&cond_expr));
			// MAY have changed cond_expr. 
			if(!(tmp==ExprValue::None))
			{  can_optimize=(tmp.is_null() ? (-1) : (+1));  }
		}
		
		break;
	}
	
	// Check the statements: 
	if(if_stmt)
	{
		fail+=if_stmt->DoExprTF(ti,(TreeNode**)&if_stmt);
		if(!ti->tp.is_const)  is_const=0;
	}
	if(else_stmt)
	{
		fail+=else_stmt->DoExprTF(ti,(TreeNode**)&else_stmt);
		if(!ti->tp.is_const)  is_const=0;
	}
	
	if(!is_const)
	{  ti->tp.is_const=0;  }
	ti->tp.is_lvalue=0;
	ti->tp.sevt.SetUnknown();
	ti->tp.has_effect=1;
	ti->tp.try_ieval=0;  // no expression but stmt
	
	// We may only use ReplaceBy() if update_tn is set (hence the 
	// caller is aware of the possibility). 
	assert(update_tn && *update_tn==this);
	// Do some optimization if we can...
	if(can_optimize)
	{
		if(can_optimize>0)
		{  *update_tn=RemoveChild(if_stmt);  }
		else if(else_stmt)
		{  *update_tn=RemoveChild(else_stmt);  }
		else
		{  *update_tn=new TNExpressionStmt(GetLocationRange());  }
		ReplaceBy(*update_tn);
	}
	
	return(fail);
}


int TNJumpStmt::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	int fail=0;
	switch(jumptype)
	{
		case JT_None:  assert(0);  break;
		case JT_Break:
			//in_scope NULL here (could be changed). 
			assert(!in_scope);
			
			ti->tp.is_const=1;
			ti->tp.is_lvalue=0;
			ti->tp.sevt.SetUnknown();
			ti->tp.has_effect=1;
			
			// Nothing more to do. evalfunc stays NULL. 
			
			break;
		case JT_Return:
		{
			// in_scope is the AniAnonScope where the return is 
			// called from; the AniFunction must be somewhere up 
			// in the hierarchy. 
			assert(in_scope);
			
			if(down.first())
			{
				fail+=down.first()->DoExprTF(ti);
				if(!fail && ti->tp.try_ieval)
				{  down.first()->DoTryIEval();  }
				ti->tp.try_ieval=0;  // no expr but stmt
			}
			else
			{
				// Case "return;" (without expression). 
				ti->tp.sevt.SetVoid();
				ti->tp.is_lvalue=0;
				ti->tp.is_const=1;
				ti->tp.has_effect=1;
				ti->tp.try_ieval=0;  // no expr but stmt
			}
			if(fail)  break;
			
			// Need to find the associated function: 
			AniGlue_ScopeBase *ani_function;
			{
				TreeNode *wlk=parent();
				for(;wlk && wlk->NType()!=TN_FunctionDef; wlk=wlk->parent());
				// If this assert fails, we're not inside a function. 
				// This is a bug because this had to be found in 
				// registration step. 
				assert(wlk);
				ani_function=(AniGlue_ScopeBase*)
					((TNFunctionDef*)wlk)->function;
			}
			assert(ani_function);
			
			// Check if the return type matches the function return 
			// type: 
			ExprValueType rtype;
			ani_function->CG_GetReturnType(&rtype);  // user functions, only
			AssignmentFuncBase *assfunc=NULL;
			int rv;
			if(rtype.TypeID()==EVT_Void && ti->tp.sevt.TypeID()==EVT_Void)
			{  rv=2;  }
			else
			{  rv=rtype.CanAssignType(ti->tp.sevt,&assfunc);  }
			assert(rv>=0);
			if(!rv)
			{
				DELETE(assfunc);
				Error(GetLocationRange(),
					"function \"%s\" should return %s (not %s)\n",
					ani_function->CG_GetName().str(),
					rtype.TypeString().str(),ti->tp.sevt.TypeString().str());
				++fail;  break;
			}
			
			// Store for eval... erm: for _exec_: 
			evalfunc=(TNJumpStmt::EvalFunc*)
				ani_function->CG_GetJumpStmtEvalFunc(this,assfunc,in_scope);
			if(!evalfunc) DELETE(assfunc);
			assert(evalfunc);
			
			//ti->tp.is_const= <unchanged>;
			ti->tp.is_lvalue=0;
			ti->tp.sevt.SetUnknown();
			ti->tp.has_effect=1;
			ti->tp.try_ieval=0;  // no expr but stmt
		}  break;
		default:  assert(0);
	}
	return(fail);
}


int TNCompoundStmt::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	// First, process children: 
	int fail=0;
	bool has_effect=0;
	for(TreeNode *i=down.first(); i; i=i->next)
	{
		fail+=i->DoExprTF(ti,(TreeNode**)&i);
		if(ti->tp.has_effect)  has_effect=1;
	}
	
	// Remove all empty statements in compound stmt: 
	for(TreeNode *_i=down.first(); _i; )
	{
		assert(_i->NType()==TN_Statement);
		TNStatement *stmt=(TNStatement*)_i;
		_i=_i->next;
		
		if(stmt->StmtType()==TNS_ExpressionStmt && stmt->down.is_empty())
		{  delete RemoveChild(stmt);  }
	}
	
	#if 0  /* DAMN! */
	// NOTE: We cannot do that here because removing compounds will 
	//       mix up addressation... So, the only compound removal 
	//       is done in registration step and that one will not remove 
	//       compounds which got empty due to optimization. 
	//----------
	// If this TNCompoundStmt only contains one other compound stmt and 
	// nothing else, take over the children and remove the compound stmt: 
	if(!down.is_empty() && down.first()==down.last() && 
	   ((TNStatement*)down.first())->StmtType()==TNS_CompoundStmt)
	{
		//fprintf(stderr,"%s: removing empty compound stmt scope.\n",
		//		down.first()->GetLocationRange().PosRangeString().str());
		TransferChildrenFrom(down.first());
		delete RemoveChild(down.first());
	}
	#endif
	
	ti->tp.has_effect=has_effect;
	ti->tp.try_ieval=0;  // just to be sure
	
	return(fail);
}


int TNDeclarator::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	// We need to get the ani scope. And thus, we must get down to 
	// the tree leaf. 
	if(decltype!=DT_Initialize)
	{
		// NOTE: Due to constant folding at the beginning there is no 
		//       need to look at the expression in DT_Array. 
		// currently, only trivial value expressions are allowed (see 
		// core/fixinctype.cc). 
		// (This assert is just here to make sure to detect it here 
		// once the other code is changed.) 
		if(decltype==DT_Array && down.first()!=down.last())
		{  assert(((TNExpression*)down.last())->ExprType()==TNE_Value);  }
		
		// We need to have an evalfunc even if there is no 
		// initializer (because we then need prototype value 
		// setup): 
		if(decltype==DT_Name)
		{
			TreeNode *walker=this;
			for(; walker->parent()->NType()==TN_Declarator;
				walker=walker->parent());
			if(((TNDeclarator*)walker)->decltype!=DT_Initialize)
			{
				//fprintf(stderr,"DECL: %s  walker: %d (%d)\n",
				//	((TNIdentifier*)down.first())->CompleteStr().str(),
				//	((TNDeclarator*)walker)->decltype,DT_Initialize);
				TNIdentifier *our_idf=(TNIdentifier*)down.first();
				assert(our_idf->NType()==TN_Identifier);
				TNDeclarator *top_decl=(TNDeclarator*)walker;
				top_decl->evalfunc=(TNDeclarator::EvalFunc*)our_idf
					->ani_scope->CG_GetDeclaratorEvalFunc(top_decl,our_idf);
				assert(top_decl->evalfunc);
			}
		}
		
		// down.first() is always the next declarator if not DT_Name. 
		// down.first() is the TNIdentifier if DT_Name. 
		// Thus, always use down.first(). 
		assert(down.first());
		return(down.first()->DoExprTF(ti));
	}
	
	// DT_Initialize here. 
	assert(down.count()==2);
	
	int fail;
	do {
		// Get the type of the declaration. 
		fail=down.first()->DoExprTF(ti);
		if(fail) break;
		TFInfo::Type our_type(ti->tp);
		
		// Look at the initializing expression: 
		assert(!ti->in_init);
		ti->in_init=1;
		fail+=down.last()->DoExprTF(ti);
		ti->in_init=0;
		if(fail)  break;
		
		// Okay, so we want to assign the "type" ti->tp.sevt to 
		// a variable of type our_type. 
		// NO INCOMPLETE TYPES HERE. 
		
		// See if we can assign the initializer to the variable: 
		if(our_type.sevt.CanAssignType(ti->tp.sevt,&assfunc)<=0)
		{
			Error(down.last()->GetLocationRange(),
				"%s: cannot initialize var of type %s with type %s\n",
				down.last()->GetLocationRange().PosRangeString().str(),
				our_type.sevt.TypeString().str(),
				ti->tp.sevt.TypeString().str());
			++fail;  break;
		}
		
		// Store for eval: (First find associated identifier.) 
		TreeNode *our_idf=down.first();
		for(; our_idf->NType()==TN_Declarator; our_idf=our_idf->down.first());
		assert(our_idf->NType()==TN_Identifier);
		evalfunc=(TNDeclarator::EvalFunc*)((TNIdentifier*)our_idf)
			->ani_scope->CG_GetDeclaratorEvalFunc(this,(TNIdentifier*)our_idf);
		assert(evalfunc);
		//if(!evalfunc)  {  ++fail;  break;  }
		
		// Store our type: 
		//ti->tp.sevt=our_type.sevt;  PREVIOUS (correct) SOLUTION
		ti->tp.sevt.SetVoid();  // declarator nowadays returns void
		ti->tp.is_lvalue=0;
		//ti->tp.is_const= <unmodified>
		ti->tp.has_effect=1;
		//ti->tp.try_ieval MUST STAY unchanged; see if() below. 
		
		if(ti->tp.try_ieval)
		{
			ExprValue tmp=down.last()->DoTryIEval();
			ti->tp.try_ieval=!(tmp==ExprValue::None);
		}
	} while(0);
	
	return(fail);
}


int TNNewArraySpecifier::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	
	int fail=0;
	
	// If we have a child, check it; else use void type. 
	if(!down.first())
	{
		ti->tp.sevt.SetVoid();
		ti->tp.is_lvalue=0;
		ti->tp.is_const=1;
		ti->tp.has_effect=0;
		ti->tp.try_ieval=1;   // Although there is nothing to eval...
	}
	else do
	{
		fail+=down.first()->DoExprTF(ti);
		if(fail)  break;
		const Value::CompleteType *pod=ti->tp.sevt.PODType();
		if(!pod || pod->type!=Value::VTInteger)
		{
			Error(down.first()->GetLocationRange(),
				"cannot use non-integer type %s for array size\n",
				ti->tp.sevt.TypeString().str());
			++fail;  break;
		}
		
		//ti->tp.sevt is POD,VTInteger. 
		ti->tp.is_lvalue=0;
		//ti->tp.is_const= <unmodified>
		//ti->tp.has_effect= <unmodified>
		//ti->tp.try_ieval MUST stay unmodified; see below
		if(ti->tp.try_ieval)
		{
			ExprValue tmp=down.first()->DoTryIEval();
			ti->tp.try_ieval=!(tmp==ExprValue::None);
		}
	} while(0);
	
	return(fail);
}


int TNDeclarationStmt::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	
	// First, look at the declarators: 
	int sumfail=0;
	int is_const=1;
	for(TreeNode *_tn=down.first()->next; _tn; _tn=_tn->next)
	{
		int fail=_tn->DoExprTF(ti);
		sumfail+=fail;
		if(fail)  continue;
		
		// No need to check down.first(), the TNTypeSpecifier. 
		// The declared variables have just this type attached 
		// and the TNDeclarator will complain if the types 
		// do not match. 
		
		if(!ti->tp.is_const)
		{  is_const=0;  }
	}
	
	ti->tp.is_lvalue=0;
	ti->tp.is_const=is_const;
	ti->tp.has_effect=1;
	ti->tp.try_ieval=0;  // stmt && !expr ...
	
	return(sumfail);
}


int TNAniDescStmt::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	
	int sumfail=0;
	
	// Entering the ani desc stmt. The grammar does not allow them 
	// to be nested. 
	assert(!ti->anidescstmt);
	ti->anidescstmt=this;
	
	// First, check the var name: It needs to be an lvalue or 
	// an object. If none is specified (i.e. "%{}"), then we assume 
	// "this" and hence need to be inside an object. 
	do {
		if(!name_expr)
		{
			// Check if we're inside object and non-static function: 
			// -> Check already done in registration step. 
			break;
		}
		
		// Check name child: 
		int fail=name_expr->DoExprTF(ti,(TreeNode**)&name_expr);
		sumfail+=fail;
		if(fail)  break;
		
		if(ti->tp.is_const)
		{
			Error(name_expr->GetLocationRange(),
				"ani desc stmt for const variable\n");
			++sumfail;  break;
		}
		
		int cannot_use=1;  // 0 -> CAN use; 1 -> no; 2 -> no & error written
		do {
			// Can use objects; these need not be lvalues due 
			// to internal referencing which behaves like a pointer. 
			AniGlue_ScopeBase *lasb=ti->tp.sevt.AniScope();
			if(lasb)
			{
				AniGlue_ScopeBase::AniScopeType as_type=lasb->CG_ASType();
				if(as_type==AniGlue_ScopeBase::AST_Object)
				{  cannot_use=0;  break;  }
			}
			
			// Otherwise, we need an lvalue. 
			if(!ti->tp.is_lvalue)  break;
			
			// Only vectors and scalars can (currently) be animated: 
			const Value::CompleteType *pod=ti->tp.sevt.PODType();
			if(pod)
			{
				if(pod->type!=Value::VTScalar && 
				   pod->type!=Value::VTVector )
				{
					Error(name_expr->GetLocationRange(),
						"ani desc stmt cannot be used for type %s\n",
						ti->tp.sevt.TypeString().str());
					cannot_use=2;  break;
				}
				cannot_use=0;  break;
			}
		} while(0);
		
		if(cannot_use)
		{
			if(cannot_use==1)
			{  Error(name_expr->GetLocationRange(),
				"ani desc stmt requires lvalue or object\n");  }
			++sumfail;  break;
		}
	} while(0);
	
	// Finally, check the ani desc block: 
	if(adb_child && adb_child->DoExprTF(ti,(TreeNode**)&adb_child))
	{  ++sumfail;  }
	
	ti->tp.sevt.SetVoid();  // ani block stmt
	ti->tp.is_lvalue=0;
	ti->tp.is_const=0;  // ani block never const...
	ti->tp.has_effect=1;
	ti->tp.try_ieval=0;  // stmt && !expr ...
	
	// We're leaving the ani desc stmt. 
	ti->anidescstmt=NULL;
	
	return(sumfail);
}


int TNFuncDefArgDecl::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	assert(!ti->in_init);
	
	// First, look at the declarator: 
	int fail=down.last()->DoExprTF(ti);
	
	//ti->tp.sevt= <unmodified>
	ti->tp.is_lvalue=0;
	//ti->tp.is_const= <unmodified>
	//ti->tp.has_effect= <unmodified> (??)
	
	// No need to check the type specifier in down.first(). 
	// The declarator did that job. 
	
	return(fail);
}


int TNFunctionDef::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	
	// Make sure data in tree is consistent. 
	assert((AniFunction*)func_name->ani_scope==function);
	
	// First, process arguments: 
	int fail=0;
	for(TreeNode *i=first_arg; i; i=i->next)
	{  fail+=i->DoExprTF(ti);  }
	
	// Then, look at the function body: 
	fail+=func_body->DoExprTF(ti,(TreeNode**)&func_body);
	
	return(fail);
}


int TNObject::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	// First, process children: 
	int fail=0;
	for(TreeNode *i=down.first(); i; i=i->next)
	{  fail+=i->DoExprTF(ti);  }
	
	return(fail);
}


int TNSetting::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	// First, process children: 
	int fail=0;
	for(TreeNode *i=down.first(); i; i=i->next)
	{  fail+=i->DoExprTF(ti);  }
	
	return(fail);
}


int TNAnimation::DoExprTF(TFInfo *ti,TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	// First, process children: 
	int fail=0;
	for(TreeNode *i=down.first(); i; i=i->next)
	{  fail+=i->DoExprTF(ti);  }
	
	return(fail);
}

}  // end of namespace func

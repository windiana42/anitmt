/*
 * core/a_userfunc.cc
 * 
 * ANI tree function, user-defined. 
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

#include "animation.h"

#include <ani-parser/treereg.h>
#include "execthread.h"


using namespace ANI;

//------------------------------------------------------------------------------

// Execute a non-member void function returning void. Used when calling 
// the animation block (main) function or a setting init function. 
static int EvalSimpleFunction(AniUserFunction *function,
	ExecThreadInfo *info)
{
	TNFunctionDef *fdef=(TNFunctionDef*)function->GetTreeNode();
	assert(fdef->NType()==TN_FunctionDef);
	assert(fdef->function==function);
	
	EvalStateStack *ess=info ? info->ess : NULL;
	
	switch(ess->popS())
	{
		case 0:
		{
			// Allocate stack frame. We must do that here because 
			// lookup code expects it. 
			ExecStack::Frame *sframe=new ExecStack::Frame(/*n_stack_vars=*/0,
				/*this_ptr=*/ScopeInstance());
			info->stack->PushFrame(sframe);
		}
		case 1:
			switch(fdef->func_body->Exec(info))
			{
				case TNStatement::ERS_Cont:  break;
				case TNStatement::ERS_ContextSwitch:
					ess->pushS(1);
					return(1);  // YES, 1... not ERS_ContextSwitch :)
				case TNStatement::ERS_Return:  break;  // Okay, return done. 
				case TNStatement::ERS_Break:  // fall...
				default:                      //   ...through
					assert(0);
			}
			break;
		default: assert(0);
	}
	// Done...
	return(0);
}

//------------------------------------------------------------------------------


int AniUserFunction::EvalFunc_FCall::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	AniUserFunction::FuncInstanceInfo *fiinfo=function->fiinfo;
	assert(fiinfo);
	
	EvalStateStack *ess=info ? info->ess : NULL;
	
	ScopeInstance this_ptr;
	ExecStack::Frame *sframe=NULL;
	#if 0  /* debug variant */
	short int _sv=ess->popS();
	if(_sv)  fprintf(stderr,"<%d>",(int)_sv);
	switch(_sv)
	#else  /* normal variant */
	switch(ess->popS())
	#endif
	{
		// WARNING: Extra-heavy fallthrough switch. 
		case 0:
		{
			ExprValue this_ptr_ev;
			// First action: Evalualte the first child so that we know 
			// which "this" pointer to use. etc. 
			// (Note that this can involve function calls itself, e.g. 
			// as in "foo().bar()".) 
if(!tn) // <-- NULL -> special to allow test hacks...
{  goto skipTN_A;  }
			if(tn->down.first()->Eval(this_ptr_ev,info))
			{
				ess->pushS(0);
				return(1);
			}
			
			// this_ptr_ev will be a NULL "ref" if we're not a member 
			// function. 
			// Extract "this" pointer: 
			if(!!this_ptr_ev)
			{
				//fprintf(stderr,"Calling WITH this pointer\n");
				this_ptr_ev.GetAniScope(&this_ptr);
				//assert(!!this_ptr);
				// I removed this assert because using a NULL this pointer 
				// is NOT illegal as long as the function does not access 
				// any member variables. 
			}
		}
			//else fprintf(stderr,"Calling withOUT this pointer\n");
skipTN_A:
			// Allocate stack frame (but do not push it yet): 
			// Pass correct "this" pointer for this stack frame. 
			sframe=new ExecStack::Frame(fiinfo->n_stack_vars,this_ptr);
			// This is the correct position to count the number of function 
			// calls (to this function): 
			++fiinfo->instance_counter;
			
if(!tn) // <-- NULL -> special to allow test hacks...
{  goto skipTN_B;  }
			goto skippopB;
		case 2:
			sframe=ess->popEF(NULL);
			assert(sframe);
			this_ptr=sframe->this_ptr;
		skippopB:
		{
			// Evaluate the passed args (in the order in which they were passed): 
			// NOTE: The offset for the return value was already added to arg_perm[]. 
			int acnt=ess->popL(0);
			for(TreeNode *i=ess->popTN(tn->down.first()->next);
				i; i=i->next,acnt++)
			{
				ExprValue val;
				if(i->Eval(val,info))
				{
					ess->pushTN(i);
					ess->pushL(acnt);
					ess->pushEF(sframe);
					ess->pushS(2);
					return(1);
				}
				// NOTE: We MAY use assign() directly here but ONLY 
				//       because sframe->GetValue() returns a TRUE C++ 
				//       REFERENCE to the value on the stack, so 
				//       assign() will actually change ExprValue::iev 
				//       pointer for JUST THE ExprValue ON THE STACK. 
				// We would need to set prototype values just as in 
				// a_anonscope.cc if we used any evaluation or the 
				// like on the lvalue. 
				// NOTE: We do NOT prototype the return value on the 
				//       stack because the calling code (below) relies 
				//       on ExprValue::iev being NULL is we run off the 
				//       function body without returning. 
				arg_assfunc[acnt]->Assign(
					/*lvalue=*/sframe->GetValue(arg_perm[acnt]),
					/*rvalue=*/val);
			}
			assert(acnt==args_spec);
		}
skipTN_B:
			
			// Need to assign the default values for those args which 
			// were not specified: 
			// For this, simply evaluate the declarators of the args. 
			// We need the function frame on the stack for that: 
			info->stack->PushFrame(sframe);
			// Note: The following two vars may not be used until we 
			//       are in the return path. We MAY NOT push sframe on 
			//       the ESStack (ess) because that would clone the 
			//       frame on the ESStack AND the normal stack resutlting 
			//       in one copy too much. During context switches, the 
			//       sframe is not on top of the stack but somewhere. 
			//       However, when the function returns, "our" sframe is 
			//       again on the top and we get it and the "this" pointer. 
			sframe=NULL;                   // <-- Do not use below here. 
			this_ptr=ScopeInstance::None;  // <-- Do not use below here. 
		case 3:
		{
			// Eval the declarators: 
			ExprValue tmp;
			for(int i=ess->popL(0); i<n_def_args; i++)
			{
				if(default_decl[i]->Eval(tmp,info))
				{
					ess->pushL(i);
					ess->pushS(3);
					return(1);
				}
			}
		}
		case 4:
		{
			// Now, execute the function: 
			TNFunctionDef *fdef=(TNFunctionDef*)function->GetTreeNode();
			assert(fdef->function==function);
			switch(fdef->func_body->Exec(info))
			{
				case TNStatement::ERS_Cont:  break;
				case TNStatement::ERS_ContextSwitch:
					ess->pushS(4);
					return(1);  // YES, 1... not ERS_ContextSwitch :)
				case TNStatement::ERS_Return:  break;  // Okay, return done. 
				case TNStatement::ERS_Break:  // fall...
				default:                      //   ...through
					assert(0);
			}
		}
			// Get the stack frame again. As we're returning, 
			// it must be the top frame. (May use sframe below here; use 
			// sframe->this_ptr instead of plain this_ptr.) 
			sframe=info->stack->TopFrame();
			// Get return value if any: 
			if(fiinfo->aidx_retval>=0)
			{
				retval=sframe->GetValue(fiinfo->aidx_retval);
				if(!retval || retval.GetExprValueTypeID()==EVT_Unknown)
				{
					Warning(function->NameLocation(),
						"function %s%s%s: missing value return\n",
						function->CompleteName().str(),
						tn ? " called from " : "",
						tn ? tn->down.first()->GetLocationRange().PosRangeString().str() : "");
					retval=ExprValue(ExprValue::Prototype,function->return_type);
				}
			}
			else if(fiinfo->aidx_retval==-2)
			{
				// Special case for constructor: Return "this". 
				assert(!!sframe->this_ptr);
				retval=ExprValue(ExprValue::Copy,sframe->this_ptr);
			}
			else
			{  retval=ExprValue::CVoidType;  }
			
			// Pop stack frame: 
			info->stack->Free();
			
			break;
		default:  assert(0);
	}
	
	// Uahh... that's all. 
	return(0);
}

AniUserFunction::EvalFunc_FCall::~EvalFunc_FCall()
{
	// Virtual destructor. 
	
	if(arg_assfunc)
	{
		for(ssize_t i=0; i<args_spec; i++)
		{  DELETE(arg_assfunc[i]);  }
		arg_assfunc=(AssignmentFuncBase**)LFree(arg_assfunc);
	}
	
	default_decl=(TNDeclarator**)LFree(default_decl);
	
	arg_perm=(int*)LFree(arg_perm);
}


//------------------------------------------------------------------------------

int AniUserFunction::EvalFunc_FuncReturn::Eval(TNJumpStmt *tn,
	ExecThreadInfo *info)
{
	if(tn->down.first())
	{
		ExprValue retval;
		if(tn->down.first()->Eval(retval,info))
		{  return(1);  }
		
		// NOTE: GetValue() returns a true C++ reference, so 
		// we can actually modify the ExprValue on the stack 
		// and change it's ExprValue::iev. 
		assert(assfunc);
		assfunc->Assign(
			/*lvalue=*/info->stack->GetValue(aidx_retval,up_frames),
			/*rvalue=*/retval);
	}
	
	return(0);
}

AniUserFunction::EvalFunc_FuncReturn::~EvalFunc_FuncReturn()
{
	// Not inline because virtual. 
	DELETE(assfunc);  
}

//------------------------------------------------------------------------------


int AniUserFunction::EvalFunc_Animation::Eval(ExprValue &retval,
	TNOperatorFunction * /*tn*/,ExecThreadInfo *info)
{
	// NOTE: TNOperatorFunction *tn is NULL here. 
	int rv=EvalSimpleFunction(function,info);
	if(!rv)
	{
		// We return void. 
		retval=ExprValue::CVoidType;
	}
	return(rv);
}

AniUserFunction::EvalFunc_Animation::~EvalFunc_Animation()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------


int AniUserFunction::EvalFunc_SettingInit::Eval(ExprValue &retval,
	TNOperatorFunction * /*tn*/,ExecThreadInfo *info)
{
	// NOTE: TNOperatorFunction *tn is NULL here. 
	int rv=EvalSimpleFunction(function,info);
	if(!rv)
	{
		// We return void. 
		retval=ExprValue::CVoidType;
	}
	return(rv);
}

AniUserFunction::EvalFunc_SettingInit::~EvalFunc_SettingInit()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------


int AniUserFunction::Match_FunctionArguments(LookupFunctionInfo *lfi,
	int store_evalhandler)
{
	//                         STOP. 
	// KEEP YOUR FINGERS OFF THIS FUNCTION UNLESS YOU REALLY 
	//               KNOW WHAT YOU ARE DOING. 
	
	// None of the named args in lfi should occure more than once. 
	// This is checked on higher level (because it would make no 
	// sense to check that for each function -- it has to be done 
	// once per lookup and not once per function). 
	
	// Animation may not be called directly, so function args never 
	// match: 
	if(attrib & IS_Animation)
	{  return(0);  }
	
	// If the caller provides more args than we have, we cannot be 
	// called. 
	int nargs=arglist.count();
	if(nargs<lfi->n_args)
	{  return(0);  }
	
	// Now, do the actual matching. 
	// This maps argument indices: 
	// We have arguments 0...nargs-1. For i in this range, 
	// argidx_map[i] is the index of the provided arg in *lfi 
	// which is to be passed as our argument number i. 
	int argidx_map[nargs];
	for(int i=0; i<nargs; i++)  argidx_map[i]=-1;
	
	// Loop through the passed args: 
	AniVariable *our_arg=arglist.first();
	int our_arg_idx=0;  // <-- ...first()
	int args_matching=0;     // args matching
	int args_matching_cv=0;  // args matching with type conversion
	for(int j=0; j<lfi->n_args; j++)
	{
		if(lfi->arg_name[j])
		{
			RefString name=lfi->arg_name[j]->CompleteStr();
			// Name specified. Look it up: (Could be optimized 
			// by first checking our_arg & ->next if !=NULL). 
			our_arg_idx=0;
			for(AniVariable *av=arglist.first(); av; av=av->next,our_arg_idx++)
			{
				if(av->GetName()==name)
				{  our_arg=av;  goto found;  }
			}
			// No argument named name.str(). 
			return(0);
			found:;
		}
		else if(!our_arg)  // Don't run off arg list...
		{  return(0);  }
		
		// See if we can use lfi->arg_type[j] for our_arg. 
		assert(our_arg_idx>=0 && our_arg_idx<nargs);
		if(argidx_map[our_arg_idx]>=0)
		{
			// This arg was already assigned. 
			// This means, when the caller wants to call this function, 
			// he would have to assign the current arg more than once. 
			return(0);
		}
		int rv=our_arg->GetType().CanAssignType(lfi->arg_type[j],NULL);
		if(rv==0)  // No, cannot. Type mismatch. 
		{  return(0);  }
		else if(rv==1)  ++args_matching_cv;
		else  ++args_matching;
		argidx_map[our_arg_idx]=j;
		
		// Step forward one arg in our list: 
		our_arg=our_arg->next;
		++our_arg_idx;
		if(!our_arg)
		{  assert(our_arg_idx==nargs);  our_arg_idx=-1;  }
	}
	assert(args_matching_cv+args_matching<=nargs);
	
	// Okay, now all the passed args can be mapped to our args. 
	// Let's see if we have additional args without default 
	// values which were not provided. 
	our_arg_idx=0;
	for(AniVariable *av=arglist.first(); av; av=av->next,our_arg_idx++)
	{
		assert(av->ASType()==AST_UserVar);
		if(argidx_map[our_arg_idx]>=0)  continue;  // already assigned
		// Check if the variable has a default arg: 
		if(((AniUserVariable*)av)->GetAttrib() & AniUserVariable::HAS_Initializer)
		{  ++args_matching;  continue;  }
		// Argument *av not specified. Cannot call function. 
		return(0);
	}
	
	if(store_evalhandler)
	{
		// Store information for caller to be able to quickly reproduce 
		// what we just looked up: 
		EvalFunc_FCall *evalfunc=new EvalFunc_FCall(this,lfi->n_args);
		
		// Permutation array: 
		// For the provided arg i, we need our arg number in 
		// range 0...nargs-1. 
		// Furthermore, we may need an offset for return value 
		// on the stack. 
		evalfunc->arg_perm=(int*)LMalloc(lfi->n_args*sizeof(int));
		// These are the argument assignment functions for those args 
		// which are specified (hence [0..nargs-1]): 
		evalfunc->arg_assfunc=(AssignmentFuncBase**)
			LMalloc(nargs*sizeof(AssignmentFuncBase*));
		
		for(int j=0; j<nargs; j++)
		{
			if(argidx_map[j]>=0)
			{
				assert(argidx_map[j]<lfi->n_args);
				evalfunc->arg_perm[argidx_map[j]]=j; // Offset added lateron. 
			}
		}
		// Walk the specified args: 
		for(int j=0; j<lfi->n_args; j++)
		{
			AniVariable *argv=arglist.first();
			for(int fnum=evalfunc->arg_perm[j]; fnum>0 && argv; 
				argv=argv->next,fnum--);
			assert(argv);
			assert(argv->ASType()==AST_UserVar);
			int rv=argv->GetType().CanAssignType(
				lfi->arg_type[j],&evalfunc->arg_assfunc[j]);
			assert(rv>0);  // Must be 1 or 2. 
		}
		// Add offset to arg permutation array: 
		assert(fiinfo);
		for(int j=0; j<lfi->n_args; j++)
		{  evalfunc->arg_perm[j]+=fiinfo->used_at_beginning;  }
		
		// Okay, now let's finish off with the default args. 
		// All arguments which were not yet specified need to be 
		// filled up with default args. 
		// No need for assignment functions as the default args are 
		// declarations with their own assignment function. 
		evalfunc->n_def_args=nargs-lfi->n_args;
		evalfunc->default_decl=(TNDeclarator**)LMalloc(
			evalfunc->n_def_args*sizeof(TNDeclarator*));
		AniVariable *argv=arglist.first();
		int defaidx=0;
		for(int j=0; j<nargs && argv; j++,argv=argv->next)
		{
			if(argidx_map[j]>=0)  continue;
			TNDeclarator *decl=(TNDeclarator*)argv->GetTreeNode();
			assert(decl->NType()==TN_Declarator);
			assert(decl->GetDeclType()==TNDeclarator::DT_Initialize);
			assert(defaidx<evalfunc->n_def_args);
			evalfunc->default_decl[defaidx++]=decl;
		}
		assert(!argv);
		assert(defaidx==evalfunc->n_def_args);
		
		lfi->evalfunc=evalfunc;
		lfi->ret_evt=return_type;
		//lfi->may_try_ieval stays 0 for now... maybe one time we allow that. 
	}
	
	// Okay, looks good. 
	assert(args_matching+args_matching_cv==nargs);
	return(args_matching_cv ? 1 : 2);
}


// NOTE: Return type is (TNJumpStmt::EvalFunc*). 
void *AniUserFunction::CG_GetJumpStmtEvalFunc(TNJumpStmt *tn,
	AssignmentFuncBase *assfunc,AniGlue_ScopeBase *from_scope)
{
	switch(tn->jumptype)
	{
		case TNJumpStmt::JT_Return:
		{
			int up_frames=0;
			
			{
				AniScopeBase *asb=(AniScopeBase*)from_scope;
				for(; asb && asb!=this; asb=asb->Parent(),up_frames++)
				{  assert(asb->ASType()==AST_AnonScope);  }
				// If this fails, we (=the function) are not in the 
				// hierarchy of "our" return statement...
				assert(asb);
			}
			
			// This is just a debug message: 
			//Warning(tn->GetLocationRange(),"up_frames=%d\n",up_frames);
			
			assert(fiinfo);
			// There is a special case for a constructor here. 
			// Constructor is officially void but internally rerturns 
			// the newly created object. 
			if((attrib & IS_Constructor))
			{
				assert(!tn->down.first());
				assert(!assfunc);
				assert(fiinfo->aidx_retval==-2);  // special
				
				return(new EvalFunc_FuncReturn(NULL,0,-2));
			}
			else
			{
				if(tn->down.first())  // "return EXPR;"
				{
					assert(assfunc);
					assert(fiinfo->aidx_retval>=0);
				}
				else   // "return;"
				{
					assert(!assfunc);
					assert(fiinfo->aidx_retval==-1);
				}
				
				return(new EvalFunc_FuncReturn(assfunc,up_frames,
					fiinfo->aidx_retval));
			}
			assert(0);
		}
		default:  assert(0);
	}
	
	return(NULL);
}


AniUserFunction::EvalFunc_Animation *AniUserFunction::GetAniEvalFunc()
{
	if(!(attrib & IS_Animation))
	{  return(NULL);  }
	
	EvalFunc_Animation *evalfunc=new EvalFunc_Animation(this);
	return(evalfunc);
}

AniUserFunction::EvalFunc_SettingInit *AniUserFunction::GetSettingInitEvalFunc()
{
	if(!(attrib & IS_SettingInit))
	{  return(NULL);  }
	
	EvalFunc_SettingInit *evalfunc=new EvalFunc_SettingInit(this);
	return(evalfunc);
}


int AniUserFunction::CG_GetReturnType(ANI::ExprValueType *store_here) const
{
	// May only return complete types here. 
	assert(!return_type.IsIncomplete());
	store_here->operator=(return_type);
	return(0);
}


String AniUserFunction::PrettyCompleteNameString() const
{
	String name(return_type.TypeString().str());
	name+=" ";
	name+=CompleteName();
	
	String tmp;
	name+="(";
	for(AniVariable *av=arglist.first(); av; av=av->next)
	{
		assert(av->ASType()==AST_UserVar);
		tmp.sprintf("%s %s%s%s",
			av->GetType().TypeString().str(),
			av->GetName().str(),
			(((AniUserVariable*)av)->GetAttrib() & AniUserVariable::HAS_Initializer) ? "=..." : "",
			av->next ? "," : "");
		name+=tmp;
	}
	name+=")";
	
	return(name);
}


int AniUserFunction::InitStaticVariables(ExecThreadInfo *info,int flag)
{
	int nerrors=0;
	
	if(flag>0)
	{
		// Okay, so order is important here. 
		// First, init our variables: 
		// -> Well, function args cannot be static, so this is a no-op. 
		// Next, our children: 
		nerrors+=anonscopelist.InitStaticVariables(info,flag);
	}
	else if(flag<0)
	{
		// Cleanup goes the other way round than init. 
		// (No change as long as there is only one list...)
		nerrors+=anonscopelist.InitStaticVariables(info,flag);
	}
	else assert(0);
	
	return(nerrors);
}


int AniUserFunction::FixIncompleteTypes()
{
	int nerrors=0;
	nerrors+=anonscopelist.FixIncompleteTypes();
	nerrors+=arglist.FixIncompleteTypes();
	//nerrors+=incompletelist.FixIncompleteTypes();
	nerrors+=_FixEntriesInTypeFixList(&typefix_list);
	
	if(return_type.IsIncomplete())
	{
		if(_DoFixIncompleteType(&return_type,this,
			((TNFunctionDef*)GetTreeNode())->ret_type,
			/*TNDeclarator *decl=*/NULL))
		{  ++nerrors;  }
	}
	
	return(nerrors);
}


AniScopeBase *AniUserFunction::RegisterTN_DoFinalChecks(
	ANI::TRegistrationInfo *tri)
{
	// Function works recursively. Tell all our children: 
	arglist.RegisterTN_DoFinalChecks(tri);
	anonscopelist.RegisterTN_DoFinalChecks(tri);
	//incompletelist.RegisterTN_DoFinalChecks(tri);
	
	// Okay, do the final checks. 
	// See if there is a function in the same scope with the same 
	// arguments. 
	// Hmmm...
	// This is not easy because the caller need not specify the 
	// args in the same order when using the arg names in assignments 
	// at time of function call. Best solution is probably to do nothing 
	// and print ambiguity errors when resolving the actual function 
	// calls. 
	
	// However, we could do the checks for trivial cases...
	// (like: no argument, exactly 1 argument with same name)
	
	return(NULL);  // Always NULL; that is okay. 
}


int AniUserFunction::PerformCleanup(int cstep)
{
	int nerr=0;
	nerr+=arglist.PerformCleanup(cstep);
	nerr+=anonscopelist.PerformCleanup(cstep);
	nerr+=incompletelist.PerformCleanup(cstep);
	
	switch(cstep)
	{
		case 1:
			while(!incompletelist.is_empty())
			{  delete incompletelist.first();  }  // NOT popfirst(). 
			assert(typefix_list.is_empty());
			break;
		case 2:
		{
			fiinfo=new FuncInstanceInfo();
			int used=0;
			
			// Set addressation index of return value. 
			// -1 if void retval, fiinfo->aidx_this+1 if non-void: 
			if(return_type.TypeID()!=EVT_Void)
			{  fiinfo->aidx_retval=used++;  }
			else if(attrib & IS_Constructor)
			{  fiinfo->aidx_retval=-2;  /* special */  }
			
			// Process the arguments: 
			FillInInstanceInfo(fiinfo,&arglist,/*vtype=*/2,used);
			
			// Finally fill in the return type: 
			fiinfo->used_at_beginning=used;
			if(fiinfo->aidx_retval>=0)
			{  fiinfo->var_types[fiinfo->aidx_retval]=return_type;  }
			
		}	break;
		default:  assert(0);
	}
	
	return(nerr);
}


void AniUserFunction::LookupName(const RefString &name,
	AniScopeBase::ASB_List *ret_list,int query_parent)
{
	arglist.LookupName(name,ret_list);
	// No need to look up in anonscopelist -- it is anonymous. 
	// NEVER try to look up in AniIncomplete -- that are no identifiers 
	// just placeholders from where to look up existing ones!
	if(query_parent && parent)
	{  parent->LookupName(name,ret_list,query_parent-1);  }
}


void AniUserFunction::QueueChild(AniScopeBase *asb,int do_queue)
{
	if(do_queue>0) switch(asb->ASType())
	{
		case AST_UserVar:    arglist.append((AniVariable*)asb);  break;
		case AST_AnonScope:  anonscopelist.append((AniAnonScope*)asb);  break;
		case AST_Incomplete: incompletelist.append((AniIncomplete*)asb);  break;
		default: assert(0);   // <-- Especially AST_InternalVar. 
	}
	else if(do_queue<0) switch(asb->ASType())
	{
		case AST_UserVar:    arglist.dequeue((AniVariable*)asb);  break;
		case AST_AnonScope:  anonscopelist.dequeue((AniAnonScope*)asb);  break;
		case AST_Incomplete: incompletelist.dequeue((AniIncomplete*)asb);  break;
		default: assert(0);   // <-- Especially AST_InternalVar. 
	}
}

int AniUserFunction::CountScopes(int &n_incomplete) const
{
	int n=1;
	n+=arglist.CountScopes(n_incomplete);
	n+=anonscopelist.CountScopes(n_incomplete);
	n+=incompletelist.CountScopes(n_incomplete);
	return(n);
}


AniVariable *AniUserFunction::RegisterTN_FuncDefArgDecl(ANI::TNFuncDefArgDecl *tn,
	ANI::TRegistrationInfo *tri)
{
	assert(tn->NType()==TN_FuncDefArgDecl);
	assert(tn->down.count()==2);
	
	TNDeclarator *decl=(TNDeclarator*)tn->down.last();
	
	RefString var_name;
	TNIdentifier *name_idf=NULL;
	ExprValueType evt=GetDeclarationType(
		(TNTypeSpecifier*)tn->down.first(),decl, 
		/*is_automatic_var=*/1, &var_name,&name_idf);
	if(!evt)
	{  ++tri->n_errors;  return(NULL);  }
	
	// See if the name is used more than once or reserved. 
	if(CheckVarNameAlreadyUsed(var_name,name_idf))
	{  ++tri->n_errors;  return(NULL);  }
	
	int attrib=AniUserVariable::IS_Automatic;
	if(decl->GetDeclType()==TNDeclarator::DT_Initialize)
	{  attrib|=AniUserVariable::HAS_Initializer;  }
	AniVariable *var=new AniUserVariable(var_name,tn->down.last(),this,
		evt,attrib,tri->var_seq_num);
	// Assign associated scope: 
	name_idf->ani_scope=var;
	return(var);
}


ANI::TNLocation AniUserFunction::NameLocation() const
{
	// Let's find the identifier node...
	const TreeNode *dec=GetTreeNode();  // ...which is a TNDeclarator. 
	// This works because the TNDeclarator is always the first 
	// child of itself until we reach the TNIdentifier. 
	for(;;)
	{
		dec=dec->down.first();
		if(dec->NType()==TN_Declarator)  continue;
		assert(dec->NType()==TN_Identifier);
		break;
	}
	return(((TNIdentifier*)dec)->GetLocationRange());
}


LinkedList<AniScopeBase::TypeFixEntry> *AniUserFunction::GetTypeFixList()
{
	// Not inline as virtual. 
	return(&typefix_list);
}


AniScopeBase::InstanceInfo *AniUserFunction::GetInstanceInfo()
{
	// Not inline as virtual. 
	return(fiinfo);
}


const char *AniUserFunction::CG_AniScopeTypeStr() const
{
	return("function");
}


void AniUserFunction::AniTreeDump(StringTreeDump *d)
{
	d->Append(AniScopeTypeStr());
	d->Append(" ");
	if(attrib & IS_Constructor)
	{  d->Append("[constructor]");  }
	else if(attrib & IS_Destructor)
	{  d->Append("[destructor]");  }
	else if(attrib & IS_SettingInit)
	{  d->Append("[setting_init]");  }
	else
	{  d->Append(name.str());  }
	if((attrib & IS_Constructor) && (attrib & IS_Destructor))
	{  assert(0);  }
	d->Append(":");
	if(attrib & IS_Static)
	{  d->Append(" static");  }
	if(attrib & IS_Animation)
	{  d->Append(" animation");  }
	if(!(attrib & (IS_Constructor|IS_Destructor|IS_Animation|IS_SettingInit)))
	{
		d->Append(" ");
		d->Append(return_type.TypeString().str());
		if(return_type.IsIncomplete())
		{  d->Append(" [incomplete]");  }
	}
	if(attrib & IS_Const)
	{  d->Append(" const");  }
	d->Append("\n");
	
	d->AddIndent();
	arglist.AniTreeDump(d);
	anonscopelist.AniTreeDump(d);
	incompletelist.AniTreeDump(d);
	d->SubIndent();
}


AniUserFunction::AniUserFunction(const RefString &_name,TreeNode *_treenode,
	AniAniSetObj *_parent,int _attrib) : 
	AniFunction(AST_UserFunc,_name,_treenode,_parent,_attrib)
{
	fiinfo=NULL;
}

AniUserFunction::~AniUserFunction()
{
	arglist.ClearList();
	anonscopelist.ClearList();
	incompletelist.ClearList();
	
	while(!typefix_list.is_empty())
	{  delete typefix_list.popfirst();  }
	
	if(fiinfo)
	{
		// Some verbose output: 
		fprintf(stdout,"VM: function %s called %d times\n",
			PrettyCompleteNameString().str(),fiinfo->instance_counter);
		
		// Reset counter (important for assert() which checks if all 
		// ani/set/obj instances were cleaned up correctly; for 
		// functions, the instance_counter is simply a call counter...
		fiinfo->instance_counter=0;
	}
	
	DELETE(fiinfo);
}

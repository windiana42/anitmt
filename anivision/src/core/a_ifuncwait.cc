/*
 * core/a_ifuncwait.cc
 * 
 * ANI internal function for wait, pwait, trigger. 
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
#include "aniinstance.h"


using namespace ANI;

//------------------------------------------------------------------------------
// int pwait(until=<condition>)
//      essentially the same as while(!until) yield();
// int wait(until=<condition>,poll=NUM)
// NUM: When to check if the condition gets true
//      Check is only done if one of the vars in condition changes its 
//      value. With poll=1 such a check is always done when the change 
//      occurs, for poll=1 the re-eval (=check) is only done once per 
//      thread switch (just before switching). 
//------------------------------------------------------------------------------

void AniInternalFunc_Wait::_RegisterInternalFunctions(AniAniSetObj *parent)
{
	AniInternalFunc_Wait *fp=NULL;
	RefString tmp;
	switch(parent->ASType())
	{
		case AST_Animation:
			tmp.set("pwait");
			fp=new AniInternalFunc_Wait(tmp,parent,WFT_PWait);
			
			tmp.set("wait");
			fp=new AniInternalFunc_Wait(tmp,parent,WFT_Wait);
			
			tmp.set("trigger");
			fp=new AniInternalFunc_Wait(tmp,parent,WFT_Trigger);
			
			break;
		default:  break;  // do nothing
	}
}


//------------------------------------------------------------------------------

int AniInternalFunc_Wait::EvalFunc_PWait::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	// May only be called with thread info. 
	assert(info);
	EvalStateStack *ess=info->ess;
	
	// popS() state: 
	//   0 -> initial; 
	//   1 -> re-examining (i.e. returning from yield)
	switch(ess->popS())
	{
		case 0:
		{
			// -- Entering the first time. --
			// First, there is a pretty standard arg eval step (like for simple 
			// internal functions), just without arg permutation: 
			ExprValue val[nargs];
			int idx=0;
			TreeNode *i=tn->down.first()->next;
			TreeNode *stn=ess->popTN(i);
			for(; i!=stn; i=i->next,idx++)
			{  ess->popEV(&val[idx]);  }
			ExprValue tmpval;
			for(; i; i=i->next,idx++)
			{
				if(i->Eval(tmpval,info))
				{
					ess->pushTN(i);  // <-- Do not move!
					for(--idx,i=i->prev; idx>=0; i=i->prev,idx--)
					{  ess->pushEV(val[idx]);  }
					ess->pushS(0);
					return(1);
				}
				assert(arg_assfunc[idx]);
				arg_assfunc[idx]->Assign(
					/*lvalue=*/val[idx],
					/*rvalue=*/tmpval);
			}
			//assert(idx==nargs);  // can be left away
			
			// Fast path: if the "until" arg is non-null anyways, 
			// simply return. 
			if(!val[until_idx].is_null())
			{
				retval.assign((int)0);
				return(0);
			}
			
			// Okay, we have to wait. 
			ExecThread *et=(ExecThread*)info;
			if(et->schedule_status==ExecThread::SSReady)
			{
				ess->pushS(1);
				et->schedule_status=ExecThread::SSYield;
				return(1);  // Switch contexts. 
			}
			else if(et->schedule_status==ExecThread::SSDisabled)
			{
				Warning(tn->GetLocationRange(),
					"ignoring pwait() in single threaded (non-animation) "
					"context\n");
				retval.assign((int)-1);
				return(0);
			}
			else assert(0);
		}	break; // Never reached. 
		case 1:
		{
			// Coming here again after yield. 
			// Re-examine condition: 
			ExprValue tmpval;
			TreeNode *cond_tn=tn->down.first()->next;
			for(int cnt=until_idx; cnt; cnt--,cond_tn=cond_tn->next);
			if(cond_tn->Eval(tmpval,info))
			{
				ess->pushS(1);
				return(1);
			}
			ExprValue until_val;
			arg_assfunc[until_idx]->Assign(
				/*lvalue=*/until_val,
				/*rvalue=*/tmpval);
			
			if(until_val.is_null())
			{
				// Continue waiting. 
				ExecThread *et=(ExecThread*)info;
				if(et->schedule_status==ExecThread::SSReady)
				{
					ess->pushS(1);
					et->schedule_status=ExecThread::SSYield;
					return(1);
				}
				else assert(0);
			}
			else
			{
				// Done waiting. 
				retval.assign((int)0);
				return(0);
			}
		}	break; // Never reached. 
		default: assert(0);
	}
	
	// Never reached. 
	retval.assign((int)-1);
	return(0);
}

AniInternalFunc_Wait::EvalFunc_PWait::~EvalFunc_PWait()
{
	if(arg_assfunc)
	{
		for(int i=0; i<nargs; i++)
		{  DELETE(arg_assfunc[i]);  }
		arg_assfunc=(ANI::AssignmentFuncBase**)LFree(arg_assfunc);
	}
}

//------------------------------------------------------------------------------

int AniInternalFunc_Wait::EvalFunc_Wait::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *_info)
{
	// May only be called with thread info. 
	assert(_info);
	EvalStateStack *ess=_info->ess;
	
	ExecThread *et=(ExecThread*)_info;
	
	// popS() state: 
	//   0 -> initial; 
	//   1 -> ?!
	switch(ess->popS())
	{
		case 0:
		{
			// -- Entering the first time. --
			// First, there is a pretty standard arg eval step (like for simple 
			// internal functions), just without arg permutation and with 
			// extra-magic for the var_notif. 
			ExprValue val[nargs];
			int idx=0;
			TreeNode *i=tn->down.first()->next;
			TreeNode *stn=ess->popTN(i);
			for(; i!=stn; i=i->next,idx++)
			{  ess->popEV(&val[idx]);  }
			ExprValue tmpval;
			for(; i; i=i->next,idx++)
			{
				if(idx==until_idx)
				{
					// We need an ExecThreadInfo::VarNotifier attached at 
					// the thread info to be able to register at the used 
					// variables in the until condition expression. 
					et->wait_cond.InstallVarNotifier(i);
				}
				if(i->Eval(tmpval,et))
				{
					// We may not allow context switches for cloning 
					// during eval of any wait argument. 
					if(et->schedule_status==ExecThread::SSRequest || 
					   et->schedule_status==ExecThread::SSDelay )
					{  Error(i->GetLocationRange(),
						"%s forbidden within %s()\n",
						et->schedule_status==ExecThread::SSDelay ? 
							"delaying" : "cloning",
						function->GetName().str());  abort();  }
					if(idx==until_idx)
					{
						// Temporarily detach; we attach again when we 
						// come here again. 
						et->wait_cond.DetachVarNotifier(/*temporarily=*/1);
					}
					
					ess->pushTN(i);  // <-- Do not move!
					for(--idx,i=i->prev; idx>=0; i=i->prev,idx--)
					{  ess->pushEV(val[idx]);  }
					ess->pushS(0);
					return(1);
				}
				if(idx==until_idx)
				{
					// Detach the var notifier: 
					et->wait_cond.DetachVarNotifier(/*temporarily=*/-1);
				}
				assert(arg_assfunc[idx]);
				arg_assfunc[idx]->Assign(
					/*lvalue=*/val[idx],
					/*rvalue=*/tmpval);
			}
			//assert(idx==nargs);  // can be left away
			
			// NOTE: BELOW HERE: 
			//       EITHER WE StartWaiting() OR StopWaiting() BUT 
			//        WE MUST CALL AT LEAST ONE OF THE FUNCTIONS. 
			
			// Fast path: if the "until" arg is non-null anyways, 
			// simply return. 
			if(!val[until_idx].is_null())
			{
				et->wait_cond.StopWaiting();
				retval.assign((int)0);
				return(0);
			}
			
			// See the "poll" value. 
			int poll_val = poll_idx<0 ? /*default=*/1 : 
				val[poll_idx].GetPODInt();
			
			// Okay, we have to wait. 
			if(et->schedule_status==ExecThread::SSReady)
			{
				et->wait_cond.StartWaiting(poll_val);
				ess->pushS(1);
				return(1);  // Switch contexts. 
			}
			else if(et->schedule_status==ExecThread::SSDisabled)
			{
				Warning(tn->GetLocationRange(),
					"ignoring wait() in single threaded (non-animation) context\n");
				et->wait_cond.StopWaiting();
				retval.assign((int)0);
				return(0);
			}
			else assert(0);
		}	break; // Never reached. 
		case 1:
		{
			// Coming here only to check the condition. 
			// We're still in waiting state and not in running state
			assert(et->state==ExecThread::Waiting);
			assert(et->currently_evaluating_this_wcond);
			// Re-examine condition: 
			ExprValue tmpval;
			TreeNode *cond_tn=tn->down.first()->next;
			for(int cnt=until_idx; cnt; cnt--,cond_tn=cond_tn->next);
			if(cond_tn->Eval(tmpval,et))
			{
				if(et->schedule_status==ExecThread::SSRequest || 
				   et->schedule_status==ExecThread::SSWait || 
				   et->schedule_status==ExecThread::SSDelay )
				{
					Error(cond_tn->GetLocationRange(),
						"%s forbidden from within %s condition\n",
						et->schedule_status==ExecThread::SSWait ? 
							"waiting" : 
							(et->schedule_status==ExecThread::SSDelay ?
								"delaying" : "cloning"),
						function->GetName().str());
					abort();
				}
				ess->pushS(1);
				return(1);
			}
			ExprValue until_val;
			arg_assfunc[until_idx]->Assign(
				/*lvalue=*/until_val,
				/*rvalue=*/tmpval);
			
			if(et->schedule_status==ExecThread::SSReady)
			{
				if(until_val.is_null())
				{
					// Continue waiting. 
					ess->pushS(1);
				}
				else
				{
					// Done waiting. 
					et->wait_cond.StopWaiting();
					ess->pushS(2);
				}
				// Must return SSWait in both cases. 
				et->schedule_status=ExecThread::SSWait;
				return(1);  // <-- CORRECT for both cases. 
			}
			else assert(0);
		}	break; // Never reached. 
		case 2:
		{
			// Waiting done; go on regularly. 
			assert(et->state==ExecThread::Running);
			assert(et->schedule_status==ExecThread::SSReady);
			assert(!et->currently_evaluating_this_wcond);
			retval.assign((int)0);
			return(0);
		}	break; // Never reached. 
		default: assert(0);
	}
	
	// Never reached. 
	retval.assign((int)-1);
	return(0);
}

AniInternalFunc_Wait::EvalFunc_Wait::~EvalFunc_Wait()
{
	if(arg_assfunc)
	{
		for(int i=0; i<nargs; i++)
		{  DELETE(arg_assfunc[i]);  }
		arg_assfunc=(ANI::AssignmentFuncBase**)LFree(arg_assfunc);
	}
}

//------------------------------------------------------------------------------

int AniInternalFunc_Wait::EvalFunc_Trigger::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	assert(0);  // unimplemented. 
	return(0);
}

AniInternalFunc_Wait::EvalFunc_Trigger::~EvalFunc_Trigger()
{
	
}

//******************************************************************************

int AniInternalFunc_Wait::_Match_FuncArgs_PWait(LookupFunctionInfo *lfi,
	int store_evalhandler)
{
	int args_matching=0;     // args matching
	int args_matching_cv=0;  // args matching with type conversion
	
	ExprValueType int_t=ExprValueType::RIntegerType();
	
	// Stores argument indices in lfi. 
	int until_idx=-1;
	char which_arg='\0';  // 'u','p': until,poll
	
	for(int i=0; i<lfi->n_args; i++)
	{
		if(lfi->arg_name[i])
		{
			// Look up argument: 
			RefString name=lfi->arg_name[i]->CompleteStr();
			if(name=="until")  which_arg='u';
			else return(0);
		}
		else switch(which_arg)  // <-- switch forward one arg
		{
			case '\0': which_arg='u';  break;
			case 'u':  return(0);
			default:  assert(0);
		}
		
		switch(which_arg)
		{
			case '\0':  return(0);
			case 'u':
				if(until_idx>=0)  return(0);  // Two or more "until" args.
				until_idx=i;
				break;
			default:  assert(0);
		}
		
		// Check arg type: 
		int rv=int_t.CanAssignType(lfi->arg_type[i],NULL);
		if(rv==0)  // No, cannot. Type mismatch. 
		{  return(0);  }
		else if(rv==1)  ++args_matching_cv;
		else  ++args_matching;
	}
	assert(args_matching+args_matching_cv==lfi->n_args);
	
	// Need at least "until". 
	if(until_idx<0)  return(0);
	
	if(store_evalhandler)
	{
		EvalFunc_PWait *evalfunc=new EvalFunc_PWait(this);
		
		evalfunc->nargs=lfi->n_args;
		assert(evalfunc->nargs<=1);  // <-- Remove once there are more args. 
		evalfunc->arg_assfunc=(ANI::AssignmentFuncBase**)LMalloc(
			evalfunc->nargs*sizeof(ANI::AssignmentFuncBase*));
		for(int i=0; i<evalfunc->nargs; i++)
		{  evalfunc->arg_assfunc[i]=NULL;  }
		
		// "until" argument: 
		evalfunc->until_idx=until_idx;
		int rv=int_t.CanAssignType(lfi->arg_type[evalfunc->until_idx],
			&evalfunc->arg_assfunc[evalfunc->until_idx]);
		assert(rv>0);  // NOT 0 or <0. 
		
		lfi->evalfunc=evalfunc;
		lfi->ret_evt=int_t;
		//lfi->may_try_ieval stays 0 (of course)
	}
	
	// Okay, looks good. 
	return(args_matching_cv ? 1 : 2);
}


int AniInternalFunc_Wait::_Match_FuncArgs_Wait(LookupFunctionInfo *lfi,
	int store_evalhandler)
{
	int args_matching=0;     // args matching
	int args_matching_cv=0;  // args matching with type conversion
	
	ExprValueType int_t=ExprValueType::RIntegerType();
	
	// Stores argument indices in lfi. 
	int until_idx=-1;
	int poll_idx=-1;
	char which_arg='\0';  // 'u','p': until,poll
	
	for(int i=0; i<lfi->n_args; i++)
	{
		if(lfi->arg_name[i])
		{
			// Look up argument: 
			RefString name=lfi->arg_name[i]->CompleteStr();
			if(name=="until")  which_arg='u';
			else if(name=="poll")  which_arg='p';
			else return(0);
		}
		else switch(which_arg)  // <-- switch forward one arg
		{
			case '\0': which_arg='u';  break;
			case 'u':  which_arg='p';  break;
			case 'p':  return(0);
			default:  assert(0);
		}
		
		switch(which_arg)
		{
			case '\0':  return(0);
			case 'u':
				if(until_idx>=0)  return(0);  // Two or more "until" args.
				until_idx=i;
				break;
			case 'p':
				if(poll_idx>=0)  return(0);  // Two or more "poll" args.
				poll_idx=i;
				break;
			default:  assert(0);
		}
		
		// Check arg type: 
		int rv=int_t.CanAssignType(lfi->arg_type[i],NULL);
		if(rv==0)  // No, cannot. Type mismatch. 
		{  return(0);  }
		else if(rv==1)  ++args_matching_cv;
		else  ++args_matching;
	}
	assert(args_matching+args_matching_cv==lfi->n_args);
	
	// Need at least "until". 
	if(until_idx<0)  return(0);
	
	if(store_evalhandler)
	{
		EvalFunc_Wait *evalfunc=new EvalFunc_Wait(this);
		
		evalfunc->nargs=lfi->n_args;
		assert(evalfunc->nargs<=2);  // <-- Remove once there are more args. 
		evalfunc->arg_assfunc=(ANI::AssignmentFuncBase**)LMalloc(
			evalfunc->nargs*sizeof(ANI::AssignmentFuncBase*));
		for(int i=0; i<evalfunc->nargs; i++)
		{  evalfunc->arg_assfunc[i]=NULL;  }
		
		// "until" argument: 
		evalfunc->until_idx=until_idx;
		int rv=int_t.CanAssignType(lfi->arg_type[evalfunc->until_idx],
			&evalfunc->arg_assfunc[evalfunc->until_idx]);
		assert(rv>0);  // NOT 0 or <0. 
		
		// "poll" argument if any: 
		evalfunc->poll_idx=poll_idx;
		if(poll_idx>=0)
		{
			rv=int_t.CanAssignType(lfi->arg_type[evalfunc->poll_idx],
				&evalfunc->arg_assfunc[evalfunc->poll_idx]);
			assert(rv>0);  // NOT 0 or <0. 
		}
		
		lfi->evalfunc=evalfunc;
		lfi->ret_evt=int_t;
		//lfi->may_try_ieval stays 0 (of course)
	}
	
	// Okay, looks good. 
	return(args_matching_cv ? 1 : 2);
}


int AniInternalFunc_Wait::_Match_FuncArgs_Trigger(LookupFunctionInfo *lfi,
	int store_evalhandler)
{
	// Implement me. 
	assert(0);
	return(0);
}


int AniInternalFunc_Wait::Match_FunctionArguments(LookupFunctionInfo *lfi,
	int store_evalhandler)
{
	switch(func_type)
	{
		case WFT_PWait:
			return(_Match_FuncArgs_PWait(lfi,store_evalhandler));
		case WFT_Wait:
			return(_Match_FuncArgs_Wait(lfi,store_evalhandler));
		case WFT_Trigger:
			return(_Match_FuncArgs_Trigger(lfi,store_evalhandler));
		default: assert(0);
	}
	
	return(0);  // "no match"
}


String AniInternalFunc_Wait::PrettyCompleteNameString() const
{
	switch(func_type)
	{
		case WFT_PWait:
			return(String("int pwait(int until)"));
		case WFT_Wait:
			return(String("int wait(int until,int poll=1)"));
		case WFT_Trigger:
			return(String("int trigger(int cond,#FIXME#)"));
		default: ; // Fall off. 
	}
	assert(0);
	return(String("???"));
}


void AniInternalFunc_Wait::AniTreeDump(StringTreeDump *d)
{
	d->Append(AniScopeTypeStr());
	d->Append(" ");
	switch(func_type)
	{
		case WFT_PWait:
			d->Append("pwait(until): int [special]");
			break;
		case WFT_Wait:
			d->Append("wait(until,poll=1): int [special]");
			break;
		case WFT_Trigger:
			d->Append("trigger(cond): int [special]");
			break;
		default: assert(0);
	}
	d->Append("\n");
}


AniInternalFunc_Wait::AniInternalFunc_Wait(const RefString &_name,
	AniAniSetObj *_parent,WaitFuncType _func_type) : 
	AniInternalFunction(_name,_parent,
		/*attrib=*/IS_Static,
		/*treenode=*/NULL),
	func_type(_func_type)
{
	// empty
}

AniInternalFunc_Wait::~AniInternalFunc_Wait()
{
	// empty
}

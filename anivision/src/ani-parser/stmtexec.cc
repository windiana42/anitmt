/*
 * ani-parser/stmtexec.cc
 * 
 * Principal statement execution code. 
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
#include "exprvalue.h"
#include "threadinfo.h"


#warning "**************************************************"
#warning "*******     !! CHECK ALL the FIXMEs !!     *******"
#warning "**************************************************"

// This is the actual statement execution code. 

namespace ANI
{

TNStatement::ExecRStatus TNStatement::Exec(ExecThreadInfo * /*info*/)
{
	// Must be overridden by all TNStatement-derived classes. 
	assert(0);
	
	return(ERS_Cont);
}

/*----------------------------------------------------------------------------*/

TNStatement::ExecRStatus TNExpressionStmt::Exec(ExecThreadInfo *info)
{
	if(down.first())
	{
		EvalStateStack *ess=info->ess;
		
		ExecThreadInfo::DelayedEntry *dstart;
		switch(ess->popS())  // Fallthrough switch. 
		{
			case 0:
			{
				// We may only process those delayed operators which were 
				// queued while evaluating this expression. 
				dstart=info->delayed_list.last();
				ssize_t cnt=ess->popL(0);
				for(; cnt>0; dstart=dstart->prev,cnt--);
				
				// Evaluate the expression. No interest in the retun value. 
				ExprValue tmp;
				int rv=down.first()->Eval(tmp,info);
				if(rv)
				{
					// Count number of list elements from dstart to end: 
					// We may not push the node in the delayed_list 
					// (cast to a TreeNode) because this would bring 
					// us into trouble when cloning. Hence, we save the 
					// distance to the end of the list. 
					#warning "Invent an O(1) solution..."
					dstart = dstart ? dstart->next : info->delayed_list.first();
					for(cnt=0; dstart; dstart=dstart->next,cnt++);
					ess->pushL(cnt);
					ess->pushS(0);
					return(ERS_ContextSwitch);
				}
				++info->exec_stmt_cnt;
				
				// Store the first entry to eval here: 
				dstart = dstart ? dstart->next : info->delayed_list.first();
				
				goto skippop;
			}
			case 1:
			{
				dstart=info->delayed_list.last();
				ssize_t cnt=ess->popL(-1);
				assert(cnt>=0);
				for(; cnt>0; dstart=dstart->prev,cnt--);
				assert(dstart);
			}
			skippop:
				while(dstart)
				{
					// Evaluate post increment/decrement and the like: 
					if(dstart->opfunc->evalfunc->DelayedEval(
						dstart->lval,dstart->opfunc,info))
					{
						// Count number of list elements from dstart to end: 
						ssize_t cnt=0;
						for(dstart=dstart->next; dstart;
							dstart=dstart->next,cnt++);
						ess->pushL(cnt);
						ess->pushS(1);
						return(ERS_ContextSwitch);
					}
					ExecThreadInfo::DelayedEntry *tmp=dstart;
					dstart=dstart->next;
					delete info->delayed_list.dequeue(tmp);
				}
				break;
			default: assert(0);
		}
	}
	
	return(ERS_Cont);
}


TNStatement::ExecRStatus TNIterationStmt::Exec(ExecThreadInfo *info)
{
	EvalStateStack *ess=info->ess;
	
	ExprValue tmp;
	
	if(itertype==IT_DoWhile)
	{
		switch(ess->popS())  // Fallthrough switch. 
		{
			for(;;)
			{
				case 0:
					// Exec loop body: 
					switch(loop_stmt->Exec(info))
					{
						case ERS_Cont: break;
						case ERS_ContextSwitch:
							ess->pushS(0);
							return(ERS_ContextSwitch);
						case ERS_Return:  return(ERS_Return);
						case ERS_Break:   return(ERS_Cont);  // break done. 
						default:
							assert(0);
					}
				case 1:
					// Check condition: 
					if(loop_cond)
					{
						if(loop_cond->Eval(tmp,info))
						{
							ess->pushS(1);
							return(ERS_ContextSwitch);
						}
						++info->exec_stmt_cnt;
						if(tmp.is_null())  goto breakoutA;
					}
			}
		}
		breakoutA:;
	}
	else
	{
		switch(ess->popS())  // Heavy fallthrough switch. 
		{
			case 0:
				if(for_init_stmt) switch(for_init_stmt->Exec(info))
				{
					case ERS_Cont: break;
					case ERS_ContextSwitch:
						ess->pushS(0);
						return(ERS_ContextSwitch);
					case ERS_Return:  // fall...
					case ERS_Break:   //   ...through...
					default:          //        ...here
						assert(0);
				}
			case 1:
				for(;;)
				{
					// Check condition if any: 
					if(loop_cond)
					{
						if(loop_cond->Eval(tmp,info))
						{
							ess->pushS(1);
							return(ERS_ContextSwitch);
						}
						++info->exec_stmt_cnt;
						if(tmp.is_null())  goto breakoutB;
					}
			
			case 2:
					// Exec loop body: 
					switch(loop_stmt->Exec(info))
					{
						case ERS_Cont: break;
						case ERS_ContextSwitch:
							ess->pushS(2);
							return(ERS_ContextSwitch);
						case ERS_Return:  return(ERS_Return);
						case ERS_Break:   return(ERS_Cont);  // break done. 
						default:
							assert(0);
					}
					
			case 3:
					// Inc statement (if any):
					if(for_inc_stmt) switch(for_inc_stmt->Exec(info))
					{
						case ERS_Cont: break;
						case ERS_ContextSwitch:
							ess->pushS(3);
							return(ERS_ContextSwitch);
						case ERS_Return:  // fall...
						case ERS_Break:   //   ...through...
						default:          //        ...here
							assert(0);
					}
				}  // <-- end of for()
		}
		breakoutB:;
	}
	
	return(ERS_Cont);
}


TNStatement::ExecRStatus TNSelectionStmt::Exec(ExecThreadInfo *info)
{
	EvalStateStack *ess=info->ess;
	
	short int cond=ess->popS(101);
	switch(cond)  // Fallthrough switch. 
	{
		case 101:
		{
			ExprValue tmp;
			assert(cond_expr);
			if(cond_expr->Eval(tmp,info))
			{
				ess->pushS(101);
				return(ERS_ContextSwitch);
			}
			++info->exec_stmt_cnt;
			
			cond=tmp.is_null();
		}
		case 0:
		case 1:
		case 2:
		case -1:
			if(!cond) switch(if_stmt->Exec(info))
			{
				case ERS_Cont:  break;
				case ERS_ContextSwitch:
					ess->pushS(cond);
					return(ERS_ContextSwitch);
				case ERS_Return:  return(ERS_Return);
				case ERS_Break:   return(ERS_Break);
				default:  assert(0);
			}
			else if(else_stmt) switch(else_stmt->Exec(info))
			{
				case ERS_Cont:  break;
				case ERS_ContextSwitch:
					assert(cond>=-1 && cond<=2);
					ess->pushS(cond);
					return(ERS_ContextSwitch);
				case ERS_Return:  return(ERS_Return);
				case ERS_Break:   return(ERS_Break);
				default:  assert(0);
			}
		break;
		default: assert(0);
	}
	
	return(ERS_Cont);
}


TNStatement::ExecRStatus TNJumpStmt::Exec(ExecThreadInfo *info)
{
	switch(jumptype)
	{
		case JT_Break:
		{
			// Well... this is easy. 
			return(ERS_Break);
		}
		case JT_Return:
		{
			assert(evalfunc);
			if(evalfunc->Eval(this,info))
			{  return(ERS_ContextSwitch);  }
			++info->exec_stmt_cnt;
			
			// Okay... now as we have done the evaluation we need 
			// to "jump" to the correct execution position...
			return(ERS_Return);
		}
		case JT_None:  // fall through
		default: assert(0);
	}
	
	return(ERS_Cont);
}


TNStatement::ExecRStatus TNCompoundStmt::Exec(ExecThreadInfo *info)
{
	EvalStateStack *ess=info->ess;
	
	ExecRStatus ers=ERS_Cont;
	switch(ess->popS())  // Fallthrough switch. 
	{
		case 0:
			// Must allocate stack frame, etc. 
			((AniGlue_ScopeBase*)anonscope)->CG_ExecEnterCompound(info);
		case 1:
			// Simply execute the statements sequentially: 
			for(TreeNode *tn=ess->popTN(down.first()); tn; tn=tn->next)
			{
				ers=((TNStatement*)tn)->Exec(info);
				switch(ers)
				{
					case ERS_Cont:  break;
					case ERS_ContextSwitch:
						ess->pushTN(tn);
						ess->pushS(1);
						return(ERS_ContextSwitch);
					case ERS_Return:  // fall through
					case ERS_Break:   goto breakfor;
					default:  assert(0);
				}
			}
			breakfor:
		case 2:
			// Must free stack frame, etc. 
			((AniGlue_ScopeBase*)anonscope)->CG_ExecLeaveCompound(info);
			break;
		default: assert(0);
	}
	
	return(ers);
}


TNStatement::ExecRStatus TNDeclarationStmt::Exec(ExecThreadInfo *info)
{
	EvalStateStack *ess=info->ess;
	
	ExprValue tmp;
	// Process all declarators: 
	for(TreeNode *tn=ess->popTN(down.first()->next); tn; tn=tn->next)
	{
		if(tn->Eval(tmp,info))
		{
			ess->pushTN(tn);
			return(ERS_ContextSwitch);
		}
		++info->exec_stmt_cnt;
	}
	
	return(ERS_Cont);
}


TNStatement::ExecRStatus TNAniDescStmt::Exec(ExecThreadInfo *info)
{
	EvalStateStack *ess=info->ess;
	
	// NO assignment to nvalue; we need it as lvalue. 
	ExprValue nvalue;
	switch(ess->popS())  // Fall through switch. 
	{
		case 0:
			// Get the name element: 
			if(!name_expr)
			{  nvalue=ExprValue(ExprValue::Copy,info->stack->GetThisPtr());  }
			else if(name_expr->Eval(nvalue,info))
			{
				ess->pushS(0);
				return(ERS_ContextSwitch);
			}
			
			// NO assignment to nvalue; we need it as lvalue. 
			// Exception: objects may also be non-lvalues and thus 
			// index handlers or so... the calc core eval function 
			// handles that. 
			#if 0   /* THIS IS NOT NEEDED: Calc core handles that. */
			switch(nvalue.GetExprValueTypeID())
			{
				case EVT_AniScope:
				{
					ScopeInstance tmp;
					nvalue.GetAniScope(&tmp);
					nvalue=ExprValue(ExprValue::Copy,tmp);
				} break;
				case EVT_POD: break;  // NO assign. 
				default: assert(0);
			}
			#endif
			
			goto skippop;
		case 1:
			ess->popEV(&nvalue);
		skippop:
			if(adb_child && adb_child->Eval(/*NOT retval=*/nvalue,info))
			{
				ess->pushEV(nvalue);
				ess->pushS(1);
				return(ERS_ContextSwitch);
			}
			break;
		default: assert(0);
	}
	
	return(ERS_Cont);
}

}  // end of namespace ANI. 

/*
 * ani-parser/threadinfo.cc
 * 
 * Execution thread info as used by the expression evaluation and 
 * statement execution. 
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
#include "threadinfo.h"

namespace ANI
{

// Init static variables: 
short int ExecThreadInfo::switch_context_now=0;
short int ExecThreadInfo::currently_evaluating_any_wcond=0;


void ExecThreadInfo::CallVarNotifiers(TNIdentifier *idf,ExprValue &ev)
{
	for(VarNotifier *_i=var_notifier_list.first(); _i; )
	{
		VarNotifier *i=_i;
		_i=_i->next;
		// NOTE: notifier may remove itself from var_notifier_list and 
		// then delete iself, so do not use i any longer here. 
		i->notify(this,idf,ev);
	}
}


void ExecThreadInfo::VarNotifier::notify(ExecThreadInfo *info,
	TNIdentifier *idf,ExprValue &ev)
{
	// Must be overridden when used. It's the sole 
	// purpose of this class. 
	assert(0);
}


ExecThreadInfo::ExecThreadInfo() : 
	delayed_list(),
	var_notifier_list()
{
	stack=new ExecStack();
	ess=new EvalStateStack();
	exec_stmt_cnt=0U;
	currently_evaluating_this_wcond=0;
}

ExecThreadInfo::ExecThreadInfo(_IClone,const ExecThreadInfo *src) : 
	delayed_list(),
	var_notifier_list()
{
	stack=src->stack ? new ExecStack(ExecStack::Clone,src->stack) : NULL;
	ess=src->ess ? new EvalStateStack(EvalStateStack::Clone,src->ess) : NULL;
	exec_stmt_cnt=0U;
	currently_evaluating_this_wcond=0;
	// May not clone from within waiting cond; should have been 
	// caught earlier and reported. 
	if(src->currently_evaluating_this_wcond) assert(0);
	
	for(DelayedEntry *i=src->delayed_list.first(); i; i=i->next)
	{
		// Cloning these (i.e. for "i++") may have 
		// undesired consequences (like incrementing i twice). 
		// NOTE: This is nothing special; just a consequence of the 
		//       fact that the two threads after cloning share the 
		//       same values. 
		Warning(i->opfunc->GetLocationRange(),
			"cloning delayed eval request\n");
		QueueForDelayedEval(i->opfunc,i->lval);
	}
	
	if(!src->var_notifier_list.is_empty())
	{
		// This may never happen. The eval functions must make sure 
		// to remove the notifiers when the eval routine returns 
		// (even if just for context switch). 
		Error(TNLocation(),
			"attempt to clone %d var notifiers\n",
			src->var_notifier_list.count());
		abort();
	}
}

ExecThreadInfo::~ExecThreadInfo()
{
	// Some verbose message: 
	fprintf(stdout,
		"VM:   ExecThreadInfo: %llu (expr) stmts executed.\n",
		(unsigned long long)exec_stmt_cnt);
	
	while(!var_notifier_list.is_empty())
	{  delete var_notifier_list.popfirst();  }
	
	if(!delayed_list.is_empty())
	{
		fprintf(stderr,"OOPS: %d entries in delayed list!!!\n",
			delayed_list.count());
		while(!delayed_list.is_empty())
		{  delete delayed_list.popfirst();  }
	}
	
	DELETE(ess);
	DELETE(stack);
}

}  // end of namespace ANI

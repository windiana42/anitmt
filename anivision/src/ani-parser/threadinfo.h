/*
 * ani-parser/threadinfo.h
 * 
 * Execution thread info description as used by statement execution 
 * and expression evaluation. 
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

#ifndef _ANIVISION_ANI_EXECTHREADINFO_H_
#define _ANIVISION_ANI_EXECTHREADINFO_H_ 1

#include <ani-parser/coreglue.h>
#include <ani-parser/exprvalue.h>
#include <ani-parser/execstack.h>
#include <ani-parser/evalstack.h>

namespace ANI
{

// Some derived class should be provided by core. 
// This is passed to Exec() and Eval() functions of the 
// tree nodes: 
// NOT C++ - safe. 
struct ExecThreadInfo
{
	// When this flag is set, do context switch as fast as possible 
	// (including next value/identifier eval). 
	static short int switch_context_now;
	// Currently evaluating any waiting condition?
	static short int currently_evaluating_any_wcond;
	
	// Currently evaluating the waiting condition of THIS thread?
	int currently_evaluating_this_wcond;
	
	// Thread's stack: automatic variables and "this" pointers. 
	ExecStack *stack;
	// Eval state stack to save state on thread context switches. 
	EvalStateStack *ess;
	
	// Count number of executed (expression) statements, i.e. the 
	// number of Eval() calls done from within Exec() functions. 
	u_int64_t exec_stmt_cnt;
	
	// For the delayed evaluation: 
	struct DelayedEntry : LinkedListBase<DelayedEntry>
	{
		TNOperatorFunction *opfunc;
		ExprValue lval;
		
		_CPP_OPERATORS
		DelayedEntry(TNOperatorFunction *_opfunc,const ExprValue _lval) : 
			opfunc(_opfunc),lval(_lval) {}
		~DelayedEntry() {}
	};
	LinkedList<DelayedEntry> delayed_list;
	
	// Queue operator for delayed evaluation: 
	void QueueForDelayedEval(TNOperatorFunction *opfunc,const ExprValue &lval)
		{  delayed_list.append(new DelayedEntry(opfunc,lval));  }
	
	// User-installable notifier list which is called for every 
	// evaluated TNIdentifier (after evaluation): 
	// NOTE: List held by ExecThreadInfo; may only be in ONE list 
	//       at a time. 
	struct VarNotifier : LinkedListBase<VarNotifier>
	{
		// Virtual var notification function; must be overridden. 
		// NOTE: Yes, it is safe to call info->var_notifier_list.sequeue(this) 
		//       from within the notifier (to remove it). 
		virtual void notify(ExecThreadInfo *info,TNIdentifier *idf,
			ExprValue &ev);
		
		_CPP_OPERATORS
		VarNotifier() {}
		virtual ~VarNotifier() {}
	};
	LinkedList<VarNotifier> var_notifier_list;
	
	// Used by TNIdentifier to have the var notifiers called: 
	void CallVarNotifiers(TNIdentifier *idf,ExprValue &ev);
	
	// Used to identify clone constructor. 
	enum _IClone  { _Clone };
	
	ExecThreadInfo();
	// Create cloned ExecThreadInfo; use Clone function of derived 
	// type if present. 
	ExecThreadInfo(_IClone,const ExecThreadInfo *src);
	~ExecThreadInfo();
};

}  // End of namespace ANI. 

#endif  /* _ANIVISION_ANI_EXECTHREADINFO_H_ */

/*
 * core/execthread.cc
 * 
 * Implementation of ExecThread, the execution thread class. 
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

#include "execthread.h"
#include "a_anisetobj.h"

#include <ani-parser/tree.h>

#include <calccore/ccif.h>


using namespace ANI;


// Initialize static var: 
int ExecThread::wait_cond_re_eval_cs=0;


void ExecThread::WaitCondition::notify(
	ExecThreadInfo *info,TNIdentifier *idf,ExprValue &ev)
{
	// This is a sanity check. We attached the us as VarNotifier 
	// to _this_ thread and this wait condition. 
	assert(et==info);
	// Must be marked as "in use for registration": 
	assert(wcstate==WC_RegAttached);  // and NOT WC_RegTempDetached
	
	if(et->stack->TopFrameIdx()!=vn_stack_depth)
	{
		// We may be inside a function called from within the condition. 
		// In that case, simply ignore the identifiers. 
		assert(et->stack->TopFrameIdx()>vn_stack_depth);
		return;
	}
	
	// We're on the same stack depth as the condition, hence 
	// we must register to make sure getting informed for value 
	// changes. 
	
	fprintf(stderr,"Register >%s<\n",idf->CompleteStr().str());
	// Implement me...
	if(((TNIdentifierSimple*)idf->down.last())->name.str()[0]=='$')
	{
		// Registration of "$time", "$..." internal vars is not yet supported. 
		// (We may not register at the ExprValue in that case.)
		assert(0);
	}
	
	// Register at the ExprValue: 
	ValueNotifyHandler::VNInstall(ev,/*hook=*/NULL);
}


void ExecThread::WaitCondition::vn_change(void * /*hook*/)
{
	// We're getting here becuase of the variables in the wait 
	// condition changed. This means, we may need to re-eval the 
	// wait condition. 
	
	// If this happens during eval of our own condition, ignore it: 
	if(et->currently_evaluating_this_wcond) return;
	
	// First, let's see if we're waiting at all. 
	if(wcstate==WC_Waiting)
	{
		// We're actually waiting and a value in the condition 
		// changed. Must re-eval. 
		// If the poll_val is set, we re-eval at the next regular 
		// context switch; otherwise schedule context switch now. 
		
		// Increase the "must evaulate condition flag"...
		++must_eval;
		// ...and tell the main loop about that. 
		ExecThread::wait_cond_re_eval_cs=1;  // <-- static var. 
		
		// Schedule immediate context switch if poll_val==0. 
		// BUT not if currently_evaluating_any_wcond, i.e. we're 
		// currently not evaulating any waiting condition. 
		if(!poll_val && !ExecThreadInfo::currently_evaluating_any_wcond)
		{
			//fprintf(stderr,"POCK![switch_context_now]\n");
			ExecThreadInfo::switch_context_now=1;
		}
	}
	else if(wcstate==WC_NotInUse)
	{
		// This is bad. Because in that case, we should 
		// not be attached to any ExprValue and hence not 
		// receive any change notifications. 
		assert(0);
	}
	//else if(wcstate==WC_RegAttached || wcstate==WC_RegTempDetached || 
	//	wcstate==WC_WaitPending)
	//{
	//	// We're still registering the wait or not yet waiting. 
	//	// Ignore the changes. 
	//	return;
	//}
}

void ExecThread::WaitCondition::vn_delete(void *hook)
{
	// ExprValue inside wait() condition gets deleted. 
	// For a wait condition, this may not happen. 
	// (It _may_ happen for trigger conditions, though.) 
	fprintf(stderr,"OOPS: WaitCondition::vn_delete() for wcstate=%d\n",
		wcstate);
	assert(0);
	abort();
}


void ExecThread::WaitCondition::InstallVarNotifier(TreeNode *tn_for_error)
{
	if(wcstate!=WC_NotInUse && wcstate!=WC_RegTempDetached)
	{
		Error(tn_for_error->GetLocationRange(),
			"may not use wait() inside wait() condition\n");
		abort();
	}
	
	if(wcstate==WC_RegTempDetached)
	{  assert(vn_stack_depth==et->stack->TopFrameIdx());  }
	else  // WC_NotInUse
	{  vn_stack_depth=et->stack->TopFrameIdx();  }
	
	wcstate=WC_RegAttached;
	et->var_notifier_list.append(this);
}


void ExecThread::WaitCondition::DetachVarNotifier(int temporarily)
{
	assert(wcstate==WC_RegAttached);
	et->var_notifier_list.dequeue(this);
	     if(temporarily>0)  wcstate=WC_RegTempDetached;
	else if(temporarily<0)  wcstate=WC_WaitPending;
	else                    wcstate=WC_NotInUse;
	// Calling temporarily=0 currently makes no sense. 
	assert(temporarily);
}


void ExecThread::WaitCondition::StartWaiting(int _poll_val)
{
	// We must be in non-attached state here, "waiting to start waiting". 
	assert(wcstate==WC_WaitPending);
	
	poll_val=_poll_val;
	must_eval=0;
	
	fprintf(stderr,"VM: Thread[%d] starts waiting (poll=%d)\n",
		et->thread_id,poll_val);
	
	// Actually start waiting: 
	wcstate=WC_Waiting;
	assert(et->schedule_status==SSReady); // <-- Caller should have reported warning/error. 
	et->schedule_status=SSWait;
	// Calling eval func will return(1) and schedule context switch. 
}


void ExecThread::WaitCondition::StopWaiting()
{
	if(wcstate!=WC_WaitPending && wcstate!=WC_Waiting)
	{  assert(0);  }
	
	// Must remove us from the ExprValue notifier chains. 
	ValueNotifyHandler::UninstallAll();
	
	// Be sure...
	must_eval=0;
	
	fprintf(stderr,"VM: Thread[%d] stops waiting\n",
		et->thread_id,poll_val);
	
	wcstate=WC_NotInUse;
	// Calling eval func will return(1) and schedule context switch. 
}


ExecThread::WaitCondition::WaitCondition(ExecThread *_et)
{
	et=_et;
	wcstate=WC_NotInUse;
	vn_stack_depth=-1;
	must_eval=0;
}

ExecThread::WaitCondition::WaitCondition(_IClone,const WaitCondition *src,
	ExecThread *_et)
{
	// Var notifer cannot be cloned. And there is no need because 
	// you may never clone from within a wait() condition. 
	switch(src->wcstate)
	{
		case WC_NotInUse:  break;  // Okay. 
		case WC_RegTempDetached:
			// OOPS: Probably cloning from within wait() condition. 
			// This should have been caught by the 
			// AniInternalFunc_Wait::EvalFunc_Wait::Eval(). 
			assert(0);
			abort();
			break;
		case WC_RegAttached:
			// OOPS: AniInternalFunc_Wait::EvalFunc_Wait::Eval() 
			// did not detach (at least temporarily) while doing a 
			// context switch. 
			assert(0);
			break;
		case WC_WaitPending:
			// This happens when cloning during eval of further 
			// args to wait(). May not happen either and checked by 
			// AniInternalFunc_Wait::EvalFunc_Wait::Eval() 
			assert(0);
		case WC_Waiting:
			// Hmmmm... cloning from within wait cond while already 
			// waiting??? 
			assert(0);
			abort();
			break;
		default: assert(0);
	}
	
	et=_et;
	wcstate=WC_NotInUse;
	vn_stack_depth=-1;
	must_eval=0;
}

ExecThread::WaitCondition::~WaitCondition()
{
	assert(wcstate!=WC_RegAttached);
	// Must be able to stop waiting at any time other 
	// than WC_RegAttached. 
	if(wcstate==WC_WaitPending || wcstate==WC_Waiting)
	{
		StopWaiting();
	}
	else
	{
		ValueNotifyHandler::UninstallAll();
	}
}

//------------------------------------------------------------------------------

int ExecThread::TimeDelay::StartDelay(double _delta_time)
{
	assert(!delayed);
	
	#warning "Should allow for object time, too..."
	delta_time=_delta_time;
	// FIXME: ccif[0]: 
	delay_end_time=et->context->ccif[0]->CurrentSettingTime()+delta_time;
	fprintf(stderr,"VM: Thread[%d] starts delay (dt=%g, end=%g)\n",
		et->thread_id,delta_time,delay_end_time);
	
	delayed=1;
	assert(et->schedule_status==SSReady); // <-- Caller should have reported warning/error. 
	et->schedule_status=SSDelay;
	return(0);
}


int ExecThread::TimeDelay::TestEndDelay()
{
	assert(et->state==ExecThread::Delayed);
	assert(delayed);
	
	// See if the delay is over: 
	#warning "Should allow for object time, too..."
	// FIXME: ccif[0]: 
	if(et->context->ccif[0]->CurrentSettingTime()>=delay_end_time)
	{
		// Stop delay. 
		delayed=0;
	}
	assert(et->schedule_status==SSReady);
	
	return(delayed ? 0 : 1);
}


ExecThread::TimeDelay::TimeDelay(ExecThread *_et)
{
	et=_et;
	delayed=0;
}

ExecThread::TimeDelay::TimeDelay(_IClone,const TimeDelay *src,
	ExecThread *_et)
{
	// We may not clone delayed threads. 
	// There should be no way that may happen because clone() can only 
	// be called from a running state (and not delayed state). 
	if(src->delayed)
	{  assert(0);  }
	
	et=_et;
	delayed=0;
}

ExecThread::TimeDelay::~TimeDelay()
{
	et=NULL;
}

//------------------------------------------------------------------------------

int ExecThread::TestThreadExist(int _thread_id)
{
	return(context ? (context->LookupThreadByID(_thread_id) ? 1 : 0) : -1);
}


ExecThread *ExecThread::Clone(int new_thread_id)
{
	assert(this->schedule_status==SSRequest);
	ExecThread *et=new ExecThread(_Clone,this,new_thread_id);
	this->schedule_status=new_thread_id;
	return(et);
}


ExecThread::ExecThread(_IClone,const ExecThread *src,int new_thread_id) : 
	LinkedListBase<ExecThread>(),
	ANI::ExecThreadInfo(_Clone,src),
	context(src->context),  // <-- just a pointer
	wait_cond(_Clone,&src->wait_cond,this),
	tdelay(_Clone,&src->tdelay,this)
{
	state=Inactive;  // OK. 
	schedule_status=SSDoneClone;
	thread_id=new_thread_id;
	assert(thread_id>0);
	fprintf(stderr,"VM: Cloned thread[%d] -> new thread[%d]\n",
		src->thread_id,thread_id);
}

ExecThread::ExecThread(Animation_Context *_context,int _thread_id) : 
	LinkedListBase<ExecThread>(),
	ANI::ExecThreadInfo(),
	context(_context),  // <-- just a pointer
	wait_cond(this),
	tdelay(this)
{
	state=Inactive;
	schedule_status=SSDisabled;
	thread_id=_thread_id;
	assert(thread_id>=0);  // =0 special for non-threaded environment
	fprintf(stderr,"VM: New thread[%d]\n",thread_id);
}

ExecThread::~ExecThread()
{
	fprintf(stderr,"VM: Deleting thread[%d]\n",thread_id);
	context=NULL;
}

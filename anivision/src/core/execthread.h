/*
 * core/execthread.h
 * 
 * Header for class ExecThread, the main thread representation. 
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

#ifndef _ANIVISION_ANI_EXECTHREAD_H_
#define _ANIVISION_ANI_EXECTHREAD_H_ 1

#include <ani-parser/threadinfo.h>


/*------------------------------------------------------------------------------
 * 
 * NOTE: Stack layout: 
 * 
 * Strack frame for AnonScope:
 *   var0,var1,var1,...  (0 being declared first)
 * 
 * Strack frame for AniFunction: 
 *   <retval_if_not_void> first_arg ... last_arg
 * NOTE: The "this" pointer is not passed on the _value_ stack but 
 *       separately on the stack.  
 * 
 *----------------------------------------------------------------------------*/


class AniInternalFunc_Wait;

// This is AniAnimation::Context. 
class Animation_Context;


// List hold by Animation_Context. 
struct ExecThread : 
	public ANI::ExecThreadInfo,
	LinkedListBase<ExecThread>
{
	public:
		// Set if any wait condition needs re-eval 
		// at the next context switch. 
		// INSTEAD of using this global var, we could also use 
		// a list of all ExecThreads in waiting state which need that. 
		static int wait_cond_re_eval_cs;
	private:
		// Associated context: 
		// NOTE: This is NULL if we're not in animation context. 
		Animation_Context *context;
		
	public:
		enum State
		{
			Inactive=1,  // not running; suspended other thread running
			Running,     // thread currently being executed
			Waiting,     // thread wait()ing
			Delayed,     // thread in delay()
		};
		// The thread state, i.e. running, waiting,... see above. 
		State state;
		
		// This is the thread_id of the thread; it's like the 
		// PID in UNIX environments to identify the thread. 
		// NOTE: thead_id must be >0 in multithreading environment. 
		//       thread_id=0 is reserved for the main animation block 
		//       thread which never clones. 
		int thread_id;
		
		enum 
		{
			// NOTE: Constants all <0. 
			SSDisabled=0,    // cloning/yield/... disabled; may not do it
			SSReady=-1,      // ready to clone/yield/.. (allowed, nothing scheduled)
			SSRequest=-2,    // clone() called; clone request
			SSDoneClone=-3,  // done cloning; this is the cloned thread
			SSYield=-4,      // simply make sure we re-schedule
			SSWait=-5,       // enter waiting state (wait())
			SSDelay=-6,      // enter delayed state (delay())
			SSKillMe=-7,     // kill([void]) called; kill thread
			/*anything>0*/   // done cloning; this is the original thread
		};                   // schedule_status contains the thread_id of the clone
		int schedule_status; // one of the SS* or "child"/clone thread_id. 
		
		// This is for wait() function (not pwait()). It is the current 
		// hook and there exists only one because the wait() call is not 
		// re-entrant (i.e. you may not call wait() from within a wait() 
		// condition). 
		struct WaitCondition : 
			// Value notifier to register vars in conditional expressions: 
			// (To be able to get change/delete notifications.) 
			ANI::ExecThreadInfo::VarNotifier,
			// Value change/delete notifier called when one the 
			// the ExprValues in the wait condition get changed/deleted. 
			ANI::ValueNotifyHandler
		{
			public:
				enum WCState
				{
					// Wait condition not in use and 
					// we're not currently registering the 
					// wait condition or its vars. 
					WC_NotInUse=0,
					// Currently attached VarNotifier to the 
					// thread so that we can register the vars. 
					// This is during the first evaluation of 
					// the wait condition expression. 
					WC_RegAttached,
					// Temporarily detached VarNotifier because 
					// of context switch; will go on with 
					// WC_RegAttached soon. 
					WC_RegTempDetached,
					// Called after the WC_RegAttached is done 
					// but before actually waiting. 
					// (I.e. during eval of further args to wait().)
					WC_WaitPending,
					// We're actually currently waiting. 
					WC_Waiting,
				};
			private:
				// Back pointer to our thread: 
				ExecThread *et;
				
				// Master state switch: 
				WCState wcstate;
				
				//-- State var for us as ANI::ExecThreadInfo::VarNotifier:
				// This is the stack depth we're currently at. 
				int vn_stack_depth;
				
				//-- State vars for the wait() call: 
				int poll_val;  // "poll=" as passed to wait()
				
				// [overriding virtual from ANI::ExecThreadInfo::VarNotifier]
				void notify(ANI::ExecThreadInfo *info,ANI::TNIdentifier *idf,
					ANI::ExprValue &ev);
				
				// [overriding virtuals from ANI::ValueNotifyHandler]
				void vn_change(void *hook);
				void vn_delete(void *hook);
				
			public:  _CPP_OPERATORS
				WaitCondition(ExecThread *_et);
				WaitCondition(_IClone,const WaitCondition *src,ExecThread *_et);
				// [overriding virtual]
				~WaitCondition();
				
				// This is the "must eval flag"; if set, the condition 
				// must be re-evaluated. 
				int must_eval;
				
				// Get state of the wait condition: 
				WCState GetWCState() const
					{  return(wcstate);  }
				
				// Wait condition helpers: 
				// Called by wait()'s eval function durig first eval 
				// of the wait condition so that we install a 
				// var notifier (which makes sure that the vars are 
				// getting registered for change notification): 
				void InstallVarNotifier(ANI::TreeNode *tn_for_error);
				// Detach the var notifier again after having attached 
				// it using the above function. 
				// Set temporarily=1 if this detach is just due to 
				// context switch during eval. 
				// Set temporarily=-1 if the next call will be 
				// StartWaiting() or StopWaiting(). 
				void DetachVarNotifier(int temporarily);
				// Called by the wait() eval handler to start waiting. 
				// Must have called DetachVarNotifier(-1) before. 
				// poll_val is the value as passed with the poll= arg 
				// at the wait() call. 
				// The calling eval func must return(1) after this func. 
				void StartWaiting(int poll_val);
				// Called by the wait() eval handler either without 
				// having called StartWaiting() yet (because it turns 
				// out that we do not have to wait) or because the 
				// condition came true and we stop waiting. 
				void StopWaiting();
		};
		// There is max. one wait condition per thread: 
		WaitCondition wait_cond;
		
		// This is for the delay() function: Pause a specified 
		// amount of time. 
		struct TimeDelay
		{
			// Back pointer to our thread: 
			ExecThread *et;
			
			// Delay active?
			int delayed;
			// Length of the delay (argument of delay() call): 
			double delta_time;
			// End time of delay: 
			double delay_end_time;
			
			_CPP_OPERATORS
			TimeDelay(ExecThread *_et);
			TimeDelay(_IClone,const TimeDelay *src,ExecThread *_et);
			~TimeDelay();
			
			// Called by delay() eval func to start the delay. 
			// It must then return(1) immediately. 
			int StartDelay(double delta_time);
			// Called by animation main loop to test for delay 
			// end and stop the delay if necessary. 
			// Return value: 
			//  0 -> continuing waiting
			//  1 -> delay stopped
			int TestEndDelay();
		};
		// There is max. one time delay per thread: 
		TimeDelay tdelay;
		
	public:  _CPP_OPERATORS
		// Creates new ExecThread in Inactive state with 
		// CloneState SSDisabled. 
		ExecThread(Animation_Context *_context,int _thread_id);
		// Creates cloned ExecThread; use Clone(). 
		ExecThread(_IClone,const ExecThread *src,int new_thread_id);
		~ExecThread();
		
		// Return animation context or NULL. 
		Animation_Context *Context() const
			{  return(context);  }
		
		// Clone the complete exec thread; must specify thread_id 
		// of (new) cloned thread. 
		// The new returned thread is a cloned version of 
		// this one with the state set to Inactive and 
		// CloneState SSDoneClone. 
		// Sets this->schedule_status to new_thread_id. 
		ExecThread *Clone(int new_thread_id);

		// Check if thread with passed thread_id (still) exists. 
		// Only works in animation context (i.e. context!=NULL). 
		// Returns 1 if yes, 0 otherwise (and -1 if context==NULL). 
		int TestThreadExist(int thread_id);
};

#endif  /* _ANIVISION_ANI_EXECTHREAD_H_ */

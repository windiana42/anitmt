/*
 * taskmanager.hpp
 * 
 * Task manager class header. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _RNDV_TASKMANAGER_HPP_
#define _RNDV_TASKMANAGER_HPP_ 1

// Well that is pretty much, but we won't need more: 
#include "database.hpp"
#include "tsource/tasksource.hpp"


class TaskManager : 
	public FDBase, 
	public ProcessBase,
	public TaskSourceConsumer
{
	private:
		ComponentDataBase *component_db;
		
		// 0msec timer for scheduling purposes: 
		TimerID tid0;
		
		// Task source connect re-try timer: 
		TimerID tid_ts_cwait;
		
		// Note: all existing TaskDrivers are in this queue: 
		LinkedList<TaskDriver> joblist;
		
		// This is the list of tasks (NOT jobs, do not mix up). 
		// tasklist_todo holds tasks in state ToBe*. 
		//     The ones which are currently processed have 
		//     TaskDriver pointer (td) !=NULL. 
		// tasklist_done holds tasks in in state TaskDone. 
		// Note that tasks have state TaskDone when retrieved 
		// by GetTask(). 
		// NOTE: Order is important. 
		// LAST task is processed first; new task are added at 
		// the front (inserted). 
		LinkedList<CompleteTask> tasklist_todo;
		LinkedList<CompleteTask> tasklist_done;
		
		CompleteTask *scheduled_for_start;
		
		// Number of simultanious jobs (limit and optimum): 
		int njobs;
		
		// Limit for simultanious render and filter jobs: 
		int maxjobs[_DTLast];
		
		// Number of running tasks: 
		int running_jobs[_DTLast];
		
		// Number of jobs that failed in sequence. Set to 0 for 
		// each successful job and incremented for failed ones. 
		int jobs_failed_in_sequence;
		// Max value for jobs_failed_in_sequence before govong up; 
		// 0 -> disable this feature (NOT recommended). 
		int max_failed_in_sequence;
		
		// Important flags whith self-explaining names: 
		int dont_start_more_tasks : 1;
		int caught_sigint : 3;
		int abort_on_signal : 1;  // abort on next SIGINT/SIGTERM
		int schedule_quit : 3;  // 1 -> exit(0); 2 -> exit(1)
		int schedule_quit_after : 3;  // 1 -> exit(0); 2 -> exit(1)
		int kill_task_and_quit_now : 3;  // 0,1,2
		
		// State flags (A): 
		int connected_to_tsource : 1;
		
		// Call DoneTask() for all done tasks before calling 
		// GetTask() the next time. 
		int ts_done_all_first : 1;
		
		// Padding bits: 
		int _pad : 16;
		
		// State flags (B): 
		TSAction pending_action;
		
		// tasklist_todo thresholds: 
		// If there are less than todo_thresh_low (>=1) tasks in 
		// the todo queue, the exchange with the task queue is 
		// started. No more than todo_thresh_high tasks will ever 
		// be in the task queue. 
		int todo_thresh_low;
		int todo_thresh_high;
		
		TSAction tsgod_next_action;  // used by _TS_GetOrDoneTask()
		int nth_call_to_get_task;  // increased for each call to TSGetTask. 
		void _TS_GetOrDoneTask();
		
		inline bool _ProcessedTask(const CompleteTask *ctsk);
		
		bool _TS_CanDo_DoneTask(CompleteTask **special_done=NULL);
		bool _TS_CanDo_GetTask(bool can_done);
		
		void _CheckStartTasks();
		int _CheckStartExchange();
		
		// Do things which are said to be `scheduled'... 
		// Called by timernotify(): 
		void _schedule(TimerInfo *);
		inline void ReSched()
			{  UpdateTimer(tid0,0,0);  }
		
		// These are called by _schedule(): 
		int _StartProcessing();   // actually starts things 
		int _LaunchJobForTask(CompleteTask *ctsk);
		void _HandleFailedJob(CompleteTask *ctsk);
		
		// Overriding virtual from TaskSourceConsumer: 
		int tsnotify(TSNotifyInfo *);
		// Overriding virtual from FDBase: 
		int signotify(const SigInfo *);
		int timernotify(TimerInfo *);
	public: _CPP_OPERATORS_FF
		TaskManager(ComponentDataBase *cdb,int *failflag=NULL);
		~TaskManager();
		
		// These are called by the constructor/destructor of TaskDriver: 
		// Return value: 0 -> OK; !=0 -> failed
		int RegisterTaskDriver(TaskDriver *td);
		void UnregisterTaskDriver(TaskDriver *td);
		
		// Called by TaskDriver when ever estat/esdetail changed. 
		void StateChanged(TaskDriver *td);
		// Called by TaskDriver when he is done and wants to get deleted: 
		void IAmDone(TaskDriver *td);
};

#endif  /* _RNDV_TASKMANAGER_HPP_ */

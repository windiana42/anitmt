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
#include "tdriver/driverif.hpp"


class TaskManager : 
	public FDBase, 
	public TaskSourceConsumer,
	public par::ParameterConsumer_Overloaded
{
	private:
		ComponentDataBase *component_db;
		
		TaskDriverInterface *interface;
		
		// 0msec timer for scheduling purposes: 
		TimerID tid0;
		
		// Task source connect re-try timer: 
		TimerID tid_ts_cwait;
		
		HTime starttime;  // same as ProcessManager::starttime. 
		
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
		
		int dev_null_fd;  // always open
		
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
		int told_interface_to_quit : 1;  // PleaseQuit() called on interface?
		int kill_tasks_and_quit_now : 1;  // 0,1
		int sched_kill_tasks : 3;  // 0,1,2
		
		// State flags (A): 
		int connected_to_tsource : 1;
		
		// Call DoneTask() for all done tasks before calling 
		// GetTask() the next time. 
		int ts_done_all_first : 1;
		
		// If this is 0, the load poll timer must be running; always 1 
		// if the feature is turned off. 
		int load_permits_starting : 1;
		
		// This is set if we receive a SIGTSTP and unset with SIGCONT: 
		int exec_stopped : 1;
		
		// Padding bits: 
		int : ((100*sizeof(int)*8 - 20)%(sizeof(int)*8));
		
		// Set by parameters, default to values set by program name: 
		RefString tsource_name;
		RefString tdinterface_name;
		RefString opmode_name;
		
		long load_poll_msec;   // or -1
		TimerID tid_load_poll;  // NULL if no load control enabled. 
		
		// Load values multiplied with 100; low<=0 -> disable
		int load_low_thresh,load_high_thresh;
		int load_control_stop_counter;
		// Max value measured during runtime: 
		int max_load_measured;
		
		// State flags (B): 
		TSAction pending_action;
		int last_pend_done_frame_no;  // frame number if pending_action==ADoneTask. 
		
		int lpf_hist_size;
		int lpf_hist_idx;
		int *last_proc_frames;
		
		// tasklist_todo thresholds: Now hidden in interface. 
		
		TSAction tsgod_next_action;  // used by _TS_GetOrDoneTask()
		int nth_call_to_get_task;  // increased for each call to TSGetTask. 
		void _TS_GetOrDoneTask();
		
		inline bool _ProcessedTask(const CompleteTask *ctsk);
		
		bool _TS_CanDo_DoneTask(CompleteTask **special_done=NULL);
		bool _TS_CanDo_GetTask(bool can_done);
		
		void _CheckStartTasks();
		inline void _DoScheduleForStart(CompleteTask *ctsk);
		inline void _KillScheduledForStart();
		int _CheckStartExchange();
		
		int _GetLoadValue();
		void _StartLoadPolling();
		void _DoCheckLoad();
		void _DisableLoadFeature();
		
		// Do things which are said to be `scheduled'... 
		// Called by timernotify(): 
		void _schedule(TimerInfo *);
		inline void ReSched()
			{  UpdateTimer(tid0,0,0);  }
		
		// These are called by _schedule(): 
		int _StartProcessing();   // actually starts things 
		
		// Do some things with new tasks (set up state & TaskParams): 
		int _DealWithNewTask(CompleteTask *ctsk);
		
		void _PrintTaskExecStatus(TaskExecutionStatus *tes);
		void _DoPrintTaskExecuted(TaskParams *tsb,TaskStructBase *tp,
			const char *binpath,bool was_processed);
		void _PrintDoneInfo(CompleteTask *ctsk);
		
		// Initialisation of parameter stuff: 
		int _SetUpParams();
		
		// Simply call fdmanager()->Quit(status) and write 
		void _DoQuit(int status);
		
		// Overriding virtual from TaskSourceConsumer: 
		int tsnotify(TSNotifyInfo *);
		// Overriding virtual from FDBase: 
		int signotify(const SigInfo *);
		int timernotify(TimerInfo *);
		// Overriding virtual from ParameterConsumer: 
		int CheckParams();
	public: _CPP_OPERATORS_FF
		TaskManager(ComponentDataBase *cdb,int *failflag=NULL);
		~TaskManager();
		
		static bool IsARenderedTask(const CompleteTask *ctsk);
		static bool IsAFilteredTask(const CompleteTask *ctsk);
		static bool IsPartlyRenderedTask(const CompleteTask *ctsk);
		static bool IsPartlyFilteredTask(const CompleteTask *ctsk);
		
		// Needed by NJobsChanged() and LDR task source: 
		int Get_njobs()
			{  return(interface->Get_njobs());  }
		// This is needed by the LDR task source: 
		const HTime *Get_starttime()
			{  return(&starttime);  }
		
		// ******** INTERFACE TO TaskDriverInterface ********
		
		// Called by interface; the LDR interface calls this when the 
		// first client was connected successfully, the local interface 
		// calls this immediately. 
		void ReallyStartProcessing(int error_occured=0);
		
		// Called by TaskDriverInterface becuase a TaskDriver unregisterd 
		// or because new LDR clients are connected or others disconnect. 
		// njobs_changed: 0,1 and special value -1
		void CheckStartNewJobs(int njobs_changed);
		
		void HandleSuccessfulJob(CompleteTask *ctsk);
		void HandleFailedTask(CompleteTask *ctsk,int running_jobs);
		
		// Get /dev/null fd. 
		int DevNullFD()
			{  return(dev_null_fd);  }
};

#endif  /* _RNDV_TASKMANAGER_HPP_ */

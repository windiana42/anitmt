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



struct TaskManager_TaskList
{
	// This is the list of tasks (NOT jobs, do not mix up). 
	// tasklist.todo holds tasks in state ToBe*. 
	//     The ones which are currently processed have 
	//     TaskDriver pointer (td) !=NULL and are in 
	//     tasklist.proc. 
	// tasklist.done holds tasks in in state TaskDone. 
	// Note that tasks have state TaskDone when retrieved 
	// by GetTask(). 
	// NOTE: Order is important. 
	// FIRST task is processed first; new task are added at 
	// the end (appended). 
	LinkedList<CompleteTask> todo;
	LinkedList<CompleteTask> proc;
	LinkedList<CompleteTask> done;
	// These contain the number of elements in the lists: 
	int todo_nelem;
	int proc_nelem;
	int done_nelem;

	_CPP_OPERATORS_FF
	TaskManager_TaskList(int *failflag=NULL);
	~TaskManager_TaskList();
};


class TaskManager : 
	public FDBase, 
	public TimeoutBase,
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
		
		// For execution time timeout: 
		RefString exec_timeout_spec;
		TimeoutID timeout_id;
		
		HTime starttime;  // same as ProcessManager::starttime. 
		// When the last work cycle started (first task from active task 
		// source or HTime::Invalid). [only active task source]
		ProcessManager::ProcTimeUsage ptu_self_start_work,ptu_chld_start_work;
		int work_cycle_count;   // only active task source
		
		// For more info, see above. 
		TaskManager_TaskList tasklist;
		
		CompleteTask *scheduled_for_start;
		
		int dev_null_fd;  // always open
		
		// Number of jobs that failed in sequence. Set to 0 for 
		// each successful job and incremented for failed ones. 
		int jobs_failed_in_sequence;
		// Max value for jobs_failed_in_sequence before govong up; 
		// 0 -> disable this feature (NOT recommended). 
		int max_failed_in_sequence;
		
		// Important flags whith self-explaining names: 
		int dont_start_more_tasks : 1;   // USE DontStartMoreTasks(). 
		int caught_sigint : 3;
		int abort_on_signal : 1;  // abort on next SIGINT/SIGTERM
		int schedule_quit : 3;  // 1 -> exit(0); 2 -> exit(1)
		int schedule_quit_after : 3;  // 1 -> exit(0); 2 -> exit(1)
		int recovering : 1;   // like the _quit* stuff but for re-start
		int told_interface_to_quit : 1;  // PleaseQuit() called on interface?
		int kill_tasks_and_quit_now : 1;  // 0,1
		int sched_kill_tasks : 3;  // 0,1,2
		
		int failed_in_sequence_reported : 1;
		
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
		int : ((100*sizeof(int)*8 - 22)%(sizeof(int)*8));
		
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
		
		// tasklist.todo/proc/done thresholds: Now hidden in interface. 
		
		TSAction tsgod_next_action;  // used by _TS_GetOrDoneTask()
		int nth_call_to_get_task;  // increased for each call to TSGetTask. 
		void _TS_GetOrDoneTask();
		
		// Use this to set dont_start_more_tasks to 1 (and under some 
		// circumstances also when it is already 1): 
		void DontStartMoreTasks();
		
		// Dump state for debugging reason. 
		void _DumpInternalState();
		
		static inline bool _ProcessedTask(const CompleteTask *ctsk);
		
		bool _TS_CanDo_DoneTask(CompleteTask **special_done=NULL);
		bool _TS_CanDo_GetTask(bool can_done);
		
		void _CheckStartTasks();
		inline void _DoScheduleForStart(CompleteTask *ctsk);
		inline void _KillScheduledForStart(int was_launched=0);
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
		
		void _TakeFreshTask(CompleteTask *ctsk,int from_take_task);
		// Do some things with new tasks (set up state & TaskParams): 
		int _DealWithNewTask(CompleteTask *ctsk);
		
		void _PrintTaskExecStatus(CompleteTask::TES *tes,const char *rnd_flt);
		void _DoPrintTaskExecuted(TaskParams *tsb,TaskStructBase *tp,
			const char *binpath,bool was_processed);
		void _PrintDoneInfo(CompleteTask *ctsk);
		
		void _EmitCPUStats(const char *title,HTime *elapsed,
			ProcessManager::ProcTimeUsage *ptu_self,
			ProcessManager::ProcTimeUsage *ptu_chld);
	
		// Initialisation of parameter stuff: 
		int _SetUpParams();
		
		// Simply call fdmanager()->Quit(status) and write info. 
		void _DoQuit(int status);
		
		// This is the code to actually quit. It is three steps: 
		// Instead of calling fdmanager()->Quit(status), use this function. 
		// (Read on in taskmanager.cpp.)
		int _actually_quit_status;
		int _actually_quit_step;   // 0 -> don't quit; 
		void _ActuallyQuit(int status);
		void _DestructCleanup(int real_destructor=0);
		
		void _ActOnSignal(int signo,int real_signal);
		
		// Overriding virtual from TaskSourceConsumer: 
		int tsGetDebugInfo(TSDebugInfo *dest);
		
		// Overriding virtual from TaskSourceConsumer: 
		int tsnotify(TSNotifyInfo *);
		// Overriding virtual from FDBase: 
		int signotify(const SigInfo *);
		int timernotify(TimerInfo *);
		// Overriding virtual from TimeoutBase: 
		int timeoutnotify(TimeoutInfo *);
		// Overriding virtual from ParameterConsumer: 
		int CheckParams();
	public: _CPP_OPERATORS_FF
		TaskManager(ComponentDataBase *cdb,int *failflag=NULL);
		~TaskManager();
		
		static bool IsARenderedTask(const CompleteTask *ctsk);
		static bool IsAFilteredTask(const CompleteTask *ctsk);
		static bool IsPartlyRenderedTask(const CompleteTask *ctsk);
		static bool IsPartlyFilteredTask(const CompleteTask *ctsk);
		
		// Returns string "completely", "partly" or "not" depending on 
		// how the task was processed. If non-NULL, level returns the same 
		// info (2 -> completely, 1 -> partly, 0 -> not processed). 
		static char *Completely_Partly_Not_Processed(const CompleteTask *ctsk,
			int *level=NULL);
		
		// Needed by NJobsChanged() and LDR task source: 
		int Get_njobs()
			{  return(interface->Get_njobs());  }
		// This is needed by the LDR task source: 
		const HTime *Get_starttime()
			{  return(&starttime);  }
		int Get_todo_thresh_high()
			{  return(interface ? interface->Get_todo_thresh_high() : -1);  }
		
		// Used by active task source to announce work cycle start. 
		void PrintWorkCycleStart();
		
		// ******** INTERFACE TO TaskDriverInterface ********
		
		// Called by interface; the LDR interface calls this when the 
		// first client was connected successfully, the local interface 
		// calls this immediately. 
		void ReallyStartProcessing(int error_occured=0);
		
		// Called by TaskDriverInterface becuase a TaskDriver unregisterd 
		// or because new LDR clients are connected or others disconnect. 
		// njobs_changed: 0,1 and special value -1
		void CheckStartNewJobs(int njobs_changed);
		
		// These only accept tasks in proc queue; ctsk->d ptr must 
		// already be NULL. (ctsk->d.any()=0) 
		void HandleSuccessfulJob(CompleteTask *ctsk);
		void HandleFailedTask(CompleteTask *ctsk,int running_jobs);
		
		// This is called by the LDR driver when the task was finally 
		// downloaded to the client. Will trigger launch of next task if 
		// needed. 
		void LaunchingTaskDone(CompleteTask *ctsk);
		
		// Tell TaskManager that the passed CompleteTask (which must 
		// currently be queued in tasklist.proc (not .done, of course) 
		// has to be done again. TaskManager may wish to put the task 
		// into the done queue [failed] if that happens too often or 
		// normally back into todo list. ctsk->d.any() must be NULL. 
		void PutBackTask(CompleteTask *ctsk);
		
		// Find task by TaskID. 
		// Returns NULL in case the task is unknown. 
		// Should check if the returned task is actually assigned to the 
		// right client / task driver. 
		// ONLY todo & proc LISTS ARE SEARCHED. 
		CompleteTask *FindTaskByTaskID(u_int32_t task_id);
		
		// Get /dev/null fd. 
		int DevNullFD()
			{  return(dev_null_fd);  }
		
		// For TaskDriverInterface only. Be careful with it. 
		const TaskManager_TaskList *GetTaskList()
			{  return(&tasklist);  }
};

inline const TaskManager_TaskList *TaskDriverInterface::GetTaskList()
	{  return(component_db()->taskmanager()->GetTaskList());  }

inline void TaskDriverInterface::PutBackTask(CompleteTask *ctsk)
	{  component_db()->taskmanager()->PutBackTask(ctsk);  }

inline CompleteTask *TaskDriverInterface::FindTaskByTaskID(u_int32_t task_id)
	{  return(component_db()->taskmanager()->FindTaskByTaskID(task_id));  }

#endif  /* _RNDV_TASKMANAGER_HPP_ */

/*
 * taskmanager.hpp
 * 
 * Task manager class header. 
 * 
 * Copyright (c) 2002--2004 by Wolfgang Wieser (wwieser@gmx.de) 
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
	public par::ParameterConsumer_Overloaded,
	public RendViewAdmin_CommandExecuter
{
	public:
		enum // when to dump tasks
		{
			DTSK_ReportBack=    0x001,    // when reported back to task source
			DTSK_ReportBackDone=0x002,    // when reporting back is done
			DTSK_Arrival=       0x004,    // when task request arrives via LDR
			DTSK_InitialQueue=  0x008,    // when reported to be done by task source
			DTSK_NowRendered=   0x010,    // when rendering is done
			DTSK_NowFiltered=   0x020,    // when filtering is done
			DTSK_ALLFLAGS=      0x03f
		};
		enum // for fit_strategy_flag
		{
			FSF_Run=0,     // normal operation
			FSF_Tight      // task source reported no more available tasks
		};
		
		// Accumulate timings: 
		struct AccumulatedTime
		{
			HTime utime;    // user time
			HTime stime;    // system time
			HTime elapsed;  // accumulated elapsed time
			int njobs;      // number of jobs in statistic
			
			// Reset everything to 0: 
			void Reset();
			
			_CPP_OPERATORS_FF
			AccumulatedTime(int *failflag=NULL);
			~AccumulatedTime() {}
		};
	private:
		ComponentDataBase *component_db;
		
		TaskDriverInterface *interface;
		
		// 0msec timer for scheduling purposes: 
		TimerID tid0;
		
		// Task source connect re-try timer: 
		TimerID tid_ts_cwait;
		
		// For execution time timeout: 
		TimeoutID exec_timeout_id;
		RefString exec_timeout_spec;
		RefString exec_timeout_sig_spec;
		int exec_timeout_action;   // 'i'nt, 't'erm, 'a'bort
		// For cycle execution timeout: 
		long cyc_idle_timeout;   // -1 -> disable
		TimeoutID cyc_timeout_id;
		
		HTime starttime;  // same as ProcessManager::starttime. 
		// When the last work cycle started (first task from active task 
		// source or HTime::Invalid). [only active task source]
		ProcessManager::ProcTimeUsage ptu_self_start_work,ptu_chld_start_work;
		int work_cycle_count;   // only active task source
		// When the last work cycle was finished: 
		// Initially, this is equal to starttime. 
		HTime last_work_cycle_end_time;
		
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
		int told_tasksource_to_quit : 1;  // TSDisconnect(/*do_quit=*/1) called?
		int kill_tasks_and_quit_now : 1;  // 0,1
		int sched_kill_tasks : 3;  // 0,1,2
		int told_want_no_more_tasks : 1;  // did we tell task source that we 
		                                  // do not want more tasks?
		
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
		enum ExecStopStatus exec_stop_status : 3;
		
		// Special flag: only set while starting; used by 
		// _KillScheduledForStart(). 
		int launching_special_flag : 1;
		
		// Fit strategy flag, see enum above (FSF_*): 
		int fit_strategy_flag : 3;
		
		// Padding bits: 
		int : ((100*sizeof(int)*8 - 30)%(sizeof(int)*8));
		
		// Set by parameters, default to values set by program name: 
		RefString opmode_name;
		// These are no longer supported (LDR forwarder). Gave up plans. 
		//RefString tsource_name;
		//RefString tdinterface_name;
		
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
		
		// Flags mask when to dump task info. 
		int dtsk_short_info;
		int dtsk_long_info;
		// Corresponding arg: 
		RefString dumptask_spec;
		
		// Daemonize (active task sources, only): 
		// 0 -> no; !=0 -> yes: 
		//  1 -> do not close, 2 -> close stdin, 3 -> close all
		int do_daemonize;
		RefString daemon_spec;
		ParamInfo *daemon_pi;
		
		// Statistics for accumulated used time (over all tasks): 
		AccumulatedTime acctime_render;
		AccumulatedTime acctime_filter;
		
		// tasklist.todo/proc/done thresholds: Now hidden in interface. 
		
		TSAction tsgod_next_action;  // used by _TS_GetOrDoneTask()
		int nth_call_to_get_task;  // increased for each call to TSGetTask. 
		void _TS_GetOrDoneTask();
		
		// Use this to set dont_start_more_tasks to 1 (and under some 
		// circumstances also when it is already 1): 
		void DontStartMoreTasks();
		// This is called to tell the active task source to deliver 
		// no more tasks to us. 
		void _NoMoreNewTasksFromTSource();
		
		// Dump state for debugging reason. 
		void _DumpInternalState();
		// Dump complete task list for debugging: 
		void _DumpCompleteTaskList();
		void _DoDumpTaskList(const char *name,int nelem,CompleteTask *first);
		
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
		
		// Enable/disable work cycle timeout. 
		void _ControlWorkCycleTimeout(int install);
		
		// Do things which are said to be `scheduled'... 
		// Called by timernotify(): 
		void _schedule(TimerInfo *);
		// NOTE: _resched_interval is 0 msec in normal operation; larger 
		//       values are for debugging only. 
		long _resched_interval;
		inline void ReSched(int /*_lineno*/)  // _lineno: line number for debugging
			{  UpdateTimer(tid0,_resched_interval,0);  }
		
		// These are called by _schedule(): 
		int _StartProcessing();   // actually starts things 
		
		void _TakeFreshTask(CompleteTask *ctsk,int from_take_task);
		// Do some things with new tasks (set up state & TaskParams): 
		int _DealWithNewTask(CompleteTask *ctsk);
		// Schedule giving back all (but may_keep) tasks from todo queue: 
		void _DoHandleGiveBackTasks(int may_keep);
		
		void _PrintDoneInfo(CompleteTask *ctsk);
		
		void _EmitCPUStats(const char *title,HTime *elapsed,
			ProcessManager::ProcTimeUsage *ptu_self,
			ProcessManager::ProcTimeUsage *ptu_chld);
		
		void _AccumulateTimeStat(TaskDriverType dtype,
			TaskExecutionStatus *tes);
		void _EmitAccumulatedTime(HTime &elapsed);
		
		// Initialisation of parameter stuff: 
		int _SetUpParams();
		
		// YOU MUST call ReSched() after this function. 
		// This is always called due to error and thus returns error. 
		void _QuitOrBeginRecovery();
		// Simply call fdmanager()->Quit(status) and write info. 
		// OR: Complete recovery and restart. 
		void _DoQuit(int status);
		
		// This is the code to actually quit. It is three steps: 
		// Instead of calling fdmanager()->Quit(status), use this function. 
		// (Read on in taskmanager.cpp.)
		int _actually_quit_status;
		int _actually_quit_step;   // 0 -> don't quit; 
		void _ActuallyQuit(int status);
		void _DestructCleanup(int real_destructor=0);
		
		void _HandleStopCont(int action);
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
		// Overriding virtual from RendViewAdmin_CommandExecuter: 
		int admin_command(ADMCmd *cmd,RVAPGrowBuffer *dest);
	public: _CPP_OPERATORS_FF
		TaskManager(ComponentDataBase *cdb,int *failflag=NULL);
		~TaskManager();
		
		// Needed by NJobsChanged() and LDR task source: 
		int Get_njobs()
			{  return(interface->Get_njobs());  }
		// READ COMMENT in driverif.hpp
		int Get_JobLimit(TaskDriverType dtype)
			{  return(interface->Get_JobLimit(dtype));  }

		// This is needed by the LDR task source: 
		const HTime *Get_starttime()
			{  return(&starttime);  }
		int Get_todo_thresh_high()
			{  return(interface ? interface->Get_todo_thresh_high() : -1);  }
		
		// idle_since: return end of last work cycle if non-NULL
		int Get_work_cycle_count(HTime *idle_since)
		{
			if(idle_since)  *idle_since=last_work_cycle_end_time;
			return(work_cycle_count);
		}
		
		// Return current exec stop status. Interesting for 
		// driver interface. 
		ExecStopStatus Get_ExecStopStatus()
			{  return(exec_stop_status);  }
		
		// Get the fit strategy flag (tells you if there are no more 
		// available tasks (FSF_Tight) or not): 
		int GetFitStrategyFlag() const
			{  return(fit_strategy_flag);  }
		
		// Used at various places to dump task info if needed: 
		// fmt_hdr is printed as header (short info).NO newline. 
		// May be NULL for default msg. 
		void DumpTaskInfo(CompleteTask *ctsk,const char *fmt_hdr,
			int dtsk_flag,int vlevel);
		int TestDumpTaskInfo(int dtsk_flag)
			{  return(dtsk_flag & (dtsk_short_info|dtsk_long_info));  }
		
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
		void HandleFailedTask(CompleteTask *ctsk,int running_jobs,
			TaskDriverType which_dtype_failed);
		
		// This is called by the LDR driver when the task was finally 
		// downloaded to the client. Will trigger launch of next task if 
		// needed. 
		void LaunchingTaskDone(CompleteTask *ctsk);
		
		// Tell TaskManager that the passed CompleteTask (which must 
		// currently be queued in tasklist.proc (not .done, of course) 
		// has to be done again. TaskManager may wish to put the task 
		// into the done queue [failed] if that happens too often or 
		// normally back into todo list. ctsk->d.any() must be NULL. 
		// Note: running_jobs just needed to be able to pass it to 
		//       HandleFailedTask() if needed. 
		void PutBackTask(CompleteTask *ctsk,int running_jobs);
		
		// Called by interface to notify task manager that the tasks 
		// (normally clients) now all stopped or are all running again. 
		// Use with care. 
		void ChangeExecStopStatus(ExecStopStatus new_ess);
		
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

inline void TaskDriverInterface::PutBackTask(CompleteTask *ctsk,int running_jobs)
	{  component_db()->taskmanager()->PutBackTask(ctsk,running_jobs);  }

inline CompleteTask *TaskDriverInterface::FindTaskByTaskID(u_int32_t task_id)
	{  return(component_db()->taskmanager()->FindTaskByTaskID(task_id));  }

#endif  /* _RNDV_TASKMANAGER_HPP_ */

/*
 * driverif.hpp
 * 
 * Task driver interface (virtualisation). 
 * 
 * Copyright (c) 2001--2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#ifndef _RNDV_TDRIVER_DRIVERINTERFACE_HPP_
#define _RNDV_TDRIVER_DRIVERINTERFACE_HPP_ 1

class ComponentDataBase;
class TaskDriver;
struct TaskManager_TaskList;


// NOTE: USED FOR ARRAY SUBSCRIPT. Must stay in range 0..._ESS_Last-1. 
enum ExecStopStatus
{
	ESS_Running=0,
	ESS_Stopping,
	ESS_Stopped,
	ESS_Continuing,
	_ESS_Last
};
// Returns string representation of the enum above. 
extern char *ExecStopStatus_String(ExecStopStatus ess);


class TaskDriverInterface
{
	private:
		ComponentDataBase *_comp_db;
	
	protected:
		// tasklist_todo thresholds: 
		// If there are less than todo_thresh_low (>=1) tasks in 
		// the todo queue, the exchange with the task queue is 
		// started. No more than todo_thresh_high tasks will ever 
		// be in the task queue. 
		// NOTE: We're talking about todo queue, NOT proc queue. 
		int todo_thresh_low;
		int todo_thresh_high;
		// This means that we start reporting done tasks as done 
		// when this many tasks are in the done queue: 
		int done_thresh_high;
		
		void NewTask_SetUpState(CompleteTask *ctsk);
		
		// Get the tasklist_todo from the TaskManager. Useful if the 
		// task driver interface has to search in it if a client dies. 
		inline const TaskManager_TaskList *GetTaskList();
		
		// See taskmanager.hpp / PutBackTask(). 
		void PutBackTask(CompleteTask *ctsk,int running_jobs);
		
		// Find task by TaskID. 
		// Returns NULL in case the task is unknown. 
		// Should check if the returned task is actually assigned to the 
		// right client. 
		CompleteTask *FindTaskByTaskID(u_int32_t task_id);
		
	public:  _CPP_OPERATORS_FF
		TaskDriverInterface(ComponentDataBase *cdb,int *failflag=NULL);
		virtual ~TaskDriverInterface();
		
		ComponentDataBase *component_db()
			{  return(_comp_db);  }

		/************* INTERFACE TO TaskManager *************/
		
		// Get task queue thresholds: 
		int Get_todo_thresh_low()   {  return(todo_thresh_low);   }
		int Get_todo_thresh_high()  {  return(todo_thresh_high);  }
		int Get_done_thresh_high()  {  return(done_thresh_high);  }
		
		// Needed by LDR task source and TaskManager: 
		virtual int Get_njobs() HL_PureVirt(-1)
		virtual int Get_nrunning() HL_PureVirt(-1)  // runnning means "assigned" for LDR
		// This is only for the local driver, others will not know and 
		// return -1. This is one reason why LDR source can only work 
		// with local driver, hence there is no LDR forwarder. 
		virtual int Get_JobLimit(TaskDriverType) HL_PureVirt(-2)
		
		// Write processing info; called from StartProcessing(): 
		// when: 0 -> start; 1 -> end
		// const char: "beginning to" or "waiting for". 
		// Use: "Okay, %s work: blah blah"
		virtual void WriteProcessingInfo(int /*when*/,const char *) HL_PureVirt(;)
		
		// Called by TaskMagaer; must call TaskManager::ReallyStartProcessing() 
		// when the first tasks can be executed (local interface: immediately, 
		// LDR interface: first client connected). 
		virtual void ReallyStartProcessing() HL_PureVirt(;)
		
		// Isn't that self-explaining? 
		virtual int AreThereJobsRunning() HL_PureVirt(0)
		
		// Called for every new task obtained from TaskSource: 
		virtual int DealWithNewTask(CompleteTask *) HL_PureVirt(1)
		
		// Decide on task to start and return it. 
		// Return NULL if there is no task to start. 
		virtual CompleteTask *GetTaskToStart(
			TaskManager_TaskList * /*tasklist*/,int /*schedule_quit*/) HL_PureVirt(NULL)
		
		// Actually launch a job for a task. No check if we may do that. 
		// Return value: 0 -> OK; -1,-2 -> failed
		// +1 -> FAILED to start launch but NO HandleFailedJob() called; 
		//       MUST BE HANDELED & try again. 
		// ctsk->d.any() MUST BE 0 unless retval is 0. 
		virtual int LaunchTask(CompleteTask *) HL_PureVirt(1)
		
		// signo=STGTSTP or SIGCONT: stop or cont all jobs. 
		// Return value: 0 -> okay, stopped
		//               1 -> in progress (LDR)
		// In the latter case, the interface must use TaskManager's 
		// ChangeExecStopStatus() to inform the task manager when done. 
		virtual int StopContTasks(int /*signo*/) HL_PureVirt(-1)
		
		// Schedule SIGTERM & SIGKILL on all jobs. 
		// reason: TTR_JK_UserInterrupt, TTR_JK_ServerError
		// Returns number of killed jobs. 
		virtual int TermAllJobs(TaskTerminationReason /*reason*/) HL_PureVirt(0)
		
		// Called when everything is done to disconnect from the clients. 
		// Local interface can handle that quickly. Also called for recovery! 
		virtual void PleaseQuit() HL_PureVirt(;)
		// Called when recovery is done to restart. 
		virtual void RecoveryDone() HL_PureVirt(;)
		
		// Tell LDR client to give back tasks in todo queue (all 
		// but may_keep many). No meaning for local interface. 
		// Special values for may_keep: 
		//    0x7fffffff -> give back all not processed
		//    -1 -> may keep njobs (parallel jobs, as reported by client) 
		virtual void ScheduleGiveBackTasks(int /*may_keep*/) HL_PureVirt(;)
};

#endif  /* _RNDV_TDRIVER_DRIVERINTERFACE_HPP_ */

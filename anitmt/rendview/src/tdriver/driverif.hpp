/*
 * driverif.hpp
 * 
 * Task driver interface (virtualisation). 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
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
		void PutBackTask(CompleteTask *ctsk);
		
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
		
		// signo=STGTSTP/SIGCONT: stop/cont all jobs. 
		virtual void StopContTasks(int /*signo*/) HL_PureVirt(;)
		
		// Schedule SIGTERM & SIGKILL on all jobs. 
		// reason: JK_UserInterrupt, JK_ServerError
		// Returns number of killed jobs. 
		virtual int TermAllJobs(int /*reason*/) HL_PureVirt(0)
		
		// Called when everything is done to disconnect from the clients. 
		// Local interface can handle that quickly. Also called for recovery! 
		virtual void PleaseQuit() HL_PureVirt(;)
		// Called when recovery is done to restart. 
		virtual void RecoveryDone() HL_PureVirt(;)
};

#endif  /* _RNDV_TDRIVER_DRIVERINTERFACE_HPP_ */

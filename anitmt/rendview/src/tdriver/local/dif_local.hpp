/*
 * dif_local.hpp
 * 
 * Task driver interface for task manager. 
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


#ifndef _RNDV_TDRIVER_DRIVERINTERFACE_LOCAL_HPP_
#define _RNDV_TDRIVER_DRIVERINTERFACE_LOCAL_HPP_ 1

#include "../driverif.hpp"

class TaskDriverInterfaceFactory_Local;


class TaskDriverInterface_Local : 
	public TaskDriverInterface,
	private ProcessBase
{
	private:
		TaskDriverInterfaceFactory_Local *p;
		
		// Note: all existing TaskDrivers are in this queue: 
		LinkedList<TaskDriver> joblist;
		
		// Number of running jobs: 
		int running_jobs[_DTLast];
		
		
		// May also be called with td=NULL in case DealWithNewTask() fails. 
		void _HandleFailedJob(CompleteTask *ctsk,TaskDriver *td);
		
		int _DoLaunchTask(CompleteTask *ctsk,TaskDriver **td);
		int _DoDealWithNewTask(CompleteTask *ctsk);
		
		// Get number of running jobs: 
		int RunningJobs()
		{   int nrunning=0;
			for(int i=0; i<_DTLast; i++)  nrunning+=running_jobs[i];
			return(nrunning);  }
		
		void _WriteStartProcInfo(const char *msg);
		void _WriteEndProcInfo();
	public:  _CPP_OPERATORS_FF
		TaskDriverInterface_Local(TaskDriverInterfaceFactory_Local *f,int *failflag=NULL);
		~TaskDriverInterface_Local();
		
		ComponentDataBase *component_db()
			{  return(TaskDriverInterface::component_db());  }
		
		// Needed by LDR task source: 
		int Get_njobs();
		int Get_nrunning();
		
		/************* INTERFACE TO TaskManager *************/
		// ALL OVERRIDING VIRTUALS from TaskDriverInterface: 
		
		// Write processing info; called from StartProcessing(): 
		// when: 0 -> start; 1 -> end
		void WriteProcessingInfo(int when,const char *msg);
		
		// See TaskDriverInterface for more info on that: 
		void ReallyStartProcessing();
		
		// Isn't that self-explaining? 
		int AreThereJobsRunning()
			{  return(!joblist.is_empty());  }
		
		// Called for every new task obtained from TaskSource: 
		int DealWithNewTask(CompleteTask *ctsk);
		
		// Decide on task to start and return it. 
		// Return NULL if there is no task to start. 
		CompleteTask *GetTaskToStart(
			LinkedList<CompleteTask> *tasklist_todo,int schedule_quit);
		
		// Actually launch a job for a task. No check if we may do that. 
		// Return value: 0 -> OK; -1,-2 -> failed
		int LaunchTask(CompleteTask *ctsk);
		
		// signo=STGTSTP/SIGCONT: stop/cont all jobs. 
		void StopContTasks(int signo);
		
		// Schedule SIGTERM & SIGKILL on all jobs. 
		// reason: JK_UserInterrupt, JK_ServerError
		// Returns number of killed jobs. 
		int TermAllJobs(int reason);
		
		// Called when everything is done to disconnect from the clients. 
		// Local interface can handle that quickly. 
		void PleaseQuit();
		// Called when recovery is done to restart. 
		void RecoveryDone();
		
		/************* INTERFACE TO TaskDriver *************/
		
		// These are called by the constructor/destructor of TaskDriver: 
		// Return value: 0 -> OK; !=0 -> failed
		int RegisterTaskDriver(TaskDriver *td);
		void UnregisterTaskDriver(TaskDriver *td);
		
		// Called by TaskDriver when ever estat/esdetail changed. 
		void StateChanged(TaskDriver *td);
		// Called by TaskDriver when he is done and wants to get deleted: 
		void IAmDone(TaskDriver *td);
};

#endif  /* _RNDV_TDRIVER_DRIVERINTERFACE_LOCAL_HPP_ */

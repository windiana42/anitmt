/*
 * dif_ldr.hpp
 * 
 * Local distributed rendering (LDR) server task driver interface 
 * for task manager. 
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


#ifndef _RNDV_TDRIVER_DRIVERINTERFACE_LDR_HPP_
#define _RNDV_TDRIVER_DRIVERINTERFACE_LDR_HPP_ 1

#include "../driverif.hpp"

#include <hlib/cpmanager.h>
#include <hlib/cpbase.h>

class TaskDriverInterfaceFactory_LDR;


class TaskDriverInterface_LDR : 
	public TaskDriverInterface,
	private FDBase,
	private FDCopyBase
{
	private:
		TaskDriverInterfaceFactory_LDR *p;
		
		// Note: all existing LDRClient classes are in this queue: 
		LinkedList<LDRClient> clientlist;
		
		// Current (max) number of jobs: 
		int njobs;
		
		// Current number of clients which can be used: 
		int nclients;
		
		// client connect timeout timer: 
		TimerID tid_connedt_to;
		
		void _WriteStartProcInfo(const char *msg);
		void _WriteEndProcInfo();
	public:  _CPP_OPERATORS_FF
		TaskDriverInterface_LDR(TaskDriverInterfaceFactory_LDR *f,int *failflag=NULL);
		~TaskDriverInterface_LDR();
		
		ComponentDataBase *component_db()
			{  return(TaskDriverInterface::component_db());  }
		
		/************* INTERFACE TO TaskManager *************/
		// ALL OVERRIDING VIRTUALS from TaskDriverInterface: 
		
		// Write processing info; called from StartProcessing(): 
		// when: 0 -> start; 1 -> end
		void WriteProcessingInfo(int when,const char *msg);
		
		// See TaskDriverInterface for more info on that: 
		void ReallyStartProcessing();
		
		// Isn't that self-explaining? 
		int AreThereJobsRunning();
		
		// Called for every new task obtained from TaskSource: 
		int DealWithNewTask(CompleteTask *ctsk);
		
		// Decide on task to start and return it. 
		// Return NULL if there is no task to start. 
		CompleteTask *GetTaskToStart(
			LinkedList<CompleteTask> *tasklist_todo,int schedule_quit);
		
		// Actually launch a job for a task. No check if we may do that. 
		// Return value: 0 -> OK; 1,2 -> failed
		int LaunchTask(CompleteTask *ctsk);
		
		// signo=STGTSTP/SIGCONT: stop/cont all jobs. 
		void StopContTasks(int signo);
		
		// Schedule SIGTERM & SIGKILL on all jobs. 
		// reason: JK_UserInterrupt, JK_ServerError
		// Returns number of killed jobs. 
		int TermAllJobs(int reason);
		
		/************* INTERFACE TO LDRClient *************/
		
		// These are called by the constructor/destructor of LDRClient: 
		// Return value: 0 -> OK; !=0 -> failed
		int RegisterLDRClient(LDRClient *client);
		void UnregisterLDRClient(LDRClient *client);
		
		// Called by LDRClient when he is done and wants to get deleted: 
		void IAmDone(LDRClient *client);
};

#endif  /* _RNDV_TDRIVER_DRIVERINTERFACE_LDR_HPP_ */
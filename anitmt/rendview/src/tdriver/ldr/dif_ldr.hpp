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

#include <lib/netiobase_ldr.hpp>


class TaskDriverInterfaceFactory_LDR;


class TaskDriverInterface_LDR : 
	public TaskDriverInterface,
	private FDBase,
	private TimeoutBase
{
	private:
		TaskDriverInterfaceFactory_LDR *p;
		
		// Note: All existing LDRClient classes are in this queue: 
		LinkedList<LDRClient> clientlist;
		// Client which will get task reported by GetTaskToStart() 
		// when launching task. 
		LDRClient *free_client;
		
		// Current (max) number of jobs: 
		int njobs;
		
		// Current number of clients which can be used: 
		int nclients;
		
		// Counts the assigned_jobs of all clients: 
		int RunningJobs();
		
		void _JobsAddClient(LDRClient *client,int mode);
		
		// Client connect timeout: 
		TimeoutID tid_connedt_to;
		
		// Reconnect trigger timer: 
		TimerID reconnect_trigger_tid;
		
		void _StartReconnectTrigger();
		void _StopReconnectTrigger();
		
		int already_started_processing : 1;  // ReallyStartProcessing() called?
		int shall_quit : 1;   // Was PleaseQuit() called?
		
		int reconnect_trigger_running : 1;
		int dont_reconnect : 1;
		
		int : (sizeof(int)*8 - 4);   // <-- Use modulo if more than 16 bits. 
		
		// ARGUMENT MUST BE OF TYPE TaskDriverInterfaceFactory_LDR::ClientParam *
		LDRClient *NEW_LDRClient(void *p);
		
		LDRClient *_FindFreeClient();
		
		void _WriteStartProcInfo(const char *msg);
		void _WriteProcInfoUpdate();
		void _WriteEndProcInfo();
		
		void _PrintThreshAndQueueInfo(int with_thresh);
		
		void _PrintInitConnectMsg(const char *msg);
		
		void _HandleFailedLaunch(CompleteTask *ctsk,int resp_code);
		void _HandleTaskTermination(CompleteTask *ctsk);
		
		// FDBase virtuals: 
		int timernotify(TimerInfo *ti);
		// TimeoutBase virtual:
		int timeoutnotify(TimeoutInfo *ti);
	public:  _CPP_OPERATORS_FF
		TaskDriverInterface_LDR(TaskDriverInterfaceFactory_LDR *f,int *failflag=NULL);
		~TaskDriverInterface_LDR();
		
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
		int AreThereJobsRunning();
		
		// Called for every new task obtained from TaskSource: 
		int DealWithNewTask(CompleteTask *ctsk);
		
		// Decide on task to start and return it. 
		// Return NULL if there is no task to start. 
		CompleteTask *GetTaskToStart(
			TaskManager_TaskList *tasklist_todo,int schedule_quit);
		
		// Actually launch a job for a task. No check if we may do that. 
		// Return value: 0 -> OK; -1 [-2] -> failed
		int LaunchTask(CompleteTask *ctsk);
		
		// signo=STGTSTP/SIGCONT: stop/cont all jobs. 
		void StopContTasks(int signo);
		
		// Schedule SIGTERM & SIGKILL on all jobs. 
		// reason: JK_UserInterrupt, JK_ServerError
		// Returns number of killed jobs. 
		int TermAllJobs(int reason);
		
		// Called when everything is done to disconnect from the clients. 
		// Local interface can handle that quickly. Also called for recovery!
		void PleaseQuit();
		// Called when recovery is done to restart. 
		void RecoveryDone();
		
		/************* INTERFACE TO LDRClient *************/
		
		// These are called by the constructor/destructor of LDRClient: 
		// Return value: 0 -> OK; !=0 -> failed
		int RegisterLDRClient(LDRClient *client);
		void UnregisterLDRClient(LDRClient *client);
		
		// Called if connect(2) or authentification failed. 
		// The client gets removed now. 
		void FailedToConnect(LDRClient *client);
		
		// Called if we are now connected AND authenticated and 
		// can do tasks. 
		void SuccessfullyConnected(LDRClient *client);
		
		// See dif_ldr.cpp. 
		void TaskLaunchResult(CompleteTask *ctsk,int resp_code);
		void TaskTerminationNotify(CompleteTask *ctsk);
		
		// Called when a client disconnected. 
		void ClientDisconnected(LDRClient *client);
		
		// Called by LDRClient when he is done and wants to get deleted: 
		void IAmDone(LDRClient *client);
		
		// See TaskDriverInterface for this. 
		CompleteTask *FindTaskByTaskID(u_int32_t task_id)
			{  return(TaskDriverInterface::FindTaskByTaskID(task_id));  }
		
};

#endif  /* _RNDV_TDRIVER_DRIVERINTERFACE_LDR_HPP_ */

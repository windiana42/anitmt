/*
 * dif_ldr.hpp
 * 
 * Local distributed rendering (LDR) server task driver interface 
 * for task manager. 
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


#ifndef _RNDV_TDRIVER_DRIVERINTERFACE_LDR_HPP_
#define _RNDV_TDRIVER_DRIVERINTERFACE_LDR_HPP_ 1

#include "../driverif.hpp"

#include <lib/netiobase_ldr.hpp>


class TaskDriverInterfaceFactory_LDR;


class TaskDriverInterface_LDR : 
	public TaskDriverInterface,
	private FDBase,
	private TimeoutBase,
	public RendViewAdmin_CommandExecuter
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
		
		// Reconnect trigger timer: 
		TimerID reconnect_trigger_tid;
		// Timer for keepalive (ping control command): 
		TimerID keepalive_timer_tid;  // see also int keepalive_running : 1;
		
		void _StartReconnectTrigger();
		void _StopReconnectTrigger();
		
		void _SwitchKeepaliveTimer(int on_off);
		
		int already_started_processing : 1;  // ReallyStartProcessing() called?
		int shall_quit : 1;   // Was PleaseQuit() called?
		
		int reconnect_trigger_running : 1;
		int dont_reconnect : 1;
		
		int keepalive_running : 1;
		
		int : (sizeof(int)*8 - 5);   // <-- Use modulo if more than 16 bits. 
		
		// Keep some network statistics: 
		HTime net_starttime;   // needed for bandwidth calculation
		NetworkIOBase::NetIOStatistics downstream_stat,upstream_stat;
		
		// ARGUMENT MUST BE OF TYPE TaskDriverInterfaceFactory_LDR::ClientParam *
		LDRClient *NEW_LDRClient(void *p);
		
		void _WriteStartProcInfo(const char *msg);
		void _WriteProcInfoUpdate();
		void _WriteEndProcInfo();
		
		void _PrintThreshAndQueueInfo(int with_thresh);
		
		void _PrintInitConnectMsg(const char *msg);
		
		void _HandleFailedLaunch(CompleteTask *ctsk,int resp_code);
		void _HandleTaskTermination(CompleteTask *ctsk);
		
		inline int _GetFitStrategyFlag();  // -> dif_ldr.cpp
		
		int _TestClientCanDoTask(const CompleteTask *ctsk,LDRClient *client,
			bool *shall_render=NULL,bool *shall_filter=NULL);
		CompleteTask *_FindTaskForClient(const TaskManager_TaskList *tasklist,
			LDRClient *client,bool no_new_ones);
		LDRClient *_FindClientForTask(const CompleteTask *ctsk);
		
		// Stop/cont stuff: 
		// See if all clients are now stopped/running: 
		void _CheckExecStopStatusChange();
		// Tiny helper queueing stop/cont command: 
		int _DoQueueStopContForClient(LDRClient *client,
			LDR::LDRClientControlCommand l_cccmd);
		
		void _QuittingNowDone();
		
		// Simply calls PutBackTask()...
		// The RunningJobs() are only needed for emergency error handling. 
		void PutBackTask(CompleteTask *ctsk)
			{  TaskDriverInterface::PutBackTask(ctsk,RunningJobs());  }
		
		// FDBase virtuals: 
		int timernotify(TimerInfo *ti);
		// TimeoutBase virtual:
		int timeoutnotify(TimeoutInfo *ti);
		// RendViewAdmin_CommandExecuter virtual:
		int admin_command(ADMCmd *cmd,RVAPGrowBuffer *dest);
		
	public:  _CPP_OPERATORS_FF
		TaskDriverInterface_LDR(TaskDriverInterfaceFactory_LDR *f,
			int *failflag=NULL);
		~TaskDriverInterface_LDR();
		
		ComponentDataBase *component_db()
			{  return(TaskDriverInterface::component_db());  }
		
		const TaskDriverInterfaceFactory_LDR *P()  {  return(p);  }
		
		// Needed by LDR task source: 
		int Get_njobs();
		int Get_nrunning();
		int Get_JobLimit(TaskDriverType);
		
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
		
		// signo=STGTSTP or SIGCONT: stop or cont all jobs. 
		// Return value: 0 -> okay, stopped
		//               1 -> in progress (LDR)
		int StopContTasks(int signo);
		
		// Schedule SIGTERM & SIGKILL on all jobs. 
		// reason: TTR_JK_UserInterrupt, TTR_JK_ServerError
		// Returns number of killed jobs. 
		int TermAllJobs(TaskTerminationReason reason);
		
		// Called when everything is done to disconnect from the clients. 
		// Also called for recovery!
		// Calls _QuittingNowDone() when done which will inform 
		// task manager. 
		void PleaseQuit();
		// Called when recovery is done to restart. 
		void RecoveryDone();
		
		// Tell LDR client to give back all (but may_keep many) 
		// tasks from todo list. 
		void ScheduleGiveBackTasks(int may_keep);
		
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
		
		// Update response timeout (for control commands): 
		// NOTE: *ttime is NON-const; use HTime::Invalid to disable 
		//       the timeout. 
		void UpdateRespTimeout(LDRClient *client,HTime *ttime);
		
		// Called on received client control command: 
		// (Control command was executed by the client and we received 
		// confirmation.)
		void HandleControlCommandResponse(LDRClient *client,
			LDR::LDRClientControlCommand l_cccmd);
};

#endif  /* _RNDV_TDRIVER_DRIVERINTERFACE_LDR_HPP_ */

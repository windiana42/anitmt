/*
 * dif_ldr.cpp
 * 
 * Local distributed rendering (LDR) server task driver interface 
 * for task manager. 
 * 
 * Copyright (c) 2001--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "../../taskmanager.hpp"

#include "dif_ldr.hpp"
#include "dif_param.hpp"
#include "ldrclient.hpp"

#include <assert.h>

#include <lib/ldrproto.hpp>

#include <admin/rvapgrowbuffer.hpp>


using namespace LDR;

#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif


inline int TaskDriverInterface_LDR::_GetFitStrategyFlag()
{
	return(component_db()->taskmanager()->GetFitStrategyFlag());
}


// Not inline because virtual: 
int TaskDriverInterface_LDR::Get_njobs()
{
	return(njobs);
}
int TaskDriverInterface_LDR::Get_nrunning()
{
	return(RunningJobs());
}
int TaskDriverInterface_LDR::Get_JobLimit(TaskDriverType)
{
	return(-1);
}


int TaskDriverInterface_LDR::RunningJobs()
{
	#warning could speed up this by using a counter...
	int cnt=0;
	for(LDRClient *client=clientlist.first(); client; client=client->next)
	{
		cnt+=client->assigned_jobs;
		#if TESTING
		if(client->DeletePending())
		{  assert(client->assigned_jobs==0);  }
		#endif
	}
	return(cnt);
}

		
int TaskDriverInterface_LDR::AreThereJobsRunning()
{
	// We return 0 here only if we are not connected to any client 
	// any longer. In this case, the TaskManager may quit. 
	if(nclients)  return(1);
	// NOTE: We will normally not reach here...
	for(LDRClient *client=clientlist.first(); client; client=client->next)
	{  /*if(!client->DeletePending())*/  return(1);  }  // <-- NOW: ret 1 if !empty
	return(0);
}


// Called when everything is done to disconnect from the clients. 
// Also called for recovery!
// _QuittingNowDone() gets called when all clients disconnected to 
// inform TaskManager that it is done. 
void TaskDriverInterface_LDR::PleaseQuit()
{
	if(shall_quit)  return;
	
	// We will no longer re-connect if we quit: 
	dont_reconnect=1;
	_StopReconnectTrigger();
	
	// MUST BE SET before the for()-loop: 
	shall_quit=1;
	
	if(clientlist.is_empty())
	{
		// Do it immediately if there are no clients to disconnect. 
		_QuittingNowDone();
		return;
	}
	
	Verbose(TDR,"Disconnecting from clients (%d usable)...\n",nclients);
	
	// Schedule disconnect from all clients: 
	for(LDRClient *_i=clientlist.first(); _i; )
	{
		LDRClient *i=_i;
		_i=_i->next;
		if(i->DeletePending())  continue;
		if(i->Disconnect())
		{
			// Already disconnected or not connected. 
			i->DeleteMe();  // NO NEED TO DEQUEUE. 
		}
	}
}


// See PleaseQuit(). 
void TaskDriverInterface_LDR::_QuittingNowDone()
{
	assert(shall_quit);
	
	// Finally, tell task manager: 
	component_db()->taskmanager()->CheckStartNewJobs(/*special=*/-1);
}


void TaskDriverInterface_LDR::RecoveryDone()
{
	assert(nclients==0);
	assert(RunningJobs()==0);
	assert(njobs==0);   // not sure about this one
	assert(clientlist.is_empty());   // even less sure about this one
	
	dont_reconnect=0;
	_StopReconnectTrigger();  // be sure...
	
	already_started_processing=0;
	shall_quit=0;
	
	// Reset network IO counters: 
	net_starttime.SetInvalid();
	downstream_stat.Reset();
	upstream_stat.Reset();
	
	_SwitchKeepaliveTimer(0);  // be sure...
}


// Return value: 0 -> Okay, queued stop for client, else error. 
int TaskDriverInterface_LDR::_DoQueueStopContForClient(LDRClient *client,
	LDR::LDRClientControlCommand l_cccmd)
{
	int rv=client->SendControlRequest(l_cccmd);
	// We can ignore the return code. In case of failure (other than 
	// "auth not passed", the client kicks itself. (Cool, eh ;)
	if(!rv)
	{
		++client->stopcont_calls_pending;
		// This is currently the "final will": 
		client->crun_status=
			l_cccmd==LCCC_StopJobs ? ESS_Stopping : ESS_Continuing;
		return(0);
	}
	return(rv);
}

int TaskDriverInterface_LDR::StopContTasks(int signo)
{
	assert(signo==SIGCONT || signo==SIGTSTP || signo==SIGSTOP);
	
	// Send signal to the tasks: 
	Warning("Sending %s command to all clients...\n",
		signo==SIGCONT ? "CONT" : "STOP");
	
	int no_client_working=1;
	
	LDR::LDRClientControlCommand cccmd=
		(signo==SIGCONT) ? LCCC_ContJobs : LCCC_StopJobs;
	for(LDRClient *i=clientlist.first(); i; i=clientlist.next(i))
	{
		if(i->DeletePending())  continue;
		int rv=_DoQueueStopContForClient(i,cccmd);
		if(!rv)
		{  no_client_working=0;  }
	}
	
	return(no_client_working ? 0 : 1);  // 0 -> done; 1 -> in progress
}

// See if all clients are now stopped/running: 
void TaskDriverInterface_LDR::_CheckExecStopStatusChange()
{
	// Count, how many clients are in which state: 
	int counter[_ESS_Last];
	int in_transition=0;
	for(int i=0; i<_ESS_Last; i++)
	{  counter[i]=0;  }
	for(LDRClient *i=clientlist.first(); i; i=clientlist.next(i))
	{
		if(i->DeletePending() || i->auth_state!=2)  continue;
		assert(i->crun_status<_ESS_Last && i->crun_status>=0);
		if(i->stopcont_calls_pending)
		{  ++in_transition;  }
		else
		{  ++counter[i->crun_status];  }
	}
	
	// Note: I put very strict asserts here. IHMO, they should not fail 
	//   but I may be wrong because of race conditions or missing checks. 
	//   In case one of these fails, we can still simply kick an offending 
	//   client. Please report failures immediately as a bug. 
	// NOTE: ExecStopStatus is updated at the time when the apropriate 
	//       control command is queued, i.e. there is no schdule race 
	//       or sth like that. 
	TaskManager *taskman=component_db()->taskmanager();
	ExecStopStatus ess=taskman->Get_ExecStopStatus();
	switch(ess)
	{
		case ESS_Stopping:
			// Assert: see NOTE some lines above. 
			assert(counter[ESS_Running]==0);
			assert(counter[ESS_Continuing]==0);
			if(counter[ESS_Stopping]==0 && !in_transition)
			{
				// All clients stopped. Tell task manager: 
				taskman->ChangeExecStopStatus(ESS_Stopped);
			}
			break;
		case ESS_Continuing:
			// Assert: see NOTE above. 
			assert(counter[ESS_Stopped]==0);
			assert(counter[ESS_Stopping]==0);
			if(counter[ESS_Continuing]==0 && !in_transition)
			{
				// All clients continued. Tell task manager: 
				taskman->ChangeExecStopStatus(ESS_Running);
			}
			else if(counter[ESS_Running])
			{
				// Special case: While continuing, we call TaskManager's 
				// ChangeExecStopStatus() for each client which continues 
				// so that we can give it a task immediately. 
				taskman->ChangeExecStopStatus(ESS_Continuing);
			}
			break;
		case ESS_Stopped:  // fall through
		case ESS_Running:
			assert(counter[ESS_Stopping]==0);
			assert(counter[ESS_Continuing]==0);
			assert(!in_transition);
			assert(counter[ess==ESS_Running ? ESS_Stopped : ESS_Running]==0);
			break;
	}
}


// Schedule SIGTERM & SIGKILL on all jobs. 
// Returns number of clients which we sent the kill command. 
int TaskDriverInterface_LDR::TermAllJobs(TaskTerminationReason reason)
{
	// Schedule TERM & KILL for all tasks. 
	Warning("Scheduling TERM & KILL on all jobs (reason: %s)...\n",
		TaskExecutionStatus::TTR_JK_String(reason));
	
	int nkilled=0;
	LDR::LDRClientControlCommand cccmd;
	switch(reason)
	{
		case TTR_JK_UserIntr:   cccmd=LCCC_Kill_UserInterrupt;  break;
		case TTR_JK_ServerErr:  cccmd=LCCC_Kill_ServerError;    break;
		default:  assert(0);  // otherwise: bug! especially TTR_JK_Timeout!
	}
	for(LDRClient *i=clientlist.first(); i; i=clientlist.next(i))
	{
		if(i->DeletePending())  continue;
		i->SendControlRequest(cccmd);
		++nkilled;
	}
	
	// ####FIXME?! ###### What if client dies before answering the command? ######
	// IMHHHO no problem because that should also trigger a re-sched !?
	return(nkilled);
}


void TaskDriverInterface_LDR::ScheduleGiveBackTasks(int may_keep)
{
	bool wrote_msg=0;
	
	// Note: special value 0x7fffffff just forwarded. 
	for(LDRClient *i=clientlist.first(); i; i=clientlist.next(i))
	{
		if(i->DeletePending())  continue;
		if(!i->assigned_jobs)  continue;
		
		// There is a protection that we do not send too many 
		// control commands: Only re-send if the may_keep value 
		// changed or if we sent a task in between. 
		int prevval=i->_may_keep_value_sent;
		
		bool skip=1;
#warning "THIS DECISION MAKING HAS TO BE CHECKED!!!!!!!!!!!"
static int warned=0;
if(!warned)
{  Error("check me*******************dif_ldr.cpp:%d\n",__LINE__);  ++warned;  }
		do {
			if(prevval==0)  break;
			if(prevval==0x7fffffff || prevval==-1)
			{  if(i->_may_keep_value_sent==may_keep)  break;  }
			else if(may_keep!=-1 && may_keep!=0x7fffffff && may_keep>=prevval && 
				prevval!=-2)
			{  break;  }
			skip=0;
		} while(0);
		
		int send_may_keep_val=(may_keep==-1) ? i->c_jobs : may_keep;
		if(!skip && !wrote_msg)
		{
			char may_keep_str[24];
			if(may_keep==0x7fffffff)
			{  strcpy(may_keep_str,"all unprocessed");  }
			else if(may_keep==-1)
			{  strcpy(may_keep_str,"all but njobs");  }
			else
			{  snprintf(may_keep_str,24,"%d",may_keep);  }
			Verbose(TDR,"Telling clients to give back tasks (may keep: %s)\n",
				may_keep_str);
			wrote_msg=1;
		}
		Verbose(DBGV,"  Client %s: may_keep=%d, prev val=%d, do send: %s\n",
			i->_ClientName().str(),send_may_keep_val,prevval,
			skip ? "no" : "yes");
		if(!skip)
		{
			i->TellClientToGiveBackTasks(send_may_keep_val);
			i->_may_keep_value_sent=may_keep;  // ...can be -1
		}
	}
}


// Called on received client control command: 
// (Control command was executed by the client and we received confirmation.)
void TaskDriverInterface_LDR::HandleControlCommandResponse(LDRClient *client,
	LDRClientControlCommand l_cccmd)
{
	if(l_cccmd==LCCC_StopJobs || l_cccmd==LCCC_ContJobs)
	{
		--client->stopcont_calls_pending;
		// If assert fails, some bug in keeping stopcont_calls_pending up 
		// to date. 
		assert(client->stopcont_calls_pending>=0);
		
		if(!client->stopcont_calls_pending)
		{
			// Okay, the "final will" is established. 
			if(client->crun_status==ESS_Stopping)
			{  client->crun_status=ESS_Stopped;  }
			else if(client->crun_status==ESS_Continuing)
			{  client->crun_status=ESS_Running;  }
			// If the assert fails, internal error. In case we issue 
			// stop or cont and the last stop/cont was processed, this 
			// is the final outcome. Until this, we must have status 
			// stopping or continuing. 
			else assert(0);
			
			// See if all clients are now stopped/running: 
			_CheckExecStopStatusChange();
		}
	}
	// Currently happily ignore the rest. 
}


// Here is some flow diagram. 
// Accurate as of 03/2003. (Well, other comments need not be 
//  completely accurate, sigh.)
//--------------------------------------------------------------------
// TaskManager: LaunchTask()
//   -> client->SendTaskToClient(ctsk,shall...)
// 
// LDRClient::SendTaskToClient(ctsk,shall...)
//   send, receive, send...
//   -> TaskLaunchResult(ctsk,rv)
// 
// ...work...
// 
// LDRClient: TaskTerminationNotify() = _HandleTaskTermination()
//   -> reset ldrc,shall_render,filter
//   -> taskmanager->HandleFailedTask()
//      taskmanager->HandleSuccessfulJob()



// Actually launch a job for a task. No check if we may do that. 
int TaskDriverInterface_LDR::LaunchTask(CompleteTask *ctsk)
{
	// Otherwise LaunchTask() has a bug: 
	assert(ctsk && (ctsk->rt || ctsk->ft));   
	assert(!ctsk->d.any());   // May NOT YET be set (is set below). 
	
	// Return value: 
	//  1 -> could not launch (no free client)
	//  0 -> OK
	// -1,-2 -> did not start task [will not happen currently]
	
	// This should not be needed here buf safe is safe...
	if(free_client && !_TestClientCanDoTask(ctsk,free_client))
	{  free_client=NULL;  }
	
	if(!free_client)
	{
		// free_client is set in GetTaskToStart(). 
		// It can be NULL here if the client unregistered between 
		// GetTaskToStart() and actual launch. In this case, see 
		// if we can get a different one or report failure: 
		if(_GetFitStrategyFlag()==TaskManager::FSF_Run)
		{  free_client=_FindClientForTask(ctsk);  }
		// In case the fit strategy flag is FSF_Tight, return +1 here 
		// (by keeping free_client=NULL) so that we'll enter 
		// GetTaskToStart() immediately afterwards. 
		if(!free_client)
		{  return(+1);  }
	}
	
	bool shall_render,shall_filter;  // No value assigned; okay. 
	int cando=_TestClientCanDoTask(ctsk,free_client,
		&shall_render,&shall_filter);
	// NOTE: 
	//  cando=0 -> internal error (client cannot do task) [can not happen here]
	//  cando=1,2 -> shall_render or shall_filter are set correctly
	assert(cando);
	assert(shall_render || shall_filter);  // Otherwise cando should be 0. 
	
	// First, make sure we cancel free_client: 
	LDRClient *c=free_client;
	free_client=NULL;
	
	// Assign client to task (must be done here): 
	ctsk->d.ldrc=c;
	ctsk->d.shall_render=shall_render;
	ctsk->d.shall_filter=shall_filter;
	
	int rv=c->SendTaskToClient(ctsk);
	// NOTE: In case there is an error which MAY & DOES happen, then 
	//       call TaskLaunchResult(ctsk,TRC_<to_be_added>) and return(-1). 
	if(rv)
	{
		// rv=1 -> another task is currently scheduled to be sent
		// rv=2 -> enough tasks were assigned
		// rv=3 -> client does not want (any) more tasks
		//   [all three may not happen as CanDoTask() was called]
		// rv=-1 -> auth_state!=2 [then we may not try to launch a task]
		// rv=-2 -> nothing to do for task (ctsk->rt,ft=NULL or 
		//         !shall_render/filter or !ctsk)
		fprintf(stderr,"DifLDR:%d: internal error: rv=%d\n",__LINE__,rv);
		assert(0);
	}
	
	// Okay, the task manager now "thinks" that the task is running and 
	// everything works fine. 
	// All other failures until we actually get the LDRTaskResponse 
	// (i.e. client accepted task) are received via TaskLaunchResult(). 
	
	// All the ugly stuff (putting back a task, etc) is done by 
	// TaskLaunchResult() [or later]. 
	
	// NOTE: njobs is already incremented here (i.e. >=1). 
	assert(njobs>0);
	// (Otherwise we might try to launch more tasks than we are allowed to 
	// launch or even more than we CAN launch. Or how it was expressed 
	// originally: ...so that we do not get tasks again.)
	
	// Implement the "LRU-like method": Move this client to the 
	// end of the list: 
	if(clientlist.last()!=c)
	{
		clientlist.dequeue(c);
		clientlist.append(c);
	}
	
// OOPS!!! May only return if we really started to send the task. 
// Must put back task if this fails. -> no, TaskLaunchResult() does it. 
Verbose(DBG,"DifLDR: LaunchTask: njobs=%d (client:%d), nclients=%d\n",
	njobs,c->assigned_jobs,nclients);
	
	return(0);
}


// This gets called by and after LaunchTask() if launching the task failed 
// (or was done successfully in case resp_code==0). 
// This is only called for LAUNCHING the task; i.e. until we get 
// LDRTaskResponse. If the actual execution of the task (done by client) 
// failed, ###FIXME### must be called. 
// --equal text--
// Called until LDRTaskResponse was received. The client could not (try to) 
// execute a job because an error occured before that. Action: feed task into 
// a different client. [Also called if connection error, etc. occured.]
void TaskDriverInterface_LDR::TaskLaunchResult(CompleteTask *ctsk,
	int resp_code)
{
	assert(ctsk);
	
	LDRClient *c=ctsk->d.ldrc;
	assert(c);  // Otherwise bug near/in LaunchTask(). 
	
	// Reset this value (see ScheduleGiveBackTasks()): 
	c->_may_keep_value_sent=-2;  // Really -2, not -1. 
	
	if(resp_code)
	{
		// Failure. 
		
		// NOTE: This cannot happend ATM because LDRClient::_ParseTaskResponse() 
		//       kicks the client if the response code indicates error (!=0). 
		//       So, if there will ever be a resp code which we handle here 
		//       (i.e. one which does not result in the client being kicked) 
		//       implement this routine. [calling fkt: _ParseTaskResponse()]
		// Also, see below (-> LaunchingTaskDone). 
		
		// Do not forget to remove client: 
		// AND WHEN DOING THAT PUTTING TASK FROM proc QUEUE TO todo OR done QUEUE. 
		ctsk->d.ldrc=NULL;
		ctsk->d.shall_render=0;
		ctsk->d.shall_filter=0;
		
		fprintf(stderr,"implement me (resp_code=%d)!\n",resp_code);
		// Feed task into a different client (do that only XX times). oops...
		// 1) Client error -> feed different client because this one was kicked
		// 2) other error (unknown render desc...) 
		//      -> can use _HandleFailedTask/_HandleTaskTermination() 
		//      (in the mean time) to make it fail. 
		assert(0);
	}
	
	// In any case, tell TaskManager that the launch is 
	// done (becuase it may want to launch a further task): 
	// ##FIXME## this could be merged with error TaskManager's error 
	//           handling if resp_code!=0. 
	//           [CURRENTLY, resp_code=0 always, see above]
	component_db()->taskmanager()->LaunchingTaskDone(ctsk);
}


// Called when client reported task as done and all files 
// were transferred. (Reported as done does not imply success; 
// client may have failed completely to launch a proggy; 
// see ctsk->rtes, ctsk->ftes for details; ctsk->status 
// still untouched.) 
void TaskDriverInterface_LDR::TaskTerminationNotify(CompleteTask *ctsk)
{
	assert(ctsk && ctsk->d.any());
	
	_HandleTaskTermination(ctsk);
}


// Failure or success on client side during execution of the task. 
// Execution status transferred by client are stored in CompleteTask. 
void TaskDriverInterface_LDR::_HandleTaskTermination(CompleteTask *ctsk)
{
	// ###FIXME### Stupid problem comes to my mind: 
	// Say we have a render + filter task and the first client only 
	// renders it. Then we make it a filter task. But in case the 
	// render output was not uploaded, we will not be able to download 
	// the file. WHAT WILL WE DO? 
	//   Current solution: Relay on the user to be smart enough and 
	//   not force such a situation. 
	
	// Must set ctsk->state in this function. 
	
	bool should_render=ctsk->d.shall_render;
	bool should_filter=ctsk->d.shall_filter;
	assert(should_render || should_filter);
	
	// Make sure client is no longer attached to task: 
	ctsk->d.ldrc=NULL;
	ctsk->d.shall_render=0;
	ctsk->d.shall_filter=0;
	
	Verbose(DBG,"HandleTTerm(state=%d): render: should=%s, success=%s; "
		"fillter: should=%s, success=%s [frame %d]\n",
		ctsk->state,
		should_render ? "yes" : "no",
		ctsk->rtes.tes.IsCompleteSuccess() ? "yes" : "no",
		should_filter ? "yes" : "no",
		ctsk->ftes.tes.IsCompleteSuccess() ? "yes" : "no",
		ctsk->frame_no);
	
	// Do it sequentially: 
	bool reported=0;
	if(should_render)
	{
		assert(ctsk->rt);
		assert(ctsk->state==CompleteTask::ToBeRendered);
		if(ctsk->rtes.tes.IsCompleteSuccess())
		{
			if(should_filter && ctsk->ftes.tes.rflags!=TTR_Unset)
			{
				assert(ctsk->ft);
				if(ctsk->ftes.tes.IsCompleteSuccess())
				{
					ctsk->state=CompleteTask::TaskDone;
					component_db()->taskmanager()->HandleSuccessfulJob(ctsk);
					reported=1;
				}
				else
				{
					// Rendering was success but filtering failed. 
					
					ctsk->state=CompleteTask::ToBeFiltered;
					
					// Note: RunningJobs() is for emergency error handling only. 
					component_db()->taskmanager()->HandleFailedTask(
						ctsk,RunningJobs(),DTFilter);
					
					reported=1;
				}
			}
			else
			{
				if(ctsk->ft)  ctsk->state=CompleteTask::ToBeFiltered;
				else  ctsk->state=CompleteTask::TaskDone;
				
				component_db()->taskmanager()->HandleSuccessfulJob(ctsk);
				reported=1;
			}
		}
		else
		{
			if(ctsk->ftes.tes.rflags!=TTR_Unset) // <-- !FILTER!
			{  Error("difLDR: OOPS: filter status set although rendering "
				"not %s. [frame %d]\n",
				ctsk->rtes.tes.rflags==TTR_Unset ? "done" : "successful",
				ctsk->frame_no);  }
			
			if(ctsk->rtes.tes.rflags==TTR_Unset && 
			   ctsk->ftes.tes.rflags==TTR_Unset )  // <-- just for sanity
			{
				PutBackTask(ctsk);
				reported=1;
			}
			else
			{
				//ctsk->state=<untouched> (=ToBeRendered)
				
				// Note: RunningJobs() is for emergency error handling only. 
				component_db()->taskmanager()->HandleFailedTask(
					ctsk,RunningJobs(),DTRender);
				
				reported=1;
			}
		}
	}
	if(!reported && should_filter)
	{
		assert(ctsk->ft);
		assert(ctsk->state==CompleteTask::ToBeFiltered);
		if(ctsk->ftes.tes.IsCompleteSuccess())
		{
			ctsk->state=CompleteTask::TaskDone;
			component_db()->taskmanager()->HandleSuccessfulJob(ctsk);
			reported=1;
		}
		else
		{
			if(ctsk->ftes.tes.rflags==TTR_Unset)
			{
				PutBackTask(ctsk);
				reported=1;
			}
			else
			{
				//ctsk->state=<untouched> (=ToBeFiltered). 
				
				// Note: RunningJobs() is for emergency error handling only. 
				component_db()->taskmanager()->HandleFailedTask(
					ctsk,RunningJobs(),DTFilter);
				
				reported=1;
			}
		}
	}
	
	assert(reported);  // Will currently never fail :)
}


// Called for every new task obtained from TaskSource: 
int TaskDriverInterface_LDR::DealWithNewTask(CompleteTask *ctsk)
{
	// First, set up ctsk->state: 
	TaskDriverInterface::NewTask_SetUpState(ctsk);
	
	TaskDriverInterfaceFactory_LDR::DTPrm *prm=p->prm;
	
	// ctsk->rtp and ctsk->ftp is NULL here. 
	
	// Okay, RenderTask and FilterTask (derived from TaskStructBase) 
	// are the two structures which are passed from the task source to 
	// the task driver (interface); {Render,Filter}TaskParams come from 
	// the TaskDriver(Interface). 
	// So, we cannot pass {Render,Filter}TaskParams to the LDR client 
	// but only the TaskStructBase-derived classes. 
	// Ergo: no need to set up ctsk->rtp, ctsk->ftp. 
	
	// However, we must adjust the timeouts in RenderTask/FilterTask. 
	
	for(int i=0; i<_DTLast; i++)
	{
		TaskStructBase *tsb=NULL;
		switch(i)
		{
			case DTRender:  tsb=ctsk->rt;  break;
			case DTFilter:  tsb=ctsk->ft;  break;
			default:  assert(0);  break;
		}
		
		if(!tsb)  continue;
		
		TaskDriverInterfaceFactory_LDR::DTPrm *p=&prm[i];
		
		if(p->timeout>0 && tsb->timeout>0 && 
		   p->timeout<tsb->timeout )
		{  tsb->timeout=p->timeout;  }
	}
	
	// Dump all the information to the user (if requested):
	component_db()->taskmanager()->DumpTaskInfo(ctsk,
		NULL,TaskManager::DTSK_InitialQueue,VERBOSE_TDI);
	
	return(0);
}


// This is used by _FindTaskForClient() and _FindClientForTask().
// Test if the passed task can be done by the passed client.
// NOTE: Will answer "no" instead of "partially", if client can 
//       only filter but not render and the task is not yet 
//       rendered. 
// If shall_render/shall_filter is set, store proper values 
// ONLY IF RETURN VALUE IS 1 or 2. 
// Return 
//   0 -> no
//   1 -> partially
//   2 -> fully
int TaskDriverInterface_LDR::_TestClientCanDoTask(
	const CompleteTask *ctsk,LDRClient *client,
	bool *shall_render,bool *shall_filter)
{
	if(!client || !client->CanDoTask())
	{  return(0);  }
	
	// Values: 1 -> yes; 0 -> no; -1 -> no task 
	int can_render = (ctsk->rt && ctsk->state==CompleteTask::ToBeRendered) ? 
		client->IsSupportedDesc(DTRender,ctsk->rt->rdesc) : (-1);
	int can_filter = ctsk->ft ? // ctsk->state is IRRELEVANT HERE 
		client->IsSupportedDesc(DTFilter,ctsk->ft->fdesc) : (-1);
	// If the task is render & filter and the client can do both, 
	// then it can be done "fully".
	if(can_render==1 && can_filter==1)
	{
		if(shall_render)  *shall_render=1;
		if(shall_filter)  *shall_filter=1;
		return(2);
	}
	// If the task is render-only or filter-only and the client 
	// can do that, then it can be done "fully".
	if(can_render==1 && can_filter==-1)
	{
		if(shall_render)  *shall_render=1;
		if(shall_filter)  *shall_filter=0;
		return(2);
	}
	if(can_filter==1 && can_render==-1)
	{
		if(shall_render)  *shall_render=0;
		if(shall_filter)  *shall_filter=1;
		return(2);
	}
	// If the task is render and filter and the client can do only 
	// render, then it can be done "partly"
	if(can_render==1 && can_filter==0)
	{
		if(shall_render)  *shall_render=1;
		if(shall_filter)  *shall_filter=0;
		return(1);
	}
	// Otherwise, it cannot be done: 
	// (Especially, we may not try to filter a not-yet-rendered task.) 
	return(0);
}

// See if there is a task which could be done by the passed client. 
CompleteTask *TaskDriverInterface_LDR::_FindTaskForClient(
	const TaskManager_TaskList *tasklist,LDRClient *client,bool no_new_ones)
{
	if(!client || !client->CanDoTask())
	{  return(NULL);  }
	
	// Return first task in task list which can be done by the 
	// passed client; tasks which can be done completely get precedence 
	// over those which can only be done partly by a client.  
	
	CompleteTask *rf_only=NULL;
	for(CompleteTask *i=tasklist->todo.first(); i; i=i->next)
	{
		assert(i->state!=CompleteTask::TaskDone);
		assert(!i->d.any());   // We're in todo list, not proc list...
		
		// no_new_ones means: only tasks which were already rendered 
		// but not yet filtered. More generally: only ones which were 
		// already processed. 
		if(no_new_ones && !i->ProcessedTask())
		{  continue;  }
		
		int state=_TestClientCanDoTask(i,client);
		if(state==2)  return(i);
		if(state==1 && !rf_only)  rf_only=i;
	}
	return(rf_only);
}

// Find a client which can do the passed task. 
// Returns first match or NULL. 
LDRClient *TaskDriverInterface_LDR::_FindClientForTask(
	const CompleteTask *ctsk)
{
	if(!ctsk)  return(NULL);
	
	LDRClient *king=NULL;
	
	if(_GetFitStrategyFlag()==TaskManager::FSF_Run)
	{
		// Give clients points according and choose the client 
		// with the most points. 
		// Clients which can do the task completely get priority 
		// over those which can do it only partly. 
		// NOTE: Important: if several clients seem equally-qualified, 
		//       we take the FIRST one in the list. 
		int king_points=-0x7fffffff;
		for(LDRClient *i=clientlist.first(); i; i=i->next)
		{
			if(i->DeletePending())  continue;
			int state=_TestClientCanDoTask(ctsk,i);
			if(!state)  continue;
			
			// can do partly -> 100 points
			// can do fully -> 200 points
			int points=state*100;
			
			// If the client has fewer jobs than he should run 
			// in parallel, give it some points (max. 50): 
			if(i->assigned_jobs<i->c_jobs && i->c_jobs)
			{  points+=50*(i->c_jobs-i->assigned_jobs)/i->c_jobs;  }
			
			if(points>king_points)  // NOT >=
			{  king=i;  king_points=points;  }
		}
	}
	else if(_GetFitStrategyFlag()==TaskManager::FSF_Tight)
	{
		// Task source reported no more available tasks so 
		// clients without tasks have priority. 
		// Furthermore, no client gets more tasks than it has CPUs. 
		int king_points=-0x7fffffff;
		for(LDRClient *i=clientlist.first(); i; i=i->next)
		{
			if(i->DeletePending())  continue;
			int state=_TestClientCanDoTask(ctsk,i);
			if(!state)  continue;
			
			if(i->assigned_jobs>=i->c_jobs)  continue;
			
			// can do partly -> 2 points
			// can do fully -> 4 points
			int points=state*2;
			
			// If the client has fewer jobs than he should run 
			// in parallel, give it points (max. 200): 
			if(i->assigned_jobs<i->c_jobs && i->c_jobs)
			{  points+=200*(i->c_jobs-i->assigned_jobs)/i->c_jobs;  }
			
			if(points>king_points)  // NOT >=
			{  king=i;  king_points=points;  }
		}
	}
	else assert(0);
	
	return(king);
}


// Decide on task to start and return it. 
// Return NULL if there is no task to start. 
CompleteTask *TaskDriverInterface_LDR::GetTaskToStart(
	TaskManager_TaskList *tasklist,int schedule_quit)
{
	CompleteTask *startme=NULL;
	bool no_client_can_work=0;
	
	if(_GetFitStrategyFlag()==TaskManager::FSF_Run && 
	   !tasklist->todo.is_empty())
	{
		// First, see if we have a task for free_client: 
		// This can deal with free_client=NULL and checks CanDoTask(). 
		// NOTE: CanDoTask()=false can have different reasons here:
		//   TaskManager calls GetTaskToStart() twice without actually 
		//   launching the task in between. Maybe, the started task 
		//   was just cancelled for what reason ever. 
		
		// free_client will be NULL here normally and _FindTaskForClient() 
		// will return NULL then. 
		startme=_FindTaskForClient(tasklist,free_client,
			/*no_new_ones=*/schedule_quit);
		if(startme)  goto gottask;
	}
	free_client=NULL;
	
	// The following algo was for tight task situation but I deciced 
	// to use it for normal operation, too: 
// The code below in #if 0 was the old code: 
#if 0
	if(!tasklist->todo.is_empty())
	{
		// See if we have a task for a different client: 
		// NOTE: We use an LRU-like method here: We always check the client 
		//       list first-to-last if a client can do a task. If this client 
		//       got its task, then this client is re-queued at the end. 
		for(LDRClient *i=clientlist.first(); i; i=i->next)
		{
			if(i->DeletePending())  continue;
			startme=_FindTaskForClient(tasklist,i,
				/*no_new_ones=*/schedule_quit);
			if(startme)
			{  free_client=i;  goto gottask;  }
		}
	}
#endif
	
	if(!tasklist->todo.is_empty())
	{
		// In case the task situation is tight (i.e. task source 
		// reported no more available tasks), iterate through the 
		// tasks and give the task to the client which fits best. 
		// (In case this client is no longer available when actually 
		// launching, we do not launch the task but return here 
		// shortly later.) 
		
		// Okay, first see if there is any client which can do 
		// work: 
		no_client_can_work=1;
		for(LDRClient *i=clientlist.first(); i; i=i->next)
		{
			if(i->DeletePending())  continue;
			if(!i->CanDoTask())  continue;
			if(_GetFitStrategyFlag()==TaskManager::FSF_Run || 
			   i->assigned_jobs<i->c_jobs)
			{  no_client_can_work=0;  break;  }
		}
		
		// Loop through the tasks: 
		if(!no_client_can_work)
			for(CompleteTask *i=tasklist->todo.first(); i; i=i->next)
		{
			assert(i->state!=CompleteTask::TaskDone);
			assert(!i->d.any());   // We're in todo list, not proc list...
			
			// schedule_quit means: only tasks which were already rendered 
			// but not yet filtered. More generally: only ones which were 
			// already processed. 
			if(schedule_quit && !i->ProcessedTask())
			{  continue;  }
			
			LDRClient *c=_FindClientForTask(i);
			if(c)
			{
				free_client=c;
				startme=i;
				goto gottask;
			}
		}
	}
	
	// We're still here, so...
	if(_GetFitStrategyFlag()==TaskManager::FSF_Tight && !no_client_can_work)
	{
		// There is no client which can do any of the tasks or there 
		// is no task which could be assigned to any client. 
		// As the fit strategy flag is FSF_Tight, we should now 
		// see if we could take away a task from a different client. 
		// However, in case no client has less jobs than CPUs, we 
		// should just wait. 
		// The greatest danger here is getting trapped in a 
		// assign-giveback-assign-giveback-assign-... loop. 
		// That is why in FSF_Tight we don't ever give clients 
		// more tasks than they have CPUs. 
		
		int clients_which_need_jobs=0;
		int tasks_above_quota=0;
		for(LDRClient *i=clientlist.first(); i; i=i->next)
		{
			if(i->DeletePending())  continue;
			
//fprintf(stderr,"Client %s: Cando=%d, ass=%d, CPUs=%d, %d,%d,%d,%d,%d\n",
//	i->_ClientName().str(),i->CanDoTask(),i->assigned_jobs,i->c_jobs,
//	i->auth_state==2,i->assigned_jobs<i->c_task_thresh_high,!i->tri.scheduled_to_send,!i->client_no_more_tasks,i->crun_status==ESS_Running);
			
			if(i->CanDoTask() && i->assigned_jobs<i->c_jobs)
			{  ++clients_which_need_jobs;  }
			
			if(i->assigned_jobs>i->c_jobs)
			{  tasks_above_quota+=i->assigned_jobs-i->c_jobs;  }
		}
		if(tasks_above_quota && clients_which_need_jobs)
		{
			// Trigger the give-back. 
			Verbose(TDR,"Tight task situation (idle clients: %d, "
				"tasks avail: %d); triggering give-back.\n",
				clients_which_need_jobs,tasks_above_quota);
			// We may call that as often as we like because there is a 
			// protection in ScheduleGiveBackTasks() which prevents that 
			// we send more requests than necessary (on a per-client basis). 
			ScheduleGiveBackTasks(/*may_keep=*/0);
		}
	}
	
	gottask:;
	// We may not set startme->d.ldrc=free_client here because 
	// this is not allowed before actually launching. 
	// Instead, we save free_client. 
	
	if(startme)
	{
		assert(free_client);
		assert(_TestClientCanDoTask(startme,free_client));
		assert(!startme->d.any());
	}
	else
	{  assert(!free_client);  }
	
	if(!startme && IsVerbose(DBG))
	{
		Verbose(DBG,"LDR: Cannot get task to start. (todo=%d, proc=%d, "
			"done=%d, squit=%s)\n",
			tasklist->todo_nelem,tasklist->proc_nelem,tasklist->done_nelem,
			schedule_quit ? "yes" : "no");
		for(LDRClient *i=clientlist.first(); i; i=i->next)
		{
			if(i->DeletePending())  continue;
			Verbose(DBG,"  Client %s: assigned=%d, njobs=%d, max=%d, cando=%s\n",
				i->_ClientName().str(),
				i->assigned_jobs,i->c_jobs,i->c_task_thresh_high,
				i->CanDoTask() ? "yes" : "no");
			Verbose(DBGV,"    ioplock=%d, sched=%p, trs=%d, req=%d,%d, auth=%s, "
				"sched=%s, no_more=%s, ess=%s\n",
				i->outpump_lock,i->tri.scheduled_to_send,i->tri.task_request_state,
				i->tri.req_file_type,i->tri.req_file_idx,
				i->auth_state==2 ? "y" : "n",i->tri.scheduled_to_send ? "y" : "n",
				i->client_no_more_tasks ? "y" : "n",
				ExecStopStatus_String(i->crun_status));
		}
	}
	else if(startme && IsVerbose(DBG))
	{  Verbose(DBG,"LDR: Task to start: [frame %d] for %s (GetTaskToStart())\n",
		startme->frame_no,free_client->_ClientName().str());  }
	
	return(startme);
}


void TaskDriverInterface_LDR::_PrintInitConnectMsg(const char *msg)
{
	char tmp[32];
	if(p->connect_timeout>=0)
	{  snprintf(tmp,32,"%ld msec",p->connect_timeout);  }
	else
	{  strcpy(tmp,"[disabled]");  }
	Verbose(TDR,"%s (timeout: %s):\n",msg,tmp);
}


void TaskDriverInterface_LDR::_PrintThreshAndQueueInfo(int with_thresh)
{
	if(with_thresh)
	{
		Verbose(TDI,"  Todo-thresh: low=%d, high=%d; done-thresh: high=%d\n",
			todo_thresh_low,todo_thresh_high,done_thresh_high);
	}
	Verbose(TDI,"  Task queue: todo=%d, proc=%d, done=%d\n",
		GetTaskList()->todo_nelem,GetTaskList()->proc_nelem,
		GetTaskList()->done_nelem);
}


void TaskDriverInterface_LDR::_WriteStartProcInfo(const char *msg)
{
	// Write out useful verbose information: 
	VerboseSpecial("Okay, %s work: max %d parallel tasks on %d client%s.",
		msg,njobs,nclients,nclients==1 ? "" : "s");
	
	_PrintThreshAndQueueInfo(1);
}

void TaskDriverInterface_LDR::_WriteProcInfoUpdate()
{
	if(!shall_quit && already_started_processing)
	{
		VerboseSpecial("Update: max %d parallel tasks (currently %d) on %d client%s",
			njobs,RunningJobs(),nclients,nclients==1 ? "" : "s");
		
		_PrintThreshAndQueueInfo(1);
	}
	if(shall_quit && clientlist.is_empty())
	{
		VerboseSpecial("Update: All clients disconnected.");
		_PrintThreshAndQueueInfo(0);
	}
}

void TaskDriverInterface_LDR::_WriteEndProcInfo()
{
	// Number of unexpected disconnects, number of client quits 
	// avg number of simultanious clients, 
	// avg number of task / client 
	// CPU usage sum (min/max) 
	// per-client statistics (client must send it)
	
	// Dump some useful info...
	NetworkIOBase::DumpNetIOStatistics(
		VERBOSE_TDR,&downstream_stat,&upstream_stat,
		&net_starttime,"Complete network activity (this work cycle):",
		/*indent=*/"  ");
	
}

void TaskDriverInterface_LDR::WriteProcessingInfo(int when,const char *msg)
{
	if(when==0)
	{  _WriteStartProcInfo(msg);  }
	else if(when==1)
	{  _WriteEndProcInfo();  }
	else assert(0);
}


// ARGUMENT MUST BE OF TYPE TaskDriverInterfaceFactory_LDR::ClientParam *
LDRClient *TaskDriverInterface_LDR::NEW_LDRClient(void *_p)
{
	TaskDriverInterfaceFactory_LDR::ClientParam *p=
		(TaskDriverInterfaceFactory_LDR::ClientParam *)_p;
	
	LDRClient *client=NEW1<LDRClient>(this);
	if(client)
	{
		client->cp=p;
		p->client=client;
	}
	return(client);
}


void TaskDriverInterface_LDR::ReallyStartProcessing()
{
	// Okey, let's begin. We call TaskManager::ReallyStartProcessing() 
	// when the first client is connected. 
	
	{
		// Print some useful information: 
		const char *_disabled_str="[disabled]";
		char cto[32],rci[32];
		if(p->connect_timeout>=0)
		{  snprintf(cto,32,"%ld msec",p->connect_timeout);  }
		if(p->reconnect_interval>=0)
		{  snprintf(rci,32,"%ld sec",p->reconnect_interval/1000);  }
		Verbose(TDR,"  Connect timeout: %s;  Reconnect interval: %s\n",
			p->connect_timeout>=0 ? cto : _disabled_str,
			p->reconnect_interval>=0 ? rci : _disabled_str);
		if(p->keepalive_interval>=0)
		{  snprintf(cto,32,"%ld sec",p->keepalive_interval/1000);  }
		if(p->cmd_resp_timeout>=0)
		{  snprintf(rci,32,"%ld msec",p->cmd_resp_timeout);  }
		Verbose(TDR,"  Keepalive timer: %s,  %sresponse timeout: %s%s\n",
			p->keepalive_interval>=0 ? cto : _disabled_str,
			p->keepalive_interval>=0 ? "" : "(",
			p->cmd_resp_timeout>=0 ? rci : _disabled_str,
			p->keepalive_interval>=0 ? "" : ")");
		Verbose(TDR,"  Client limits: njobs<=%d; task thresh<=%d\n",
			p->max_jobs_per_client,p->max_high_thresh_of_client);
	}
	
	// Check reconnect interval: 
	if(p->reconnect_interval<0)
	{
		// We do no re-connecting. 
		dont_reconnect=1;
	}
	
	_PrintInitConnectMsg("Simultaniously initiating connections to all clients");
	
	HTime conn_to(HTime::Invalid);   // Connection timeout. 
	int n_connecting=0;
	for(TaskDriverInterfaceFactory_LDR::ClientParam *i=p->cparam.first();
		i; i=i->next)
	{
		// Important: we are now connecting: 
		i->shall_reconnect=0;   // Set to 1 again when connection lost. 
		
		int fail=1;
		LDRClient *c=NEW_LDRClient(i);
		do {
			if(!c)
			{
				if(!dont_reconnect)
				{  i->shall_reconnect=2;  _StartReconnectTrigger();  }
				break;
			}
			
			int rv=c->ConnectTo(i);
			if(rv<0)
			{  c->DeleteMe();  c=NULL;  fail=0;  break;  }
			
			// NOTE!!! CODE DUPLICATION. timernotify() DOES THE SAME. 
			
			++n_connecting;
			if(p->connect_timeout>=0)
			{
				if(conn_to.IsInvalid())  // Not yet set up -- do it. 
				{
					conn_to.SetCurr();
					conn_to.Add(p->connect_timeout,HTime::msec);
				}
				// If there is already a timeout, simply update it: 
				if(c->_tid_connect_to)
				{  UpdateTimeout(c->_tid_connect_to,conn_to);  }
				else
				{
					c->_tid_connect_to=InstallTimeout(conn_to,/*dptr=*/c);
					if(!c->_tid_connect_to)  break;
				}
			}
			
			fail=0;
		} while(0);
		
		if(fail)
		{
			if(c)
			{  c->DeleteMe();  c=NULL;  }
			
			assert(!free_client);
			while(!clientlist.is_empty())
			{  clientlist.popfirst()->DeleteMe();  }
			
			Error("Failed to set up LDR client representation.\n");
			already_started_processing=1;
			component_db()->taskmanager()->ReallyStartProcessing(/*error=*/1);
			return;
		}
	}
	
	if(!n_connecting)
	{
		assert(!free_client);
		while(!clientlist.is_empty())
		{  clientlist.popfirst()->DeleteMe();  }
		
		Error("No usable clients left in list. Giving up.\n");
		already_started_processing=1;
		component_db()->taskmanager()->ReallyStartProcessing(/*error=*/1);
		return;
	}
	
	HTime left(HTime::Invalid);
	if(!conn_to.IsInvalid())
	{  left=conn_to-HTime(HTime::Curr);  }
	Verbose(TDR,"Waiting%s%s for %d LDR connections to establish.\n",
		left.IsInvalid() ? "" : " ",
		left.IsInvalid() ? "" : left.PrintElapsed(/*with_msec=*/0),
		n_connecting);
}


void TaskDriverInterface_LDR::_StartReconnectTrigger()
{
	if(!reconnect_trigger_running && !dont_reconnect)
	{
		UpdateTimer(reconnect_trigger_tid,p->reconnect_interval,10);
		reconnect_trigger_running=1;
	}
}
void TaskDriverInterface_LDR::_StopReconnectTrigger()
{
	if(reconnect_trigger_running)
	{
		UpdateTimer(reconnect_trigger_tid,-1,0);
		reconnect_trigger_running=0;
	}
}


#define KA_INTERVAL_DIV 3

// Only switch if necessary. 
void TaskDriverInterface_LDR::_SwitchKeepaliveTimer(int on_off)
{
	if(on_off)
	{
		// Switch on: 
		if(!keepalive_running && p->keepalive_interval>=0)
		{
			// Note: The keepalive timer is triggered every inteval/3 msec 
			//       but we send the ping control command only to clients 
			//       which need it. This makes sure that they are not all 
			//       sent at the same time. 
			long val=p->keepalive_interval/KA_INTERVAL_DIV;
			if(!val)  val=1;  // Should never happen because user spec is in SECONDS. 
			UpdateTimer(keepalive_timer_tid,val,FDAT_FirstLater | 30);
			keepalive_running=1;
		}
	}
	else
	{
		// Switch off: 
		if(keepalive_running)
		{
			UpdateTimer(keepalive_timer_tid,-1,0);
			keepalive_running=0;
		}
	}
}


void TaskDriverInterface_LDR::UpdateRespTimeout(LDRClient *client,HTime *ttime)
{
	const HTime *orig=TimeoutTime(client->_tid_control_resp);
	assert(orig);
	
	if(ttime->IsInvalid())
	{
		// Disable timeout only if not already disabled: 
		if(orig->IsInvalid())  return;
	}
	else
	{
		// Calculate when the timeout will happen: 
		ttime->Add(p->cmd_resp_timeout,HTime::msec);
		
		// Update timeout only if necessary: 
		if((*orig)==(*ttime))  return;
	}
	
	int rv=UpdateTimeout(client->_tid_control_resp,*ttime);
	assert(rv==0);
}


int TaskDriverInterface_LDR::timernotify(TimerInfo *ti)
{
	Verbose(DBG,"--<TDIF_LDR::timernotify>--\n");
	
	if(ti->tid==reconnect_trigger_tid)
	{
		if(dont_reconnect)
		{
			_StopReconnectTrigger();
			return(0);
		}
		
		// See if we (try to) re-connect to some clients: 
		int msg_written=0;
		int may_stop_trigger=1;
		int n_connecting=0;
		HTime conn_to(HTime::Invalid);   // Connection timeout. 
		for(TaskDriverInterfaceFactory_LDR::ClientParam *i=p->cparam.first();
			i; i=i->next)
		{
			// If this assert fails, there is a great internal error: 
			// We may not have shall_reconnect set if there is a client 
			// (i.e. already connected). 
			assert(!i->shall_reconnect || !i->client);
			if(i->client || !i->shall_reconnect)  continue;
			if(i->shall_reconnect>1)
			{
				// Shall be done lateron. 
				--i->shall_reconnect;
				may_stop_trigger=0;
				continue;
			}
			// i->shall_reconnect=1 here. 
			
			if(!msg_written)
			{
				_PrintInitConnectMsg("Initiating re-connect to lost clients");
				msg_written=1;
			}
			
			// Important: we are now re-connecting: 
			i->shall_reconnect=0;
			
			LDRClient *c=NEW_LDRClient(i);
			int fail=1;
			do {
				if(!c)
				{
					if(!dont_reconnect)
					{  i->shall_reconnect=2;  /*may_stop_trigger=0; <-- below */  }
					break;
				}
				
				int rv=c->ConnectTo(i);
				if(rv<0)
				{
					c->DeleteMe();  c=NULL;  // --> shall_reconnect=1; in unregister
					may_stop_trigger=0;
					fail=0;  break;
				}
				
				// NOTE!!! CODE DUPLICATION. ReallyStartProcessing() DOES THE SAME. 
				
				++n_connecting;
				if(p->connect_timeout>=0)
				{
					if(conn_to.IsInvalid())  // Not yet set up -- do it. 
					{
						conn_to.SetCurr();
						conn_to.Add(p->connect_timeout,HTime::msec);
					}
					// If there is already a timeout, simply update it: 
					if(c->_tid_connect_to)
					{  UpdateTimeout(c->_tid_connect_to,conn_to);  }
					else
					{
						c->_tid_connect_to=InstallTimeout(conn_to,/*dptr=*/c);
						if(!c->_tid_connect_to)  break;
					}
				}
				
				fail=0;
			} while(0);
			
			if(fail)
			{
				if(c)
				{  c->DeleteMe();  c=NULL;  }
				
				// This is because during unregistering, a failed client 
				// may have set reconnect flag again. 
				may_stop_trigger=0;
				
				// #### What shall we do here? Better not set dont_reconnect=1; 
				//      but try again later...?
				Error("Failed to set up LDR client representation.\n");
				dont_reconnect=1;
				_StopReconnectTrigger();
				return(0);
			}
		}
		
		if(n_connecting)
		{  Verbose(TDR,"Waiting for %d LDR re-connects to establish.\n",
			n_connecting);  }
		
		if(may_stop_trigger)
		{
			// There are no clients which shall be re-connected the 
			// next time. 
			
			int wrong=0;
			for(TaskDriverInterfaceFactory_LDR::ClientParam *i=p->cparam.first();
				i; i=i->next)
			{
				if(!i->shall_reconnect)  continue;
				// If this assert fails, there is a great internal error: 
				// We may not have shall_reconnect set if there is a client 
				// (i.e. already connected). 
				assert(!i->client);
				wrong=1;  break;
			}
			if(wrong)
			{
				fprintf(stderr,"OOPS: would illegally stop reconnect trigger.\n");
			}
			else
			{
				_StopReconnectTrigger();
			}
		}
		
		// We may only be here if there is at least one client which we 
		// can try to re-connect to. 
		if(!msg_written && may_stop_trigger)
		{  fprintf(stderr,"BUG?! reconnect trigger but no client to "
			"re-connect.\n");  }
	}
	else if(ti->tid==keepalive_timer_tid)
	{
		int n_dealt_with=0;
		Verbose(DBG,"Sending keepalive to clients:");
		
		// Okay, schedule keepalive for all clients which need it: 
		HTime cmp_time(*ti->current);
		cmp_time.Sub(p->keepalive_interval,HTime::msec);
		for(LDRClient *i=clientlist.first(); i; i=i->next)
		{
			if(i->DeletePending())  continue;
			
			// Avoid sending pings too closely but DO queue more than one 
			// ping in case we do not get a response. This will protect us 
			// from blocked poll(POLLOUT) because when trying to queue 
			// more than 3 keepalive pings we give up (client kicks itself). 
			if(i->_ping_skipme_counter)
			{  --i->_ping_skipme_counter;  }
			
			// There is currently no timeout for quit disconnect. 
			// If disconnect takes too long, we simply kick it. 
			if(i->client_quit_queued==3)  // 3 -> received confirmation
			{
				// NOTE: caught_keepalive_during_quit is a 5-bit value. 
				if(i->caught_keepalive_during_quit>=1)
				{
					Warning("Client %s: Seems to be stuck during quit. Kicking.\n",
						i->_ClientName().str());
					i->_KickMe();
				}
				++i->caught_keepalive_during_quit;
				if(IsVerbose(DBG))
				{  Verbose(DBG," (%s)\n",i->_ClientName().str());  }
				++n_dealt_with;
				continue;
			}
			
			// If the last_response_time is invalid, then the client did 
			// not yet pass auth. The connect timeout makes sure we 
			// time out in this state if it takes too long. 
			if(i->last_response_time.IsInvalid())  continue;
			// If we got a response, there is no need to send a keepalive 
			// packet -- obviously the client _IS_ alive. 
			if(i->last_response_time>=cmp_time)  continue;
			
			// Okay, for downloads of really large files, it may happen 
			// that the download takes very long resulting in 3 keepalive 
			// requests being queued and the client kicking itself. 
			// So, if we could send data, the client still seems to be 
			// there. (Hey, we need to trust TCP anyways...)
			// [The same goes for large uploads.]
			if(!i->last_datapump_io_time.IsInvalid() && 
				i->last_datapump_io_time>=cmp_time)  continue;
			
			if(!i->_ping_skipme_counter)
			{
				// Send ping control command: 
				int rv=i->SendControlRequest(LCCC_PingPong);
				// Ignore return value because client kicks itself on failure. 
				assert(rv!=-2);   // It is PROBABLY a bug if that fails (not sure). 
				i->_ping_skipme_counter=KA_INTERVAL_DIV;
				if(IsVerbose(DBG))
				{  Verbose(DBG," [%s:rv=%d]",i->_ClientName().str(),rv);  }
				++n_dealt_with;
			}
		}
		Verbose(DBG,n_dealt_with ? "\n" : " [none]\n");
	}
	else assert(0);
	
	return(0);
}


int TaskDriverInterface_LDR::timeoutnotify(TimeoutInfo *ti)
{
	Verbose(DBG,"--TDIF_LDR::timeoutnotify--<>--\n");
	
	// We have the client pointer attached to the timeout as data pointer, 
	// so finding the associated client is easy. 
	// Obtain client pointer: 
	LDRClient *client=(LDRClient*)(ti->dptr);
	assert(client);   // Will give NULL ptr deref otherwise and IS A BUG. 
	
	// This should actually not happen but be sure...
	if(client->DeletePending())
	{  return(0);  }
	
	if(client->_tid_connect_to==ti->tid)
	{
		// See in what state the client is: 
		if(client->connected_state==0 || client->auth_state==2)
		{
			// Why are we here?
			Verbose(DBG,"Spurious connect timeout for client %s. (BUG?)\n",
				client->_ClientName().str());
			// NOTE: We do NOT kill the timeout so we need not alloc a 
			//       new one the next time. 
			return(0);
		}
		
		// He has do die...
		Warning("Client %s: Connection timed out during %s.\n",
			client->_ClientName().str(),
			client->connected_state==1 ? "connect" : "auth");
		/*if(client->Disconnect())
		{
			// Already disconnected: Delete; then destructor will 
			// unregister it: 
			client->DeleteMe();
		}*/
		// NO, that (= the code above) was much too nice: 
		client->_KickMe();
	}
	else if(client->_tid_control_resp==ti->tid)
	{
		// Okay, see if that is actually true or if the timeout 
		// was just forgot to be disabled: 
		HTime curr;
		if(ti->current)  curr=*ti->current;
		else  curr=HTime::Curr;
		
		HTime mintime(curr);
		mintime.Sub(p->cmd_resp_timeout);
		
		CommandQueue::CQEntry *cqent=NULL;
		int rv=client->cmd_queue.CheckExpiredEntries(&mintime,&cqent);
		if(rv==0 || rv==1)
		{
			HTime delta(HTime::Invalid);
			if(cqent)
			{  delta=curr-cqent->sendtime;  }
			Error("Client %s: OOPS: Spurious response timeout (rv=%d, "
				"fdtime=%s, delta=%s).\n",
				client->_ClientName().str(),rv,
				curr.PrintTime(/*local=*/1,/*with_msec=*/1),
				delta.IsInvalid() ? "???" : delta.PrintElapsed(/*with_msec=*/1));
		}
		else
		{
			assert(cqent);   // Otherwise big and simple bug. 
			Warning("Client %s: Response timeout (timeout=%ld, "
				"elapsed=%ld msec) passed.\n",
				client->_ClientName().str(),
				p->cmd_resp_timeout,
				cqent->sendtime.MsecElapsed(&curr));
			
			client->_KickMe();
		}
	}
	else assert(0);  // data consistency (BUG otherwise)
	
	return(0);
}


int TaskDriverInterface_LDR::admin_command(ADMCmd *cmd,RVAPGrowBuffer *dest)
{
	if(!strcmp(cmd->arg[0],"lsc"))
	{
		dest->printf("Currently %d LDR clients:\n",clientlist.count());
		dest->printf("  ntsk/ctsk run     connected client\n");
		char ntsk_tmp[16];
		char ctsk_tmp[16];
		HTime current(HTime::MostCurr);
		for(LDRClient *i=clientlist.first(); i; i=i->next)
		{
			if(i->auth_state==2)
			{  snprintf(ntsk_tmp,16,"%4d",i->assigned_jobs);  }
			else  strcpy(ntsk_tmp,"auth");
			
			if(i->client_no_more_tasks)  strcpy(ctsk_tmp,"nomo");
			else  snprintf(ctsk_tmp,16,"%4d",i->c_jobs);
			
			char *crun_tmp="???";
			switch(i->crun_status)
			{
				case ESS_Running:     crun_tmp="run";  break;
				case ESS_Stopping:    crun_tmp="STP";  break;
				case ESS_Stopped:     crun_tmp="stp";  break;
				case ESS_Continuing:  crun_tmp="CNT";  break;
			}
			
			dest->printf("  %s/%s %s %13s %s\n",
				ntsk_tmp,ctsk_tmp,
				crun_tmp,
				(current-i->connected_since).PrintElapsed(/*msec=*/0),
				i->_ClientName().str());
		}
		
		return(0);
	}
	else if(!strcmp(cmd->arg[0],"?"))
	{
		dest->printf("lsc: list clients\n");
		return(0);
	}
	
	return(1);
}


// Do njobs calculation because there are now more/less clients. 
// mode: +1 -> add; -1 -> subtract
void TaskDriverInterface_LDR::_JobsAddClient(LDRClient *client,int mode)
{
	int oldval=njobs;
	if(client->_counted_as_client && mode<0)
	{
		--nclients;
		client->_counted_as_client=0;
		njobs-=client->c_jobs;
	}
	if(!client->_counted_as_client && mode>0)
	{
		++nclients;
		client->_counted_as_client=1;
		njobs+=client->c_jobs;
	}
	
	// Make sure, keepalive timer is switched off/on: 
	_SwitchKeepaliveTimer(/*on=*/(nclients>0));
	
	#warning could adjust todo_thresh_low/high if we get more clients. 
	// todo_thresh_low/high is the number of tasks in todo queue (NOT in proc 
	// queue). So, we normally need not change it. BUT for 100 clients, we 
	// should probably have more than the default 2..5 tasks in todo to be 
	// able to serve task at any time. 
	//todo_thresh_low=...
	//todo_thresh_high=...
	
	#if TESTING
	int cnt=0;
	for(LDRClient *i=clientlist.first(); i; i=i->next)
	{  if(i->auth_state==2)  ++cnt;  }
	assert(cnt==nclients);
	#endif
	
	// May be called if shall_quit=1. 
	if(already_started_processing)
	{  component_db()->taskmanager()->CheckStartNewJobs(
		(oldval!=njobs) ? 1 : 0);  }
}


void TaskDriverInterface_LDR::SuccessfullyConnected(LDRClient *client)
{
	assert(client->auth_state==2);
	assert(!client->_counted_as_client);
	
	// Well, gonna delete the connect timeout: 
	KillTimeout(client->_tid_connect_to);
	client->_tid_connect_to=NULL;
	
	// ...and got to make sure that the keepalive timer is running: 
	// (This is done by _JobsAddClient() below.) 
	
	// Set the network statistics start time if not yet set: 
	if(net_starttime.IsInvalid())
	{  net_starttime=client->connected_since;  }
	
	// First, adjust njobs and task queue threshs: 
	_JobsAddClient(client,+1);
	_WriteProcInfoUpdate();
	
	if(!already_started_processing)
	{
		// Great. We can start working. 
		already_started_processing=1;
		component_db()->taskmanager()->ReallyStartProcessing(/*error=*/0);
	}
	
	// Tricky... if we are stopped or stopping, tell the client here: 
	ExecStopStatus ess=component_db()->taskmanager()->Get_ExecStopStatus();
	if(ess==ESS_Stopping || ess==ESS_Stopped)
	{  _DoQueueStopContForClient(client,LCCC_StopJobs);  }
}


void TaskDriverInterface_LDR::FailedToConnect(LDRClient *client)
{
	// Yes, this is easy...
	client->DeleteMe();
}


void TaskDriverInterface_LDR::ClientDisconnected(LDRClient *client)
{
	if(!client)  return;
	
	// Make sure disconnected client is not our next "free client": 
	if(free_client==client)
	{  free_client=NULL;  }
	
	// Dump some nice info: 
	client->DumpNetIOStatistics(VERBOSE_TDR,&client->connected_since,
		/*side=*/+1,client->_ClientName().str());
	
	// Let's see if the client has tasks. In this case, we must 
	// "put back" the tasks so that a different client gets them. 
	if(client->assigned_jobs)
	{
		Verbose(TDR,"Client %s: Putting back %d tasks:",
			client->_ClientName().str(),client->assigned_jobs);
		bool no_more_verbose=0;
		
		// Note: The one task which might be on the fly (not completely 
		//       transferred already has d.ldrc set properly and also 
		//       counts for assigned_jobs. So we do not leave it out here. 
		// NOTE: I traverse the list backwards to put the highest frame 
		//       back first (because PutBackTask() inserts it at beginning). 
		// NOTE!! When the task failed because a missing file or sth like 
		//        that, then there is no point in putting it into todo 
		//        queue again because it is likely to fail again and again. 
		//        Im this case, LDRClient set the TaskTerminationReason 
		//        and rflags are no longer TTR_Unset. TaskManager knows 
		//        to deal with that in PutBackTask(). 
		for(const CompleteTask *_i=GetTaskList()->proc.last(); _i; )
		{
			// This is needed because PutBackTask() may want to re-order or 
			// dequeue the task. 
			const CompleteTask *i=_i;
			_i=_i->prev;
			
			if(i->d.ldrc!=client)  continue;
			
			CompleteTask *ctsk=(CompleteTask*)i;  // <-- cast away the const 
			if(!no_more_verbose)
			{
				Verbose(TDR," [%d]",ctsk->frame_no);
				
				// In case the task was marked failed (see "NOT!!" above), 
				// TaskManager will dump some info. 
				if((ctsk->d.shall_render && ctsk->rtes.tes.rflags!=TTR_Unset) || 
				   (ctsk->d.shall_filter && ctsk->ftes.tes.rflags!=TTR_Unset) )
				{
					Verbose(TDR,client->assigned_jobs>1 ? " ...\n" : "\n");
					no_more_verbose=1;
				}
			}
			
			// Tell the TaskManager. 
			ctsk->d.ldrc=NULL;
			ctsk->d.shall_render=0;
			ctsk->d.shall_filter=0;
			PutBackTask(ctsk);  // Will re-queue into todo or done list. 
			
			if((--client->assigned_jobs)<=0)  break;
		}
		if(!no_more_verbose)
		{  Verbose(TDR,"\n");  }
		// If this assert fails, there is a bug in assigned task counting: 
		assert(client->assigned_jobs==0);
	}
	#if TESTING
	// There may not be any CompleteTasks which still reference 
	// the client. Note that when triggering this bug trap, the reason 
	// can also be incorrect client->assigned_jobs counting. 
	for(const CompleteTask *i=GetTaskList()->proc.first(); i; i=i->next)
	{  assert(i->d.ldrc!=client);  }
	#endif
	
	// Kill the (connect) timeout (if any): 
	KillTimeout(client->_tid_connect_to);
	client->_tid_connect_to=NULL;
	
	// Finally, delete client: 
	client->DeleteMe();
}


// These are called by the constructor/destructor of LDRClient: 
int TaskDriverInterface_LDR::RegisterLDRClient(LDRClient *client)
{
	if(!client)  return(0);
	
	// Check if queued: 
	if(clientlist.prev(client) || client==clientlist.first())  return(1);
	
	// Allocate timeout: 
	client->_tid_control_resp=InstallTimeout(HTime(HTime::Invalid),
		/*dptr=*/client);
	if(!client->_tid_control_resp)
	{  return(1);  }
	
	clientlist.append(client);
	return(0);
}


void TaskDriverInterface_LDR::UnregisterLDRClient(LDRClient *client)
{
	if(!client)  return;
	
	// Check if queued: 
	if(clientlist.prev(client) || client==clientlist.first())
	{
		// Dequeue client: 
		// MUST BE DONE before _JobsAddClient(). 
		clientlist.dequeue(client);
	}
	
	if(client==free_client);
	{  free_client=NULL;  }
	
	_JobsAddClient(client,-1);
	
	// Update statistics (server side here): 
	downstream_stat.Add(&client->out.stat);
	upstream_stat.Add(&client->in.stat);
	
	// Save the client param pointer: 
	TaskDriverInterfaceFactory_LDR::ClientParam *client_cp=client->cp;
	
	// Kill the (connect) timeout (if any): 
	KillTimeout(client->_tid_connect_to);
	client->_tid_connect_to=NULL;
	// Kill client pointer from ClientParams: 
	client->cp->client=NULL;
	// Un-Set client's ClientParam: 
	client->cp=NULL;
	
	// Make sure we also kill the resp timeout: 
	KillTimeout(client->_tid_control_resp);
	client->_tid_control_resp=NULL;
	
	// Tell the user (only if the client was authenticated; this 
	// prevents messages for failed connection attempts): 
	if(client->auth_state==2)
	{  _WriteProcInfoUpdate();  }
	
	// See if all clients are now stopped/running: 
	// (Must be called after client's removal from clientlist and 
	// I call it after _WriteProcInfoUpdate() but before 
	// _StartReconnectTrigger()...) 
	_CheckExecStopStatusChange();
	
	// We may re-try to connect to the client: 
	if(!dont_reconnect)
	{
		// In case reconnect_trigger_running, we set here 2 because then, not 
		// the next trigger but the one after the next will do the re-connect. 
		client_cp->shall_reconnect = reconnect_trigger_running ? 2 : 1;
		_StartReconnectTrigger();
	}
	
	if(already_started_processing)
	{
		if(shall_quit && !nclients)
		{  _QuittingNowDone();  }
	}
	else if(clientlist.is_empty())
	{
		Error("No usable clients left in list. Giving up.\n");
		already_started_processing=1;
		component_db()->taskmanager()->ReallyStartProcessing(/*error=*/1);
	}
}


TaskDriverInterface_LDR::TaskDriverInterface_LDR(
	TaskDriverInterfaceFactory_LDR *f,int *failflag) : 
	TaskDriverInterface(f->component_db(),failflag),
	FDBase(failflag),
	TimeoutBase(failflag),
	RendViewAdmin_CommandExecuter(f->component_db()->adminport(),failflag),
	clientlist(failflag),
	net_starttime(HTime::Invalid),
	downstream_stat(),
	upstream_stat()
{
	p=f;
	
	free_client=NULL;
	
	// Initial vals: 
	nclients=0;
	njobs=0;  // NOT -1
	
	todo_thresh_low=p->todo_thresh_low;
	todo_thresh_high=p->todo_thresh_high;
	done_thresh_high=p->done_thresh_high;
	
	already_started_processing=0;
	shall_quit=0;
	keepalive_running=0;
	reconnect_trigger_running=0;
	dont_reconnect=0;
	
	int failed=0;
	
	reconnect_trigger_tid=InstallTimer(-1,0);
	keepalive_timer_tid=InstallTimer(-1,0);
	if(!reconnect_trigger_tid || !keepalive_timer_tid)  ++failed;
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskDriverInterface_LDR");  }
}

TaskDriverInterface_LDR::~TaskDriverInterface_LDR()
{
	// Note: clients have to be deleted via DeleteMe(). 
	// It's too late here for that. 
	assert(clientlist.is_empty());
	assert(!free_client);
}

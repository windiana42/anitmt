/*
 * ldr.hpp
 * 
 * Task source implementing Local Distributed Rendering. 
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

#ifndef _RNDV_TSOURCE_LDR_HPP_
#define _RNDV_TSOURCE_LDR_HPP_ 1

#include "../tasksource.hpp"

#include <lib/netiobase_ldr.hpp>


class TaskSourceFactory_LDR;
class ComponentDataBase;
class TaskSource_LDR;


// Describing a server connection. Currently, only 
// one server may be connected at a time which is 
// the first in the list. 
struct TaskSource_LDR_ServerConn : 
	LinkedListBase<TaskSource_LDR_ServerConn>,
	NetworkIOBase_LDR
{
	private:
		TaskSource_LDR *back;
		
		union
		{
			char expect_chresp[LDRChallengeLength];
			int now_conn_auth_code;
		};
		
		// LDE Commands (host order)
		LDR::LDRCommand next_send_cmd;
		LDR::LDRCommand expect_cmd;
		
		inline TaskSourceFactory_LDR *P();
		inline ComponentDataBase *component_db();
		
		// Called to close down the connection or if it was closed down. 
		// reason: 0 -> general / error
		//         1 -> received Cmd_QuitNow
		//         2 -> auth failure
		void _ConnClose(int reason);
		
		int _AtomicSendData(LDR::LDRHeader *d);
		int _AtomicRecvData(LDR::LDRHeader *d,size_t len);
		
		// Returns packet length or 0 -> error. 
		size_t _CheckRespHeader(
			LDR::LDRHeader *d,size_t read_len,
			size_t min_len,size_t max_len);
		
		// This is filled in when we get a task until all 
		// the file downloading, etc. is done, the task is 
		// passed to the TaskManager and/or LDRTaskResponse 
		// is sent to the server. 
		enum TRINextAction
		{
			TRINA_None=0,
			TRINA_Complete,   // -> tell TaskManager()
			TRINA_Response,   // send TaskResponse
			TRINA_FileReq,    // send file request
			TRINA_FileRecvH,  // receive file header
			TRINA_FileRecvB   // receive file body
		};
		struct TaskRequestInfo
		{
			CompleteTask *ctsk;  // May be NULL if we refuse task. 
			u_int32_t task_id;
			int resp_code;   // TaskResponseCode: TRC_* or -1 for "unset"
			// This is only used if resp_code==TRC_Accepted: 
			TRINextAction next_action;
			// Of file request currently active...
			u_int16_t req_file_type;
			u_int16_t req_file_idx;
			TaskFile *req_tfile;
		} tri;  // task request info
		
		int _StartSendNextFileRequest();
		void _TaskRequestComplete();
		
		// Packet handling: 
		int _SendChallengeRequest();
		int _RecvChallengeResponse();
		int _SendNowConnected();
		
		int _HandleReceivedHeader(LDR::LDRHeader *hdr);
		
		int _ParseTaskRequest(RespBuf *buf);
		int _GetFileInfoEntries(TaskFile::IOType iotype,
			CompleteTask::AddFiles *dest,char **buf,char *bufend,int nent);
		int _ParseTaskRequest_Intrnl(RespBuf *buf,TaskRequestInfo *tri);
		
		int _AuthSConnFDNotify(FDInfo *fdi);
		
		int cpnotify_outpump_done(FDCopyBase::CopyInfo *cpi);
		int cpnotify_outpump_start();
		int cpnotify_inpump(FDCopyBase::CopyInfo *cpi);
		
		// Overriding virtual from FDCopyBase: 
		int fdnotify2(FDBase::FDInfo *fdi);
		int cpnotify(FDCopyBase::CopyInfo *cpi);
	public:  _CPP_OPERATORS_FF
		TaskSource_LDR_ServerConn(TaskSource_LDR *back,int *failflag=NULL);
		~TaskSource_LDR_ServerConn();
		int Setup(int sock,MyAddrInfo *addr);
		
		// Called after TellTaskManagerToGetTask(): 
		void TaskManagerGotTask();
		
		MyAddrInfo addr;  // server address
		
		// This is 1 if that is the authenticated server we are 
		// taking orders from. (Only sconn.first().) 
		private: int authenticated;
		public: int Authenticated()  {  return(authenticated);  }
};


class TaskSource_LDR : 
	public TaskSource,
	public FDBase
{
	private:
		// Our parameters: 
		TaskSourceFactory_LDR *p;
		
		// 0 msec response/schedule timer: 
		TimerID rtid;
		
		// PollID for socket we're listening to: 
		PollID l_pid;
		
		// What we're currently doing: 
		TSAction pending;
		CompleteTask *active_taketask;
		
		// Are we connected? This is just a dummy to ensure 
		// correct behavior of the other classes...
		int connected;  // <- to TaskManager, NOT to server. 
		
		// Struct TaskSource_LDR_ServerConn describing a 
		// connection to a server. 
		LinkedList<TaskSource_LDR_ServerConn> sconn;
		
		// Update/start rtid timer: 
		inline void _StartSchedTimer()
			{  UpdateTimer(rtid,0,0);  }
		inline void _StopSchedTimer()
			{  UpdateTimer(rtid,-1,0);  }
		
		
		// overriding virtuals from FDbase: 
		int timernotify(TimerInfo *);
		int fdnotify(FDInfo *);
		
		// overriding virtuals from TaskSource: 
		int srcConnect(TaskSourceConsumer *);
		int srcGetTask(TaskSourceConsumer *);
		int srcDoneTask(TaskSourceConsumer *,CompleteTask *ct);
		int srcDisconnect(TaskSourceConsumer *);
		
		long ConnectRetryMakesSense();
		TaskSourceType GetTaskSourceType(TaskSourceConsumer *persistent);
	public: _CPP_OPERATORS_FF
		TaskSource_LDR(TaskSourceFactory_LDR *,int *failflag=NULL);
		~TaskSource_LDR();
		
		TaskSourceFactory_LDR *P()
			{  return(p);  }
		
		// Returns authenticated server if we have one; else NULL. 
		TaskSource_LDR_ServerConn *GetAuthenticatedServer()
		{
			TaskSource_LDR_ServerConn *sc=sconn.first();
			return((sc && sc->Authenticated()) ? sc : NULL);
		}
		
		void ServerHasNowAuthenticated(TaskSource_LDR_ServerConn *sc)
		{
			sconn.dequeue(sc);
			sconn.insert(sc);
		}
		
		// Called by TaskSource_LDR_ServerConn::_ConnClose(). 
		void ConnClose(TaskSource_LDR_ServerConn *sc,int reason);
		
		// Called by TaskSource_LDR_ServerConn: 
		void TellTaskManagerToGetTask(CompleteTask *ctsk);
};

#endif  /* _RNDV_TSOURCE_LDR_HPP_ */

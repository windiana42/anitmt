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
		
		FDBase::TimerID schedule_tid;
		int outpump_scheduled;
		
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
			
		enum IOPumpLock
		{
			IOPL_Unlocked=0,
			IOPL_Upload
		};
		// This is needed for some packets so that they do not get 
		// "interrupted" by other ones, e.g. we may not send a 
		// FileRequest between FileUpload and [file data]. 
		IOPumpLock outpump_lock;
		
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
			TaskFile req_tfile;
			
			TaskRequestInfo(int *failflag) : req_tfile(failflag) {}
			~TaskRequestInfo() {}
		} tri;  // task request info
		
		int _StartSendNextFileRequest();
		void _TaskRequestComplete();
		
		enum TDINextAction
		{
			TDINA_None=0,
			TDINA_Complete,   // -> can tell TaskManager via tsnotify
			TDINA_SendDone,   // send LDRTaskDone (before uploading files)
			TDINA_FileSendH,  // send file header
			TDINA_FileSendB   // send (upload) file (body)
		};
		struct TaskDoneInfo
		{
			CompleteTask *done_ctsk;
			TDINextAction next_action;
			// Next file to upload: 
			u_int16_t upload_file_type;
			// This is the size we told the client in the header; 
			// this must be the copy limit. 
			int64_t upload_file_size;
			TaskFile upload_file;
			
			TaskDoneInfo(int *failflag) : upload_file(failflag) {}
			~TaskDoneInfo() {}
		} tdi;  // task done info
		
		int _SendNextFileUploadHdr();
		int _ParseFileDownload(RespBuf *buf);
		
		// Packet handling: 
		int _SendChallengeRequest();
		int _RecvChallengeResponse();
		int _SendNowConnected();
		
		int _HandleReceivedHeader(LDR::LDRHeader *hdr);
		
		int _ParseTaskRequest(RespBuf *buf);
		int _GetFileInfoEntries(TaskFile::IOType iotype,
			CompleteTask::AddFiles *dest,char **buf,char *bufend,int nent,
			CompleteTask *ctsk_for_error,RefString *prepend_path);
		int _ParseTaskRequest_Intrnl(RespBuf *buf,TaskRequestInfo *tri);
		int _TaskRequest_SetUpTaskFiles(CompleteTask *ctsk);
		
		int _AuthSConnFDNotify(FDInfo *fdi);
		
		int cpnotify_outpump_done(FDCopyBase::CopyInfo *cpi);
		int cpnotify_outpump_start();
		int cpnotify_inpump(FDCopyBase::CopyInfo *cpi);
		void schedule_outpump_start();
		
		// Overriding virtual from FDCopyBase: 
		int fdnotify2(FDBase::FDInfo *fdi);
		int cpnotify(FDCopyBase::CopyInfo *cpi);
		int timernotify(FDBase::TimerInfo *ti);
	public:  _CPP_OPERATORS_FF
		TaskSource_LDR_ServerConn(TaskSource_LDR *back,int *failflag=NULL);
		~TaskSource_LDR_ServerConn();
		int Setup(int sock,MyAddrInfo *addr);
		
		MyAddrInfo addr;  // server address
		
		// This is 1 if that is the authenticated server we are 
		// taking orders from. (Only sconn.first().) 
		private: int authenticated;
		public: int Authenticated()  {  return(authenticated);  }
		
		// Called after TellTaskManagerToGetTask(): 
		void TaskManagerGotTask();
		
		// Report passed task as done to the server. 
		// Do not free it (done by TaskSource_LDR). 
		void TellServerDoneTask(CompleteTask *ctsk);
		
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
		
		// This task is the one srcDoneTask() is currnetly working on. 
		CompleteTask *current_done_task;
		
		// 0 -> nothing/deleted task; 1 -> just received srcDoneTask; 
		// 2 -> waiting info spread to server 
		int done_task_status : 3;
		
		// Are we connected? This is just a dummy to ensure 
		// correct behavior of the other classes...
		int connected : 2;  // <- to TaskManager, NOT to server. 
		
		// This is 1 if we are recovering, i.e. the time from 
		// connection close to auth server until we're ready again. 
		int recovering : 3;
		// Set during recovery: 
		//  1 -> unexpected conn close
		//  2 -> quit request from server
		int quit_reason : 3;
		
		int : (sizeof(int)*8-11);  // padding
		
		// Struct TaskSource_LDR_ServerConn describing a 
		// connection to a server. 
		LinkedList<TaskSource_LDR_ServerConn> sconn;
		
		// Update/start rtid timer: 
		inline void _StartSchedTimer()
			{  UpdateTimer(rtid,0,0);  }
		inline void _StopSchedTimer()
			{  UpdateTimer(rtid,-1,0);  }
		
		DoneTaskStat _ProcessDoneTask();
		
		// overriding virtuals from FDbase: 
		int timernotify(TimerInfo *);
		int fdnotify(FDInfo *);
		
		// overriding virtuals from TaskSource: 
		int srcConnect(TaskSourceConsumer *);
		int srcGetTask(TaskSourceConsumer *);
		int srcDoneTask(TaskSourceConsumer *,CompleteTask *ct);
		int srcDisconnect(TaskSourceConsumer *);
		
		long ConnectRetryMakesSense();
		void SetPersistentConsumer(TaskSourceConsumer *persistent);
	public: _CPP_OPERATORS_FF
		TaskSource_LDR(TaskSourceFactory_LDR *,int *failflag=NULL);
		~TaskSource_LDR();
		
		TaskSourceFactory_LDR *P()
			{  return(p);  }
		
		// Returns authenticated server if we have one; else NULL. 
		TaskSource_LDR_ServerConn *GetAuthenticatedServer()
		{
			TaskSource_LDR_ServerConn *sc=sconn.first();
			return((sc && sc->Authenticated() && !sc->DeletePending()) ? sc : NULL);
		}
		
		// Returns 1 if if we are recovering, i.e. the time from 
		// connection close to auth server until we're ready again. 
		int Recovering() const
			{  return(recovering);  }
		
		void ServerHasNowAuthenticated(TaskSource_LDR_ServerConn *sc);
		
		// Called by TaskSource_LDR_ServerConn::_ConnClose(). 
		void ConnClose(TaskSource_LDR_ServerConn *sc,int reason);
		
		// Called by TaskSource_LDR_ServerConn: 
		void TellTaskManagerToGetTask(CompleteTask *ctsk);
		//   (reponse to TaskSource_LDR_ServerConn::TellServerDoneTask())
		void TaskReportedDone(CompleteTask *ctsk); 
};

#endif  /* _RNDV_TSOURCE_LDR_HPP_ */

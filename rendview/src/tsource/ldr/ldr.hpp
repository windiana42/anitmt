/*
 * ldr.hpp
 * 
 * Task source implementing Local Distributed Rendering. 
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

#ifndef _RNDV_TSOURCE_LDR_HPP_
#define _RNDV_TSOURCE_LDR_HPP_ 1

#include "../tasksource.hpp"

#include <lib/netiobase_ldr.hpp>
#include <lib/cmdqueue.hpp>


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
		
		// Used during auth: 
		char expect_chresp[LDRChallengeRespLength];
		int now_conn_auth_code;
		
		// Used during auth for server time correction calc: 
		// Client time when client sends the ChallengeRequest. 
		HTime chreq_send_time;
		// Modfication time correction: 
		// (To be added to local mtime before comparing to server mtime.) 
		HTime mtime_corr;
		// Min and max Server-client time difference: 
		HTime server_client_dtmin,server_client_dtmax;
		
		// LDR Commands (host order)
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
		
		// Control command queue (receiver side): 
		// NOTE: Data in the CommandQueue::CQEntry::data fields 
		//       is in NETWORK ORDER. 
		CommandQueue cmd_queue;
		// The control command queue entry which we are just sending 
		// or NULL: This ensures that we can insert AND append entries 
		// to the send queue at any time. 
		CommandQueue::CQEntry *sent_cmd_queue_ent;
		
		// Set when QuitFromServerNow() is called: 
		int must_quit_from_server;
		
		// This is the idle/keepalive timer: I could rather use 
		// a TimeoutBase::TimeoutID but there is not much point for 
		// TimeoutBase if we have just one timeout. 
		// NOTE: Timer also used as connect timeout (until auth). 
		FDBase::TimerID keepalive_tid;
		// The keepalive interval in msec as passed by the server, 
		// multiplied with P()->keepalive_mult; -1 -> disabled
		long keepalive_timeout;
		
		// When we last got progress info from the data pump: 
		HTime last_datapump_io_time;
		
	public:
		// Time when the connection was made: 
		HTime connected_since;
	private:
		
		int _SendNextFileUploadHdr();
		int _ParseFileDownload(RespBuf *buf);
		int _ParseCommandRequest(RespBuf *buf);
		
		// Packet handling: 
		int _SendChallengeRequest();
		int _RecvChallengeResponse();
		int _SendNowConnectedDenied();
		
		int _CreateLDRNowConnectedPacket(RespBuf *dest);
		
		int _HandleReceivedHeader(LDR::LDRHeader *hdr);
		
		int _ParseTaskRequest(RespBuf *buf);
		int _GetFileInfoEntries(TaskFile::IOType iotype,
			CompleteTask::AddFiles *dest,char **buf,char *bufend,int nent,
			CompleteTask *ctsk_for_error,RefString *prepend_path);
		int _ParseTaskRequest_Intrnl(RespBuf *buf,TaskRequestInfo *tri);
		int _TaskRequest_SetUpTaskFiles(CompleteTask *ctsk,
			const char *r_io,int r_io_len,int64_t r_in_size,const HTime *r_in_mtime,
			const char *f_io,int f_io_len,int64_t f_in_size,const HTime *f_in_mtime);
		
		void _ServerDisconnectMessage();
		
		int _AuthSConnFDNotify(FDInfo *fdi);
		
		inline void _ResetKeepaliveTimeout(const HTime *curr);
		
		int cpnotify_handle_errors(FDCopyBase::CopyInfo *cpi);
		int cpnotify_outpump_done(FDCopyBase::CopyInfo *cpi);
		int cpnotify_outpump_start();
		int cpnotify_inpump(FDCopyBase::CopyInfo *cpi);
		inline void schedule_outpump_start();
		
		// Overriding virtuals from FDCopyBase: 
		int fdnotify2(FDBase::FDInfo *fdi);
		int cpnotify(FDCopyBase::CopyInfo *cpi);
		int cpprogress(FDCopyBase::ProgressInfo *pi);
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
		
		// This is called when WE initiate a quit from the server. 
		// Calls ConnClose(reason=3) when done. 
		// Return: 0 -> in progress; 1 -> already done. 
		int QuitFromServerNow();
		
		// Tell server we do not want more tasks. 
		// Read comment of implementation in ldrsconn.cpp. 
		// Return: 
		//   0 -> sending command; 
		//   1 -> !auth || !conn
		//  -1 -> failure (alloc/queue); called _ConnClose(). 
		int DontWantMoreTasks();
		
		// Just needed as bug trap in TaskSource_LDR: 
		const CompleteTask *_GetCurrTask() const
			{  return(tri.ctsk);  }
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
		// Now, this is set when the task manager tells us to quit. 
		int ts_quit : 1;
		
		int : (sizeof(int)*8-12);  // padding
		
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
		int srcDisconnect(TaskSourceConsumer *,int do_quit);
		
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
		int RecoveringOrQuitting() const
			{  return(recovering || ts_quit);  }
		
		void ServerHasNowAuthenticated(TaskSource_LDR_ServerConn *sc);
		
		// Called by TaskSource_LDR_ServerConn::_ConnClose(). 
		void ConnClose(TaskSource_LDR_ServerConn *sc,int reason);
		
		// Called by TaskSource_LDR_ServerConn: 
		void TellTaskManagerToGetTask(CompleteTask *ctsk);
		//   (reponse to TaskSource_LDR_ServerConn::TellServerDoneTask())
		void TaskReportedDone(CompleteTask *ctsk); 
		// Execute simple command NOW (on the stack!)
		// Retval: that of tsnotify(). 
		int TellTaskManagerToExecSimpleCCmd(ClientControlCommand cccmd);
		// Well, the function name says it all...
		// (Put all but may_keep tasks from todo queue into done queue.)
		// Retval: currently only 0. 
		int TellTaskManagerToGiveBackTasks(int may_keep);
};

#endif  /* _RNDV_TSOURCE_LDR_HPP_ */

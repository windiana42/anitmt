/*
 * ldrclient.hpp
 * 
 * LDR task driver stuff: LDR client representation on server side. 
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


#ifndef _RNDV_TDRIVER_LDRCLIENT_HPP_
#define _RNDV_TDRIVER_LDRCLIENT_HPP_ 1

#include <lib/cmdqueue.hpp>


// THE LINKED TASK DRIVER LIST IS HELD BY TaskDriverInterface_Local
// (which is the right hand of TaskManager).  
// (NOT BY ComponentDataBase). 
class LDRClient : 
	public LinkedListBase<LDRClient>,
	public NetworkIOBase_LDR
{
	friend class TaskDriverInterface_LDR;
	private:
		typedef TaskDriverInterfaceFactory_LDR::ClientParam ClientParam;
		
		// We never talk to TaskManager directly but via TaskDriverInterface_LDR. 
		TaskDriverInterface_LDR *tdif;
		
		// Allocated by TaskDriverInterface; it is responsible for it. 
		ClientParam *cp;
		
		// List of supported render and filter descs which are known 
		// to the component data base (others are not worth saving). 
		// Index: DTRender, DTFilter. 
		int n_supp_descs[_DTLast];
		const RF_DescBase **supp_desc[_DTLast];   // LMalloc()'ed.
		// Cache for last successfully looked up entry in supp_desc[]. 
		const RF_DescBase *_cache_known_supported[_DTLast];
		
		// connected_state: 2 -> connected; 1 -> waiting for conn; 0 -> not conn.
		int connected_state : 3;  // 0,1,2
		int auth_state : 3;   // authenticated ? 0 -> no; 
		                      // 1 -> waiting for NowConnected; 2 -> YES
		int _counted_as_client : 1;  // USED BY TaskDriverInterface_LDR
		// This flag gets set increased if the keepalive timer 
		// wants to send ping control command during quit (more precisely: 
		// between quit confirmation and actual client disconnect). 
		// It cannot, of course, but we want a timeout (so that quit does 
		// not take forever). 
		int caught_keepalive_during_quit : 5;
		// Set when we queued a LCCC_ClientQuit request. 
		int client_quit_queued : 3;  // 1 -> queued; 2 -> sent; 3 -> recv confirm
		// This is set when the client reports that is does not want 
		// any more tasks. Also set when quit gets scheduled. 
		int client_no_more_tasks : 1;
		// About tasks on client: running, stopping, stopped, continuing: 
		// MANAGED BY Driver Interface. 
		ExecStopStatus crun_status : 4;
		
		int : (sizeof(int)*8 - 22);   // <-- Use modulo if more than 16 bits. 
		
		// This is also used by driver interface. 
		// Number of pending stop/cont control commands which have not 
		// yet got confirmed by HandleControlCommandResponse(). 
		int stopcont_calls_pending;
		
		// For use by TaskDriverInterface_LDR: 
		TimeoutBase::TimeoutID _tid_connect_to;
		// Control command response timeout: 
		TimeoutBase::TimeoutID _tid_control_resp;
		
		// Next command to send to client: 
		// THESE HAVE NO EFFECT IF AUTH IS DONE. 
		LDR::LDRCommand next_send_cmd;
		LDR::LDRCommand expect_cmd;  // what we expect to get
		
		// Time when we received the last response from the client: 
		// Only valid after successful auth. 
		HTime last_response_time;
		// This is the time the data pump last sent or received data 
		// to/from the client. 
		HTime last_datapump_io_time;
		
		enum IOPumpLock
		{
			IOPL_Unlocked=0,
			IOPL_Download
		};
		// This is needed for some packets so that they do not get 
		// "interrupted" by other ones, e.g. we may not send a 
		// TaskRequest between FileDownload and [file data]. 
		IOPumpLock outpump_lock;
		
		enum TaskRequestState
		{
			TRC_None=0,
			TRC_SendTaskRequest,
			TRC_WaitForResponse,
			TRC_SendFileDownloadH,  // header
			TRC_SendFileDownloadB   // body
		};
		struct _TRI
		{
			// Task scheduled to be sent to the client or NULL. 
			// This is set until the task is completely sent (including 
			// all the files). 
			CompleteTask *scheduled_to_send;
			// As told by task manager / task driver interface: 
			// shall_render, shall_filter: see scheduled_to_send->d.shall_*
			// This is CompleteTask::{r,f}add.nfiles without skipped ones. 
			int non_skipped_radd_files;
			int non_skipped_fadd_files;
			// Stores what has to be done next for task *scheduled_to_send; 
			// Send file or main task struct, etc. 
			TaskRequestState task_request_state;
			// Which file we will send next: 
			u_int16_t req_file_type;
			u_int16_t req_file_idx;
			// This is the size we told the client in the header; 
			// this must be the copy limit. 
			int64_t req_file_size;
			TaskFile req_tfile;
			// When we started to serve for download: 
			HTime send_start_time;
			
			_TRI(int *failflag) : req_tfile(failflag),
				send_start_time(HTime::Invalid)  { }
			~_TRI() {}
		} tri;  // task request info
		
		enum TaskDoneState
		{
			TDC_None=0,
			TDC_WaitForResp,  // wait for file header or final task status
			TDC_UploadBody    // file body will be / is currently uploaded
		};
		struct _TDI
		{
			CompleteTask *done_ctsk;
			// These indicate what the client should have done. 
			int should_render : 1;
			int should_filter : 1;
			TaskDoneState task_done_state;
			// These are saved until we finally store them in 
			// CompleteTask *done_ctsk when done. 
			TaskExecutionStatus save_rtes;
			TaskExecutionStatus save_ftes;
			// File getting uploaded: 
			TaskFile recv_file;
			// When we started to receive for upload: 
			HTime recv_start_time;
			
			_TDI(int *failflag) : recv_file(failflag),
				recv_start_time(HTime::Invalid)  { }
			~_TDI() {}
		} tdi;  // task done info
		
		// Client data: 
		int c_jobs;  // njobs reported by client. 
		int assigned_jobs;  // number of jobs the client shall do 
		int c_task_thresh_high;  // high task thresh of client 
			// (client will never get more then this many tasks)
		
		// Last may_keep value in LCCC_GiveBackTasks; may also 
		// be -1 (c_jobs) or -2 (not set). 
		// MANAGED BY TaskDriverInterface_LDR. 
		int _may_keep_value_sent;
		
		// Control command queue (sender side): 
		// NOTE: Data in the CommandQueue::CQEntry::data fields 
		//       is in NETWORK ORDER. 
		CommandQueue cmd_queue;
		// The control command queue entry which we are just sending 
		// or NULL: This ensures that we can insert AND append entries 
		// to the send queue at any time. 
		CommandQueue::CQEntry *sent_cmd_queue_ent;
		// Number of queued ping (keepalive) requests: 
		int queued_ping_requests;
		// Used by TaskDriverInterface_LDR: 
		int _ping_skipme_counter;
		
		// Time when the connection was made: 
		HTime connected_since;
		
		// Returns nice client name: 
		RefString _ClientName();
		
		// These store packets in RespBuf *dest: 
		int _StoreChallengeResponse(LDR::LDRChallengeRequest *d,RespBuf *dest);
		int _Create_TaskRequest_Packet(RespBuf *dest);
		
		// Accept and return LDRHeader in HOST order. 
		int     _AtomicSendData(LDR::LDRHeader *d);
		ssize_t _AtomicRecvData(LDR::LDRHeader *d,size_t len,size_t min_len);
		
		// Write just that message: 
		void _ClientDisconnectMessage();
		
		const char *_ParseTaskDone_QuickInfoString(bool shall,
			const TaskExecutionStatus *tes);
		
		// Helper for _Create_TaskRequest_Packet(): 
		void _LDRStoreInputFileInfo(TaskDriverType dtype,
			TaskFile *infile,TaskFile *outfile,
			char *dptr,u_int64_t *save_size,LDR::LDRTime *save_mtime);
		
		// Helper of fdnotify(): 
		int _DoFinishConnect(FDBase::FDInfo *fdi);
		int _DoAuthHandshake(FDBase::FDInfo *fdi);
		void _DoSendQuit(FDBase::FDInfo *fdi);
		
		int _HandleReceivedHeader(LDR::LDRHeader *hdr);
		int _ParseNowConnected(RespBuf *buf);
		int _ParseNowConnected_ReadDescs(TaskDriverType dtype,const char *src,size_t len);
		int _ParseFileRequest(RespBuf *buf);
		int _ParseTaskResponse(RespBuf *buf);
		int _ParseTaskDone(RespBuf *buf);
		int _ParseFileUpload(RespBuf *buf,const HTime *fdtime);
		int _ParseDoneComplete(RespBuf *buf);
		int _ParseControlResponse(RespBuf *buf);
		
		void _InspectAndFixAddFiles(CompleteTask::AddFiles *af,CompleteTask *ctsk_for_msg);
		
		// Called on every error which results in a client disconnect. 
		// If the passed task is non-NULL, this task is marked as failed. 
		// For "why", pass one of the EF_JK_LDRFail* values. 
		//  "why" may have any value if this_task_failed=NULL (ignored). 
		void _KickMe(CompleteTask *this_task_failed=NULL,int why=0);
		// Used by _KickMe to mark tasks failed: 
		void _MarkTaskFailed(CompleteTask *ctsk,int why);
		void _DoMarkJobFailed(CompleteTask *ctsk,TaskDriverType dtype,
			TaskExecutionStatus *tts,int why);
		
		int _AuthConnFDNotify(FDBase::FDInfo *fdi);
		
		// Update the resp timeout (if needed): 
		void _UpdateRespTimeout();
		
		// Used by SendControlRequest(), TellClientToGiveBackTasks(): 
		int _DoQueueControlRequest(LDR::LDRClientControlCommand cccmd,
			char *data,size_t datalen);
		
		int cpnotify_handle_errors(FDCopyBase::CopyInfo *cpi);
		int cpnotify_outpump_done(FDCopyBase::CopyInfo *cpi);
		int cpnotify_outpump_start();
		int cpnotify_inpump(FDCopyBase::CopyInfo *cpi);
		
		// FDBase (contained in FDCopyBase) virtual: 
		int fdnotify2(FDBase::FDInfo *fdi);
		// FDCopyBase virtuals:
		int cpnotify(FDCopyBase::CopyInfo *cpi);
		int cpprogress(FDCopyBase::ProgressInfo *pi);
	public:  _CPP_OPERATORS_FF
		// Driver name copied into RefString. 
		LDRClient(TaskDriverInterface_LDR *tdif,int *failflag=NULL);
		virtual ~LDRClient();
		
		inline ComponentDataBase *component_db()
			{  return(tdif->component_db());  }
		
		// Search in supported render/filter descs for passed *desc 
		// (component data base object). Uses a 1-entry cache per dtype 
		// so successfully looking up the same RF_DescBase several 
		// times will be fast. 
		// Return value: 
		//   1 -> supported
		//   0 -> not supported
		//  -1 -> dtype out of range or desc==NULL
		int IsSupportedDesc(TaskDriverType dtype,const RF_DescBase *desc);
		
		// TASK MANAGER INTERFACE via TaskDriverInterface/TaskDriverInterfac_LDR: 
		
		// Start connection to passed client. (socket -> nonblock -> connect)
		// Return value: 
		//   0 -> connecting...
		//   1 -> connected successfully without delay
		//  -1 -> error
		int ConnectTo(ClientParam *);
		
		// Check if the client can do a task (i.e. connected and it has 
		// less than c_task_thresh_high tasks assigned.). 
		// That is, SendTaskToClient() can be called. 
		int CanDoTask()
		{  return(auth_state==2 && assigned_jobs<c_task_thresh_high && 
			!tri.scheduled_to_send && !client_no_more_tasks && 
			crun_status==ESS_Running);  }
		
		// Actually start sending the passed task to the client. 
		// Only one task at a time can be sent to the client. 
		// Return value: 
		//  1 -> busy doing the previous SendTaskToClient() command; 
		//       try again later 
		//  2 -> client has enough tasks (assigned_jobs>=c_jobs)
		//  3 -> client does not want any more tasks 
		//       (LCCC_CN_NoMoreTasks, client_no_more_tasks)
		//  0 -> Okay, in progress
		// -1 -> not (completely) connected (auth_state!=2)
		// -2 -> ctsk=NULL or nothing to do (ctsk->rt,ft=NULL and/or 
		//       ctsk->d.shall_{render,filter}=false)
		int SendTaskToClient(CompleteTask *ctsk);
		
		// Send Cmd_TaskCtrlRequest to client. 
		// Return value: 
		//   0 -> scheduled sending...
		//  -1 -> alloc failure/too many commands (kicked)
		//  -2 -> not (completely) connected (auth_state!=2)
		int SendControlRequest(LDR::LDRClientControlCommand cccmd);
		
		// Tell client to give back not-yet-processed tasks. 
		// It may keep (i.e. not give back) may_keep many 
		// not-yet-processed tasks. Sends a control request. 
		// Retval: like SendControlRequest(). 
		int TellClientToGiveBackTasks(int may_keep);
		
		// Disconnect from client. Immediately quit if not connected. 
		// Return value: 
		//  1 -> already disconnected
		//  0 -> wait for disconnect to happen. 
		int Disconnect();
};

#endif  /* _RNDV_TDRIVER_LDRCLIENT_HPP_ */

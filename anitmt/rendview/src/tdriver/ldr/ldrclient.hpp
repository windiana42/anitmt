/*
 * ldrclient.hpp
 * 
 * LDR task driver stuff: LDR client representation on server side. 
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


#ifndef _RNDV_TDRIVER_LDRCLIENT_HPP_
#define _RNDV_TDRIVER_LDRCLIENT_HPP_ 1


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
		
		// connected_state: 2 -> connected; 1 -> waiting for conn; 0 -> not conn.
		int connected_state : 3;  // 0,1,2
		int auth_passed : 1;
		int _counted_as_client : 1;  // USED BY TaskDriverInterface_LDR
		int send_quit_cmd : 3;  // 1 -> must send; 2 -> sent; 3 -> done
		int : (sizeof(int)*8 - 8);   // <-- Use modulo if more than 16 bits. 
		
		// Next command to send to client: 
		// THESE HAVE NO EFFECT IF AUTH IS DONE. 
		LDR::LDRCommand next_send_cmd;
		LDR::LDRCommand expect_cmd;  // what we expect to get
		
		enum TaskRequestState
		{
			TRC_None=0,
			TRC_SendTaskRequest,
			TRC_WaitForResponse,
			TRC_SendFileDownloadH,  // header
			TRC_SendFileDownloadB   // body
		};
		struct
		{
			// Task scheduled to be sent to the client or NULL. 
			// This is set until the task is completely sent (including 
			// all the files). 
			CompleteTask *scheduled_to_send;
			// Stores what has to be done next for task *scheduled_to_send; 
			// Send file or main task struct, etc. 
			TaskRequestState task_request_state;
			// Which file we will send next: 
			u_int16_t req_file_type;
			u_int16_t req_file_idx;
			// This is the size we told the client in the header; 
			// this must be the copy limit. 
			int64_t req_file_size;
			TaskFile *req_tfile;
		} tri;  // task request info
		
		enum TaskDoneState
		{
			TDC_None=0,
			TDC_WaitForResp,  // wait for file header or final task status
			TDC_UploadBody    // file body will be / is currently uploaded
		};
		struct
		{
			CompleteTask *done_ctsk;
			TaskDoneState task_done_state;
			// These are saved until we finally store them in 
			// CompleteTask *done_ctsk when done. 
			TaskExecutionStatus save_rtes;
			TaskExecutionStatus save_ftes;
			// File getting uploaded: 
			TaskFile *recv_file;
		} tdi;  // task done info
		
		// Client data: 
		int c_jobs;  // njobs reported by client. 
		int assigned_jobs;  // number of jobs the client shall do 
		int c_task_thresh_high;  // high task thresh of client 
			// (client will never get more then this many tasks)
		
		// Returns nice client name: 
		RefString _ClientName();
		
		// These store packets in RespBuf *dest: 
		int _StoreChallengeResponse(LDR::LDRChallengeRequest *d,RespBuf *dest);
		int _Create_TaskRequest_Packet(CompleteTask *ctsk,RespBuf *dest);
		
		// Accept and return LDRHeader in HOST order. 
		int     _AtomicSendData(LDR::LDRHeader *d);
		ssize_t _AtomicRecvData(LDR::LDRHeader *d,size_t len,size_t min_len);
		
		// Returns packet length or 0 -> error. 
		size_t _CheckRespHeader(LDR::LDRHeader *d,size_t read_len,
			size_t min_len,size_t max_len);

		// Helper of fdnotify(): 
		int _DoFinishConnect(FDBase::FDInfo *fdi);
		int _DoAuthHandshake(FDBase::FDInfo *fdi);
		void _DoSendQuit(FDBase::FDInfo *fdi);
		
		int _HandleReceivedHeader(LDR::LDRHeader *hdr);
		int _ParseFileRequest(RespBuf *buf);
		int _ParseTaskResponse(RespBuf *buf);
		int _ParseTaskDone(RespBuf *buf);
		int _ParseFileUpload(RespBuf *buf);
		int _ParseDoneComplete(RespBuf *buf);
		
		// Called on every error which results in a client disconnect. 
		void _KickMe(int do_send_quit=0);
		
		int _AuthConnFDNotify(FDBase::FDInfo *fdi);
		
		int cpnotify_outpump_done(FDCopyBase::CopyInfo *cpi);
		int cpnotify_outpump_start();
		int cpnotify_inpump(FDCopyBase::CopyInfo *cpi);
		
		// FDBase (contained in FDCopyBase) virtual: 
		int fdnotify2(FDBase::FDInfo *fdi);
		// FDCopyBase virtual:
		int cpnotify(FDCopyBase::CopyInfo *cpi);
	public:  _CPP_OPERATORS_FF
		// Driver name copied into RefString. 
		LDRClient(TaskDriverInterface_LDR *tdif,int *failflag=NULL);
		virtual ~LDRClient();
		
		inline ComponentDataBase *component_db()
			{  return(tdif->component_db());  }
		
		// TASK MANAGER INTERFACE via TaskDriverInterface/TaskDriverInterfac_LDR: 
		
		// Start connection to passed client. (socket -> nonblock -> connect)
		// Return value: 
		//   0 -> connecting...
		//   1 -> connected successfully without delay
		//  -1 -> error
		int ConnectTo(ClientParam *);
		
		// Check if the client can do a task (i.e. connected and it has 
		// less than c_jobs tasks assigned.). 
		// That is, SendTaskToClient() can be called. 
		int CanDoTask()
		{  return(auth_passed && assigned_jobs<c_task_thresh_high && 
			!tri.scheduled_to_send);  }
		
		// Actually start sending the passed task to the client. 
		// Only one task at a time can be sent to the client. 
		// Return value: 
		//  1 -> busy doing the previous SendTaskToClient() command; 
		//       try again later 
		//  2 -> client has enough tasks (assigned_jobs>=c_jobs)
		//  0 -> Okay, in progress
		// -1 -> not (completely) connected (auth_passed=0)
		// -2 -> ctsk=NULL or nothing to do (ctsk->rt=NULL && ctsk->ft=NULL)
		int SendTaskToClient(CompleteTask *ctsk);
		
		// Disconnect from client. Immediately quit if not connected. 
		// Return value: 
		//  1 -> already disconnected
		//  0 -> wait for disconnect to happen. 
		int Disconnect();
};

#endif  /* _RNDV_TDRIVER_LDRCLIENT_HPP_ */

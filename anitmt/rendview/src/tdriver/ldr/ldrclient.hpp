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


#include <lib/ldrproto.hpp>


// THE LINKED TASK DRIVER LIST IS HELD BY TaskDriverInterface_Local
// (which is the right hand of TaskManager).  
// (NOT BY ComponentDataBase). 
class LDRClient : 
	public LinkedListBase<LDRClient>
{
	friend class TaskDriverInterface_LDR;
	private:
		typedef TaskDriverInterfaceFactory_LDR::ClientParam ClientParam;
		
		// We never talk to TaskManager directly but via TaskDriverInterface_LDR. 
		TaskDriverInterface_LDR *tdif;
		
		// Allocated by TaskDriverInterface; it is responsible for it. 
		ClientParam *cp;
		
		// Socket for client connection: 
		int sock_fd;
		FDBase::PollID pollid;
		
		inline void PollFD(short events)
			{  tdif->PollFD(pollid,events);  }
		inline void ShutdownFD()
			{  tdif->ShutdownFD(pollid);  sock_fd=-1;  }
		inline void UnpollFD()
			{  tdif->UnpollFD(pollid);  sock_fd=-1;  }
		
		// connected_state: 2 -> connected; 1 -> waiting for conn; 0 -> not conn.
		int connected_state : 3;  // 0,1,2
		int auth_passed : 1;
		int _counted_as_client : 1;  // USED BY TaskDriverInterface_LDR
		int send_quit_cmd : 3;  // 1 -> must send; 2 -> sent; 3 -> done
		int : (sizeof(int)*8 - 8);   // <-- Use modulo if more than 16 bits. 
		
		// Next command to send to client: 
		LDR::LDRCommand next_send_cmd;
		LDR::LDRCommand last_recv_cmd;
		LDR::LDRCommand expect_cmd;  // what we expect to get
		struct RespBuf
		{
			size_t alloc_len;
			char *data;
			// Note: while the RespBuf is in use, this is !=Cmd_NoCommand; 
			// You may only modify the buffer if that is Cmd_NoCommand. 
			LDR::LDRCommand content;
		};
		// Send and receive buffers (used for different things): 
		RespBuf recv_buf;
		RespBuf send_buf;
		
		// Return value: 1 -> alloc failure 
		inline int _ResizeRespBuf(RespBuf *buf,size_t newlen);
		
		#if 0
		#error HACK ME...
		struct DataPump
		{
			FDCopyBase::CopyID cpid;  // or NULL if inactive
			LDR::LDRCommand cmd;  // or Cmd_NoCommand if not active
		};
		DataPump send_pump,recv_pump;
		#endif
		
		// Client data: 
		int c_jobs;  // njobs reported by client. 
		int assigned_jobs;  // number of jobs the client shall do 
		
		// Returns nice client name: 
		RefString _ClientName();
		
		// These store packets in RespBuf *dest: 
		int _StoreChallengeResponse(LDR::LDRChallengeRequest *d,RespBuf *dest);
		int _Create_DoTask_Packet(CompleteTask *ctsk,RespBuf *dest);
		
		int     _AtomicSendData(LDR::LDRHeader *d);
		ssize_t _AtomicRecvData(LDR::LDRHeader *d,size_t len,size_t min_len);
		
		// Returns packet length or 0 -> error. 
		size_t _CheckRespHeader(LDR::LDRHeader *d,size_t read_len,
			size_t min_len,size_t max_len);

		// Helper of fdnotify(): 
		int _DoFinishConnect(FDBase::FDInfo *fdi);
		int _DoAuthHandshake(FDBase::FDInfo *fdi);
		void _DoSendQuit(FDBase::FDInfo *fdi);
		
		#if 0
		#error HACK ME...
		// Calling TaskDriverInterface_LDR::DoCopyFdBuf(): 
		CopyID DoCopyFD2Buf(int fd,char *buf,size_t len)
			{  return(tdif->DoCopyFdBuf(this,fd,buf,len,-1));  }
		CopyID DoCopyBuf2FD(int fd,const char *buf,size_t len)
			{  return(tdif->DoCopyFdBuf(this,fd,(char*)buf,len,+1));  }
		#endif
		
		// Called via TaskDriverInterface_LDR: 
		void fdnotify(FDBase::FDInfo *fdi);
		void cpnotify(FDCopyBase::CopyInfo *cpi);
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
		{  return(auth_passed && assigned_jobs<c_jobs /*&& MISSING!!!*/ );  }
		
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

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
		
		// connected_state: 2 -> connected; 1 -> waiting for conn; 0 -> not conn.
		int connected_state : 3;  // 0,1,2
		int auth_passed : 1;
		int _counted_as_client : 1;  // USED BY TaskDriverInterface_LDR
		int send_quit_cmd : 3;  // 1 -> must send; 2 -> sent; 3 -> done
		int : 26;  // padding
		
		// Next command to send to client: 
		LDR::LDRCommand next_send_cmd;
		LDR::LDRCommand last_recv_cmd;
		LDR::LDRCommand expect_cmd;  // what we expect to get
		// Response buffer (used for different things): 
		size_t resp_buf_alloc_len;
		char *resp_buf;
		
		// Client data: 
		int c_jobs;  // njobs reported by client. 
		
		// Return value: 1 -> alloc failure 
		inline int _ResizeRespBuf(size_t newlen);
		
		inline void PollFD(short events)
			{  tdif->PollFD(pollid,events);  }
		inline void ShutdownFD()
			{  tdif->ShutdownFD(pollid);  sock_fd=-1;  }
		inline void UnpollFD()
			{  tdif->UnpollFD(pollid);  sock_fd=-1;  }
		
		// Returns nice client name: 
		RefString _ClientName();
		
		int _StoreChallengeResponse(LDR::LDRChallengeRequest *d);
		
		int     _AtomicSendData(LDR::LDRHeader *d);
		ssize_t _AtomicRecvData(LDR::LDRHeader *d,size_t len,size_t min_len);
		
		// Returns packet length or 0 -> error. 
		size_t _CheckRespHeader(LDR::LDRHeader *d,size_t read_len,
			size_t min_len,size_t max_len);

		// Helper of fdnotify(): 
		int _DoFinishConnect(FDBase::FDInfo *fdi);
		int _DoAuthHandshake(FDBase::FDInfo *fdi);
		void _DoSendQuit(FDBase::FDInfo *fdi);
		
		// Called via TaskDriverInterface_LDR: 
		void fdnotify(FDBase::FDInfo *fdi);
		
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
		
		// Disconnect from client. Immediately quit if not connected. 
		// Return value: 
		//  1 -> already connected
		//  0 -> wait for disconnect to happen. 
		int Disconnect();
};

#endif  /* _RNDV_TDRIVER_LDRCLIENT_HPP_ */

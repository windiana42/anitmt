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
		
		int : 29;  // padding
		
		inline void PollFD(short events)
			{  tdif->PollFD(pollid,events);  }
		inline void ShutdownFD()
			{  tdif->ShutdownFD(pollid);  }
		inline void UnpollFD()
			{  tdif->UnpollFD(pollid);  sock_fd=-1;  }
		
		// Returns nice client name: 
		RefString _ClientName();
		
		// Helpers of fdnotify(): 
		int _DoFinishConnect(FDBase::FDInfo *fdi);
		
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
};

#endif  /* _RNDV_TDRIVER_LDRCLIENT_HPP_ */

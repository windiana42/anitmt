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
	public FDBase
{
	friend class TaskDriverInterface_LDR;
	private:
		typedef TaskDriverInterfaceFactory_LDR::ClientParam ClientParam;
		
		// We never talk to TaskManager directly but via TaskDriverInterface_LDR. 
		TaskDriverInterface_LDR *tdif;
		
		// Overriding virtuals: 
		int timernotify(TimerInfo *ti);
		
	public:  _CPP_OPERATORS_FF
		// Driver name copied into RefString. 
		LDRClient(TaskDriverInterface_LDR *tdif,int *failflag=NULL);
		virtual ~LDRClient();
		
		inline ComponentDataBase *component_db()
			{  return(tdif->component_db());  }
		
		// TASK MANAGER INTERFACE via TaskDriverInterface/TaskDriverInterfac_LDR: 
		
		// Start connection to passed client. (socket -> nonblock -> connect)
		int ConnectTo(ClientParam *);
};

#endif  /* _RNDV_TDRIVER_LDRCLIENT_HPP_ */

/*
 * dif_param.hpp
 * 
 * Header for parameter & factory class of LDR task driver interface. 
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

#ifndef _RNDV_TDRIVER_LDR_PARS_HPP_
#define _RNDV_TDRIVER_LDR_PARS_HPP_ 1

class ComponentDataBase;


class TaskDriverInterfaceFactory_LDR : 
	public TaskDriverInterfaceFactory
{
	friend class TaskDriverInterface_LDR;
	private:
		// We may access component_db(). 
		
		// As set by user: (ONLY USE FOR PARAM CODE!)
		int thresh_param_low, thresh_param_high;
		// todo_thresh_low, todo_thresh_high in TaskDriverInterface. 
		
		struct DTPrm
		{
			// Execution timeout or -1: 
			long timeout;
		} prm[_DTLast];
		
		
		int _SetUpParams(TaskDriverType dtype,Section *top);
		int _RegisterParams();
		
		// Overriding virtuial from ParameterConsumer: 
		int CheckParams();
		// Overriding virtuial from TaskSourceFactory: 
		int FinalInit();  // called after CheckParams(), before Create()
	public: _CPP_OPERATORS_FF
		TaskDriverInterfaceFactory_LDR(ComponentDataBase *cdb,int *failflag=NULL);
		~TaskDriverInterfaceFactory_LDR();
		
		// Called on program start to set up the TaskDriverInterfaceFactory_LDR. 
		// TaskDriverInterfaceFactory_LDR registers at ComponentDataBase. 
		// Return value: 0 -> OK; >0 -> error. 
		static int init(ComponentDataBase *cdb);
		
		// Create a TaskDriverInterface_LDR: (Call after FinalInit())
		TaskDriverInterface *Create();
};

#endif  /* _RNDV_TDRIVER_LDR_PARS_HPP_ */

/*
 * tdfactory.hpp
 * 
 * Task driver interface factory class. 
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


#ifndef _RNDV_TASKDRIVERINTERFACEFACTORY_HPP_
#define _RNDV_TASKDRIVERINTERFACEFACTORY_HPP_ 1

//#include "../database.hpp"
class ComponentDataBase;

#include <lib/taskmanagement.h>


// Linked list hold by ComponentDataBase. 
class TaskDriverInterfaceFactory : 
	public LinkedListBase<TaskDriverInterfaceFactory>,
	public par::ParameterConsumer_Overloaded
{
	protected:
		ComponentDataBase *_component_db;
		
		// "local", "ldr" (<- server)
		const char *tdif_name;
		
	public:  _CPP_OPERATORS_FF
		TaskDriverInterfaceFactory(const char *_ts_name,
			ComponentDataBase *cdb,int *failflag=NULL);
		virtual ~TaskDriverInterfaceFactory();
		
		ComponentDataBase *component_db()
			{  return(_component_db);  }
		
		// Return value: 0 -> OK; else error 
		// Called after CheckParams(), before Create(). 
		virtual int FinalInit() HL_PureVirt(1)
		
		// Called on program start to set up the TaskDriverInterfaceFactory 
		// classes, so that they register at the ComponentDataBase. 
		// Return value: 0 -> OK; >0 -> failed. 
		static int init_factories(ComponentDataBase *cdb);
		
		// Create a TaskSource: 
		virtual TaskDriverInterface *Create() HL_PureVirt(NULL)
		
		// Get name and description string: 
		const char *DriverInterfaceName() const
			{  return(tdif_name);  }
		virtual const char *DriverInterfaceDesc() const HL_PureVirt(NULL)
};

#endif  /* _RNDV_TASKDRIVERINTERFACEFACTORY_HPP_ */

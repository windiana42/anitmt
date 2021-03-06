/*
 * tsfactory.hpp
 * 
 * Task source factory class. 
 * 
 * Copyright (c) 2002--2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#ifndef _RNDV_TASKSOURCEFACTORY_HPP_
#define _RNDV_TASKSOURCEFACTORY_HPP_ 1

//#include "../database.hpp"
class ComponentDataBase;

#include <lib/taskmanagement.h>


// Linked list hold by ComponentDataBase. 
class TaskSourceFactory : 
	public LinkedListBase<TaskSourceFactory>,
	public par::ParameterConsumer_Overloaded
{
	protected:
		ComponentDataBase *_component_db;
		
		// "local", "nrp" 
		const char *ts_name;
		
	public:  _CPP_OPERATORS_FF
		TaskSourceFactory(const char *_ts_name,
			ComponentDataBase *cdb,int *failflag=NULL);
		virtual ~TaskSourceFactory();
		
		ComponentDataBase *component_db()
			{  return(_component_db);  }
		
		// Return value: 0 -> OK; else error 
		// Called after CheckParams(), before Create(). 
		virtual int FinalInit() HL_PureVirt(1)
		
		// Called on program start to set up the TaskSourceFactory classes, 
		// so that they register at the ComponentDataBase. 
		// Return value: 0 -> OK; >0 -> failed. 
		static int init_factories(ComponentDataBase *cdb);
		
		// Create a TaskSource: 
		virtual TaskSource *Create() HL_PureVirt(NULL)
		
		// Get name and description strings: 
		const char *TaskSourceName() const
			{  return(ts_name);  }
		virtual const char *TaskSourceDesc() const HL_PureVirt(NULL)
};

#endif  /* _RNDV_TASKSOURCEFACTORY_HPP_ */

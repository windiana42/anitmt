/*
 * taskmanager.hpp
 * 
 * Task manager class header. 
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

#ifndef _RNDV_TASKMANAGER_HPP_
#define _RNDV_TASKMANAGER_HPP_ 1

// Well that is pretty much, but we won't need more: 
#include "database.hpp"
#include "tsource/tasksource.hpp"


class TaskManager : 
	public FDBase, 
	public ProcessBase,
	public TaskSourceConsumer
{
	private:
		ComponentDataBase *component_db;
		TaskSource *tasksource;
		
		// Note all existing TaskDrivers are in this queue: 
		LinkedList<TaskDriver> tasklist;
		
		void _StartProcessing();
		
		// Overriding virtual from TaskSourceConsumer: 
		int tsnotify(TSNotifyInfo *);
		// Overriding virtual from FDBase: 
		int signotify(const SigInfo *);
		int timernotify(TimerInfo *);
	public: _CPP_OPERATORS_FF
		TaskManager(ComponentDataBase *cdb,int *failflag=NULL);
		~TaskManager();
		
		// These are called by the constructor/destructor of TaskDriver: 
		// Return value: 0 -> OK; !=0 -> failed
		int RegisterTaskDriver(TaskDriver *td);
		void UnregisterTaskDriver(TaskDriver *td);
		
		// Called by TaskDriver when ever estat/esdetail changed. 
		void StateChanged(TaskDriver *td);
		// Called by TaskDriver when he is done and wants to get deleted: 
		void IAmDone(TaskDriver *td);
};

#endif  /* _RNDV_TASKMANAGER_HPP_ */

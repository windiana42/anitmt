/*
 * taskmanager.cpp
 * 
 * Task manager class implementation. 
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


#include "taskmanager.hpp"

#include <assert.h>


// Called by TaskDriver when ever estat/esdetail changed. 
void TaskManager::StateChanged(TaskDriver *)
{

}

// Called by TaskDriver when he is done and wants to get deleted: 
void TaskManager::IAmDone(TaskDriver *td)
{
	if(!td)  return;
	
	// Okay, must connect to the task source and tell her that 
	// the task is done. 
	#warning OOO!!! Must also pass success/error code!
	int rv=TSConnect();
	assert(rv==0);
}


#warning FIXME: quick hack: 
static CompleteTask *ctr=NULL;

int TaskManager::tsnotify(TSNotifyInfo *ni)
{
	#warning FIXME: quick hack: 
	switch(ni->action)
	{
		case ANone:  assert(0);  break;
		case AConnect:
		{
			assert(ni->connstat==CSConnected);
			if(ctr)
			{
				int rv=TSDoneTask(ctr);
				assert(rv==0);
			}
			else
			{
				int rv=TSGetTask();
				assert(rv==0);
			}
			break;
		}
		case ADoneTask:
		{
			assert(ni->donestat==DTSOkay);
			assert(ctr);
			ctr=NULL;
			
			// ...and also delete the task driver. 
			// This is the first one in the list: 
			tasklist.first()->DeleteMe();  // will unregister
			
			int rv=TSGetTask();
			assert(rv==0);
			break;
		}
		case AGetTask:
		{
			if(ni->getstat==GTSNoMoreTasks)
			{
				fprintf(stderr,"--No more tasks--\n");
				ctr=NULL;  // to be sure. 
			}
			else
			{
				assert(ni->getstat==GTSGotTask);
				ctr=ni->ctask;
				assert(ctr);
			}
			// Now, must disconnect. 
			int rv=TSDisconnect();
			assert(rv==0);
			break;
		}
		case ADisconnect:
		{
			assert(ni->disconnstat==DSOkay);
			
			// Actually start renderer: 
			if(ctr)
			{
				TaskDriverFactory *tdf=ctr->rt->rdesc->dfactory;
				assert(tdf);
				
				TaskDriver *td=tdf->Create();
				assert(td);
				
				#warning Passing NULL as TaskParams is a VERY BAD IDEA!
				int rv=td->Run(ctr->rt,NULL);
				if(rv)
				{
					fprintf(stderr,"RUN returned %d\n",rv);
					abort();
				}
			}
			else
			{
				// Quit. 
				fdmanager()->Quit(0);
			}
			
			break;
		}
	}
	
	return(0);
}


int TaskManager::signotify(const SigInfo *si)
{
	if(si->info.si_signo==SIGINT)
	{
		fprintf(stderr,"Caught SIGINT. Aborting for now... (FIXME)\n");
		fdmanager()->Quit(1);
	}
	
	return(0);
}


int TaskManager::timernotify(TimerInfo *ti)
{
	if(!tasksource)
	{
		// We're in the 0msec timer installed at the constuctor to get 
		// things running: 
		KillTimer(ti->tid);
		_StartProcessing();
		return(0);
	}
	
	assert(0);
	
	return(0);
}


// Called via timernotify() after everything has been set up and 
// as the first action in the main loop. Used to get the TaskSource 
// and begin actually doing something...
void TaskManager::_StartProcessing()
{
	assert(tasksource==NULL);  // yes, must be NULL here. 
	
	#warning fixme: this is a quick hack...
	Verbose("Choosing local task source...");
	TaskSourceFactory *tsf=component_db->FindSourceFactoryByName("local");
	assert(tsf);
	
	tasksource=tsf->Create();
	assert(tasksource);
	
	UseTaskSource(tasksource);
	Verbose("OK\n");
	
	int rv=TSConnect();
	assert(rv==0);
}


// These are called by the constructor/destructor of TaskDriver: 
int TaskManager::RegisterTaskDriver(TaskDriver *td)
{
	if(!td)  return(0);
	
	// Check if queued: 
	if(tasklist.prev(td) || td==tasklist.first())  return(1);
	
	tasklist.append(td);
	
	
	return(0);
}

void TaskManager::UnregisterTaskDriver(TaskDriver *td)
{
	if(!td)  return;
	// Check if queued: 
	if(!tasklist.prev(td) && td!=tasklist.first())  return;
	
	// Check state...
	if(td->estat==TaskDriver::ESNone)
	{
		// Well, seems this one did not do anything. So 
		// there should not be a problem. 
		return;
	}
	//if(td->estat, td->esdetail, td->pingo)
	#warning missing..
	
	
	// Dequeue task: 
	tasklist.dequeue(td);
}


TaskManager::TaskManager(ComponentDataBase *cdb,int *failflag) :
	FDBase(failflag),
	ProcessBase(failflag),
	TaskSourceConsumer(failflag),
	tasklist(failflag)
{
	component_db=cdb;
	component_db->_SetTaskManager(this);
	tasksource=NULL;
	
	int failed=0;
	
	//if(FDBase::SetManager(1))
	//{  ++failed;  }
	
	// Install 0msec timer to start the stuff: 
	if(!InstallTimer(0,-1))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskMan");  }
}


TaskManager::~TaskManager()
{
	// Make sure there are no tasks left: 
	assert(tasklist.is_empty());
	
	UseTaskSource(NULL);
	if(tasksource)
	{  delete tasksource;  tasksource=NULL;  }
}

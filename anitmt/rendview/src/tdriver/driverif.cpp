/*
 * dif_local.cpp
 * 
 * Local task driver interface for task manager (virtualisation). 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
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


TaskDriverInterface::TaskDriverInterface(ComponentDataBase *cdb,int *failflag)
{
	_comp_db=cdb;
	
	todo_thresh_low=-1;  // initial value
	todo_thresh_high=-1;
}

TaskDriverInterface::~TaskDriverInterface()
{
}

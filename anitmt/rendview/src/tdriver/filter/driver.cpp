/*
 * driver.cpp
 * 
 * Filter driver interface: implementation. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

//#include "../../database.hpp"
//#include "filter.hpp"

#include <tsource/taskfile.hpp>
#include <tsource/tasksource.hpp>

#include <assert.h>

#include <fcntl.h>


// Open input/output file. 
// dir: direction: -1 -> input; +1 -> output 
// Return value: 
//    >=0 -> valid FD
//     -1 -> file or file->str() NULL or dir==0
//     -2 -> open( failed (see errno)
int FilterDriver::OpenIOFile(RefString *file,int dir)
{
	#warning should be moved to lib/; then remove the "#include <fcntl.h>"
	if(!file || !file->str())
	{  return(-1);  }
	
	int fd=-1;
	if(dir<0)  // Open for input: 
	{  fd=open(file->str(),O_RDONLY);  }
	else if(dir>0)  // Open for output: 
	{  fd=open(file->str(),O_WRONLY | O_CREAT | O_TRUNC,0666);  }
	else
	{  return(-1);  }
	
assert(0);
	if(fd<0)
	{
		int tmp=errno;
		
		#warning report error
		
		errno=tmp;
		return(-2);
	}
	
	return(fd);
}


int FilterDriver::Run(
	const TaskStructBase *tsb,
	const TaskParams *tp)
{
	assert(tsb);
	assert(tsb->dtype==DTFilter);
	assert(!tp || tp->dtype==DTFilter);
	
	FilterTask *tsk=(FilterTask*)tsb;
	FilterTaskParams *tskp=(FilterTaskParams*)tp;
	
	return(Execute(tsk,tskp));
}


FilterDriver::FilterDriver(TaskDriverFactory *df,TaskDriverInterface_Local *tdif,
	int *failflag) : 
	TaskDriver(df,tdif,failflag)
{
	/*int failed=0;
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("FilterDriver");  }*/
}


FilterDriver::~FilterDriver()
{
	// ~TaskDriver() unregisteres us. 
}


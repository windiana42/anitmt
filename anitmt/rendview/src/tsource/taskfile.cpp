/*
 * taskfile.cpp
 * 
 * Task file class; holding any file used by rendview. 
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


#include "taskfile.hpp"

#include <stdio.h>
#include <stdlib.h>


void TaskFile::_forbidden()
{
	fprintf(stderr,"Attempt to copy TaskFile.\n");
	abort();
}


TaskFile::TaskFile(FType ft,IOType iot,int *failflag=NULL) : 
	hdpath(failflag)
{
	ftype=ft;
	iotype=iot;
	
	// Register at file data base: 
}


TaskFile::~TaskFile()
{
	ftype=FTNone;
	iotype=IOTNone;
	
	// Unregister at file database: 
}


/*
 * get_load.cpp
 * Get the system load. 
 * Currenly this works with linux. 
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

#include "prototypes.hpp"
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


// Returns the system load multiplied with 100: 
// Returns -1 on systems where that is not supported. 
int GetLoadValue()
{
	static const char *load_src="/proc/loadavg";
	static const char *err_msg="Failed to %s %s (%s). "
		"No load control feature available.\n";
	static int feature_not_avail=0;
	
	if(feature_not_avail)
	{  return(-1);  }
	
	int fd;
	int lvi=-1;
	do {
		// Open load source: 
		fd=open(load_src,O_RDONLY);
		if(fd<0)
		{  Warning(err_msg,"open",load_src,strerror(errno));  break;  }
		
		// Read string: 
		char buf[64];
		ssize_t rd=read(fd,buf,64-1);
		if(rd<0)
		{  Warning(err_msg,"read",load_src,strerror(errno));  break;  }
		assert(rd<=64-1);
		
		// Parse load val. We use the first value as it responds quickest. 
		buf[rd]='\0';
		char *end;
		double lv=strtod(buf,&end);
		lvi=int(lv*100.0+0.5);
		if(end<=buf || lvi<0)
		{  Warning(err_msg,"parse","load value",load_src);  lvi=-1;  break;  }
	}  while(0);
	
	if(fd>=0)  close(fd);
	if(lvi<0)
	{  feature_not_avail=1;  return(-1);  }
	
	return(lvi);
}

/*
 * get_ncpus.cpp
 * Get the number of CPUs in a system. 
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
#include <ctype.h>
#include <string.h>
#include <assert.h>


// mhz_sum: sum of all CPU MHz values 
//    Returns <0 on error. 
// Return value: 
// <=0 -> could not be detected
// >=1 -> number of CPUs. 
static int _GetNumberOfCPUs(double *mhz_sum)
{
	FILE *fp=fopen("/proc/cpuinfo","r");
	if(!fp)
	{  return(-1);  }
	
	int ncpu=0;
	if(mhz_sum)
	{  *mhz_sum=0.0;  }
	
	int bufsz=256;
	char buf[bufsz];
	
	for(;;)
	{
		if(!fgets(buf,bufsz,fp))
		{
			if(!feof(fp))  ncpu=-2;
			break;
		}
		int len=strlen(buf);
		if(len+1>=bufsz)
		{  ncpu=-3;  break;  }
		char *line=buf;
		int info=-1;
		if(!strncmp(line,"processor",9))
		{  info=0;  line+=9;  }
		else if(!strncmp(line,"cpu MHz",7))
		{  info=1;  line+=7;  }
		if(info<0)  continue;
		while(isspace(*line))  ++line;
		if(*line!=':')
		{  ncpu=-4;  break;  }
		switch(info)
		{
			case 0:  // processor
				++ncpu;
				break;
			case 1:
			{
				if(!mhz_sum)  break;
				char *end;
				double v=strtod(line+1,&end);
				if(isspace(*end) || !(*end))
				{  *mhz_sum+=v;  }
				else
				{  *mhz_sum=-1.0;  mhz_sum=NULL;  }
			}  break;
			default: assert(0); break;
		}
	}
	
	fclose(fp);
	return(ncpu);
}


// Returns number of CPUs; assumes one CPU is detection fails. 
int GetNumberOfCPUs()
{
	double mhz_sum;
	int ncpu=_GetNumberOfCPUs(&mhz_sum);
	if(ncpu>0)
	{
		Verbose("Okay, system has %d CPU%s (%s%.0f MHz)\n",
			ncpu,(ncpu==1) ? "" : "s",
			(ncpu==1) ? "" : "sum: ",mhz_sum);
	}
	else
	{  Verbose("Unable to detect number of CPUs. Assuming 1.\n");  }
	return(ncpu>0 ? ncpu : 1);
}

/*
 * get_ncpus.cpp
 * Get the number of CPUs in a system. 
 * Currenly this works with linux and all systems providing 
 * sysconf(). 
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


#if HAVE_SYSCONF
static int NCPUsBySysconf()
{
	const char *scf="Warning: sysconf(%s) failed.\n";
	long n=sysconf(_SC_NPROCESSORS_ONLN);
	if(n<0)
	{  Warning(scf,"NPROCESSORS_ONLN");  }
	
	long nconf=sysconf(_SC_NPROCESSORS_CONF);
	if(nconf<0)
	{  Warning(scf,"NPROCESSORS_CONF");  }
	
	if(n<0 && nconf<0)  return(-1);
	if(n<0)      n=nconf;
	if(nconf<0)  nconf=n;
	
	if(n!=nconf)
	{  Warning("System reports %d CPUs but %s%d are online.\n",
		int(nconf),(n<nconf ? "only " : ""),int(n));  }
	else if(!n)
	{  Warning("System reports 0 CPUs online (quite few, eh?)\n");  }
	
	return((n<=0) ? (-1) : int(n));
}
#endif


// Returns number of CPUs; assumes one CPU and warn if detection fails. 
int GetNumberOfCPUs()
{
	int n=-1;
	do {
#if HAVE_SYSCONF
		n=NCPUsBySysconf();
		if(n>0)  break;
#endif
		// Insert more checks here...
		
	}  while(0);
	
	if(n<=0)
	{
		Warning("Unable to detect number of CPUs. Assuming 1.\n");
		n=1;
	}
	else
	{  Verbose(MiscInfo,"Okay, system has %d CPU%s.\n",n,n>1 ? "s" : "");  }
	
	return(n);
}


#if 0
/* OLD CODE, parsing /proc/cpuinfo. */

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
#endif

/*
 * procstring.cc
 * 
 * Contains useful strings for error reporting related to ProcessManager 
 * and ProcessBase. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#define HLIB_IN_HLIB 1
#include <hlib/htime.h>
#include "fdmanager.h"
#include "fdbase.h"
#include "procmanager.h"
#include "procbase.h"


const char *ProcessBase::PSDetail_SyscallString(PSDetail x)
{
	switch(x)
	{
		case PSExecFailed:      return("execve");
		case PSDupFailed:       return("dup2");
		case PSChrootFailed:    return("chroot");
		case PSChdirFailed:     return("chdir");
		case PSNiceFailed:      return("nice");
		case PSSetSidFailed:    return("setsid");
		case PSSetGidFailed:    return("setgid");
		case PSSetUidFailed:    return("setuid");
		case PSFunctionFailed:  return("[function]");
		case PSUnknownError:    return("[unknown]");
		default:;  // against warning
	}
	return("???");
}


int ProcessBase::StartProcessErrorString(pid_t rv,char *buf,size_t len)
{
	ssize_t srv=0;
	char *det=NULL;
	if(rv<0) switch(rv)
	{
		case SPS_PathAllocFailed:  det="path";          goto afail;
		case SPS_ArgAllocFailed:   det="arg";           goto afail;
		case SPS_EvnAllocFailed:   det="environment";   goto afail;
		case SPS_LMallocFailed:    det="generic";            afail:;
			srv=snprintf(buf,len,"%s allocation failure",det);
			break;
		case SPS_AccessFailed:
			srv=snprintf(buf,len,"binary to execute not found");
			break;
		case SPS_ForkFailed:       det="fork";    goto sfail;
		case SPS_PipeFailed:       det="pipe";    goto sfail;
		case SPS_FcntlPipeFailed:  det="fcntl(pipe)";  sfail:;
			srv=snprintf(buf,len,"while calling %s: %s",det,strerror(errno));
			break;
		case SPS_IllegalFlags:      det="flags";  goto ifail;
		case SPS_SearchPathError:   det="spath";  goto ifail;
		case SPS_ArgListError:      det="args";        ifail:;
			srv=snprintf(buf,len,"internal error (%s)",det);
			break;
		case SPS_ProcessLimitExceeded:  
			srv=snprintf(buf,len,"process limit exceeded");
			break;
		default:
			srv=snprintf(buf,len,"internal error %d (BUG!)\n",rv);
			break;
	}
	else
	{  srv=snprintf(buf,len,"successfully forked (PID %ld)\n",long(rv));  }
	
	return(srv>=ssize_t(len) ? 1 : 0);
}


int ProcessBase::ProcessStatusString(const ProcStatus *ps,char *buf,size_t len)
{
	ssize_t srv=-10000;
	switch(ps->action)
	{
		case PSFailed:
		{
			const char *det=PSDetail_SyscallString(ps->detail);
			if(ps->detail==PSUnknownError || *det=='?')
			{  srv=snprintf(buf,len,"unknown error (%d)",ps->detail);  }
			else if(ps->detail==PSFunctionFailed)
			{  srv=snprintf(buf,len,"custom function failed with code %d",
				ps->estatus);  }
			else
			{  srv=snprintf(buf,len,"while calling %s: %s",
				det,strerror(ps->estatus));  }
		}  break;
		case PSDead:
			switch(ps->detail)
			{
				case PSExited:
					if(ps->estatus)
					{  srv=snprintf(buf,len,"exited with non-zero code %d",
						ps->estatus);  }
					else
					{  srv=snprintf(buf,len,"exited successfully");  }
					break;
				case PSKilled:  // fall through
				case PSDumped:
					srv=snprintf(buf,len,"was killed by signal %d%s",
						ps->estatus,
						ps->detail==PSDumped ? " (core dumped)" : "");
					break;
				default:;  // against warning
			}
			break;
		case PSStopCont:
			srv=snprintf(buf,len,"%s (signal %d)",
				ps->detail==PSStop ? "was stopped" : "continued",
				ps->estatus);
			break;
		case PSExecuted:
			srv=snprintf(buf,len,"is now running");
			break;
	}
	if(srv==-10000)
	{  srv=snprintf(buf,len,"[illegal code %d,%d (BUG!)]",
		ps->action,ps->detail);  }
	
	return(srv>=ssize_t(len) ? 1 : 0);
}

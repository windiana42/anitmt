/*
 * param.cpp
 * 
 * Implementation of parameter & factory class of LDR task source. 
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

#include <lib/myaddrinfo.hpp>

#include "ldr.hpp"
#include "param.hpp"

#include <assert.h>
#include <ctype.h>

#include <lib/ldrproto.hpp>



using namespace LDR;

#define UnsetNegMagic  (-29659)

		
const char *TaskSourceFactory_LDR::TaskSourceDesc() const
{
	return("Use the LDR task source to let rendview act as LDR client "
		"getting frames to process from an LDR server.");
}


static void _CheckAlloc(int x)
{
	if(x)
	{  Error("%s.\n",cstrings.allocfail);  abort();  }
}


// Create a LDR TaskSource (TaskSource_LDR): 
TaskSource *TaskSourceFactory_LDR::Create()
{
	return(NEW1<TaskSource_LDR>(this));
}


int TaskSourceFactory_LDR::CreateWorkingDir()
{
	int failed=0;
	
	// See if -wdir was used: 
	working_in_wdir=0;
	if(IsSet(work_chdir_pi))
	{  working_in_wdir=1;  }
	if(work_chdir.str() && !strcmp(work_chdir.str(),"none"))
	{  working_in_wdir=0;  }
	if(working_in_wdir)
	{
		pid_t mypid=getpid();
		int autoname=0;
		if(work_chdir.str() && work_chdir.str()[0]=='\0')
		{  work_chdir.deref();  }
		if(!work_chdir)
		{
			_CheckAlloc(work_chdir.sprintf(0,
				"/tmp/ldrclient-%d",int(mypid)) );
			autoname=1;
		}
		
		if(work_chdir.str()[0]!='/')
		{
			Error("Please use absolute path for -wdir.\n");
			working_in_wdir=0;
			return(1);
		}
		
		Verbose(TSI,"LDR client: Working in ...\b\b\b");
		
		#define DIRMODE 0700
		int rv=mkdir(work_chdir.str(),DIRMODE);
		int errno0=errno;
		RefString name0(work_chdir);
		if(rv<0 && errno==EEXIST && autoname)
		{
			for(int i=0; i<10; i++)
			{
				_CheckAlloc(work_chdir.sprintf(0,
					"/tmp/ldrclient-%d-%d",int(mypid),i) );
				rv=mkdir(work_chdir.str(),DIRMODE);
				if(!rv || errno!=EEXIST)  break;
			}
		}
		if(rv)
		{
			Verbose(TSI,"\"%s\" [failure]\n",name0.str());
			Error("Failed to create directory \"%s\" (-wdir): %s\n",
				name0.str(),strerror(errno0));
			++failed;
		}
		
		if(!failed)
		{
			Verbose(TSI,"\"%s\"...\b\b\b",work_chdir.str());
			
			if(chdir(work_chdir.str())<0)
			{
				errno0=errno;
				Verbose(TSI,"[failure]\n");
				Error("Failed to chdir into \"%s\" (-wdir): %s\n",
					work_chdir.str(),strerror(errno0));
				++failed;
				
				if(rmdir(work_chdir.str())<0)
				{
					Error("Failed to rmdir \"%s\" (-wdir): %s\n",
						work_chdir.str(),strerror(errno0));
					++failed;
				}
			}
		}
		
		if(!failed)
		{
			Verbose(TSI," OK.\n");
			working_in_wdir=2;
		}
		else
		{  working_in_wdir=0;  }
	}
	return(failed);
}

int TaskSourceFactory_LDR::DeleteWorkingDir()
{
	if(working_in_wdir)
	{
		working_in_wdir=0;
		// Must delete working dir. 
		chdir("..");  // <-- in case the next one fails
		chdir("/");
		if(rmdir(work_chdir.str()))
		{
			Error("Failed to delete temporary working dir \"%s\": %s\n",
				work_chdir.str(),strerror(errno));
			return(1);
		}
		else
		{  Verbose(TSI,"LDR task sorce: Deleted temp working dir \"%s\".\n",
			work_chdir.str());  }
	}
	return(0);
}


// Return string representation of server net: 
RefString TaskSourceFactory_LDR::ServerNetString(
	TaskSourceFactory_LDR::ServerNet *sn)
{
	char bits_tmp[32];
	if(sn->mask==0xffffffffU)
	{  strcpy(bits_tmp,"[single]");  }
	else
	{
		// I simply count bits here. 
		int mbits=0;
		u_int32_t tmp=sn->mask;
		while(tmp)
		{
			if(tmp&1)  ++mbits;
			tmp>>=1;
		}
		snprintf(bits_tmp,32,"[mask: %d]",mbits);
	}
	RefString tmp;
	tmp.sprintf(0,"%s (%s) %s",
		sn->name.str(),
		sn->adr.GetAddress(/*with_port=*/0).str(),
		bits_tmp);
	return(tmp);
}


// If "", unset; append "/" if needed. 
static void _PathFixupAppendSlash(RefString *s)
{
	if(!s->len())
	{  s->set(NULL);  return;  }
	if(s->str()[s->len()-1]!='/')
	{  _CheckAlloc(s->append("/"));  }
}


int TaskSourceFactory_LDR::FinalInit()
{
	int failed=0;
	
	// We need to do a little test for padding issues...
	LDRCheckCorrectLDRPaketSizes();
	
	if(timestamp_thresh==-0x7ffffffe)  // i.e. "unset"
	{
		if(server_time_correction_enabled)
		{  timestamp_thresh=-1;  }
		else
		{
			timestamp_thresh=-3001;
			Warning("LDR: Server time correction disabled.\n");
		}
	}
	
	if(CreateWorkingDir())
	{  ++failed;  }
	
	// Resolve the hosts (in server net list): 
	if(!failed && !server_net_list.is_empty())
	{
		Verbose(TSP,"Looking up server networks...\n");
		for(ServerNet *sn=server_net_list.first(); sn; sn=sn->next)
		{
			int rv=sn->adr.SetAddressError(sn->name.str(),/*port=*/0);
			if(rv)
			{  ++failed;  continue;  }
		}
		
		// Check for duplicate entries: 
		for(ServerNet *_sn=server_net_list.first(); _sn; )
		{
			ServerNet *sn=_sn;  _sn=_sn->next;
			int dup=0;
			for(ServerNet *_i=sn->next; _i; )
			{
				ServerNet *i=_i;  _i=_i->next;
				// We could be more sophisticated here and detect if 
				// two server net specs overlap... 
				if(!(i->adr==sn->adr) || i->mask!=sn->mask)  continue;
				delete server_net_list.dequeue(i);
				++dup;
			}
			if(dup)
			{  Warning("Removed duplicate server net entry %s (%s)\n",
				sn->name.str(),sn->adr.GetAddress(/*with_port=*/0).str());  }
		}
		
		// Do simple sanity check if net address & ~mask is 0: 
		for(ServerNet *sn=server_net_list.first(); sn; sn=sn->next)
		{
			// To explain this warning: 
			// Say you have the 24-bit subnet 192.168.1.0/24. 
			// If you specify 192.168.1.0/24 as server net, then all is 
			//   okay. 
			// If you specify the host 192.168.1.17/24, then his is the 
			//   same as when spefifying the net (...1.0) but you get this 
			//   warning. 
			if(sn->adr.CheckNetBitsZero(sn->mask))
			{  Warning("Server net entry %s looks like host, using net.\n",
				ServerNetString(sn).str());  }
		}
	}
	
	// Okay, do most checks before asking the user for the password...
	_PathFixupAppendSlash(&radd_path);
	_PathFixupAppendSlash(&fadd_path);
	
	if(!failed)
	{
		LDRGetPassIfNeeded(&password,"Enter client password: ",NULL);
		if(!password.str())
		{  Warning("LDR client: no server password specified "
			"(authentification disabled).\n");  }
	}
	
	// Okay, LDR task source listenes to port listen_port: 
	// Actually bind to that port: 
	do {
		if(failed)  break;
		failed=1;
		
		MyAddrInfo addr;
		addr.SetPassiveAny(listen_port);
		
		// Open socket: 
		listen_fd=addr.socket();
		if(listen_fd<0)
		{  Error("Failed to create inet socket: %s\n",
			strerror(errno));  break;  }
		
		// Bind socket to port: 
		if(addr.bind(listen_fd))
		{  Error("Failed to bind to port %d (for LDR): %s\n",addr.GetPort(),
			strerror(errno));  break;  }
		
		// Say we want connections to that port: 
		if(MyAddrInfo::listen(listen_fd,/*backlog=*/2))
		{  Error("Failed to listen to port %d (for LDR): %s\n",addr.GetPort(),
			strerror(errno));  break;  }
		
		// We want non-blocking IO, right?
		if(SetNonblocking(listen_fd))
		{  Error("Failed to set nonblocking IO for listen socket: %s",
			strerror(errno));  break;  }
		
		// Successfully done. 
		failed=0;
	} while(0);
	if(failed && listen_fd>0)
	{  MyAddrInfo::close(listen_fd);  listen_fd=-1;  }
	
	if(!failed)
	{
		Verbose(TSI,"LDR task source: Listening on port %d (procotol version %d)\n",
			listen_port,LDRProtocolVersion);
		Verbose(TSI,"  Server authentification (password): %s\n",
			password.str() ? "enabled" : "disabled");
		Verbose(TSI,"  Allowed server nets:%s\n",
			server_net_list.is_empty() ? " [all]" : "");
		for(ServerNet *sn=server_net_list.first(); sn; sn=sn->next)
		{  Verbose(TSI,"    %s\n",ServerNetString(sn).str());  }
		
		char attmp[24],katmp[16];
		if(auth_timeout<0)  strcpy(attmp,"[disabled]");
		else  snprintf(attmp,24,"%ld msec",auth_timeout);
		if(keepalive_mult<0)  strcpy(katmp,"[disabled]");
		else  snprintf(katmp,24,"%d.%d",keepalive_mult/10,keepalive_mult%10);
		Verbose(TSI,"  Auth timeout: %s;    keepalive mult: %s\n",attmp,katmp);
		
		{
			RefString wdir;
			int rv=GetCurrentWorkingDir(&wdir);
			Verbose(TSI,"  Working directory: %s%s\n",
				(rv==-1) ? cstrings.allocfail : wdir.str(),
				working_in_wdir ? " [created]" : "");
		}
		
		Verbose(TSI,"  Transferring files:\n"
			"    Render:  source: %-6s   dest: %s     additional: %s\n"
			"    Filter:                   dest: %s     additional: %s\n",
			transfer.render_src ? 
				(always_download_rf_in ? "always" : "yes") : "no",
			transfer.render_dest ? "yes" : "no ",
			transfer.r_additional  ? "yes" : "no",
			transfer.filter_dest ? "yes" : "no ",
			transfer.f_additional  ? "yes" : "no");
		Verbose(TSI,"  Deleting temp files:\n"
			"           | Input           | Output          | Additional\n"
			"    Render | %-15s | %-15s | %-15s\n"
			"    Filter | %-15s | %-15s | %-15s\n",
			TaskFile::DeleteSpecString(rin_delspec),
			TaskFile::DeleteSpecString(rout_delspec),
			TaskFile::DeleteSpecString(radd_delspec),
			TaskFile::DeleteSpecString(fin_delspec),
			TaskFile::DeleteSpecString(fout_delspec),
			TaskFile::DeleteSpecString(fadd_delspec));
		
		Verbose(TSI,"  Re-dload: %s;  time corr: %s;  TS thresh: %ld msec\n",
			re_download_enabled ? "yes" : "no",
			server_time_correction_enabled ? "yes" : "no",
			timestamp_thresh);
	}
	
	return(failed ? 1 : 0);
}


int TaskSourceFactory_LDR::CheckParams()
{
	int failed=0;
	
	if(listen_port==UnsetNegMagic)
	{  listen_port=DefaultLDRPort;  }
	
	if(listen_port<=0 || listen_port>65535)
	{
		Error("LDR: Illegal port number %d\n",listen_port);
		++failed;
	}
	
	if(auth_timeout<=0)
	{  auth_timeout=-1;  }
	if(keepalive_mult<=0)
	{  keepalive_mult=-1;  }
	
	if(keepalive_mult<12 && keepalive_mult>=0)
	{  Warning("LDR keepalive multiplicator (-kamult) very small (%d -> %d.%d)\n",
		keepalive_mult,keepalive_mult/10,keepalive_mult%10);  }
	
	if(transfer_spec_str.str())
	{
		const char *s=transfer_spec_str.str();
		int f_error=0;
		while(*s)
		{
			const char *e=s;
			int mode=0;
			while(isalpha(*e))  ++e;
			
			if(*e=='+')  mode=1;
			else if(*e=='-')  /*mode=0*/;
			else
			{  ++f_error;  break;  }
			
			do {
				if(e-s==1)
				{
				 	if(*s=='d')
					{
						transfer.render_dest=mode;
						transfer.filter_dest=mode;
						break;
					}
				 	else if(*s=='a')
					{
						transfer.r_additional=mode;
						transfer.f_additional=mode;
						break;
					}
				}
				else if(e-s==2)
				{
					if(*s=='r')
					{
						if(s[1]=='s')
						{  transfer.render_src=mode;  break;  }
						else if(s[1]=='d')
						{  transfer.render_dest=mode;  break;  }
						else if(s[1]=='a')
						{  transfer.r_additional=mode;  break;  }
					}
					else if(*s=='f')
					{
						if(s[1]=='d')
						{  transfer.filter_dest=mode;  break;  }
						else if(s[1]=='a')
						{  transfer.f_additional=mode;  break;  }
					}
				}
				else if(e-s==3)
				{
					if(*s=='a' && s[1]=='l' && s[2]=='l')
					{
						transfer.render_src=mode;
						transfer.render_dest=mode;
						transfer.filter_dest=mode;
						transfer.r_additional=mode;
						transfer.f_additional=mode;
						break;
					}
				}
				++f_error;  goto breakboth;
			} while(0);
			
			s=e+1;
		}
		breakboth:;
		if(f_error)
		{
			Error("Invalid transfer spec \"%s\".\n",transfer_spec_str.str());
			++failed;
		}
	}
	
	// NOTE: Additional files which are to be deleted "when frame done" 
	//       are only deleted if they are no longer used by any task we 
	//       are doing and are goind to do concerning the current 
	//       task queue. 
	//  (More exactly: they are destroyed when the InternalTaskFile record 
	//   gets destroyed and that is the case when there are no more refs 
	//   to the file in question.)
	
	// Change default: If we do not transfer the file, we do 
	// not delete it. 
	if(!transfer.render_src)
	{  rin_delspec=TaskFile::DontDel;  }
	if(!transfer.render_dest)
	{  rout_delspec=TaskFile::DontDel;  fin_delspec=TaskFile::DontDel;  }
	if(!transfer.filter_dest)
	{  fout_delspec=TaskFile::DontDel;  }
	if(!transfer.r_additional)
	{  radd_delspec=TaskFile::DontDel;  }
	if(!transfer.f_additional)
	{  fadd_delspec=TaskFile::DontDel;  }
	
	// Delete specs: MUST BE PARSED AFTER TRANSFER SPEC. 
	failed+=_ParseDeleteSpec(&rin_delstr,"r-delin",&rin_delspec);
	failed+=_ParseDeleteSpec(&rout_delstr,"r-delout",&rout_delspec);
	failed+=_ParseDeleteSpec(&radd_delstr,"r-deladd",&radd_delspec);
	failed+=_ParseDeleteSpec(&fin_delstr,"f-delin",&fin_delspec);
	failed+=_ParseDeleteSpec(&fout_delstr,"f-delout",&fout_delspec);
	failed+=_ParseDeleteSpec(&fadd_delstr,"f-deladd",&fadd_delspec);
	
	// Only patse the server nets here; look them up in FinialInit(). 
	if(server_net_str.str() && *server_net_str.str())
	{
		// Parse in server net spec: 
		#warning EXTEND THIS TO IPV6. 
		const char *str=server_net_str.str();
		for(;;)
		{
			// Skip whitespace: 
			while(isspace(*str))  ++str;
			if(!(*str))  break;
			// Okay, read in host name/IP: 
			const char *host=str;
			while(*str && *str!='/' && !isspace(*str))  ++str;
			// Alloc node: 
			ServerNet *sn=NEW<ServerNet>();
			_CheckAlloc(!sn);
			server_net_list.append(sn);
			// Copy serer name: 
			_CheckAlloc(sn->name.set0(host,str-host));;
			// Parse in mask (if any): 
			// mask=0xffffffffU here from the constructor. 
			if(*str=='/')
			{
				char *ptr=(char*)(str+1);
				long na = isdigit(*ptr) ? strtol(ptr,&ptr,10) : (-1);
				if(ptr<=str || na<0 || na>32)
				{
					if(na==-1)
					{  while(*ptr && !isspace(*ptr))  ++ptr;  }
					Error("Illegal server net spec \"%.*s\".\n",
						ptr-host,host);
					++failed;
				}
				else if(*ptr && !isspace(*ptr))
				{
					// Skip garbage: 
					while(*ptr && !isspace(*ptr))  ++ptr;
					// And report error (it's secutity issue here, so 
					// I expect that this is specified correctly. 
					Error("Garbage at end of server net spec \"%.*s\".\n",
						ptr-host,host);
					++failed;
				}
				else
				{
					// Set the first na bits in sn->mask (net order): 
					
					// Yes, this damn stuff is necessary because x<<32 will not 
					// alter x at all. 
					if(na==32)  sn->mask=0xffffffffU;
					else if(na==0)  sn->mask=0U;
					else
					{  sn->mask=htonl(
						u_int32_t(0xffffffffU) << u_int32_t(32-na));  }
				}
				str=ptr;
			}
		}
	}
	server_net_str.deref();
	
	return(failed ? 1 : 0);
}


int TaskSourceFactory_LDR::_ParseDeleteSpec(RefString *str,const char *optname,
	TaskFile::DeleteSpec *ds)
{
	const char *val=str->str();
	if(!val)  return(0);  /// keep default
	size_t vallen=strlen(val);
	if(vallen<=5 && !strncmp(val,"frame",vallen))  
	{  *ds=TaskFile::DelFrame;  }
	else if(vallen<=5 && !strncmp(val,"cycle",vallen))  
	{  *ds=TaskFile::DelCycle;  }
	else if(vallen<=4 && !strncmp(val,"exit",vallen))  
	{  *ds=TaskFile::DelExit;  }
	else if(vallen<=5 && !strncmp(val,"never",vallen))  
	{  *ds=TaskFile::DontDel;  }
	else
	{
		Error("Illegal value for %s (possible values: "
			"frame,cycle,exit,never).\n",optname);
		return(1);
	}
		
	// String no longer needed: 
	str->deref();
	return(0);
}


int TaskSourceFactory_LDR::_RegisterParams()
{
	if(SetSection("L","LDR (Local Distributed Rendering) task source: "
		"\"LDR client\" settings:"))
	{  return(-1);  }
	
	AddParam("port","inet port to listen for connections from LDR server",
		&listen_port);
	
	AddParam("password","password required by the server to be allowed "
		"to connect to this client; leave away or \"none\" to disable; "
		"\"prompt\" to prompt for one; \"file:PATH\" to read it from PATH",
		&password);
	
	AddParam("servernet","space separated list of hosts or networks/hosts with "
		"netmasks (e.g. 192.168.1.0/24 or myhost/24) to allow as servers",
		&server_net_str);
	
	work_chdir_pi=ParameterConsumer::AddParam(
		"wdir",PTOptPar,
		"change into /tmp/ldrclient-<pid> (or any other dir specified) "
		"before starting to work (use \"none\" to switch off)",
		&work_chdir,par::default_string_handler,/*flags=*/PNoDefault);
	
	AddParam("transfer","specify which files shall/shall not be tranferred "
		"from the server to THIS client (files not being transferred are "
		"assumed to be found on the hd becuase e.g. it is an NFS volume); "
		"possible files:\n"
		"  \"rs\", \"rd\", \"ra\"  -> render source, dest, additional files\n"
		"        \"fd\", \"fa\"        -> filter source, dest, additional files\n"
		"  \"d\", \"a\"   -> all dest, additional files\n"
		"  \"all\"      -> all files\n"
		"concatenation using postfix `+' (transfer) and `-' (do not transfer); "
		"e.g. \"a+rs-rd-fd-\"; (default: \"all+\")",&transfer_spec_str);
	
	AddParam("r-delin",
		"when to delete render input files; possible values:\n"
		"  \"f(rame)\" -> when frame was processed  |  \"e(xit)\"  -> on exit\n"
		"  \"c(ycle)\" -> when work cycle is done   |  \"n(ever)\" -> never\n"
		"Note: Only downloaded files are ever deleted.",
		&rin_delstr);
	AddParam("r-delout",
		"when to delete render output files (see r-delin)",&rout_delstr);
	AddParam("r-deladd",
		"when to delete additional render files (see r-delin)",&radd_delstr);
	AddParam("f-delin",
		"when to delete render input files; possible values:",&fin_delstr);
	AddParam("f-delout",
		"when to delete render output files (see r-delin)",&fout_delstr);
	AddParam("f-deladd",
		"when to delete additional render files (see r-delin)",&fadd_delstr);
	
	AddParam("r-adddir","where to put additional files for renderer",
		&radd_path);
	AddParam("f-adddir","where to put additional files for filter",
		&fadd_path);
	
	AddParam("kamult","(keepalive multiplicator) server keepalive time / 10 "
		"multiplied with this value is the idle timeout; if connection is "
		"idle longer, it is shut down, -1 to disable",
		&keepalive_mult);
	AddParam("atimeout","auth timeout; max time in _msec_ for complete "
		"server authentication to take place; -1 to disable",
		&auth_timeout);
	
	AddParam("re-download","if enabled, download files again even if the "
		"timestamp on the server side did not change (default: disabled)",
		&re_download_enabled);
	AddParam("always-download-input|adli","if enabled, download primary "
		"render/filter input file even if presence, size and time stamp "
		"suggest against that (default: disabled)",
		&always_download_rf_in);
	AddParam("time-correction","if enabled (default), compensate for "
		"differences of client and server system time",
		&server_time_correction_enabled);
	AddParam("timestamp-thresh","negative values pessimize the time stamp "
		"check for files by the specified number of msec; this is used to "
		"compensate slightly different system times as well as time stamp "
		"resolution (better leave untouched and rely on -L-time-correction)",
		&timestamp_thresh);
	return(add_failed ? (-1) : 0);
}


// Called on program start to set up the TaskSourceFactory_LDR. 
// TaskSourceFactory_LDR registers at ComponentDataBase. 
// Return value: 0 -> OK; >0 -> error. 
int TaskSourceFactory_LDR::init(ComponentDataBase *cdb)
{
	TaskSourceFactory_LDR *s=NEW1<TaskSourceFactory_LDR>(cdb);
	if(!s)
	{
		Error("Failed to initialize LDR task source.\n");
		return(1);
	}
	Verbose(BasicInit,"[LDR] ");
	return(0);
}


TaskSourceFactory_LDR::TaskSourceFactory_LDR(
	ComponentDataBase *cdb,int *failflag) : 
	TaskSourceFactory("LDR",cdb,failflag),
	password(failflag),
	server_net_str(failflag),
	server_net_list(failflag),
	transfer_spec_str(failflag),
	radd_path(failflag),
	fadd_path(failflag),
	rin_delstr(failflag),rout_delstr(failflag),radd_delstr(failflag),
	fin_delstr(failflag),fout_delstr(failflag),fadd_delstr(failflag),
	work_chdir(failflag)
{
	listen_fd=-1;
	
	listen_port=UnsetNegMagic;
	
	transfer.render_src=1;
	transfer.render_dest=1;
	transfer.filter_dest=1;
	transfer.r_additional=1;
	transfer.f_additional=1;
	
	timestamp_thresh=-0x7ffffffe;  // msec
	re_download_enabled=0;
	always_download_rf_in=0;
	server_time_correction_enabled=1;
	
	rin_delspec= TaskFile::DelFrame;
	rout_delspec=TaskFile::DelFrame;
	radd_delspec=TaskFile::DelExit;
	fin_delspec= TaskFile::DelFrame;
	fout_delspec=TaskFile::DelFrame;
	fadd_delspec=TaskFile::DelExit;
	
	work_chdir_pi=NULL;
	working_in_wdir=0;
	
	auth_timeout=10000;   // MSEC 
	keepalive_mult=25;
	
	int failed=0;
	
	if(_RegisterParams())
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskSourceFactory_LDR");  }
}

TaskSourceFactory_LDR::~TaskSourceFactory_LDR()
{
	// Make sure the listening FD is shut down: 
	if(listen_fd>=0)
	{
		MyAddrInfo::shutdown(listen_fd);
		MyAddrInfo::close(listen_fd);
		listen_fd=-1;
	}
	
	DeleteWorkingDir();
	
	while(!server_net_list.is_empty())
	{  delete server_net_list.popfirst();  }
	
	password.zero();
}

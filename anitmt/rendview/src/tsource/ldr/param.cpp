/*
 * param.cpp
 * 
 * Implementation of parameter & factory class of LDR task source. 
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
	
	// Okay, do most checks before asking the user for the password...
	_PathFixupAppendSlash(&radd_path);
	_PathFixupAppendSlash(&fadd_path);
	
	LDRGetPassIfNeeded(&password,"Enter client password: ",NULL);
	if(!password.str())
	{  Warning("LDR client: no server password specified "
		"(authentification disabled).\n");  }
	
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
		{  Error("Failed to open inet socket: %s\n",
			strerror(errno));  break;  }
		
		// Bind socket to port: 
		if(addr.bind(listen_fd))
		{  Error("Failed to bind to port %d: %s\n",addr.GetPort(),
			strerror(errno));  break;  }
		
		// Say we want connections to that port: 
		if(MyAddrInfo::listen(listen_fd,/*backlog=*/2))
		{  Error("Failed to listen to port %d: %s\n",addr.GetPort(),
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
		Verbose(TSI,"  Transferring files:\n"
			"    Render source: %s     Render dest: %s\n"
			"    Filter dest:   %s     Additional:  %s\n",
			transfer.render_src  ? "yes" : "no ",
			transfer.render_dest ? "yes" : "no",
			transfer.filter_dest ? "yes" : "no ",
			transfer.additional  ? "yes" : "no");
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
				 	if(*s=='a')  {  transfer.additional=mode;  break;  }
				}
				else if(e-s==2)
				{
					if(*s=='r' && s[1]=='s')
					{  transfer.render_src=mode;  break;  }
					else if(*s=='r' && s[1]=='d')
					{  transfer.render_dest=mode;  break;  }
					else if(*s=='f' && s[1]=='d')
					{  transfer.filter_dest=mode;  break;  }
				}
				else if(e-s==3)
				{
					if(*s=='a' && s[1]=='l' && s[2]=='l')
					{
						transfer.additional=mode;
						transfer.render_src=mode;
						transfer.render_dest=mode;
						transfer.filter_dest=mode;
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
	if(!transfer.additional)
	{  radd_delspec=TaskFile::DontDel;  fadd_delspec=TaskFile::DontDel;  }
	
	// Delete specs: MUST BE PARSED AFTER TRANSFER SPEC. 
	failed+=_ParseDeleteSpec(&rin_delstr,"r-delin",&rin_delspec);
	failed+=_ParseDeleteSpec(&rout_delstr,"r-delout",&rout_delspec);
	failed+=_ParseDeleteSpec(&radd_delstr,"r-deladd",&radd_delspec);
	failed+=_ParseDeleteSpec(&fin_delstr,"f-delin",&fin_delspec);
	failed+=_ParseDeleteSpec(&fout_delstr,"f-delout",&fout_delspec);
	failed+=_ParseDeleteSpec(&fadd_delstr,"f-deladd",&fadd_delspec);
	
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
		
		// Resolve the hosts: 
		if(!server_net_list.is_empty())
		{
			Verbose(TSP,"Looking up server networks...\n");
			for(ServerNet *sn=server_net_list.first(); sn; sn=sn->next)
			{
				int rv=sn->adr.SetAddressError(sn->name.str(),/*port=*/0);
				if(rv)
				{  ++failed;  continue;  }
			}
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
		"\"prompt\" to prompt for one",&password);
	
	AddParam("servernet","space separated list of hosts or networks/hosts with "
		"netmasks (e.g. 192.168.1.0/24 or myhost/24) to allow as servers",
		&server_net_str);
	
	AddParam("transfer","specify which files shall/shall not be tranferred "
		"from the server to THIS client (files not being transferred are "
		"assumed to be found on the hd becuase e.g. it is an NFS volume); "
		"possible files:\n"
		"  \"a\"   -> additional files   \"rd\" -> render dest\n"
		"  \"rs\"  -> render source      \"fd\" -> filter dest\n"
		"  \"all\" -> all files\n"
		"concatenation using postfix `+' (transfer) and `-' (do not transfer); "
		"e.g. \"a+rs-rd-fd-\"; (default: \"all+\")",&transfer_spec_str);
	
	AddParam("r-delin",
		"when to delete render input files; possible values:\n"
		"  \"f(rame)\" -> when frame was processed  |  \"e(xit)\"  -> on exit\n"
		"  \"c(ycle)\" -> when work cycle is done   |  \"n(ever)\" -> never",
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
	fin_delstr(failflag),fout_delstr(failflag),fadd_delstr(failflag)
{
	listen_fd=-1;
	
	listen_port=UnsetNegMagic;
	
	transfer.render_src=1;
	transfer.render_dest=1;
	transfer.filter_dest=1;
	transfer.additional=1;
	
	timestamp_thresh=1001;  // msec
	
	rin_delspec= TaskFile::DelFrame;
	rout_delspec=TaskFile::DelFrame;
	radd_delspec=TaskFile::DelExit;
	fin_delspec= TaskFile::DelFrame;
	fout_delspec=TaskFile::DelFrame;
	fadd_delspec=TaskFile::DelExit;
	
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
		::shutdown(listen_fd,2);
		::close(listen_fd);
		listen_fd=-1;
	}
	
	while(!server_net_list.is_empty())
	{  delete server_net_list.popfirst();  }
	
	password.zero();
}

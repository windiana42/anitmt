/*
 * adminshell/main.cpp
 * 
 * Main routine for RendView admin shell. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "adminshell.hpp"

#include <hlib/parmanager.h>
#include <hlib/srccmd.h>


char *prg_name="rvadminshell";


#define BUG_REPORT_URL "http://www.cip.physik.uni-muenchen.de/~wwieser/ani/rendview/"

static par::ParameterManager *SetUpParManager()
{
	par::ParameterManager *parman=NEW<par::ParameterManager>();
	if(!parman)
	{  return(NULL);  }
	
	parman->SetVersionInfo(
		VERSION,          // version string
		"rendview",       // package name
		"rvadminshell");  // program name
	
	parman->SetLicenseInfo(
		// --lincense--
		"RendView's RVAdminShell may be distributed under the terms of the "
		"GNU General Public License version 2 as published by the Free "
		"Software Foundation.\n"
		"This program is provided AS IS with NO WARRANTY OF ANY KIND, "
		"INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS "
		"FOR A PARTICULAR PURPOSE.\n"
		"See the GNU GPL for details.",
		// --author--
		"RVAdminShell (c) 2004 by Wolfgang Wieser.\n"
		"Bug report instructions: See " BUG_REPORT_URL);
	
	parman->AdditionalHelpText(
		"RVAdminShell is a utility which allows the remote administration of "
		"running RendView processes.\n\n"
		"Example:\n"
		"# rvadminshell [server]\n");
	
	#if 0
	if(enable_color_stdout)
	{
		parman->SetHighlightStrings(
			console_blue_start,console_blue_end,
			console_red_start,console_red_end);
	}
	
	parman->SetConsolePrintfFunction(
		&MessageLogger::_vaError,&MessageLogger::_vaWarning,NULL);
	#endif
	
	return(parman);
}


static int MAIN(int argc,char **argv,char **envp)
{
	int fail=0;
	
	par::ParameterManager *parman=NULL;
	FDManager *fdman=NULL;
	RVAdminShell *admshell=NULL;
	
	par::CmdLineArgs *cmdline=NULL;
	
	// First action: Set up the parameter system...
	if(!fail) do {
		parman=SetUpParManager();
		if(!parman)
		{
			fprintf(stderr,"Failed to initialize parameter manager.\n");
			fail=1;  break;
		}
		
		cmdline=NEW3<par::CmdLineArgs>(argc,argv,envp);
		if(!cmdline)
		{
			fprintf(stderr,"Failed to initialize cmdline buffer.\n");
			fail=1;  break;
		}
		
		fdman=NEW<FDManager>();
		if(!fdman)
		{
			fprintf(stderr,"Failed to initialize FD manager.\n");
			fail=1;  break;
		}
		
		admshell=NEW1<RVAdminShell>(parman);
		if(!admshell)
		{
			fprintf(stderr,"Failed to initialize RV admin shell.\n");
			fail=1;  break;
		}
		
	} while(0);
	
	int will_exit=0;
	
	// NOW IT IS TIME TO LOOK AT THE COMMAND LINE ARGS: 
	if(!fail) do {
		par::ParameterSource_CmdLine cmd_src(parman,&fail);
		if(fail) 
		{
			fprintf(stderr,"Failed to initialize "
				"command line parameter source.\n");
			fail=1;  break;
		}
		
		// Then, parse the args passed via cmd line: 
		if(cmd_src.ReadCmdLine(cmdline))
		{  fail=1;  break;  }
		
		// Check if info query options were passed; exit in this case: 
		if(cmd_src.n_iquery_opts)
		{  will_exit=1;  break;  }
  		
		int rv=cmd_src.WriteParams();
		if(rv)
		{
			fprintf(stderr,"Failed to store the command line "
				"parameters (%d)\n",rv);
			fail=1;  break;
		}
	} while(0);
	
	if(!fail && !will_exit) do {
		
		if(parman->CheckParams())
		{  fail=1;  break;  }
		if(admshell->Run())
		{  fail=1;  break;  }
		
	} while(0);
	
	// Get rid of some stuff: 
	if(cmdline)  {  delete cmdline;  cmdline=NULL;  }
	
	// NOW IT IS TIME TO REALLY DO THINGS...
	if(!fail && !will_exit) do {
		
		fail=fdman->MainLoop();
		
	} while(0);
	
	// Cleanup: 
	DELETE(admshell);
	DELETE(fdman);
	DELETE(parman);
	
	return(fail ? 1 : 0);
}


static void abort_handler(int)
{
	fprintf(stderr,
		"The message above describes a bug in RVAdminShell.\n"
		"Please submit a full bug report. (See " BUG_REPORT_URL ")\n");
	HReadLine::EmergencyCleanup();
	_exit(1);
}


static int sigfpe_caught=0;
static void fpe_handler(int)
{
	if(!sigfpe_caught)
	{  fprintf(stderr,
		"Just caught a SIGFPE (floating point exception). Will continue.\n"
		"However, please report that as a non-critical bug. "
		"(See " BUG_REPORT_URL ")\n");  }
	else
	{  fprintf(stderr,
		"Warning: Caught SIGFPE for the %dth time.\n",sigfpe_caught+1);  }
	++sigfpe_caught;
}


int main(int argc,char **argv,char **envp)
{
	prg_name=GetPrgName(argv[0]);
	if(!prg_name)
	{  fprintf(stderr,"RVAdminShell: argv[0] is NULL. Aborting.\n");  abort();  }
	
	InstallSignalHandler(SIGABRT,&abort_handler,0);
	InstallSignalHandler(SIGFPE,&fpe_handler,0);
	
	int rv=MAIN(argc,argv,envp);
	
	// Allocation debugging: 
	LMallocUsage lmu;
	LMallocGetUsage(&lmu);
	fprintf(stderr,"%s: %sAlloc: %u bytes in %d chunks; Peak: %u by,%d chks; "
		"(%u/%u/%u)%s\n",
		prg_name,
		lmu.curr_used ? "*** " : "",
		lmu.curr_used,lmu.used_chunks,lmu.max_used,lmu.max_used_chunks,
		lmu.malloc_calls,lmu.realloc_calls,lmu.free_calls,
		lmu.curr_used ? " ***" : "");
	
	if(lmu.curr_used || lmu.used_chunks)
	{
		fprintf(stderr,"\33[1;31m");
		fprintf(stderr,"%s: YOU PROBABLY FOUND A MEMORY LEAK IN %s.\n"
			"%s: PLEASE FIND OUT HOW TO REPRODUCE IT AND REPORT THIS BUG TO THE AUTHOR\n",
			prg_name,prg_name,prg_name);
		fprintf(stderr,"\33[00m");
	}
	if(sigfpe_caught)
	{  fprintf(stderr,"%s: --> Caught %d SIGFPE (Floating point exceptions). "
		"Please bugreport. <--\n",prg_name,sigfpe_caught);  }
	
	return(rv);
}

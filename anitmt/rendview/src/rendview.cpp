/*
 * rendview.hpp
 * 
 * Implementation of RendView's main routine. 
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
#include "tsource/tsfactory.hpp"

// These are only needed for srandom()...
#include <time.h>
#include <unistd.h>

char *prg_name=NULL;


static const char *fti="Failed to initialize %s.\n";

static int MAIN(int argc,char **argv,char **envp)
{
	srandom(time(NULL)*getpid());
	
	Verbose("Setting up managers: [FD] ");
	FDManager *fdman=NEW<FDManager>();
	if(!fdman)
	{
		Error(fti,"FD manager");
		return(1);
	}
	
	Verbose("[process] ");
	ProcessManager *procman=NEW1<ProcessManager>(envp);
	if(!procman)
	{
		Error(fti,"process manager");
		return(1);
	}
	// FIXME?
	procman->TermKillDelay(1000);
	
	Verbose("[parameter] ");
	par::ParameterManager *parman=NEW<par::ParameterManager>();
	if(!parman)
	{
		Error(fti,"parameter manager");
		return(1);
	}
	parman->SetVersionInfo(
		VERSION,      // version string
		NULL,         // package name
		"rendview");  // program name
	parman->AdditionalHelpText(
		"RendView is a utility to render (and postprocess) animations. "
		"While the actual rendering is done by some render application "
		"like e.g. POVRay and the real animation logic is computed by a "
		"animation calculator like aniTMT, rendview can be used to "
		"have the render program actually generate the frames calculated. "
		"RendView has sopthistcated job control and allows you to start "
		"several processes in parallel as well as only processing "
		"changed/not-yet-done frames.\n"
		"NOTE: Pressig ^C (SIGINT) on the terminal will tell RendView to "
		"start no more processes and wait for all current jobs to "
		"exit; pressing ^C twice will kill all jobs. In either way "
		"rendview exits cleanly. Pressing ^C three times makes "
		"RendView abort. SIGTERM acts like two ^C.");
		// ...as well as only rendering changed frames or resuming operation
		// ...and later create a film of them...
	
	Verbose("[CDB] ");
	ComponentDataBase *cdb=NEW1<ComponentDataBase>(parman);
	if(!cdb)
	{
		Error(fti,"component data base");
		return(1);
	}
	
	Verbose("[Task] ");
	TaskManager *taskman=NEW1<TaskManager>(cdb);
	if(!cdb)
	{
		Error(fti,"task manager");
		return(1);
	}
	
	
	Verbose("OK\n");
	
	int fail=0;
	int will_exit=0;
	fail+=ImageFormat::init(cdb);
	fail+=TaskDriverFactory::init(cdb);
	fail+=TaskSourceFactory::init_factories(cdb);
	
	// NOW IT IS TIME TO LOOK AT THE COMMAND LINE ARGS: 
	par::CmdLineArgs cmdline(argc,argv,envp);
	if(!fail) do {
		par::ParameterSource_CmdLine cmd_src(parman,&fail);
		if(fail) 
		{
			Error(fti,"command line parameter source");
			fail=1;  break;
		}
		
		if(cmd_src.ReadCmdLine(&cmdline))
		{  fail=1;  break;  }
		
		// Check if info query options were passed; exit in this case: 
		if(cmd_src.n_iquery_opts)
		{  will_exit=1;  break;  }
  		
		int rv=cmd_src.WriteParams();
		if(rv)
		{
			Error("Failed to store the command line parameters (%d)\n",rv);
			fail=1;  break;
		}
		
		// Read in config files: 
		par::ParameterSource_File file_src(parman,&fail);
		if(fail) 
		{
			Error(fti,"file parameter source");
			fail=1;  break;
		}
		
		// Read in render and filter descriptions: 
		if(cdb->ReadInDescFiles(&file_src))
		{  fail=1;  break;  }
		
		rv=cmd_src.Override(/*below=*/&file_src);
		if(rv)
		{
			Error("Parameter override errors (%d)\n",rv);
			fail=1;  break;
		}
		
		rv=cmd_src.WriteParams();  // (again)
		if(rv)
		{
			Error("Failed to store the command line parameters (%d)\n",rv);
			fail=1;  break;
		}
		
		if(parman->CheckParams())
		{  fail=1;  break;  }
	} while(0);
	
	if(!fail && !will_exit) do {
		
		// #### go on (if needed)
		
	} while(0);
		
	// NOW IT IS TIME TO REALLY DO THINGS...
	if(!fail && !will_exit) do {
		
		fail=fdman->MainLoop();
		
	} while(0);
	
	// Cleanup: 
	Verbose("Cleanup:");
    if(taskman)  Verbose(" [Task");       delete taskman;
    if(cdb)      Verbose("] [CDB");        delete cdb;
    if(parman)   Verbose("] [parameter");  delete parman;
    if(procman)  Verbose("] [process");    delete procman;
    if(fdman)    Verbose("] [FD");           delete fdman;
	Verbose("] OK\n");
	
	Verbose("Exiting: status=%s\n",fail ? "failure" : "success");
	return(fail ? 1 : 0);
}


int main(int argc,char **argv,char **envp)
{
	prg_name=GetPrgName(argv[0]);
	
	int rv=MAIN(argc,argv,envp);
	
	// Allocation debugging: 
	LMallocUsage lmu;
	LMallocGetUsage(&lmu);
	fprintf(stderr,"%s: %sAlloc: %u bytes in %d chunks; Peak: %u bytes; "
		"(%u/%u/%u)%s\n",
		prg_name,
		lmu.curr_used ? "*** " : "",
		lmu.curr_used,lmu.used_chunks,lmu.max_used,
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
	
	return(rv);
}


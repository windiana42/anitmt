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


char *prg_name=NULL;


static const char *fti="Failed to initialize %s.\n";

static int MAIN(int argc,char **argv,char **envp)
{
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
		"Rendview is a utility to render (and postprocess) animations. "
		"While the actual rendering is done by some render application "
		"like e.g. POVRay and the real animation logic is computed by a "
		"animation calculator like aniTMT, rendview can be used to "
		"have the render program actually generate the frames calculated "
		"[and later create a film of them].");
	
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
	//InitImageFormats(cdb); FIXME: MISSING...
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
	if(taskman)  delete taskman;    Verbose(" [Task]");
	if(cdb)      delete cdb;        Verbose(" [CDB]");
	if(parman)   delete parman;     Verbose(" [parameter]");
	if(procman)  delete procman;    Verbose(" [process]");
	if(fdman)    delete fdman;      Verbose(" [FD]");
	Verbose(" OK\n");
	
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
		fprintf(stderr,"%s: YOU PROBABLY FOUND A MEMORY LEAK IN %s.\n"
			"%s: PLEASE FIND OUT HOW TO REPRODUCE IT AND REPORT THIS BUG TO THE AUTHOR\n",
			prg_name,prg_name,prg_name);
	}
	
	return(rv);
}


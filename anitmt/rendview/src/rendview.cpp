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
#include "tdriver/tdfactory.hpp"

#include <hlib/fdmanager.h>
#include <hlib/timeoutmanager.h>


char *prg_name=NULL;


static const char *fti="Failed to initialize %s.\n";


static volatile void _SeedRandom()
{
	HTime tmp(HTime::Curr);
	srandom(tmp.Get(HTime::seconds));
}

static int MAIN(int argc,char **argv,char **envp)
{
	// Before we do anything, set these up: 
	// This also checks if we may have color output: 
	InitRVOutputParams(argc,argv,envp);
	
	// First, seed the random generator: 
	_SeedRandom();
	
	FDManager *fdman=NULL;
	TimeoutManager *timeoutman=NULL;
	ProcessManager *procman=NULL;
	par::ParameterManager *parman=NULL;
	ComponentDataBase *cdb=NULL;
	TaskManager *taskman=NULL;
	
	// Okay, then... Here we go...
	int fail=1;
	do {
		Verbose(BasicInit,"Setting up managers: [FD] ");
		
		fdman=NEW<FDManager>();
		if(!fdman)
		{  Error(fti,"FD manager");  break;  }
		
		Verbose(BasicInit,"[timeout] ");
		timeoutman=NEW<TimeoutManager>();
		if(!timeoutman)
		{  Error(fti,"timeout manager");  break;  }
		
		Verbose(BasicInit,"[process] ");
		procman=NEW1<ProcessManager>(envp);
		if(!procman)
		{  Error(fti,"process manager");  break;  }
		// FIXME?
		procman->TermKillDelay(1000);
		
		Verbose(BasicInit,"[parameter] ");
		parman=NEW<par::ParameterManager>();
		if(!parman)
		{  Error(fti,"parameter manager");  break;  }
		
		parman->SetVersionInfo(
			VERSION,      // version string
			NULL,         // package name
			"rendview");  // program name
	parman->SetLicenseInfo(
		// --lincense--
		"RendView may be distributed under the terms of the GNU General "
		"Public License version 2 as published by the Free Software "
		"Foundation.\n"
		"This program is provided AS IS with NO WARRANTY OF ANY KIND, "
		"INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS "
		"FOR A PARTICULAR PURPOSE.\n"
		"See the GNU GPL for details.",
		// --author--
		"RendView (c) 2001--2002 by Wolfgang Wieser.\n"
		"Bug report instructions: See http://anitmt.sf.net/rendview/");
	parman->AdditionalHelpText(
		"RendView is a utility to render (and postprocess) animations. "
		"While the actual rendering is done by some render application "
		"like e.g. POVRay and the real animation logic is computed by a "
		"animation calculator like aniTMT, rendview can be used to "
		"have the render program actually generate the frames calculated. "
		"RendView has sopthistcated job control and allows you to start "
		"several processes in parallel as well as only processing "
		"changed/not-yet-done frames. Further features include resuming "
		"partly-rendered images, load control and lots of informative "
		"output.\n"
		"NOTE: Pressig ^C on the terminal (SIGINT) will tell RendView to "
		"start no more processes and wait for all current jobs to "
		"exit; pressing ^C twice will kill all jobs. In either way "
		"rendview exits cleanly. Pressing ^C three times makes "
		"RendView abort. SIGTERM acts like two ^C. SIGTSTP (^Z) will "
		"make RendView stop all tasks and then als stop; SIGCONT (fg or "
		"bg in bash) makes them continue all again.\n"
		"NOTE: COLORED OUTPUT is automatically switched off for non-ttys "
		"but you can manually force it off/on using --nocolor/--color."
		"\n\n** LDR is in experimental state **"
		"\n\nExample:\n"
		"rendview --rdfile=/dir/to/renderers.par -l-rd=povray3.5 "
		"-l-n=240 -l-f0=10 -l-size=320x200 -l-oformat=ppm -l-cont "
		"-ld-r-quiet ");
		// ...as well as only rendering changed frames or resuming operation
		// ...and later create a film of them...
		
		Verbose(BasicInit,"[CDB] ");
		cdb=NEW1<ComponentDataBase>(parman);
		if(!cdb)
		{  Error(fti,"component data base");  break;  }
		
		Verbose(BasicInit,"[Task] ");
		taskman=NEW1<TaskManager>(cdb);
		if(!taskman)
		{  Error(fti,"task manager");  break;  }
		
		Verbose(BasicInit,"OK\n");
		fail=0;
	}
	while(0);
	
	int will_exit=0;
	if(!fail)
	{
		fail+=ImageFormat::init(cdb);
		fail+=TaskDriverFactory::init(cdb);
		fail+=TaskSourceFactory::init_factories(cdb);
		fail+=TaskDriverInterfaceFactory::init_factories(cdb);
	}
	
	// NOW IT IS TIME TO LOOK AT THE COMMAND LINE ARGS: 
	if(!fail) do {
		par::CmdLineArgs cmdline(argc,argv,envp);
		
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
	} while(0);
	
	if(!fail && !will_exit) do {
		
		if(parman->CheckParams())
		{  fail=1;  break;  }
		
	} while(0);
		
	// NOW IT IS TIME TO REALLY DO THINGS...
	if(!fail && !will_exit) do {
		
		fail=fdman->MainLoop();
		
	} while(0);
	
	// Cleanup: 
	Verbose(BasicInit,"Cleanup:");
    if(taskman)     Verbose(BasicInit," [Task");        delete taskman;
    if(cdb)         Verbose(BasicInit,"] [CDB");        delete cdb;
    if(parman)      Verbose(BasicInit,"] [parameter");  delete parman;
    if(procman)     Verbose(BasicInit,"] [process");    delete procman;
    if(timeoutman)  Verbose(BasicInit,"] [timeout");    delete timeoutman;
    if(fdman)    Verbose(BasicInit,"] [FD");            delete fdman;
	Verbose(BasicInit,"] OK\n");
	
	Verbose(BasicInit,"Exiting: status=%s\n",fail ? "failure" : "success");
	return(fail ? 1 : 0);
}


static void abort_handler(int)
{
	Error(
		"The message above describes a bug in RendView.\n"
		"Please submit a full bug report. (See http://anitmt.sf.net/rendview/)\n");
	_exit(1);
}


int main(int argc,char **argv,char **envp)
{
	prg_name=GetPrgName(argv[0]);
	if(!prg_name)
	{  fprintf(stderr,"RendView: argv[0] is NULL. Aborting.\n");  abort();  }
	
	InstallSignalHandler(SIGABRT,&abort_handler,0);
	
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

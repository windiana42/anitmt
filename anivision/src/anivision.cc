/*
 * anivision.cc
 * 
 * Main AniVision program file. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "anivision.h"

#include <hlib/parmanager.h>
#include <hlib/srccmd.h>

#include <hlib/htime.h>
#include <../config.h>

#include <signal.h>


char *prg_name=PACKAGE;
#define BUG_REPORT_URL "http://www.cip.physik.uni-muenchen.de/~wwieser/ani/anivision/"

// In case I get trouble with memory management...
extern int hlib_allocdebug_trace;
extern size_t hlib_allocdebug_abort_seq;
extern void (*hlib_allocdebug_checkhdl)(void);


// Memory allocations failures are fatal for AniVision. 
static void LMallocFailureHandler(size_t size_for_warning_message)
{
	LMallocUsage lmu;
	LMallocGetUsage(&lmu);
	fprintf(stderr,
		"OOPS: Failed to allocate %u bytes.\n"
		"      Current allocation stats: used=%u/%d (max=%u/%d) [%u,%u,%u]\n",
		size_for_warning_message,
		lmu.curr_used,lmu.used_chunks,
		lmu.max_used,lmu.max_used_chunks,
		lmu.malloc_calls,lmu.realloc_calls,lmu.free_calls);
	abort();
	_exit(1);
}


static par::ParameterManager *SetUpParManager()
{
	par::ParameterManager *parman=NEW<par::ParameterManager>();
	if(!parman)  return(NULL);
	
	parman->SetVersionInfo(
		VERSION,       // version string
		PACKAGE,       // package name
		"anivision");  // program name
	
	parman->SetLicenseInfo(
		// --lincense--
		"AniVision may be distributed under the terms of the "
		"GNU General Public License version 2 as published by the Free "
		"Software Foundation.\n"
		"This program is provided AS IS with NO WARRANTY OF ANY KIND, "
		"INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS "
		"FOR A PARTICULAR PURPOSE.\n"
		"See the GNU GPL for details.",
		// --author--
		"AniVision (c) 2003--2004 by Wolfgang Wieser.\n"
		"Bug report instructions: See " BUG_REPORT_URL);
	
	parman->AdditionalHelpText(
		"AniVision is a animation utility featuring a complete programming "
		"language. It can be used in combination with POVRay.\n\n"
		"Example:\n"
		"# anivision [file.ani]\n");
	
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
	AniVisionMaster *avmaster=NULL;
	
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
		
		avmaster=NEW1<AniVisionMaster>(parman);
		if(!avmaster)
		{
			fprintf(stderr,"Failed to initialize AniVision.\n");
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
		
	} while(0);
	
	// Get rid of some stuff: 
	if(cmdline)  {  delete cmdline;  cmdline=NULL;  }
	
	// NOW IT IS TIME TO REALLY DO THINGS...
	if(!fail && !will_exit) do {
		
		fail=avmaster->Run();
		
	} while(0);
	
	// Cleanup: 
	DELETE(avmaster);
 	DELETE(parman);
	
	return(fail ? 1 : 0);
}


static void abort_handler(int)
{
	fprintf(stderr,
		"The message above probably describes a bug in AniVision.\n"
		"Please submit a full bug report. (See " BUG_REPORT_URL ")\n");
	_exit(1);
}

static volatile int sigfpe_caught=0;
static void fpe_handler(int)
{
	++sigfpe_caught;
	fprintf(stderr,
		"Warning: Caught SIGFPE for the %dth time.\n",sigfpe_caught+1);
}


int main(int argc,char **argv,char **envp)
{
	prg_name=GetPrgName(argv[0]);
	fprintf(stdout,"** This is AniVision-%s **\n",VERSION);
	
	// Make sure allocation failures get fatal: 
	lmalloc_failure_handler=&LMallocFailureHandler;
	
	// This only exists if allocation debugging is enabled in HLib: 
	// (Linker error otherwise.) 
	//hlib_allocdebug_trace=1;
	// Set this to find desired allocation call with debugger. 
	//hlib_allocdebug_abort_seq=1000;
	//hlib_allocdebug_checkhdl=&ANI::SomeFunction;
	
	//fprintf(stderr,"sizeof(EV)=%d; sizeof(IEV)=%d\n",
	//	sizeof(ANI::ExprValue),sizeof(ANI::InternalExprValue));
	
	InstallSignalHandler(SIGABRT,&abort_handler,0);
	InstallSignalHandler(SIGFPE,&fpe_handler,0);
	
	// Actually run the main routine: 
	int rv=MAIN(argc,argv,envp);
	
	// Allocation debugging: 
	LMallocUsage lmu;
	LMallocGetUsage(&lmu);
	fprintf(stderr,"%s: %sLMalloc: used=%u/%d, peak=%u/%d (%u/%u/%u)%s\n",
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
	{  fprintf(stderr,"%s: Caught %d SIGFPE (Floating point exceptions).\n",
		prg_name,sigfpe_caught);  }
	
	fprintf(stderr,"%s: Normal termination with code=%d (%s)\n",
		prg_name,rv,rv ? "failure" : "success");
	
	return(rv);
}

//------------------------------------------------------------------------------

int AniVisionMaster::Run()
{
	int rv=0;
	
	ANI::Ani_Parser *a_parser;
	AniAnimation *ani=NULL;
	do
	{
		a_parser=new ANI::Ani_Parser();
		
		// Static initialisation. IMPORTANT: 
		ANI::ExprValueType::_InitStaticVals();
		ANI::OpFunc_InitStaticVals();
		
		// Initialize calc cores: 
		CC::CCIF_Factory::InitFactories();
		
		HTime start(HTime::Curr);
		RefString str_or_file;
		str_or_file=prim_file;
		fprintf(stdout,"ANI: Parsing primary input file \"%s\"...\n",
			str_or_file.str());
		rv=a_parser->ParseAnimation(str_or_file,/*is_file=*/1);
		HTime end(HTime::Curr);
		fprintf(stdout,"ANI:   Parse=%d   (elapsed: %s)\n",rv,
			(end-start).PrintElapsed(1));
		
		/*fprintf(stdout,",----------------------------<complete source dump>----------------------------.\n");
		a_parser->DumpTree(stdout);
		fprintf(stdout,"`------------------------------------------------------------------------------'\n");*/
		
		// If there were no errors, perform registration step: 
		if(rv!=0) break;
		
		start=HTime::Curr;
		RefString ani_name_tmp;
		ani_name_tmp.set("[animation]");
		ani=new AniAnimation(ani_name_tmp,a_parser->GetRootNode());
		
		rv=a_parser->DoRegistration(ani);
		end=HTime::Curr;
		
		fprintf(stderr,"ANI: DoRegistration=%d   (elapsed: %s)\n",rv,
			(end-start).PrintElapsed(1));
		int n_incomplete=0,n_scopes=ani->CountScopes(n_incomplete);
		fprintf(stdout,"ANI:   Number of ani scopes: %d  "
			"(and %d incomplete place holders)\n",n_scopes,n_incomplete);
		
		// Dump ani object tree: (before type fixup)
		/*{
			StringTreeDump strtd;
			ani->AniTreeDump(&strtd);
			fprintf(stdout,",---------------------------<complete ani scope dump>--------------------------.\n");
			strtd.Write(stdout);
			fprintf(stdout,"`------------------------------------------------------------------------------'\n");
		}*/
		
		// Dump after optimization: 
		//fprintf(stdout,",----------------------------<complete source dump>----------------------------.\n");
		//a_parser->DumpTree(stdout);
		//fprintf(stdout,"`------------------------------------------------------------------------------'\n");
		
		if(rv)  break;
		
		// We'd now need some constant folding code to solve all type 
		// names and array sizes of member vars. 
		// This actually still needs to be implemented. Currently, only 
		// REAL constants are accepted. 
		start=HTime::Curr;
		rv=ani->FixIncompleteTypes();
		end=HTime::Curr;
		fprintf(stdout,"AMI: FixIncompleteTypes=%d   (elapsed: %s)\n",rv,
			(end-start).PrintElapsed(1));
		
		// Dump ani object tree: (after incomplete type fixup)
		/*{
			StringTreeDump strtd;
			ani->AniTreeDump(&strtd);
			fprintf(stdout,",---------------------------<complete ani scope dump>--------------------------.\n");
			strtd.Write(stdout);
			fprintf(stdout,"`------------------------------------------------------------------------------'\n");
		}*/
		
		if(rv)  break;
		
		// Perform instance info creation step: 
		fprintf(stdout,"ANI: Creating instance info entries...\n");
		rv=ani->PerformCleanup(/*cstep=*/2);  // CSTEP 2
		fprintf(stdout,"ANI:   Instance info entries: %d errors\n",rv);
		
		if(rv)  break;
		
		// We now perform the (complete) type fixup step. 
		start=HTime::Curr;
		rv=a_parser->PerformExprTF();
		end=HTime::Curr;
		fprintf(stdout,"ANI: PerformExprTF=%d   (elapsed: %s)\n",rv,
			(end-start).PrintElapsed(1));
		int n_incomplete_before=0;
		n_scopes=ani->CountScopes(n_incomplete_before);
		ani->PerformCleanup(/*cstep=*/1);  // CLEANUP STEP 1
		n_incomplete=0;
		n_scopes=ani->CountScopes(n_incomplete);
		fprintf(stdout,"AMI:   Ani scopes: %d  "
			"(+%d incomplete; %d before cleanup)\n",
			n_scopes,n_incomplete,n_incomplete_before);
		if(rv)  break;
		
		fprintf(stdout,",----------------------------<complete source dump>----------------------------.\n");
		a_parser->DumpTree(stdout);
		fprintf(stdout,"`------------------------------------------------------------------------------'\n");
		
		// Dump ani object tree: (after complete type fixup)
		{
			StringTreeDump strtd;
			ani->AniTreeDump(&strtd);
			fprintf(stdout,",---------------------------<complete ani scope dump>--------------------------.\n");
			strtd.Write(stdout);
			fprintf(stdout,"`------------------------------------------------------------------------------'\n");
		}
		
		// Execute the animation. Currently, the user cannot decide which one. 
		rv=ani->ExecAnimation(this->ani_name);
		switch(rv)
		{
			case 0:  break;
			case -2:
				if(!ani_name)
				{  fprintf(stderr,"%s: No animation present in file\n",
					prg_name);  }
				else
				{  fprintf(stderr,"%s: Unknown animation \"%s\"\n",
					prg_name,ani_name.str());  }
				break;
			default:  assert(0);
		}
		if(rv) break;
	} while(0);
	
	// This will delete the main TreeNode tree: 
	// NOTE: Main tree gets deleted first and issues CG_DeleteNotify() 
	//       for attached ANI nodes...
	// Reason: One Ani scope can be attached to more than one TreeNode, 
	// hence the ani scope cannot do correct cleanup regarding the 
	// TreeNode pointers. 
	if(a_parser) delete a_parser;
	// This will delete the ANI tree: 
	if(ani) delete ani;
	
	// Cleanup factories: 
	CC::CCIF_Factory::CleanupFactories();
	
	// CLEANUP static data: 
	ANI::OpFunc_CleanupStaticVals();
	ANI::ExprValueType::_CleanupStaticVals();
	
	//fprintf(stderr,"ANI: %u bytes for %d tree nodes allocated.\n",
	//	ANI::TreeNode::tree_nodes_tot_size,ANI::TreeNode::n_alloc_tree_nodes);
	
	//fprintf(stderr,"sizeof(TreeNode)=%u\n",sizeof(ANI::TreeNode));
	
	return(rv ? 1 : 0);
}


int AniVisionMaster::CheckParams()
{
	int failed=0;
	
	if(more_than_one_error)
	{  return(1);  }
	
	if(!prim_file)
	{
		fprintf(stderr,"%s: No primary input file specified.\n",prg_name);
		++failed;
	}
	
	return(failed ? 1 : 0);
}


int AniVisionMaster::parse(const Section *,SPHInfo *si)
{
	do {
		if(!si->arg)  break;
		if(si->sect!=CurrentSection())  break;
		if(si->sect!=si->bot_sect)  break;
		if(si->arg->name!=si->nend)  break;
		
		const char *str=si->arg->arg.str();
		if(!*str || *str=='-')  break;
		
		if(!prim_file)
		{
			prim_file.set(str);
		}
		else if(!more_than_one_error)
		{
			fprintf(stderr,"%s: More than one primary file specified.\n",
				prg_name);
			more_than_one_error=1;
		}
		
		return(0);
	} while(0);
	
	return(2);
}


int AniVisionMaster::_SetUpParams()
{
	if(SetSection(NULL))
	{  return(1);  }
	
	add_failed=0;
	AddParam("ani",
		"name of animation to execute (defaults to first one)",
		&ani_name);
	
	if(SectionParameterHandler::Attach(CurrentSection()))
	{  ++add_failed;  }
	
	return(add_failed ? 1 : 0);
}


AniVisionMaster::AniVisionMaster(par::ParameterManager *parman,int *failflag) : 
	par::ParameterConsumer_Overloaded(parman,failflag),
	par::SectionParameterHandler(parman,failflag),
	prim_file(failflag),
	ani_name(failflag)
{
	int failed=0;
	
	// Set default/initial values: 
	more_than_one_error=0;
	
	if(_SetUpParams())  ++failed;
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("AniVisionMaster");  }
}

AniVisionMaster::~AniVisionMaster()
{
}

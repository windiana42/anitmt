/*
 * hreadline.cc
 * 
 * Implementation of class HReadLine, a HLib-compatible GNU readline(3) 
 * interface. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include <hlib/prototypes.h>  /* MUST BE FIRST */

#include "hreadline.h"

#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <assert.h>


// Initialize static data: 
HReadLine *HReadLine::manager=NULL;

// rl_already_prompted

void HReadLine::EnableInput()
{
	if(!input_enabled)
	{
		FDChangeEvents(pollid,/*set_ev=*/POLLIN,/*clear_ev=*/0);
		input_enabled=1;
		
		rl_instream=stdin;
		rl_callback_handler_install(/*prompt=*/prompt.str(),
			/*lhandler=*/&HReadLine::_line_callback);
		
		rl_redisplay();
	}
}

void HReadLine::SetPrompt(const RefString &_prompt)
{
	prompt=_prompt;
	rl_set_prompt(prompt.str());
}

void HReadLine::DisableInput()
{
	if(input_enabled)
	{
		FDChangeEvents(pollid,/*set_ev=*/0,/*clear_ev=*/POLLIN);
		input_enabled=0;
		
		rl_callback_handler_remove();
		rl_instream=NULL;
	}
}


void HReadLine::EmergencyCleanup()
{
	if(manager)
	{
		rl_callback_handler_remove();
		rl_instream=NULL;
	}
}


void HReadLine::_line_callback(char *line)  // static
{
	if(manager)
	{  manager->line_callback(line);  }
	
	// Important: readline(3) allocated the line using malloc(3), 
	// so we need to free it here. 
	if(line)
	{  ::free(line);  }
}


void HReadLine::line_callback(char *line)
{
	RLNInfo rli;
	
	if(line)
	{
		rli.status=0;
		if(rli.line.set(line))
		{  rli.status=-1;  }
	}
	else
	{  rli.status=1;  }
	
	// Call virtual function: 
	int rv=rlnotify(&rli);
	
	if(rv && !!line)
	{  add_history(rli.line.str());  }
}


int HReadLine::fdnotify(FDInfo *fdi)
{
	// See if it is for us. 
	// As we may derive some class from this one, we may get here 
	// for other FDs as well. 
	if(fdi->pollid!=pollid) return(0);
	
	if(fdi->revents & (POLLHUP|POLLIN))
	{
		// Tell readline to read input. 
		assert(input_enabled);
		rl_callback_read_char();
	}
	
	return(0);
}


HReadLine::HReadLine(int *failflag) : 
	FDBase(failflag),
	prompt(failflag)
{
	int failed=0;
	
	input_enabled=0;
	
	pollid=NULL;
	if(PollFD(fileno(stdin),0,NULL,&pollid)<0)  ++failed;
	
	// There may only be one!
	if(manager) ++failed;
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("HRL");  }
	
	// Set up global pointer for readline callbacks: 
	manager=this;
	//rl_instream=stdin;
	//rl_callback_handler_install(/*prompt=*/NULL,
	//	/*lhandler=*/&HReadLine::_line_callback);
}

HReadLine::~HReadLine()
{
	UnpollFD(pollid);
	input_enabled=0;
	
	// Cleanup...
	if(manager==this)
	{
		// Unregister at readline(3): 
		rl_callback_handler_remove();
		
		manager=NULL;
	}
}

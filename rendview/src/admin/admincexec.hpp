/*
 * admincexec.hpp
 * 
 * Admin command executor base class. 
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

#ifndef _RNDV_ADMIN_ADMINCEXEC_HPP_
#define _RNDV_ADMIN_ADMINCEXEC_HPP_ 1

#include <lib/prototypes.hpp>


class RendViewAdminPort;
class RVAPGrowBuffer;

// Derive classes from this class if they shall be able to execute 
// commands from the admin shell. 
class RendViewAdmin_CommandExecuter : 
	LinkedListBase<RendViewAdmin_CommandExecuter>
{
	friend class RendViewAdminPort;
	friend class LinkedList<RendViewAdmin_CommandExecuter>;
	private:
		RendViewAdminPort *adminprt;
		
	public:
		struct ADMCmd
		{
			// The complete command string: 
			const RefString *command;
			// The command split into args: 
			int argc;
			const char **arg;
		};
		
	protected:
		// This is called for the command to be executed. 
		// You can rely on cmd->argc being >=1. 
		// Return value: 
		//  0 -> OK, executed
		//  1 -> unknown command
		virtual int admin_command(ADMCmd *cmd,RVAPGrowBuffer *dest)
			{  return(1);  }
		
	public:  _CPP_OPERATORS_FF
		RendViewAdmin_CommandExecuter(RendViewAdminPort *_adminprt,int *failflag=NULL);
		virtual ~RendViewAdmin_CommandExecuter();
};

#endif  /* _RNDV_ADMIN_ADMINCEXEC_HPP_ */

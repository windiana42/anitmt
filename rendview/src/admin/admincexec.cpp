/*
 * admincexec.cpp
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

#include "admincexec.hpp"
#include "adminport.hpp"


RendViewAdmin_CommandExecuter::RendViewAdmin_CommandExecuter(
	RendViewAdminPort *_adminprt,int * /*failflag*/) : 
	LinkedListBase<RendViewAdmin_CommandExecuter>()
{
	adminprt=_adminprt;
	
	if(adminprt)
	{  adminprt->Register(this);  }
}

RendViewAdmin_CommandExecuter::~RendViewAdmin_CommandExecuter()
{
	if(adminprt)
	{  adminprt->Unregister(this);  }
}

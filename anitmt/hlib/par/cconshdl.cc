/*
 * cconshdl.cc
 * 
 * Default (console) implementation of ParameterConsumer's 
 * (virtual) error/warning handlers. 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#define HLIB_IN_HLIB 1
#include "parmanager.h"
#include "parconsumer.h"

namespace par
{

void ParameterConsumer::CheckParamError(ParamInfo *pi)
{
	// Okay, if that gets called, we must complain. 
	if(pi->spectype==STNone)  return;   // never happens. 
	
	// Get full parameter name: 
	size_t len=parmanager()->FullParamName(pi,NULL,0);
	char tmp[len+1];
	parmanager()->FullParamName(pi,tmp,len+1);
	
	if(pi->spectype==STAtLeastOnce ||
	   (pi->spectype==STExactlyOnce && !pi->nspec))
	{  parmanager()->cerr_printf("%s: required %s `%s´ not specified.\n",
		prg_name,ParamTypeString(pi->ptype),tmp);  }
	else if(pi->spectype==STExactlyOnce)
	{  parmanager()->cerr_printf("%s: required %s `%s´ must be specified "
		"exactly once.\n",prg_name,ParamTypeString(pi->ptype),tmp);  }
	else if(pi->spectype==STMaxOnce)
	{  parmanager()->cerr_printf("%s: optional %s `%s´ may not be "
		"specified more than once.\n",
		prg_name,ParamTypeString(pi->ptype),tmp);  }
	else
	{  parmanager()->cerr_printf("%s: %s `%s´: illegal spectype %d\n",
		prg_name,ParamTypeString(pi->ptype),tmp,pi->spectype);  abort();  }
}

}  // end of namespace par

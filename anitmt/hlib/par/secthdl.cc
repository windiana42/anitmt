/*
 * secthdl.cc
 * 
 * Implementation of section parameter handler. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "secthdl.h"


namespace par
{

int SectionParameterHandler::Attach(const char *section,Section *top)
{
	Section *s=parmanager()->FindSection(section,top);
	if(!s)  return(-1);
	return(Attach(s));
}

}  // end of namespace par 

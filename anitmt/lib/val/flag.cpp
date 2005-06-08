/*
 * flag.cpp
 * 
 * Implementation of non-inline support stuff for flag. 
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser > wwieser -a- gmx -*- de < 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "flag.hpp"

namespace vect
{

std::ostream& operator<<(std::ostream& s,const Flag &m)
{
	s << (m.x ? "yes" : "no");
	return(s);
}

}  // namespace end

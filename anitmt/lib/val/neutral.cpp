/*
 * neutral.cpp
 * 
 * Implementation of non-inline support stuff for neutral type. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser > wwieser -a- gmx -*- de < 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "neutral.hpp"

namespace vect
{

std::ostream& operator<<(std::ostream& s,const Neutral0 &)
{
	s << "Neutral0";
	return(s);
}

std::ostream& operator<<(std::ostream& s,const Neutral1 &)
{
	s << "Neutral1";
	return(s);
}


#if 0
std::ostream& operator<<(std::ostream& s,const Neutral &n)
{
	s << (n.n ? "Neutral1" : "Neutral0");
	return(s);
}
#endif

}  // namespace end

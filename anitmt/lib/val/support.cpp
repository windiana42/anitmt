/*
 * support.cpp
 * 
 * Implementation of non-inline support routines needed by 
 * value library. 
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

#include "support.hpp"

namespace vect
{

double epsilon=0.000000001;  // Max. difference for comparisons. 

const char *_YesNo(bool x)
{  return(x ? "yes" : "no");  }

}  // namespace end

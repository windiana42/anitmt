/*
 * ivector.cpp
 * 
 * Implenemtation of non-inline internally used vector functions. 
 * 
 * Copyright (c) 2000--2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "internals.hpp"

// Most is done inline for speed increase. 

namespace internal_vect
{
namespace internal
{

// Suffix 1 for 1-dim array (vector). 
ostream& stream_write_array1(ostream& s,const double *x,int n)
{
	s << "<";
	if(n>0)
	{
		s << *x;
		for(int i=1; i<n; i++)
		{  s << "," << x[i] ;  }
	}
	s << ">";
	return(s);
}

}  // end of namespace internal
}  // namespace end 


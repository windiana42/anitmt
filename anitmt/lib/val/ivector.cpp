/*
 * ivector.cpp
 * 
 * Implenemtation of non-inline internally used vector functions. 
 * 
 * Copyright (c) 2000--2002 by Wolfgang Wieser > wwieser -a- gmx -*- de < 
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

#include <iomanip>

// Most is done inline for speed increase. 

namespace internal_vect
{
namespace internal
{

// Suffix 1 for 1-dim array (vector). 
std::ostream& stream_write_array1(std::ostream& s,const double *x,int n)
{
	std::streamsize width = s.width();
	std::streamsize col_width = (width - n + 1) / n;

	s << std::setw(1) << "<";
	if(n>0)
	{
		s << std::setw(col_width) << *x;
		for(int i=1; i<n; i++)
		{  s << "," << std::setw(col_width) << x[i];  }
	}
	s << ">";

	return(s);
}

}  // end of namespace internal
}  // namespace end 


/*
 * flag.hpp
 * 
 * Header for the flag value type. 
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _NS_vect_flag_HPP_
#define _NS_vect_flag_HPP_ 1

#include <iostream>

namespace vect
{

class Flag
{
	private:
		bool x;
	public:
		Flag(const Flag &f) : x(f.x) { }
		Flag(bool f) : x(f) { }
		Flag() { }
		~Flag() { }
		
		operator bool() const  {  return(x);  }
		bool val() const  {  return(x);  }
		
		friend std::ostream& operator<<(std::ostream& s,const Flag &m);
};

}  // namespace end 

#endif  /* _NS_vect_flag_HPP_*/

/*
 * string.hpp
 * 
 * Header for string value type. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _NS_vect_string_HPP_
#define _NS_vect_string_HPP_ 1

#include <string>

namespace vect
{

class String : public std::string
{
	public:
		String() : std::string() { }
		String(std::string s) : std::string(s) { }
		String(const String &s) : std::string(s) { }
};

inline bool operator!(const String &a)
	{  return(a == std::string());  }

}  // namespace end 

#endif  /* _NS_vect_string_HPP_*/
/*
 * neutral.hpp
 * 
 * Header for the neutral value type. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _NS_vect_neutral_HPP_
#define _NS_vect_neutral_HPP_ 1

#include <config.h>

#include <iostream>

namespace vect
{

class Neutral0
{
	public:
		Neutral0(const Neutral0 &) { }
		Neutral0() { }
		~Neutral0() { }
		
		Neutral0 &operator=(const Neutral0 &)  {  return(*this);  }
		
		friend std::ostream& operator<<(std::ostream& s,const Neutral0 &n);
};

inline bool operator==(const Neutral0 &, const Neutral0 &)  { return(true);  }
inline bool operator!=(const Neutral0 &, const Neutral0 &)  { return(false); }


class Neutral1
{
	public:
		Neutral1(const Neutral0 &) { }
		Neutral1() { }
		~Neutral1() { }
		
		Neutral1 &operator=(const Neutral1 &)  {  return(*this);  }
		
		friend std::ostream& operator<<(std::ostream& s,const Neutral1 &n);
};

inline bool operator==(const Neutral1 &, const Neutral1 &)  { return(true);  }
inline bool operator!=(const Neutral1 &, const Neutral1 &)  { return(false); }

//inline bool operator==(const Neutral0 &, const Neutral1 &)  { return(false); }
//inline bool operator==(const Neutral1 &, const Neutral0 &)  { return(false); }
//inline bool operator!=(const Neutral0 &, const Neutral1 &)  { return(true);  }
//inline bool operator!=(const Neutral1 &, const Neutral0 &)  { return(true);  }


#if 0
class Neutral
{
	private:
		short int n;
	public:
		Neutral(const Neutral &s) : n(s.n) { }
		// Set up neutral elem. s must be 0 or 1. 
		// 0 -> addition neutral
		// 1 -> multiplication neutral
		Neutral(int s) : n(s) { }
		~Neutral() { }
		
		// Mostly used internally by matrix/vector code: 
		operator bool()  {  return(n);  }
		
		operator int() const  {  return(n);  }
		int val() const  {  return(n);  }
		
		friend bool operator==(const Neutral &a,const Neutral &b);
		friend bool operator!=(const Neutral &a,const Neutral &b);
		
		friend std::ostream& operator<<(std::ostream& s,const Neutral &n);
};

inline bool operator==(const Neutral &a,const Neutral &b)
	{  return(a.n==b.n);  }
inline bool operator!=(const Neutral &a,const Neutral &b)
	{  return(a.n!=b.n);  }

#endif

}  // namespace end 

#endif  /* _NS_vect_neutral_HPP_*/

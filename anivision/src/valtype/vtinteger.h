/*
 * valtype/vtinteger.h
 * 
 * Integer value header. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _VALTYPES_VT_INTEGER_H_
#define _VALTYPES_VT_INTEGER_H_ 1

class Integer : public ValueBase
{
	public:
		static const VType vtype=VTInteger;
	private:
		int x;
	public:
		// Constructor: 
		inline Integer(const Integer &_x) : ValueBase(),x(_x.x) {}
		inline Integer(int _x) : ValueBase(),x(_x) {}
		inline ~Integer() {}
		
		// Implicit conversion: 
		inline operator int() const  {  return(x);  }
		inline int val() const  {  return(x);  }
		
		// Assignment: 
		inline int operator=(int _x)  {  x=_x;  return(x);  }
		inline void set(int _x)  {  x=_x;  }
		
		// Compare: 
		inline bool operator==(int b) const  {  return(x==b);  }
		inline bool operator!=(int b) const  {  return(x!=b);  }
		inline bool operator<=(int b) const  {  return(x<=b);  }
		inline bool operator>=(int b) const  {  return(x>=b);  }
		inline bool operator<(int b) const  {  return(x<b);  }
		inline bool operator>(int b) const  {  return(x>b);  }
		
		// Needed for templates...
		inline bool is_null() const  {  return(!x);  }
		
		// Make string representation of value [overridden virtual]: 
		String ToString() const;
		ValueBase *clone() const;   // <-- [overridden virtual]
		void forceset(ValueBase *vb);  //  <-- [overridden virtual]
}__attribute__((__packed__));

#endif  /* _VALTYPES_VT_INTEGER_H_ */

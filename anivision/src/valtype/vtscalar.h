/*
 * valtype/vtscalar.h
 * 
 * Scalar (i.e. "double float") value header. 
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

#ifndef _VALTYPES_VT_SCALAR_H_
#define _VALTYPES_VT_SCALAR_H_ 1

class Scalar : public ValueBase
{
	public:
		static const VType vtype=VTScalar;
	private:
		double x;
	public:
		// Constructor: 
		inline Scalar(const Scalar &_x) : ValueBase(),x(_x.x) {}
		inline Scalar(double _x) : ValueBase(),x(_x) {}
		inline ~Scalar() {}
		
		// Implicit conversion: 
		inline operator double() const  { return(x);  }
		inline double val() const  {  return(x);  }
		
		// Assignment: 
		inline double operator=(double _x)  {  x=_x;  return(x);  }
		inline void set(double _x)  {  x=_x;  }
		
		// Compare: 
		inline bool operator<=(double b) const  {  return(x<=b);  }
		inline bool operator>=(double b) const  {  return(x>=b);  }
		inline bool operator<(double b) const  {  return(x<b);  }
		inline bool operator>(double b) const  {  return(x>b);  }
		inline bool operator==(double b) const
			{  return(fabs(x-b)<=default_eps);  }
		inline bool operator!=(double b) const
			{  return(fabs(x-b)>default_eps);  }
		
		// Needed for templates...
		inline bool is_null() const  {  return(fabs(x)<=default_eps);  }
		
		// Make string representation of value [overridden virtual]: 
		String ToString() const;
		ValueBase *clone() const;   // <-- [overridden virtual]
		void forceset(ValueBase *vb);  //  <-- [overridden virtual]
}__attribute__((__packed__));

#endif  /* _VALTYPES_VT_SCALAR_H_ */

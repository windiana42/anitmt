/*
 * scalar.hpp
 * 
 * Header for scalar value type. 
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

#ifndef _NS_vect_scalar_HPP_
#define _NS_vect_scalar_HPP_ 1

#include "support.hpp"

namespace vect
{

class Scalar
{
	private:
		double x;
	public:
		Scalar(double i) :        x(i)   {}
		Scalar() :                x(0.0) {}
		Scalar(const Scalar &s) : x(s.x) {}
		~Scalar() {}
		
		operator double() const  {  return(x);  }
		double val() const  {  return(x);  }
		
		// Operators comparing scalars (using epsilon): 
		friend bool operator==(const Scalar &,const Scalar &);
		friend bool operator!=(const Scalar &,const Scalar &);
		friend bool operator==(const Scalar &,double);
		friend bool operator!=(const Scalar &,double);
		friend bool operator==(double,const Scalar &);
		friend bool operator!=(double,const Scalar &);
		friend bool operator==(const Scalar &,int);
		friend bool operator!=(const Scalar &,int);
		friend bool operator==(int,const Scalar &);
		friend bool operator!=(int,const Scalar &);
		
		// Returns 1, if this scalar is 0 (exactly: if |this->x| <= epsilon )
		bool operator!() const {  return(fabs(x)<=epsilon);  }
		bool is_null() const {  return(fabs(x)<=epsilon);  }
		
		// addition/subtraction operators: 
		Scalar &operator-=(double a)  {  x-=a;  return(*this);  }
		Scalar &operator+=(double a)  {  x+=a;  return(*this);  }
		
		// multiplication/division operators: 
		Scalar &operator*=(double a)  {  x*=a;  return(*this);  }
		Scalar &operator/=(double a)  {  x/=a;  return(*this);  }
		
		friend ostream& operator<<(ostream& s,const Scalar &m);
};

// Operators comparing scalars are using epsilon: 
// If the difference between two scalars is larger than epsilon, 
// they are considered different, else equal. 
inline bool operator==(const Scalar &a,const Scalar &b)
	{  return(fabs(a.x-b.x)<=epsilon);  }
inline bool operator!=(const Scalar &a,const Scalar &b)
	{  return(fabs(a.x-b.x)>epsilon);  }
inline bool operator==(double a,const Scalar &b)
	{  return(fabs(a-b.x)<=epsilon);  }
inline bool operator!=(double a,const Scalar &b)
	{  return(fabs(a-b.x)>epsilon);  }
inline bool operator==(const Scalar &a,double b)
	{  return(fabs(a.x-b)<=epsilon);  }
inline bool operator!=(const Scalar &a,double b)
	{  return(fabs(a.x-b)>epsilon);  }
inline bool operator==(int a,const Scalar &b)
	{  return(fabs(double(a)-b.x)<=epsilon);  }
inline bool operator!=(int a,const Scalar &b)
	{  return(fabs(double(a)-b.x)>epsilon);  }
inline bool operator==(const Scalar &a,int b)
	{  return(fabs(a.x-double(b))<=epsilon);  }
inline bool operator!=(const Scalar &a,int b)
	{  return(fabs(a.x-double(b))>epsilon);  }

inline Scalar abs(const Scalar &a)
	{  return(fabs(a)); }

inline Scalar sqrt(const Scalar &a)
	{  return(::sqrt(double(a))); }

inline ostream& operator<<(ostream& s,const Scalar &m)
	{  s << m.x;  return(s);  }

}  // namespace end 

#endif  /* _NS_vect_scalar_HPP_ */

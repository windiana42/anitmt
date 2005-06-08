/*
 * scalar.hpp
 * 
 * Header for scalar value type. 
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

#ifndef _NS_vect_scalar_HPP_
#define _NS_vect_scalar_HPP_ 1

#include "support.hpp"
#include "neutral.hpp"

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
		Scalar(Neutral0) :        x(0.0) {}
		Scalar(Neutral1) :        x(1.0) {}
		~Scalar() {}
		
		// Assignment operators: 
		Scalar &operator=(double v)  {  x=v;  return(*this);  }
		Scalar &operator=(Neutral0)  {  x=0.0;  return(*this);  }
		Scalar &operator=(Neutral1)  {  x=1.0;  return(*this);  }
		
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
		
		// Comparing to neutral types (using epsilon): 
		// non-member: operator==/!=(const Scalar &,Neutral[01]);
		// non-member: operator==/!=(Neutral[01],const Scalar &);
		
		// Returns 1, if this scalar is 0 (exactly: if |this->x| <= epsilon )
		bool operator!() const {  return(fabs(x)<=epsilon);  }
		bool is_null() const {  return(fabs(x)<=epsilon);  }
		// Compare to 1.0: 
		bool is_one()  const {  return(fabs(x-1.0)<=epsilon);  }
		
		// addition/subtraction operators: 
		Scalar &operator-=(double a)  {  x-=a;  return(*this);  }
		Scalar &operator+=(double a)  {  x+=a;  return(*this);  }
		
		// multiplication/division operators: 
		Scalar &operator*=(double a)  {  x*=a;  return(*this);  }
		Scalar &operator/=(double a)  {  x/=a;  return(*this);  }
		
		friend std::ostream& operator<<(std::ostream& s,const Scalar &m);
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

// Comparing to neutral types (using epsilon): 
inline bool operator==(const Scalar &a,Neutral0)
	{  return(a.is_null());  }
inline bool operator==(const Scalar &a,Neutral1)
	{  return(a.is_one());  }
inline bool operator==(Neutral0,const Scalar &a)
	{  return(a.is_null());  }
inline bool operator==(Neutral1,const Scalar &a)
	{  return(a.is_one());  }
inline bool operator!=(const Scalar &a,Neutral0)
	{  return(!a.is_null());  }
inline bool operator!=(const Scalar &a,Neutral1)
	{  return(!a.is_one());  }
inline bool operator!=(Neutral0,const Scalar &a)
	{  return(!a.is_null());  }
inline bool operator!=(Neutral1,const Scalar &a)
	{  return(!a.is_one());  }


inline Scalar abs(const Scalar &a)
	{  return(fabs(a)); }

inline Scalar sqrt(const Scalar &a)
	{  return(::sqrt(double(a))); }

inline Scalar floor(const Scalar &a)
	{  return(::floor(double(a))); }

inline Scalar ceil(const Scalar &a)
	{  return(::ceil(double(a))); }

inline Scalar trunc(const Scalar &a)
	{  return(::floor(double(a))); }

inline Scalar round(const Scalar &a)
	{  return(::floor(double(a)+0.5)); }

inline std::ostream& operator<<(std::ostream& s,const Scalar &m)
	{  s << m.x;  return(s);  }

}  // namespace end 

#endif  /* _NS_vect_scalar_HPP_ */

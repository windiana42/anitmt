/*
 * valtype/vtrange.h
 * 
 * Range value (double..double) header. 
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

#ifndef _VALTYPES_VT_RANGE_H_
#define _VALTYPES_VT_RANGE_H_ 1

class Range : public ValueBase
{
	public:
		static const VType vtype=VTRange;
	private:
		RValidity valid;
		double a __attribute__((aligned(sizeof(double))));
		double b __attribute__((aligned(sizeof(double))));
		
		// Essentially does what operator-() does but this 
		// version modifies *this. 
		void _op_minus();
	public:
		// Constructor: 
		inline Range() : ValueBase(),valid(RInvalid) {}
		inline Range(const Range &r) : ValueBase(),
			valid(r.valid),a(r.a),b(r.b) {}
		inline Range(double _a,double _b,RValidity _v=RValidAB) : ValueBase(),
			valid(_v),a(_a),b(_b) {}
		// The _ignored parts should be set to RInvalid to give sense. 
		inline Range(double _a,RValidity /*_ignored*/) : ValueBase(),
			valid(RValidA),a(_a) {}
		inline Range(RValidity /*_ignored*/,double _b) : ValueBase(),
			valid(RValidB),b(_b) {}
		inline ~Range() {}
		
		// Get Validity flags: 
		inline RValidity Valid() const
			{  return(valid);  }
		// Get range values; vals are undefined if not valid. 
		inline double val_a() const
			{  return(a);  }
		inline double val_b() const
			{  return(b);  }
		
		// Assignment: 
		inline void set(const Range &r)
			{  a=r.a;  b=r.b;  valid=r.valid;  }
		inline void set(double _a,double _b)
			{  a=_a;  b=_b;  valid=RValidAB;  }
		inline Range &operator=(const Range &r)
			{  set(r);  return(*this);  }
		
		inline void set_a(double _a)
			{  a=_a;  valid=(RValidity)(valid|RValidA);  }
		inline void set_b(double _b)
			{  b=_b;  valid=(RValidity)(valid|RValidB);  }
		
		// Get length of the range or NAN. 
		inline double length() const
			{  return(valid==RValidAB ? (b-a) : (NAN));  }
		
		// Unary operators: 
		// Note: -(a..b) = -b..-a; validity flags swap as well. 
		inline Range operator+() const  {  return(*this);  }
		Range operator-() const;
		
		// These should only be used for RValidAB ranges: 
		// Add a constant offset to a range / subtract...
		inline Range &operator+=(double x)  {  a+=x;  b+=x;  return(*this);  }
		inline Range &operator-=(double x)  {  a-=x;  b-=x;  return(*this);  }
		friend Range operator+(const Range &c,double x);
		friend Range operator-(const Range &c,double x);
		friend Range operator+(double x,const Range &c);
		friend Range operator-(double x,const Range &c);
		// These should only be used for RValidAB ranges: 
		// Multiply range with scalar; note that for x<0 operator-() 
		// is invoked. 
		inline Range &operator*=(double x)
			{  if(x<0.0) { _op_minus(); x=-x; }  a*=x; b*=x; return(*this);  }
		inline Range &operator/=(double x)
			{  if(x<0.0) { _op_minus(); x=-x; }  a/=x; b/=x; return(*this);  }
		friend Range operator*(const Range &c,double x);
		friend Range operator/(const Range &c,double x);
		friend Range operator*(double x,const Range &c);
		
		// Compare ranges: 
		// The validity flags must match and the valid borders 
		// are compared using default_eps. 
		inline bool operator==(const Range &c) const
		{
			if(valid!=c.valid)  return(0);
			if((valid & RValidA) && fabs(a-c.a)>default_eps)  return(0);
			if((valid & RValidB) && fabs(b-c.b)>default_eps)  return(0);
			return(1);
		}
		inline bool operator!=(const Range &c) const
			{  return(!operator==(c));  }
		
		// Needed for templates...
		// A range is null if both borders are valid and the length 
		// of the range is <epsilon. 
		inline bool is_null() const
			{  return(valid==RValidAB && fabs(b-a)<=default_eps);  }
		
		// Make string representation of value [overridden virtual]: 
		String ToString() const;
		ValueBase *clone() const;   // <-- [overridden virtual]
		void forceset(ValueBase *vb);  //  <-- [overridden virtual]
}__attribute__((__packed__));


// These should only be used for RValidAB ranges: 
// Add a constant offset to a range / subtract...
inline Range operator+(const Range &c,double x)
	{  Range r;  r.valid=c.valid;  r.a=c.a+x;  r.b=c.b+x;  return(r);  }
inline Range operator-(const Range &c,double x)
	{  Range r;  r.valid=c.valid;  r.a=c.a-x;  r.b=c.b-x;  return(r);  }
inline Range operator+(double x,const Range &c)
	{  Range r;  r.valid=c.valid;  r.a=c.a+x;  r.b=c.b+x;  return(r);  }
inline Range operator-(double x,const Range &c)
	{  Range r;  r.valid=c.valid;  r.a=c.a-x;  r.b=c.b-x;  return(r);  }

// These should only be used for RValidAB ranges: 
// Multiply range with scalar; note that for x<0 operator-() 
// is invoked. 
inline Range operator*(const Range &c,double x)
	{  Range r(c);  r*=x;  return(r);  }
inline Range operator/(const Range &c,double x)
	{  Range r(c);  r/=x;  return(r);  }
inline Range operator*(double x,const Range &c)
	{  Range r(c);  r*=x;  return(r);  }


#endif  /* _VALTYPES_VT_RANGE_H_ */

/*
 * valtype/vtvector.
 * 
 * Vector value header. 
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

#ifndef _VALTYPES_VT_VECTOR_H_
#define _VALTYPES_VT_VECTOR_H_ 1

// This checks various important things; do not disable. 
#define vec_assert(x) assert(x)

#define TmpForAssignRet(__n,_ASS) \
	Vector tmp(Vector::NoInit,__n); \
	for(int i=0; i<__n; i++)  tmp[i]=_ASS; \
	return(tmp)


class Matrix;

class Vector : public ValueBase
{
	public:
		static const VType vtype=VTVector;
		// This is the place to tune the default vector size: 
		static const int default_n=3;
		
		enum _NullVect   { Null };
		enum _PromoVect  { Promo };  // promoted
		enum _NoInitVect { NoInit };
	private:
		// Number of elements of the vector (dimension). 
		int n;
		// Array of vector components [n]: 
		double *xp;
		
		inline void _EnsureDim(int dim)
			{  if(n!=dim)  xp=FREEALLOC(xp,n=dim);  }
		
		// Special constructor: pass ALLOC()ated mem and dim: 
		enum _DirectConstruct { _Direct };
		inline Vector(_DirectConstruct,double *_xp,int _n)
			{  xp=_xp;  n=_n;  }
	public:
		// Constructor: 
		inline Vector(const Vector &v) : ValueBase()
			{  xp=ALLOC<double>(n=v.n);  CopyArray(xp,v.xp,n);  }
		inline Vector() : ValueBase()  // Empty vector; be careful. 
			{  xp=NULL;  n=0;  }
		inline Vector(double x) : ValueBase()
			{  xp=ALLOC<double>(n=1);  xp[0]=x;  }
		inline Vector(double x,double y) : ValueBase()
			{  xp=ALLOC<double>(n=2);  xp[0]=x;  xp[1]=y;  }
		inline Vector(double x,double y,double z) : ValueBase()
			{  xp=ALLOC<double>(n=3);  xp[0]=x;  xp[1]=y;  xp[2]=z;  }
		inline Vector(double x,double y,double z,double t) : ValueBase()
			{  xp=ALLOC<double>(n=4);  xp[0]=x;  xp[1]=y;  xp[2]=z;  xp[3]=t;  }
		inline Vector(const double *_xp,int _n) : ValueBase()  // array _xp[_n]
			{  xp=ALLOC<double>(n=_n);  CopyArray(xp,_xp,n);  }
		// Create n-dim <0,0,0,0,0>: Vector(Vector::Null,n)
		inline Vector(_NullVect,int _n) : ValueBase()
			{  xp=ALLOC<double>(n=_n);  for(int i=0; i<n; i++)  xp[i]=0.0;  }
		// Create promoted vector <v,v,v,v,v>: Vector(Vector::Promo,v,5)
		inline Vector(_PromoVect,double v,int _n) : ValueBase()
			{  xp=ALLOC<double>(n=_n);  for(int i=0; i<n; i++)  xp[i]=v;  }
		// Special constructor: create uninitialized vector
		inline Vector(_NoInitVect,int _n) : ValueBase()
			{  xp=ALLOC<double>(n=_n);  }
		// Destructor: [overriding virtual]
		inline ~Vector()
			{  FREE(xp);  }
		
		// Assignment: 
		inline void set(const Vector &v)
			{  _EnsureDim(v.n);  CopyArray(xp,v.xp,n);  }
		inline void set(double x)
			{  _EnsureDim(1);  xp[0]=x;  }
		inline void set(double x,double y)
			{  _EnsureDim(2);  xp[0]=x;  xp[1]=y;  }
		inline void set(double x,double y,double z)
			{  _EnsureDim(3);  xp[0]=x;  xp[1]=y;  xp[2]=z;  }
		inline void set(double x,double y,double z,double t)
			{  _EnsureDim(4);  xp[0]=x;  xp[1]=y;  xp[2]=z;  xp[3]=t;  }
		inline void set(double *_xp,int _n)   // array _xp[_n]
			{  _EnsureDim(_n);  CopyArray(xp,_xp,n);  }
		inline void set(_NullVect)   // won't change dim
			{  for(int i=0; i<n; i++)  xp[i]=0.0; }
		inline void set(_NullVect,int _n)  // will change dim
			{  _EnsureDim(_n);  for(int i=0; i<n; i++)  xp[i]=0.0; }
		inline void set(_PromoVect,double v)  // won't change dim
			{  for(int i=0; i<n; i++)  xp[i]=v;  }
		inline void set(_PromoVect,double v,int _n)  // will change dim
			{  _EnsureDim(_n);  for(int i=0; i<n; i++)  xp[i]=v;  }
		inline Vector &operator=(const Vector &v)
			{  set(v);  return(*this);  }
		inline Vector &operator=(_NullVect)   // won't change dim
			{  set(Vector::Null);  return(*this);  }
		
		// Get size of vector: 
		inline int dim() const
			{  return(n);  }
		
		// Get vector component: 
		// This returns a reference and can thus be used as 
		// lvalue, too. NO RANGE CKECK. 
		inline double &operator[](int i)  {  return(xp[i]);  }
		// rvalue, only: 
		inline double operator[](int i) const  {  return(xp[i]);  }
		inline double val(int i) const  {  return(xp[i]);  }
		
		// And now the arithmetics. 
		// Note that vector size (dimension) mismatch will lead to abort. 
		
		// Unary operators: 
		inline Vector operator+() const  {  return(*this);  }
		inline Vector operator-() const  {  TmpForAssignRet(n,-xp[i]);  }
		
		// Addition/Subtraction of two vectors: 
		friend Vector operator+(const Vector &a,const Vector &b);
		friend Vector operator-(const Vector &a,const Vector &b);
		
		// Multiplication/division of a vector with/by a scalar: 
		friend Vector operator*(const Vector &a,double b);
		friend Vector operator*(double a,const Vector &b);
		friend Vector operator/(const Vector &a,double b);
		
		inline Vector &operator+=(const Vector &b)
			{
				vec_assert(n==b.n);
				for(int i=0; i<n; i++)  xp[i]+=b.xp[i];
				return(*this);
			}
		inline Vector &operator-=(const Vector &b)
			{
				vec_assert(n==b.n);
				for(int i=0; i<n; i++)  xp[i]-=b.xp[i];
				return(*this);
			}
		inline Vector &operator*=(double b)
			{  for(int i=0; i<n; i++)  xp[i]*=b;  return(*this);  }
		inline Vector &operator/=(double b)
			{  for(int i=0; i<n; i++)  xp[i]/=b;  return(*this);  }
		
		// Multiplication of matrix and (column) vector:
		friend Vector operator*(const Matrix &a,const Vector &b);
		// There is NO *= operator because it needs a temporary anyways. 
		
		// SCALAR multiplication:
		friend double dot(const Vector &a,const Vector &b);
		
		// VECTOR multiplication: (3d vectors only)
		friend Vector cross(const Vector &a,const Vector &b);
		
		// Comparing vectors: 
		inline bool is_null(double eps) const
			{ for(int i=0;i<n;i++) if(fabs(xp[i])>eps) return(0);  return(1); }
		inline bool is_null() const   // <-- overrides a virtual. 
			{  return(is_null(default_eps));  }
		// These operators simply use default_eps: 
		inline bool operator==(const Vector &b) const
		{
			vec_assert(n==b.n);
			for(int i=0; i<n; i++)
			{  if(fabs(xp[i]-b.xp[i])>Vector::default_eps) return(0);  }
			return(1);
		}
		inline bool operator!=(const Vector &b) const
			{  return(!operator==(b));  }
		// This is a special routine which checks 
		// |a-b| / ( |a|+|b| ) < eps   in case   |a|+|b| > 1e-100. 
		friend bool is_equal(const Vector &a,const Vector &b,
			double eps=Vector::default_eps);
		
		// Return vector length and its square (the latter is faster): 
		double abs2() const
			{ double r=0.0; for(int i=0;i<n;i++) r+=xp[i]*xp[i]; return(r); }
		double abs()  const  {  return(sqrt(abs2()));  }
		
		// Stretch vector to length 1: 
		Vector &normalize()
			{ double f=abs(); for(int i=0;i<n;i++) xp[i]*=f;  return(*this); }
		friend Vector normalize(const Vector &v);
		
		// Make string representation of value [overridden virtual]: 
		String ToString() const;
		ValueBase *clone() const;   // <-- [overridden virtual]
		void forceset(ValueBase *vb);  //  <-- [overridden virtual]
}__attribute__((__packed__));


// Addition/Subtraction of two vectors: 
inline Vector operator+(const Vector &a,const Vector &b)
	{  vec_assert(a.n==b.n);  TmpForAssignRet(a.n,a.xp[i]+b.xp[i]);  }
inline Vector operator-(const Vector &a,const Vector &b)
	{  vec_assert(a.n==b.n);  TmpForAssignRet(a.n,a.xp[i]-b.xp[i]);  }

// Multiplication/division of a vector with/by a scalar: 
inline Vector operator*(const Vector &a,double b)
	{  TmpForAssignRet(a.n,a.xp[i]*b);  }
inline Vector operator*(double a,const Vector &b)
	{  TmpForAssignRet(b.n,a*b.xp[i]);  }
inline Vector operator/(const Vector &a,double b)
	{  TmpForAssignRet(a.n,a.xp[i]/b);  }

// SCALAR multiplication:
inline double dot(const Vector &a,const Vector &b)
{
	double res=0.0;
	vec_assert(a.n==b.n);
	for(int i=0; i<a.n; i++)
	{  res+=a.xp[i]*b.xp[i];  }
	return(res);
}

// VECTOR multiplication: (3d vectors only)
inline Vector cross(const Vector &a,const Vector &b)
{
	vec_assert(a.n==3 && b.n==3);
	Vector res(Vector::NoInit,3);
	res[0]=a.xp[2]*b.xp[3]-a.xp[3]*b.xp[2];
	res[1]=a.xp[3]*b.xp[1]-a.xp[1]*b.xp[3];
	res[2]=a.xp[1]*b.xp[2]-a.xp[2]*b.xp[1];
	return(res);
}

#if 0   /* Now Member functions. */
// Comparing vectors: 
inline bool operator==(const Vector &a,const Vector &b)
{
	vec_assert(a.n==b.n);
	for(int i=0; i<a.n; i++)
	{  if(fabs(a.xp[i]-b.xp[i])>Vector::default_eps) return(0);  }
	return(1);
}
inline bool operator!=(const Vector &a,const Vector &b)
{  return(!operator==(a,b));  }
#endif

inline bool is_equal(const Vector &a,const Vector &b,double eps)
{
	vec_assert(a.n==b.n);
	for(int i=0; i<a.n; i++) {  register double s=fabs(a.xp[i])+fabs(b.xp[i]);
		if(s>=1.0e-100 && fabs(a.xp[i]-b.xp[i])/s>eps) return(0);  }
	return(1);
}

// Length of vector: 
inline double abs(const Vector &v)   {  return(v.abs());  }
inline double abs2(const Vector &v)  {  return(v.abs2());  }

// Stretch vector to length 1: 
inline Vector normalize(const Vector &v)
	{  double f=1.0/v.abs();  TmpForAssignRet(v.n,v.xp[i]*f);  }


#undef TmpForAssignRet

#endif  /* _VALTYPES_VT_VECTOR_H_ */

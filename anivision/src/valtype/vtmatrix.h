/*
 * valtype/vtmatrix.h
 * 
 * Matrix value header. 
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

#ifndef _VALTYPES_VT_MATRIX_H_
#define _VALTYPES_VT_MATRIX_H_ 1

// This checks various important things; do not disable. 
#define mat_assert(x) assert(x)

#define FOR_RC \
	for(short int r=0; r<nr; r++) \
		for(short int c=0; c<nc; c++)
#define FOR_RC2(__nr,__nc) \
	for(short int r=0; r<__nr; r++) \
		for(short int c=0; c<__nc; c++)

#define TmpForAssignRet(__nr,__nc,_ASS) \
	Matrix tmp(Matrix::NoInit,__nr,__nc); \
	for(short int r=0; r<__nr; r++) \
		for(short int c=0; c<__nc; c++)  tmp[r][c]=_ASS; \
	return(tmp)


class Matrix : public ValueBase
{
	public:
		static const VType vtype=VTMatrix;
		// This is the place to tune the default matrix size: 
		static const short int default_r=3;
		static const short int default_c=3;
		
		enum _NullMat   { Null };
		enum _IdentMat  { Ident };  // identity
		enum _NoInitMat { NoInit };
	private:
		// Number of rows and columns in the matrix: 
		short int nr,nc;
		// Array of matrix elements [nr*nc] = [nr][nc]
		double *xp;
		
		// Index array; rvalue and lvalue; use always. 
		inline double &x(short int r,short int c)
			{  return(xp[int(nc)*r+c]);  }
		inline double x(short int r,short int c) const
			{  return(xp[int(nc)*r+c]);  }
		inline double *xptr(short int r)  { return(xp+int(nc)*r); }
		inline const double *xptr(short int r) const { return(xp+int(nc)*r); }
		
		inline void _EnsureDim(short int r,short int c)
		{
			int n=int(r)*c;
			if(n!=size())  xp=FREEALLOC(xp,n);
			nr=r; nc=c;  // even if n==_n. 
		}
		
		inline int size() const
			{  return(int(nr)*int(nc));  }
		
		// Special constructor: pass ALLOC()ated mem and nr,nc: 
		enum _DirectConstruct { _Direct };
		inline Matrix(_DirectConstruct,double *_xp,short int _r,short int _c)
			{  xp=_xp;  nr=_r;  nc=_c;  }
	public:
		// Constructor: 
		inline Matrix(const Matrix &m) : ValueBase()
			{  nr=m.nr; nc=m.nc; int n=size(); xp=ALLOC<double>(n);
				CopyArray(xp,m.xp,n);  }
		inline Matrix() : ValueBase()  // Empty matrix; be careful. 
			{  xp=NULL;  nr=0;  nc=0;  }
		// Create r x c null-matrix: Matrix(Matrix::Null,r,c)
		inline Matrix(_NullMat,short int _r,short int _c) : ValueBase()
			{  nr=_r; nc=_c;  xp=ALLOC<double>(size());  FOR_RC x(r,c)=0.0;  }
		// Create r x c identity-matrix: Matrix(Matrix::Ident,r,c)
		inline Matrix(_IdentMat,short int _r,short int _c) : ValueBase()
			{  nr=_r; nc=_c;  xp=ALLOC<double>(size());
				FOR_RC x(r,c) = (r==c ? 1.0 : 0.0);  }
		// Special constructor: create uninitialized matrix
		inline Matrix(_NoInitMat,short int _r,short int _c) : ValueBase()
			{  nr=_r; nc=_c;  xp=ALLOC<double>(size());  }
		// Destructor: [overriding virtual]
		inline ~Matrix()
			{  FREE(xp);  }
		
		// Assignment: 
		inline void set(const Matrix &m)
			{  _EnsureDim(m.nr,m.nc);  CopyArray(xp,m.xp,size());  }
		inline void set(_NullMat)     // won't change size
			{  FOR_RC x(r,c)=0.0;  }
		inline void set(_NullMat,short int _r,short int _c) // will change size
			{  _EnsureDim(_r,_c);  FOR_RC x(r,c)=0.0;  }
		inline void set(_IdentMat)     // won't change size
			{  FOR_RC x(r,c) = (r==c ? 1.0 : 0.0);  }
		inline void set(_IdentMat,short int _r,short int _c) // will change size
			{  _EnsureDim(_r,_c);  FOR_RC x(r,c) = (r==c ? 1.0 : 0.0);  }
		inline Matrix &operator=(const Matrix &m)
			{  set(m);  return(*this);  }
		inline Matrix &operator=(_NullMat)
			{  set(Matrix::Null);  return(*this);  }
		inline Matrix &operator=(_IdentMat)
			{  set(Matrix::Ident);  return(*this);  }
		
		// Get size of matrix: 
		inline int rows() const  { return(nr); }
		inline int cols() const  { return(nc); }
		
		// Get matrix component: 
		// This returns a reference and can thus be used as 
		// lvalue, too. NO RANGE CKECK. 
		//   use: Matrix m;  m[r][c]=1.0;
		inline double *operator[](int r)  {  return(xptr(r));  }
		inline const double *operator[](int r) const  {  return(xptr(r));  }
		// rvalue only: 
		inline double val(short int r,short int c) const  // Y,X
			{  return(xptr(r)[c]);  }
		
		// And now the arithmetics. 
		// Note that matrix size mismatch will lead to abort. 
		
		// Unary operators: 
		inline Matrix operator+() const  {  return(*this);  }
		inline Matrix operator-() const  {  TmpForAssignRet(nr,nc,-x(r,c));  }
		
		// Addition/Subtraction of two matrices: 
		friend Matrix operator+(const Matrix &a,const Matrix &b);
		friend Matrix operator-(const Matrix &a,const Matrix &b);
		
		// Multiplication/division of a matrix with/by a scalar: 
		friend Matrix operator*(const Matrix &a,double b);
		friend Matrix operator*(double a,const Matrix &b);
		friend Matrix operator/(const Matrix &a,double b);
		
		inline Matrix &operator+=(const Matrix &b)
			{
				mat_assert(nr==b.nr && nc==b.nc);
				FOR_RC  x(r,c)+=b.x(r,c);
				return(*this);
			}
		inline Matrix &operator-=(const Matrix &b)
			{
				mat_assert(nr==b.nr && nc==b.nc);
				FOR_RC  x(r,c)-=b.x(r,c);
				return(*this);
			}
		inline Matrix &operator*=(double b)
			{  FOR_RC  x(r,c)*=b;  return(*this);  }
		inline Matrix &operator/=(double b)
			{  FOR_RC  x(r,c)/=b;  return(*this);  }
		
		// Multiplication of matrix and matrix: 
		friend Matrix operator*(const Matrix &a,const Matrix &b);
		
		// Multiplication of matrix and (column) vector:
		friend Vector operator*(const Matrix &a,const Vector &b);
		
		// Comparing matrices: 
		inline bool is_null(double eps) const
			{  FOR_RC  if(fabs(x(r,c))>eps) return(0);  return(1);  }
		inline bool is_null() const  // <-- overrides a virtual. 
			{  return(is_null(default_eps));  }
		inline bool is_ident(double eps=default_eps) const
			{  FOR_RC  if(fabs(r==c ? (x(r,c)-1.0) : x(r,c))>eps) return(0);
				return(1);  }
		// These operators simply use default_eps: 
		inline bool operator==(const Matrix &b) const
		{
			mat_assert(nr==b.nr && nc==b.nc);
			FOR_RC2(nr,nc) if(fabs(x(r,c)-b[r][c])>Matrix::default_eps) return(0);
			return(1);
		}
		inline bool operator!=(const Matrix &b) const
			{  return(!operator==(b));  }
		// This is a special routine which checks 
		// |a-b| / ( |a|+|b| ) < eps   in case   |a|+|b| > 1e-100. 
		friend bool is_equal(const Matrix &a,const Matrix &b,
			double eps=Matrix::default_eps);
		
		// Transpose the matrix; must be square matrix for that. 
		// (Use friend function for non-square matrices.) 
		inline Matrix &transpose()
		{  mat_assert(nr==nc);
			for(short int r=1; r<nr; r++) for(short int c=0; c<r; c++)
			{ double tmp=x(r,c); x(c,r)=x(c,r); x(c,r)=tmp; }  return(*this); }
		friend Matrix transpose(const Matrix &m);  // also non-seqare
		
		// Get track of matrix (must be square): 
		inline double track() const
		{  assert(nr==nc); double s=0.0;
			for(short int i=0;i<nr;i++) s+=x(i,i);  return(s);  }
		
		// Make string representation of value [overridden virtual]: 
		String ToString() const;
		ValueBase *clone() const;   // <-- [overridden virtual]
		void forceset(ValueBase *vb);  //  <-- [overridden virtual]
}__attribute__((__packed__));


// Addition/Subtraction of two matrices: 
inline Matrix operator+(const Matrix &a,const Matrix &b)
{
	mat_assert(a.nr==b.nr && a.nc==b.nc);
	TmpForAssignRet(a.nr,a.nc,a.x(r,c)+b.x(r,c));
}
inline Matrix operator-(const Matrix &a,const Matrix &b)
{
	mat_assert(a.nr==b.nr && a.nc==b.nc);
	TmpForAssignRet(a.nr,a.nc,a.x(r,c)-b.x(r,c));
}

// Multiplication/division of a matrix with/by a scalar: 
inline Matrix operator*(const Matrix &a,double b)
	{  TmpForAssignRet(a.nr,a.nc,a.x(r,c)*b);  }
inline Matrix operator*(double a,const Matrix &b)
	{  TmpForAssignRet(b.nr,b.nc,a*b.x(r,c));  }
inline Matrix operator/(const Matrix &a,double b)
	{  TmpForAssignRet(a.nr,a.nc,a.x(r,c)/b);  }

#if 0   /* Now Member functions. */
// Comparing matrices: 
inline bool operator==(const Matrix &a,const Matrix &b)
{
	mat_assert(a.nr==b.nr && a.nc==b.nc);
	FOR_RC2(a.nr,a.nc) if(fabs(a[r][c]-b[r][c])>Matrix::default_eps) return(0);
	return(1);
}
inline bool operator!=(const Matrix &a,const Matrix &b)
{  return(!operator==(a,b));  }
#endif

inline bool is_equal(const Matrix &a,const Matrix &b,double eps)
{
	mat_assert(a.nr==b.nr && a.nc==b.nc);
	FOR_RC2(a.nr,a.nc) { register double s=fabs(a[r][c])+fabs(b[r][c]);
		if(s>=1.0e-100 && fabs(a[r][c]-b[r][c])/s>eps) return(0);  }
	return(1);
}

// Transpose matrix: 
inline Matrix transpose(const Matrix &m)
	{  TmpForAssignRet(m.nc,m.nr,m[c][r]);  }
// Track of matrix (must be square): 
inline double track(const Matrix &m)
	{  return(m.track());  }

#undef TmpForAssignRet
#undef FOR_RC
#undef FOR_RC2

#endif  /* _VALTYPES_VT_MATRIX_H_ */

/*
 * matrix.hpp
 * 
 * Matrix template; header for matrix value type. 
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

#ifndef _NS_vect_matrix_HPP_
#define _NS_vect_matrix_HPP_ 1

#ifndef GCC_HACK
 #if __GNUC__ < 3
 #define GCC_HACK 1
 #warning GCC_HACK enabled as gcc version < 3.0
 #warning **** PLEASE PASS -fno-access-control IN YOUR CXXFLAGS! ****
 #else
 #undef GCC_HACK
 #endif
#endif

#include "vector.hpp"

namespace vect
{

template<int C=4,int R=4>class Matrix
{
	private:
		internal_vect::matrix<C,R> x;
		enum NoInit { noinit };
		// This constructor is fast as it does no initialisation: 
		Matrix(NoInit) : x() {}
	public:
		enum NullMat { null };
		enum IdentMat { ident };
		enum MatRotX { matrotx };
		enum MatRotY { matroty };
		enum MatRotZ { matrotz };
		enum MatScale { matscale };
		enum MatTrans { mattrans };
		
		// Copy constructor: 
		Matrix(const Matrix<C,R> &m) : x(m.x)  {}
		//Matrix(const internal_vect::matrix<C,R> &m) : x(m)  {}
		// These generate an initialized identity matrix. 
		Matrix()         : x(0)  {}
		Matrix(IdentMat) : x(0)  {}
		// This generates an initialized null matrix. 
		Matrix(NullMat) : x()  {  x.set_null(); }
		
		// You may (of couse) use these functions but the non-member 
		// functions below (MrotateX(),...) are more convenient. 
		// * These are only available for 4x4 matrices in combination 
		// * with 3d vectors. 
		Matrix(enum MatRotX,double angle);
		Matrix(enum MatRotY,double angle);
		Matrix(enum MatRotZ,double angle);
		Matrix(enum MatScale,double fact,int idx);  // idx unchecked. 
		Matrix(enum MatScale,const Vector<3> &v);
		Matrix(enum MatTrans,double delta,int idx);  // idx unchecked. 
		Matrix(enum MatTrans,const Vector<3> &v);
		
		// Assignment operator 
		Matrix<C,R> &operator=(const Matrix<C,R> &m)  {  x=m.x;  return(*this);  }
		
		~Matrix()  {}
		
		//operator internal_vect::matrix<C,R>() const  {  return(x);  }
		
		/************************************/
		/* COLUMNS -> c -> ``X coordinate'' */
		/* ROWS    -> r -> ``Y coordinate'' */
		/* ORDER: ALWAYS c,r  ( -> x,y)     */
		/************************************/
		
		// This returns the i-th column of the vector; use a second index 
		// operator to get a single value. 
		// Using Matrix mat, you can access every element by calling 
		//   double val=mat[c][r];
		// FOR SPEED INCREASE, NO RANGE CHECK IS PERFORMED ON c, r. 
		internal_vect::matrix_column<R> operator[](int c)  {  return(x[c]);  }
		
		// Set an element of the matrix: c is the column index, r the row index. 
		// NO RANGE CHECK IS PERFORMED ON c,r. 
		// Returns *this. 
		Matrix<C,R> &operator()(int c,int r,double val)
		{  x(c,r,val);  return(*this); }
		
		// Multiplication/Division of a matrix and/by a scalar: 
		// (Divides/multiplies every element of the matrix.) 
		template<int c,int r>friend Matrix<c,r> operator*(const Matrix<c,r> &a,Scalar b);
		template<int c,int r>friend Matrix<c,r> operator*(Scalar a,const Matrix<c,r> &b);
		template<int c,int r>friend Matrix<c,r> operator/(const Matrix<c,r> &a,Scalar b);
		
		// Multiplication/Division of a matrix and/by a scalar changing *this 
		// and returning it. 
		Matrix<C,R> &operator*=(Scalar b)  {  x.mul(b);  return(*this);  }
		Matrix<C,R> &operator/=(Scalar b)  {  x.div(b);  return(*this);  }
		
		// Multiplication of a matrix and a vector: 
		// (Vector::operator*=(Matrix) is also available.)
		template<int c,int r>friend Vector<r> operator*(const Matrix<c,r> &a,const Vector<c> &b);
		template<int c,int r>friend Vector<r> operator*(const Vector<c> &a,const Matrix<c,r> &b);
		// Special versions: 
		friend Vector<3> operator*(const Matrix<4,4> &a,const Vector<3> &b);
		friend Vector<3> operator*(const Vector<3> &a,const Matrix<4,4> &b);
		
		// Multiplication of a matrix and a matrix: 
		template<int M,int L,int N>friend Matrix<L,M> operator*(
			const Matrix<N,L> &a,const Matrix<N,M> &b);
		// Version changing *this and returning it: 
		// (only for quadratic matrices -> use of <C,C> instead of <C,R>)
		Matrix<C,C> &operator*=(const Matrix<C,C> &b)
			{  x.mul(b.x);  return(*this);  }
		
		// Friend needed for vector * matrix calculation. 
		// This returns void for speed increase. 
		template<int N>friend void operator*=(Vector<N> &v,const Matrix<N,N> &m);
		// Special version: 
		friend void operator*=(Vector<3> &v,const Matrix<4,4> &m);
		
		// Addition/Subtraction of two matrices (element-by-elemnt). 
		template<int c,int r>friend Matrix<c,r> operator+(const Matrix<c,r> &a,const Matrix<c,r> &b);
		template<int c,int r>friend Matrix<c,r> operator-(const Matrix<c,r> &a,const Matrix<c,r> &b);
		Matrix<C,R> &operator+=(const Matrix<C,R> &b)  {  x.add(b.x);  return(*this);  }
		Matrix<C,R> &operator-=(const Matrix<C,R> &b)  {  x.sub(b.x);  return(*this);  }
		
		// Unary operators: 
		Matrix<C,R> operator+() const {  return(*this);  } 
		Matrix<C,R> operator-() const {  Matrix<C,R> r(noinit);  r.x.neg(x);   return(r);  }
		
		// Operators comparing matrices (are using epsilon): 
		template<int c,int r>friend bool operator==(const Matrix<c,r> &,const Matrix<c,r> &);
		template<int c,int r>friend bool operator!=(const Matrix<c,r> &,const Matrix<c,r> &);
		
		// Returns 1, if this matrix is the identity-matrix (uses epsilon). 
		bool operator!() const {  return(x.is_ident(epsilon));  }
		bool is_ident()  const {  return(x.is_ident(epsilon));  }
		
		// Returns 1, if this matrix is the null-matrix (uses epsilon). 
		bool is_null() const {  return(x.is_null(epsilon));  }
		
		// These functions calculate the inverse matrix. 
		Matrix<C,R> &invert()  {  x.invert();  return(*this);  }
		template<int c,int r>friend Matrix<c,r> invert(const Matrix<c,r> &m);
		
		// Print matrix to stream: 
		template<int c,int r>friend ostream& operator<<(ostream& s,const Matrix<c,r> &m);
};

#ifdef GCC_HACK
inline Matrix<4,4>::Matrix(enum MatRotX,double angle) : x(0)
{
	double sina=sin(angle);
	double cosa=cos(angle);
	x(1,1, cosa);  x(2,1,-sina);
	x(1,2, sina);  x(2,2, cosa);
}

inline Matrix<4,4>::Matrix(enum MatRotY,double angle) : x(0)
{
	double sina=sin(angle);
	double cosa=cos(angle);
	x(0,0, cosa);  x(2,0, sina);
	x(0,2,-sina);  x(2,2, cosa);
}

inline Matrix<4,4>::Matrix(enum MatRotZ,double angle) : x(0)
{
	double sina=sin(angle);
	double cosa=cos(angle);
	x(0,0, cosa);  x(1,0,-sina);
	x(0,1, sina);  x(1,1, cosa);
}

inline Matrix<4,4>::Matrix(enum MatScale,double fact,int idx) : x(0)
{
	x(idx,idx,fact);
}

inline Matrix<4,4>::Matrix(enum MatScale,const Vector<3> &v) : x(0)
{
	x(0,0,v[0]);
	x(1,1,v[1]);
	x(2,2,v[2]);
}

inline Matrix<4,4>::Matrix(enum MatTrans,double delta,int idx) : x(0)
{
	x(3,idx,delta);
}

inline Matrix<4,4>::Matrix(enum MatTrans,const Vector<3> &v) : x(0)
{
	x(3,0,v[0]);
	x(3,1,v[1]);
	x(3,2,v[2]);
}
#endif

template<int C,int R>inline Matrix<C,R> operator*(const Matrix<C,R> &a,Scalar b)
{  Matrix<C,R> r(Matrix<C,R>::noinit);  r.x.mul(a.x,b);  return(r);  }
template<int C,int R>inline Matrix<C,R> operator*(Scalar a,const Matrix<C,R> &b)
{  Matrix<C,R> r(Matrix<C,R>::noinit);  r.x.mul(b.x,a);  return(r);  }
template<int C,int R>inline Matrix<C,R> operator/(const Matrix<C,R> &a,Scalar b)
{  Matrix<C,R> r(Matrix<C,R>::noinit);  r.x.div(a.x,b);  return(r);  }

template<int C,int R>inline Vector<R> operator*(const Matrix<C,R> &m,const Vector<C> &v)
{  Vector<R> r(Vector<R>::noinit);  internal_vect::mult(r.x,m.x,v.x);  return(r);  }
template<int C,int R>inline Vector<R> operator*(const Vector<C> &v,const Matrix<C,R> &m)
{  Vector<R> r(Vector<R>::noinit);  internal_vect::mult(r.x,m.x,v.x);  return(r);  }
// Special versions: 
inline Vector<3> operator*(const Matrix<4,4> &m,const Vector<3> &v)
{  Vector<3> r(Vector<3>::noinit);  internal_vect::mult(r.x,m.x,v.x);  return(r);  }
inline Vector<3> operator*(const Vector<3> &v,const Matrix<4,4> &m)
{  Vector<3> r(Vector<3>::noinit);  internal_vect::mult(r.x,m.x,v.x);  return(r);  }

template<int C,int R>inline Matrix<C,R> operator+(const Matrix<C,R> &a,const Matrix<C,R> &b)
{  Matrix<C,R> r(Matrix<C,R>::noinit);  r.x.add(a.x,b.x);   return(r);  }
template<int C,int R>inline Matrix<C,R> operator-(const Matrix<C,R> &a,const Matrix<C,R> &b)
{  Matrix<C,R> r(Matrix<C,R>::noinit);  r.x.sub(a.x,b.x);   return(r);  }

// This one must use a temporary: (returns void for speed increase) 
template<int N>inline void operator*=(Vector<N> &v,const Matrix<N,N> &m)
{  Vector<N> tmp(Vector<N>::noinit); internal_vect::mult(tmp.x,m.x,v.x);  v=tmp;  }
// Special version:
inline void operator*=(Vector<3> &v,const Matrix<4,4> &m)
{  Vector<3> tmp(Vector<3>::noinit); internal_vect::mult(tmp.x,m.x,v.x);  v=tmp;  }

template<int M,int L,int N>inline Matrix<L,M> operator*(
	const Matrix<N,L> &a,const Matrix<N,M> &b)
{  Matrix<L,M> r(Matrix<L,M>::noinit);  internal_vect::mult(r.x,a.x,b.x);  return(r);  }


template<int C,int R>inline bool operator==(const Matrix<C,R> &a,const Matrix<C,R> &b)
{  return(a.x.compare_to(b.x,epsilon));  }
template<int C,int R>inline bool operator!=(const Matrix<C,R> &a,const Matrix<C,R> &b)
{  return(!a.x.compare_to(b.x,epsilon));  }

template<int C,int R>inline Matrix<C,R> invert(const Matrix<C,R> &m)
{  Matrix<C,R> r(Matrix<C,R>::noinit);  r.x.invert(m.x);  return(r);  }

template<int C,int R>inline ostream& operator<<(ostream& s,const Matrix<C,R> &m)
{  return(internal_vect::operator<<(s,m.x));  }

// **** Constructing matrices: ****
// THIS IS ONLY AVAILABLE FOR 4x4 MATRICES: 
// Scalation (?) matrices: 
inline Matrix<4,4> MscaleX(double fact)  {  return(Matrix<4,4>(Matrix<4,4>::matscale,fact,0));  }
inline Matrix<4,4> MscaleY(double fact)  {  return(Matrix<4,4>(Matrix<4,4>::matscale,fact,1));  }
inline Matrix<4,4> MscaleZ(double fact)  {  return(Matrix<4,4>(Matrix<4,4>::matscale,fact,2));  }
inline Matrix<4,4> Mscale(const Vector<3> &v)  {  return(Matrix<4,4>(Matrix<4,4>::matscale,v));  }
// Translation matrices: 
inline Matrix<4,4> MtranslateX(double d)  {  return(Matrix<4,4>(Matrix<4,4>::mattrans,d,0));  }
inline Matrix<4,4> MtranslateY(double d)  {  return(Matrix<4,4>(Matrix<4,4>::mattrans,d,1));  }
inline Matrix<4,4> MtranslateZ(double d)  {  return(Matrix<4,4>(Matrix<4,4>::mattrans,d,2));  }
inline Matrix<4,4> Mtranslate(const Vector<3> &v)  {  return(Matrix<4,4>(Matrix<4,4>::mattrans,v));  }
// Rotation matrices: 
inline Matrix<4,4> MrotateX(double angle)  {  return(Matrix<4,4>(Matrix<4,4>::matrotx,angle));  }
inline Matrix<4,4> MrotateY(double angle)  {  return(Matrix<4,4>(Matrix<4,4>::matroty,angle));  }
inline Matrix<4,4> MrotateZ(double angle)  {  return(Matrix<4,4>(Matrix<4,4>::matrotz,angle));  }
// Rotates around x,y and z in this order; angles stored in v: 
inline Matrix<4,4> Mrotate(const Vector<3> &v)
	{  return(MrotateX(v[0])*MrotateY(v[1])*MrotateZ(v[2]));  }


/** FUNCTIONS FOR 3d VECTORS AND 4x4 MATRICES: **/
//! rotates a specified angle around v 
extern Matrix<4,4> Mrotate_around(const Vector<3> &v,double angle);
//! rotates a vector to another
extern Matrix<4,4> Mrotate_vect_vect(const Vector<3> &from,const Vector<3> &to);
/*! rotates a vector to another by using a sperical rotation with the
    horizontal plane defined by the normal vector "up" */
extern Matrix<4,4> Mrotate_vect_vect_up(const Vector<3> &from,
	const Vector<3> &to,const Vector<3> &up);
/*! rotates a vector pair to another
    the first vectors of each pair will mach exactly afterwards but the second
    may differ in the angle to the first one. They will be in the same plane
    then. */
extern Matrix<4,4> Mrotate_pair_pair(
	const Vector<3> &vect1f,const Vector<3> &vect1u,
	const Vector<3> &vect2f,const Vector<3> &vect2u);
/*! spherical rotation with the horizontal plane defined through
    the normal vector up and the front vector 
    the x-coordinate of angles is used for the rotation around front
    the y-coordinate of angles is used for the rotation around up
    and the z-coordiante specifies the angle to go up from the plane */
extern Matrix<4,4> Mrotate_spherical_pair(
	const Vector<3> &front,const Vector<3> &up,const Vector<3> &angles);

//! Get the rotation from v1 to v2 around axis 
extern double get_rotation_around(
	const Vector<3> &v1,const Vector<3> &v2,const Vector<3> &axis);

/*! returns a vector which tells the resulting scalation of an x,y and z 
    vector when being multipied with Matrix m.
    To reproduce the effect of m: scale, then rotate and finally translate */
extern Vector<3> get_scale_component(const Matrix<4,4> &mat);
/*! returns a vector representing the rotation around the x, y and finally 
    the z axis that are done to a vector when being multiplied with the 
    Matrix m.
    To reproduce the effect of m: scale, then rotate and finally translate */
extern Vector<3> get_rotate_component(const Matrix<4,4> &mat);
/*! returns the position independant
    translation as vector, that is done to a vector when being multipied
    with the Matrix m
    To reproduce the effect of m: scale, then rotate and finally translate */
extern Vector<3> get_translate_component(const Matrix<4,4> &mat);

}  // namespace end 

#endif  /* _NS_vect_vector_HPP_ */

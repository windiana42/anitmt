/*
 * matrix.hpp
 * 
 * Matrix template; header for matrix value type. 
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser (wwieser@gmx.de) 
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


template<int R,int C>inline Matrix<R,C> mat_transpose(const Matrix<C,R> &m)
{  Matrix<R,C> r(Matrix<R,C>::noinit);  r.x.transpose(m.x);  return(r);  }

template<int N>inline Matrix<N,N> mat_inverse(const Matrix<N,N> &m)
{  Matrix<N,N> r(Matrix<N,N>::noinit);  r.x.inverse(m.x);  return(r);  }

template<int N>inline double mat_determinant(const Matrix<N,N> &m)
{  return(m.x.determinant());  }

template<int N>inline double mat_track(const Matrix<N,N> &m)
{  return(m.x.track());  }


// MATRIX * SCALAR: 
template<int R,int C>inline Matrix<R,C> operator*(const Matrix<R,C> &a,Scalar b)
{  Matrix<R,C> r(Matrix<R,C>::noinit);  r.x.mul(a.x,b);  return(r);  }
template<int R,int C>inline Matrix<R,C> operator*(Scalar a,const Matrix<R,C> &b)
{  Matrix<R,C> r(Matrix<R,C>::noinit);  r.x.mul(b.x,a);  return(r);  }
template<int R,int C>inline Matrix<R,C> operator/(const Matrix<R,C> &a,Scalar b)
{  Matrix<R,C> r(Matrix<R,C>::noinit);  r.x.div(a.x,b);  return(r);  }

// MATRIX * VECTOR is in vector.hpp. 

// MATRIX * MATRIX: 
template<int M,int L,int N>inline 
	Matrix<M,L> operator*(const Matrix<M,N> &a,const Matrix<N,L> &b)
{  Matrix<M,L> r(Matrix<M,L>::noinit);  internal_vect::mult(r.x,a.x,b.x);  return(r);  }

template<int R,int C>inline Matrix<R,C> operator+(const Matrix<R,C> &a,const Matrix<R,C> &b)
{  Matrix<R,C> r(Matrix<R,C>::noinit);  r.x.add(a.x,b.x);   return(r);  }
template<int R,int C>inline Matrix<R,C> operator-(const Matrix<R,C> &a,const Matrix<R,C> &b)
{  Matrix<R,C> r(Matrix<R,C>::noinit);  r.x.sub(a.x,b.x);   return(r);  }

template<int R,int C>inline bool operator==(const Matrix<R,C> &a,const Matrix<R,C> &b)
{  return(a.x.compare_to(b.x,epsilon));  }
template<int R,int C>inline bool operator!=(const Matrix<R,C> &a,const Matrix<R,C> &b)
{  return(!a.x.compare_to(b.x,epsilon));  }

// Neutral compare functions (also using epsilon): 
// Neutral0: compare against null matrix
// Neutral1: compare against identity matrix
template<int R,int C>inline bool operator==(const Matrix<R,C> &a,Neutral0)
{  return(a.is_null());  }
template<int R,int C>inline bool operator==(const Matrix<R,C> &a,Neutral1)
{  return(a.is_ident());  }
template<int R,int C>inline bool operator==(Neutral0,const Matrix<R,C> &a)
{  return(a.is_null());  }
template<int R,int C>inline bool operator==(Neutral1,const Matrix<R,C> &a)
{  return(a.is_ident());  }
template<int R,int C>inline bool operator!=(const Matrix<R,C> &a,Neutral0)
{  return(!a.is_null());  }
template<int R,int C>inline bool operator!=(const Matrix<R,C> &a,Neutral1)
{  return(!a.is_ident());  }
template<int R,int C>inline bool operator!=(Neutral0,const Matrix<R,C> &a)
{  return(!a.is_null());  }
template<int R,int C>inline bool operator!=(Neutral1,const Matrix<R,C> &a)
{  return(!a.is_ident());  }


template<int R,int C>inline std::ostream& operator<<(std::ostream& s,const Matrix<R,C> &m)
{  return(internal_vect::operator<<(s,m.x));  }


template<int R=4,int C=4>class Matrix
{
	private:
#ifdef GCC_HACK
public: // work around for the template friend problem
#endif
		internal_vect::matrix<R,C> x;
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
		Matrix(const Matrix<R,C> &m) : x(m.x)  {}
		//Matrix(const internal_vect::matrix<R,C> &m) : x(m)  {}
		// These generate an initialized identity matrix. 
		Matrix()         : x(0)  {}
		Matrix(IdentMat) : x(0)  {}
		// This generates an initialized null matrix. 
		Matrix(NullMat) : x()  {  x.set_null(); }
		// Neutral matrices (like null and ident)
		Matrix(Neutral1) : x(0)  {}
		Matrix(Neutral0) : x()  {  x.set_null(); }
		
		// You may (of couse) use these functions but the non-member 
		// functions below (MrotateX(),...) are more convenient. 
		// * These are only available for 4x4 matrices in combination 
		// * with 3d vectors. 
		Matrix(enum MatRotX,double angle);
		Matrix(enum MatRotY,double angle);
		Matrix(enum MatRotZ,double angle);
		Matrix(enum MatScale,double fact,int idx);  // idx unchecked. 
		Matrix(enum MatScale,double fact);  // scale all
		Matrix(enum MatScale,const Vector<3> &v);
		Matrix(enum MatTrans,double delta,int idx);  // idx unchecked. 
		Matrix(enum MatTrans,const Vector<3> &v);
		
		// Assignment operator 
		Matrix<R,C> &operator=(const Matrix<R,C> &m)  {  x=m.x;  return(*this);  }
		Matrix<R,C> &operator=(NullMat)    {  x.set_null();   return(*this);  }
		Matrix<R,C> &operator=(IdentMat)   {  x.set_ident();  return(*this);  }
		Matrix<R,C> &operator=(Neutral0)  {  x.set_null();   return(*this);  }
		Matrix<R,C> &operator=(Neutral1)  {  x.set_ident();  return(*this);  }
		
		~Matrix()  {}
		
		//operator internal_vect::matrix<R,C>() const  {  return(x);  }
		
		// Get matrix size (that's the template argument; this function is 
		// only useful for auto-generated code): 
		int get_nrows()    const  {  return(R);  }
		int get_ncolumns() const  {  return(C);  }
		
		/************************************/
		/* ROWS    -> r -> ``Y coordinate'' */
		/* COLUMNS -> c -> ``X coordinate'' */
		/* ORDER: ALWAYS r,c  ( -> y,x)     */
		/************************************/
		
		// This returns the i-th row of the matrix; use a second index 
		// operator to get a single value. 
		// Using Matrix mat, you can access every element by calling 
		//   double val=mat[r][c];
		// ...and modify them using 
		//   mat[r][c]=val;
		// FOR SPEED INCREASE, NO RANGE CHECK IS PERFORMED ON r, c. 
		internal_vect::matrix_row<C> operator[](int r)  {  return(x[r]);  }
		
		// Multiplication/Division of a matrix and/by a scalar: 
		// (Divides/multiplies every element of the matrix.) 
		#ifndef GCC_HACK
		template<int r,int c>friend Matrix<r,c> operator*(const Matrix<r,c> &a,Scalar b);
		template<int r,int c>friend Matrix<r,c> operator*(Scalar a,const Matrix<r,c> &b);
		template<int r,int c>friend Matrix<r,c> operator/(const Matrix<r,c> &a,Scalar b);
		#endif
		
		// Multiplication/Division of a matrix and/by a scalar changing *this 
		// and returning it. 
		Matrix<R,C> &operator*=(Scalar b)  {  x.mul(b);  return(*this);  }
		Matrix<R,C> &operator/=(Scalar b)  {  x.div(b);  return(*this);  }
		
		// Multiplication of a matrix and a vector: 
		// (Vector::operator*=(Matrix) is also available.)
		#ifndef GCC_HACK
		template<int r,int c>friend Vector<r> operator*(const Matrix<r,c> &a,const Vector<c> &b);
		template<int r,int c>friend Vector<r> operator*(const Vector<c> &a,const Matrix<r,c> &b);
		// Special versions: 
		friend Vector<3> operator*(const Matrix<4,4> &a,const Vector<3> &b);
		friend Vector<3> operator*(const Vector<3> &a,const Matrix<4,4> &b);
		#endif
		
		#ifndef GCC_HACK
		// Multiplication of a matrix and a matrix: 
		template<int M,int L,int N>friend Matrix<M,L> operator*(
			const Matrix<M,N> &a,const Matrix<N,L> &b);
		#endif
		// Version changing *this and returning it: 
		// (only for quadratic matrices -> use of <R,R> instead of <R,C>)
		Matrix<R,R> &operator*=(const Matrix<R,R> &b)
			{  x.mul(b.x);  return(*this);  }
		
		// Friend needed for vector * matrix calculation. 
		// This returns void for speed increase. 
		template<int N>friend void operator*=(Vector<N> &v,const Matrix<N,N> &m);
		#ifndef GCC_HACK
		// Special version: 
		friend void operator*=(Vector<3> &v,const Matrix<4,4> &m);
		#endif
		
		#ifndef GCC_HACK
		// Addition/Subtraction of two matrices (element-by-elemnt). 
		template<int r,int c>friend Matrix<r,c> operator+(const Matrix<r,c> &a,const Matrix<r,c> &b);
		template<int r,int c>friend Matrix<r,c> operator-(const Matrix<r,c> &a,const Matrix<r,c> &b);
		#endif
		Matrix<R,C> &operator+=(const Matrix<R,C> &b)  {  x.add(b.x);  return(*this);  }
		Matrix<R,C> &operator-=(const Matrix<R,C> &b)  {  x.sub(b.x);  return(*this);  }
		
		// Unary operators: 
		Matrix<R,C> operator+() const {  return(*this);  } 
		Matrix<R,C> operator-() const {  Matrix<R,C> r(noinit);  r.x.neg(x);   return(r);  }
		
		#ifndef GCC_HACK
		// Operators comparing matrices (are using epsilon): 
		template<int r,int c>friend bool operator==(const Matrix<r,c> &,const Matrix<r,c> &);
		template<int r,int c>friend bool operator!=(const Matrix<r,c> &,const Matrix<r,c> &);
		#endif
		
		// Returns 1, if this matrix is the identity-matrix (uses epsilon). 
		bool operator!() const {  return(x.is_ident(epsilon));  }
		bool is_ident()  const {  return(x.is_ident(epsilon));  }
		
		// Returns 1, if this matrix is the null-matrix (uses epsilon). 
		bool is_null() const {  return(x.is_null(epsilon));  }
		
		// Neutral compare functions (also using epsilon): 
		// Neutral0: compare against null matrix
		// Neutral1: compare against identity matrix
		// non-member: operator==/!=(const Matrix<r,c> &,Neutral[01])
		// non-member: operator==/!=(Neutral[01],const Matrix<r,c> &)
		
		// These functions calculate the inverse matrix. 
		Matrix<R,R> &inverse()  {  x.inverse();  return(*this);  }
		#ifndef GCC_HACK
		template<int N>friend Matrix<N,N> mat_inverse(const Matrix<N,N> &m);
		#endif
		
		// Tramspose a matrix. Obviously, the member function can only 
		// operate on quadratic matrices: 
		Matrix<R,R> &transpose()  {  x.transpose();  return(*this);  }
		#ifndef GCC_HACK
		template<int r,int c>friend Matrix<r,c> mat_transpose(const Matrix<c,r> &m);
		#endif
		
		// Calculate the determinant of a matrix: 
		// Only operates on quadratic matrices. 
		// (Will give compile error for non-quadratic matrices in imatrix.hpp.)
		double determinant() const  {  return(x.determinant());  }
		#ifndef GCC_HACK
		template<int N>friend double mat_determinant(const Matrix<N,N> &m);
		#endif
		
		// Return the track of the matrix (the sum of the diagonal 
		// elements). Works only for quadratic matrices. 
		// (Will give compile error for non-quadratic matrices in imatrix.hpp.)
		double track() const  {  return(x.track());  }
		#ifndef GCC_HACK
		template<int N>friend double mat_track(const Matrix<N,N> &m);
		#endif
		
		#ifndef GCC_HACK
		// Print matrix to stream: 
		template<int r,int c>friend std::ostream& operator<<(std::ostream& s,const Matrix<r,c> &m);
		#endif
};

#ifdef GCC_HACK
inline Matrix<4,4>::Matrix(enum MatRotX,double angle) : x(0)
{
	double sina=sin(angle);
	double cosa=cos(angle);
	x[1][1]=cosa;  x[1][2]=-sina;
	x[2][1]=sina;  x[2][2]= cosa;
}

inline Matrix<4,4>::Matrix(enum MatRotY,double angle) : x(0)
{
	double sina=sin(angle);
	double cosa=cos(angle);
	x[0][0]= cosa;  x[0][2]= sina;
	x[2][0]=-sina;  x[2][2]= cosa;
}

inline Matrix<4,4>::Matrix(enum MatRotZ,double angle) : x(0)
{
	double sina=sin(angle);
	double cosa=cos(angle);
	x[0][0]= cosa;  x[0][1]=-sina;
	x[1][0]= sina;  x[1][1]= cosa;
}

inline Matrix<4,4>::Matrix(enum MatScale,double fact,int idx) : x(0)
{
	x[idx][idx]=fact;
}

inline Matrix<4,4>::Matrix(enum MatScale,double fact) : x(0)
{
	x[0][0]=fact;
	x[1][1]=fact;
	x[2][2]=fact;
}

inline Matrix<4,4>::Matrix(enum MatScale,const Vector<3> &v) : x(0)
{
	x[0][0]=v[0];
	x[1][1]=v[1];
	x[2][2]=v[2];
}

inline Matrix<4,4>::Matrix(enum MatTrans,double delta,int idx) : x(0)
{
	x[idx][3]=delta;
}

inline Matrix<4,4>::Matrix(enum MatTrans,const Vector<3> &v) : x(0)
{
	x[0][3]=v[0];
	x[1][3]=v[1];
	x[2][3]=v[2];
}
#endif


// Special versions: 
inline Vector<3> operator*(const Matrix<4,4> &m,const Vector<3> &v)
{  Vector<3> r(Vector<3>::noinit);  internal_vect::mult(r.x,m.x,v.x);  return(r);  }
inline Vector<3> operator*(const Vector<3> &v,const Matrix<4,4> &m)
{  Vector<3> r(Vector<3>::noinit);  internal_vect::mult(r.x,m.x,v.x);  return(r);  }

inline void operator*=(Vector<3> &v,const Matrix<4,4> &m)
{  Vector<3> tmp(Vector<3>::noinit); internal_vect::mult(tmp.x,m.x,v.x);  v=tmp;  }


// **** Constructing matrices: ****
// THIS IS ONLY AVAILABLE FOR 4x4 MATRICES: 
// Scalation (?) matrices: 
inline Matrix<4,4> mat_scale_x(double fact)  {  return(Matrix<4,4>(Matrix<4,4>::matscale,fact,0));  }
inline Matrix<4,4> mat_scale_y(double fact)  {  return(Matrix<4,4>(Matrix<4,4>::matscale,fact,1));  }
inline Matrix<4,4> mat_scale_z(double fact)  {  return(Matrix<4,4>(Matrix<4,4>::matscale,fact,2));  }
inline Matrix<4,4> mat_scale(const Vector<3> &v)  {  return(Matrix<4,4>(Matrix<4,4>::matscale,v));  }
// Scale all axes: 
inline Matrix<4,4> mat_scale(double fact)   {  return(Matrix<4,4>(Matrix<4,4>::matscale,fact));  }
// Translation matrices: 
inline Matrix<4,4> mat_translate_x(double d)  {  return(Matrix<4,4>(Matrix<4,4>::mattrans,d,0));  }
inline Matrix<4,4> mat_translate_y(double d)  {  return(Matrix<4,4>(Matrix<4,4>::mattrans,d,1));  }
inline Matrix<4,4> mat_translate_z(double d)  {  return(Matrix<4,4>(Matrix<4,4>::mattrans,d,2));  }
inline Matrix<4,4> mat_translate(const Vector<3> &v)  {  return(Matrix<4,4>(Matrix<4,4>::mattrans,v));  }
// Rotation matrices: 
inline Matrix<4,4> mat_rotate_x(double angle)  {  return(Matrix<4,4>(Matrix<4,4>::matrotx,angle));  }
inline Matrix<4,4> mat_rotate_y(double angle)  {  return(Matrix<4,4>(Matrix<4,4>::matroty,angle));  }
inline Matrix<4,4> mat_rotate_z(double angle)  {  return(Matrix<4,4>(Matrix<4,4>::matrotz,angle));  }
// Rotates around x,y and z in this order; angles stored in v: 
inline Matrix<4,4> mat_rotate(const Vector<3> &v)
	{  return(mat_rotate_z(v[2])*mat_rotate_y(v[1])*mat_rotate_x(v[0]));  }


/** FUNCTIONS FOR 3d VECTORS AND 4x4 MATRICES: **/
//! rotates a specified angle around v 
extern Matrix<4,4> mat_rotate_around(const Vector<3> &v,double angle);
//! rotates a vector to another
extern Matrix<4,4> mat_rotate_vect_vect(const Vector<3> &from,const Vector<3> &to);
/*! rotates a vector to another by using a sperical rotation with the
    horizontal plane defined by the normal vector "up" */
extern Matrix<4,4> mat_rotate_vect_vect_up(const Vector<3> &from,
	const Vector<3> &to,const Vector<3> &up);
/*! rotates a vector pair to another
    the first vectors of each pair will match exactly afterwards but the second
    may differ in the angle to the first one. They will be in the same plane
    then. */
extern Matrix<4,4> mat_rotate_pair_pair(
	const Vector<3> &vect1f,const Vector<3> &vect1u,
	const Vector<3> &vect2f,const Vector<3> &vect2u);
/*! spherical rotation with the horizontal plane defined through
    the normal vector up and the front vector 
    the x-coordinate of angles is used for the rotation around front
    the y-coordinate of angles is used for the rotation around up
    and the z-coordiante specifies the angle to go up from the plane */
extern Matrix<4,4> mat_rotate_spherical_pair(
	const Vector<3> &front,const Vector<3> &up,const Vector<3> &angles);

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

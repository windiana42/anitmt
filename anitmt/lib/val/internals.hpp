/*
 * internals.hpp
 * 
 * This is a header for internal use by the value library. 
 * 
 * Copyright (c) 2000--2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _NS_internal_vect_internals_HPP_
#define _NS_internal_vect_internals_HPP_ 1

#include <stddef.h>
#include <math.h>

#include <iostream.h>


namespace internal_vect
{
template<int N> class vector;
template<int R> class matrix_column;
template<int C,int R> class matrix;

// Matrix exceptions: 
class EX_Matrix {};
// More detailed exceptions all derived from EX_Matrix: 
class EX_Matrix_Illegal_Mult     : EX_Matrix { };
class EX_Matrix_Illegal_Invert   : EX_Matrix { };
class EX_Matrix_Illegal_VectMult : EX_Matrix { };

// Multiply matrix * matrix: 
template<int M,int L,int N> void mult(matrix<L,M> &r,const matrix<N,L> &a,const matrix<N,M> &b);
// Multiply matrix * vector: 
void mult(vector<3> &r,const matrix<4,4> &m,const vector<3> &v); // special
template<int C,int R> void mult(vector<R> &r,const matrix<C,R> &m,const vector<C> &v);

// Write vector and matrix to ostream: 
template<int N> ostream& operator<<(ostream &s,const vector<N> &v);
template<int C,int R> ostream& operator<<(ostream &s,const matrix<C,R> &m);

namespace internal
{
	// Multiplication of a matrix with a vector: 
	extern void matrix_mul_vect(
			  double *rv,int rn,
		const double *m,int mc,int mr,
		const double *v,int vn)  throw(internal_vect::EX_Matrix_Illegal_VectMult);
	// Special function: multiply a 3d vector with a 4x4 matrix and 
	// get a 3d vector. 
	// The exception is thrown if the 4th element of the result is 
	// different from 1. 
	extern void matrix_mul_vect343(double *rv,const double *m,const double *v);
}

}  /* end of namespace internal_vect */

#include "ivector.hpp"
#include "imatrix.hpp"

/*** Inline Functions which take Vector and Matrix arguments: ***/

namespace internal_vect
{

// Function to multiply the vector v with matrix m, storing the resulting 
// vector in r. 
template<int C,int R> inline 
	void mult(vector<R> &r,const matrix<C,R> &m,const vector<C> &v)
	{  internal::matrix_mul_vect(r.x,R,m.x[0],C,R,v.x,C);  }

// Special functions: 
inline void mult(vector<3> &r,const matrix<4,4> &m,const vector<3> &v)
{  internal::matrix_mul_vect343(r.x,m.x[0],v.x);  }

} /* end of namespace internal_vect */

#endif  /* _NS_internal_vect_internals_HPP_ */

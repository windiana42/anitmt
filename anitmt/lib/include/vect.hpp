/*
 * vect.hpp
 * 
 * Provides prototypes and includes all files needed 
 * for namespace vect. 
 * 
 * ********************************************************************
 * ** YOU ALMOST CERTAINLY DO NOT WANT TO INCLUDE vector.hpp OR      **
 * ** matrix.hpp INTO YOUR SOURCE. USE THIS FILE (vect.hpp) INSTEAD. **
 * ********************************************************************
 * 
 * This is a part of the aniTMT animation project. 
 * 
 * Copyright (c) 2000--2001 by Wolfgang Wieser
 * Bugs, suggestions to wwieser@gmx.de. 
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 * 
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 * 
 * This program is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the 
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * 
 * See the GNU General Public License for details.
 * If you have not received a copy of the GNU General Public License,
 * write to the 
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * Revision History:
 *   Dec 2000   started writing
 *
 */

#ifndef __vect_vect_hpp__
#define __vect_vect_hpp__

#include <math.h>

namespace vect
{
	template<int N> class vector;
	template<int R> class matrix_column;
	template<int C,int R> class matrix;
	
	// Matrix exceptions: 
	class EX_Matrix {};
	class EX_Matrix_Illegal_Mult     : EX_Matrix {};
	class EX_Matrix_Illegal_Invert   : EX_Matrix {};
	class EX_Matrix_Illegal_VectMult : EX_Matrix {};
	
	// Multiply matrix * matrix: 
	template<int M,int L,int N> void mult(matrix<L,M> &r,const matrix<N,L> &a,const matrix<N,M> &b);
	// Multiply matrix * vector: 
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
			const double *v,int vn)  throw(vect::EX_Matrix_Illegal_VectMult);
	}
}

#include "vector.hpp"
#include "matrix.hpp"

/*** Inline Functions which take Vector and Matrix arguments: ***/

namespace vect
{

// Function to multiply the vector v with matrix m, storing the resulting 
// vector in r. 
template<int C,int R> inline 
	void mult(vector<R> &r,const matrix<C,R> &m,const vector<C> &v)
	{  internal::matrix_mul_vect(r._get_ptr(),R,m.x[0],C,R,v._get_ptr(),C);  }


} /* end of namespace vect */

#endif  /* __vect_vect_hpp__ */

/*
 * imatrix.hpp
 *
 * Header file containing an internally used matrix template. 
 * Quite likely, you are looking for matrix.hpp...
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


#ifndef _NS_internal_vect_imatrix_HPP_
#define _NS_internal_vect_imatrix_HPP_ 1

// NOTE: You should gain speed if you apply loop unrolling 
//       here (gcc -funroll-loops). 

// Ugly macros increase readability and reduce amount of source. 
#define FOR(_c_,_r_)  \
	for(int _c_=0; _c_<C; _c_++) \
		for(int _r_=0; _r_<R; _r_++)
#define FORN(_c_,_r_)  \
	for(int _c_=0; _c_<N; _c_++) \
		for(int _r_=0; _r_<N; _r_++)


namespace internal_vect
{

// Internal: 
namespace internal
{
	// Suffix 2 for two-dim array (matrix). 
	extern ostream& stream_write_array2(ostream& s,const double *x,int c,int r);
	
	// Multiplication of two matrices: 
	extern void matrix_mul(
		      double *rm,int rc,int rr, 
		const double *am,int ac,int ar, 
		const double *bm,int bc,int br) throw(internal_vect::EX_Matrix_Illegal_Mult);
	extern void matrix_mul(
		      double *rm,int rc,int rr, 
		const double *bm,int bc,int br) throw(internal_vect::EX_Matrix_Illegal_Mult);
	
	// Inversion of a matrix: 
	extern void matrix_invert(        // THIS MODIFIES m!!
		      double *rm,int rc,int rr, 
		      double *m, int c, int r)  throw(internal_vect::EX_Matrix_Illegal_Invert);
	extern void matrix_invert_copy(   // Does not modify m. 
		      double *rm,int rc,int rr, 
		const double *m, int c, int r)  throw(internal_vect::EX_Matrix_Illegal_Invert);
	extern void matrix_invert_copy(   // Returns inverted matrix in m. 
		      double *m,int c,int r)  throw(internal_vect::EX_Matrix_Illegal_Invert);
}


// used by matrix::operator[]. 
template<int R> class matrix_column
{
	private:
		double *x;
	public:
		matrix_column(double *_x) : x(_x)  {}
		
		// Returns the value indexed with r in the column represented 
		// by *this. 
		// NO RANGE CHECK IS PERFORMED ON r. 
		double operator[](int r) const  {  return(x[r]);  }
};


// C: number of columns, R: number of rows. 
template<int C,int R> class matrix
{
	private:
		double x[C][R];
	public:
		// Damn function which returns a pointer to the first element of x. 
		// It is needed as long as I do not succeed in making the functions 
		// which use it friends of matrix. 
		// Either I am too stupid for this or the compiler too pedantic or 
		// C++ does not allow me to do this. ($@#&%>3*!!!)
		double *_get_ptr()  const  {  return((double*)(x[0]));  }
		
		// Constructor which generates an uninitialized matrix: 
		matrix()     { }
		// Constructor which generates the identity-matrix: 
		//   1 0 0 
		//   0 1 0
		//   0 0 1
		matrix(int)  {  FOR(c,r)  x[c][r] = (c==r) ? 1.0 : 0.0;  }
		// Copy-constructor: 
		matrix(const matrix<C,R> &m)  {  FOR(c,r)  x[c][r]=m.x[c][r];  }
		
		// Assignment operator: 
		matrix<C,R> &operator=(const matrix<C,R> &m)
			{  FOR(c,r) x[c][r]=m.x[c][r];  return(*this);  }
		
		// Set this to a null-matrix / to a identity-matrix. 
		void set_null()   {  FOR(c,r)  x[c][r]=0.0;  }
		void set_ident()  {  FOR(c,r)  x[c][r] = (c==r) ? 1.0 : 0.0;  }
		
		// Get matrix_column indexed c: 
		// Do not use matrix_column in your code; only use its 
		// operator[] to be able to access any element of the matrix: 
		//   matrix<3,3> mat;
		//   double val = mat[c][r];
		// NO RANGE CHECK IS PERFORMED ON c. 
		matrix_column<R> operator[](int c)
			{  return(matrix_column<R>(x[c]));  }
		
		// Set an element of the matrix: 
		// NO RANGE CHECK IS PERFORMED ON c,r. 
		matrix<C,R> &operator()(int c,int r,double val)
			{  x[c][r]=val;  return(*this); }
		
		/**************************************************************/
		/* Functions which can be applied to non-initialized matrices */
		/* as they overwrite the content of *this:                    */
		
		matrix<C,R> &mul(const matrix<C,R> &a,double b)
			{            FOR(c,r)  x[c][r]=a.x[c][r]*b;  return(*this);  }
		matrix<C,R> &div(const matrix<C,R> &a,double b)
			{  b=1.0/b;  FOR(c,r)  x[c][r]=a.x[c][r]*b;  return(*this);  }
		
		matrix<C,R> &add(const matrix<C,R> &a,const matrix<C,R> &b)
			{  FOR(c,r)  x[c][r]=a.x[c][r]+b.x[c][r];  return(*this);  }
		matrix<C,R> &sub(const matrix<C,R> &a,const matrix<C,R> &b)
			{  FOR(c,r)  x[c][r]=a.x[c][r]-b.x[c][r];  return(*this);  }
		
		matrix<C,R> &neg(const matrix<C,R> &a)
			{  FOR(c,r)  x[c][r]=-a.x[c][r];  return(*this);  }
		
		// Inverts the matrix m and stores the inverted matrix in *this. 
		// NOTE: Only quadratic matrices may be inverted; if C!=R, 
		//       EX_Matrix_Illegal_Invert is thrown. 
		// (The identity-matrix initialisation is important.) 
		matrix<C,R> &invert(const matrix<C,R> &m)
			{  set_ident();  internal::matrix_invert_copy(x[0],C,R,m.x[0],C,R);
			   return(*this);  }
		
		/**************************************************************/
		/* Functions taking *this as argument a and overwriting *this */
		/* with the result:                                           */
		
		matrix<C,R> &mul(double b)
			{            FOR(c,r)  x[c][r]*=b;  return(*this);  }
		matrix<C,R> &div(double b)
			{  b=1.0/b;  FOR(c,r)  x[c][r]*=b;  return(*this);  }
		
		matrix<C,R> &add(const matrix<C,R> &b)
			{  FOR(c,r)  x[c][r]+=b.x[c][r];  return(*this);  }
		matrix<C,R> &sub(const matrix<C,R> &b)
			{  FOR(c,r)  x[c][r]-=b.x[c][r];  return(*this);  }
		
		matrix<C,R> &neg()
			{  FOR(c,r)  x[c][r]*=-1.0;  return(*this);  }
		
		// NOTE: This function will throw EX_Matrix_Illegal_Mult, if C!=R. 
		// There is also a more general multiplication function available; 
		// see below. 
		matrix<C,R> &mul(const matrix<C,R> &b)
			{  internal::matrix_mul(x[0],C,R,b.x[0],C,R);  return(*this);  }
		
		// Inverts the matrix *this. 
		// NOTE: Only quadratic matrices may be inverted; if C!=R, 
		//       EX_Matrix_Illegal_Invert is thrown. 
		matrix<C,R> &invert()
			{  internal::matrix_invert_copy(x[0],C,R);  return(*this);  }
		
		// Function to multiply the vector v with matrix m, storing the 
		// resulting vector in r. 
		friend void mult<>(vector<R> &r,const matrix<C,R> &m,const vector<C> &v);
		friend void mult(vector<3> &r,const matrix<4,4> &m,const vector<3> &v);
		
		friend ostream& operator<< <>(ostream &s,const matrix<C,R> &m);
		
		// Comparing matrices: 
		// Returns 1, if a is equal to *this (or each component pair does not 
		// differ more than epsilon). 
		int compare_to(const matrix<C,R> &a,double epsilon) const 
			{  FOR(c,r)  if(fabs(x[c][r]-a.x[c][r])>epsilon) return(0);  return(1);  }
		
		// Returns 1, if this is a null-matrix (no component > epsilon). 
		int is_null(double epsilon) const 
			{  FOR(c,r)  if(fabs(x[c][r])>epsilon) return(0);  return(1);  }
		
		// Returns 1, if this is an identity-matrix. 
		int is_ident(double epsilon) const  {
			FOR(c,r)
				if(fabs((c==r) ? (x[c][r]-1.0) : (x[c][r])) > epsilon)
					return(0);
			return(1);
		}
};


// Function to multiply the two matrices a and b storing the result in r. 
template<int M,int L,int N> inline 
	void mult(matrix<L,M> &r,const matrix<N,L> &a,const matrix<N,M> &b)
	{  internal::matrix_mul(r._get_ptr(),L,M,a._get_ptr(),N,L,b._get_ptr(),N,M);  }

template<int C,int R> inline 
	ostream& operator<<(ostream &s,const matrix<C,R> &m)
	{  return(internal::stream_write_array2(s,m.x[0],C,R));  }

}  /* end of namespace */

#undef FOR
#undef FORN

#endif  /* _NS_internal_vect_imatrix_HPP_  */

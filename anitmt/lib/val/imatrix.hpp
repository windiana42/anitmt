/*
 * imatrix.hpp
 *
 * Header file containing an internally used matrix template. 
 * Quite likely, you are looking for matrix.hpp...
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

#ifndef _NS_internal_vect_imatrix_HPP_
#define _NS_internal_vect_imatrix_HPP_ 1

// NOTE: You should gain speed if you apply loop unrolling 
//       here (gcc -funroll-loops). 

// Ugly macros increase readability and reduce amount of source. 
#define _mFOR(_c_,_r_)  \
	for(int _c_=0; _c_<C; _c_++) \
		for(int _r_=0; _r_<R; _r_++)
#define _mFORN(_c_,_r_)  \
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
		// NO RANGE CHECK IS PER_mFORMED ON r. 
		double operator[](int r) const  {  return(x[r]);  }
};


// C: number of columns, R: number of rows. 
template<int C,int R> class matrix
{
	private:
		double x[C][R];
	public:
		// Constructor which generates an uninitialized matrix: 
		matrix()     { }
		// Constructor which generates the identity-matrix: 
		//   1 0 0 
		//   0 1 0
		//   0 0 1
		matrix(int)  {  _mFOR(c,r)  x[c][r] = (c==r) ? 1.0 : 0.0;  }
		// Copy-constructor: 
		matrix(const matrix<C,R> &m)  {  _mFOR(c,r)  x[c][r]=m.x[c][r];  }
		
		// Assignment operator: 
		matrix<C,R> &operator=(const matrix<C,R> &m)
			{  _mFOR(c,r) x[c][r]=m.x[c][r];  return(*this);  }
		
		// Set this to a null-matrix / to a identity-matrix. 
		void set_null()   {  _mFOR(c,r)  x[c][r]=0.0;  }
		void set_ident()  {  _mFOR(c,r)  x[c][r] = (c==r) ? 1.0 : 0.0;  }
		
		// Get matrix_column indexed c: 
		// Do not use matrix_column in your code; only use its 
		// operator[] to be able to access any element of the matrix: 
		//   matrix<3,3> mat;
		//   double val = mat[c][r];
		// NO RANGE CHECK IS PER_mFORMED ON c. 
		matrix_column<R> operator[](int c)
			{  return(matrix_column<R>(x[c]));  }
		
		// Set an element of the matrix: 
		// NO RANGE CHECK IS PER_mFORMED ON c,r. 
		matrix<C,R> &operator()(int c,int r,double val)
			{  x[c][r]=val;  return(*this); }
		
		/**************************************************************/
		/* Functions which can be applied to non-initialized matrices */
		/* as they overwrite the content of *this:                    */
		
		matrix<C,R> &mul(const matrix<C,R> &a,double b)
			{            _mFOR(c,r)  x[c][r]=a.x[c][r]*b;  return(*this);  }
		matrix<C,R> &div(const matrix<C,R> &a,double b)
			{  b=1.0/b;  _mFOR(c,r)  x[c][r]=a.x[c][r]*b;  return(*this);  }
		
		matrix<C,R> &add(const matrix<C,R> &a,const matrix<C,R> &b)
			{  _mFOR(c,r)  x[c][r]=a.x[c][r]+b.x[c][r];  return(*this);  }
		matrix<C,R> &sub(const matrix<C,R> &a,const matrix<C,R> &b)
			{  _mFOR(c,r)  x[c][r]=a.x[c][r]-b.x[c][r];  return(*this);  }
		
		matrix<C,R> &neg(const matrix<C,R> &a)
			{  _mFOR(c,r)  x[c][r]=-a.x[c][r];  return(*this);  }
		
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
			{            _mFOR(c,r)  x[c][r]*=b;  return(*this);  }
		matrix<C,R> &div(double b)
			{  b=1.0/b;  _mFOR(c,r)  x[c][r]*=b;  return(*this);  }
		
		matrix<C,R> &add(const matrix<C,R> &b)
			{  _mFOR(c,r)  x[c][r]+=b.x[c][r];  return(*this);  }
		matrix<C,R> &sub(const matrix<C,R> &b)
			{  _mFOR(c,r)  x[c][r]-=b.x[c][r];  return(*this);  }
		
		matrix<C,R> &neg()
			{  _mFOR(c,r)  x[c][r]*=-1.0;  return(*this);  }
		
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
		template<int c,int r>friend void mult(vector<r> &r,const matrix<c,r> &m,const vector<c> &v);
		friend void mult(vector<3> &r,const matrix<4,4> &m,const vector<3> &v);
		
		// Matrix multiplication friend: 
		template<int M,int L,int N>friend void mult(matrix<L,M> &r,
			const matrix<N,L> &a,const matrix<N,M> &b);
		
		template<int c,int r>friend ostream& operator<<(ostream &s,const matrix<c,r> &m);
		
		// Comparing matrices: 
		// Returns 1, if a is equal to *this (or each component pair does not 
		// differ more than epsilon). 
		int compare_to(const matrix<C,R> &a,double epsilon) const 
			{  _mFOR(c,r)  if(fabs(x[c][r]-a.x[c][r])>epsilon) return(0);  return(1);  }
		
		// Returns 1, if this is a null-matrix (no component > epsilon). 
		int is_null(double epsilon) const 
			{  _mFOR(c,r)  if(fabs(x[c][r])>epsilon) return(0);  return(1);  }
		
		// Returns 1, if this is an identity-matrix. 
		int is_ident(double epsilon) const  {
			_mFOR(c,r)
				if(fabs((c==r) ? (x[c][r]-1.0) : (x[c][r])) > epsilon)
					return(0);
			return(1);
		}
};


// Function to multiply the two matrices a and b storing the result in r. 
template<int M,int L,int N> inline 
	void mult(matrix<L,M> &r,const matrix<N,L> &a,const matrix<N,M> &b)
	{  internal::matrix_mul(r.x[0],L,M,a.x[0],N,L,b.x[0],N,M);  }

template<int C,int R> inline 
	ostream& operator<<(ostream &s,const matrix<C,R> &m)
	{  return(internal::stream_write_array2(s,m.x[0],C,R));  }

}  /* end of namespace */

#undef _mFOR
#undef _mFORN

#endif  /* _NS_internal_vect_imatrix_HPP_  */

/*
 * imatrix.hpp
 *
 * Header file containing an internally used matrix template. 
 * Quite likely, you are looking for matrix.hpp...
 *
 * Copyright (c) 2000--2002 by Wolfgang Wieser (wwieser@gmx.de) 
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
#define _mFOR(_r_,_c_)  \
	for(int _r_=0; _r_<R; _r_++) \
		for(int _c_=0; _c_<C; _c_++)
#define _mFORN(_r_,_c_)  \
	for(int _r_=0; _r_<N; _r_++) \
		for(int _c_=0; _c_<N; _c_++)


namespace internal_vect
{

// Internal: 
namespace internal
{
	// Suffix 2 for two-dim array (matrix). 
	extern std::ostream& stream_write_array2(std::ostream& s,const double *x,int r,int c);
	
	// Multiplication of two matrices: 
	extern void matrix_mul(
		      double *rm,int rr,int rc, 
		const double *am,int ar,int ac, 
		const double *bm,int br,int bc) throw(internal_vect::EX_Matrix_Illegal_Mult);
	extern void matrix_mul(
		      double *rm,int rr,int rc, 
		const double *bm,int br,int bc) throw(internal_vect::EX_Matrix_Illegal_Mult);
	
	// Inversion of a matrix: 
	extern void matrix_invert(        // THIS MODIFIES m!!
		      double *rm,int rr,int rc, 
		      double *m, int r, int c)  throw(internal_vect::EX_Matrix_Illegal_Invert);
	extern void matrix_invert_copy(   // Does not modify m. 
		      double *rm,int rr,int rc, 
		const double *m, int r, int c)  throw(internal_vect::EX_Matrix_Illegal_Invert);
	extern void matrix_invert_copy(   // Returns inverted matrix in m. 
		      double *m,int r,int c)  throw(internal_vect::EX_Matrix_Illegal_Invert);
	
	// Transpose matrix: 
	extern void matrix_transpose(double *m,int r,int c);
}


// used by matrix::operator[]. 
template<int R> class matrix_row
{
	private:
		double *x;
	public:
		matrix_row(double *_x) : x(_x)  {}
		~matrix_row()  {}
		
		// Returns the value indexed with c in the row represented 
		// by *this. 
		// NO RANGE CHECK IS PER_mFORMED ON c. 
		// Reference is returned to allow for modification. 
		double &operator[](int c)         {  return(x[c]);  }   // returning ref
		double  operator[](int c)  const  {  return(x[c]);  }   // returning copy
};


// R: number of rows, C: number of columns. 
template<int R,int C> class matrix
{
	private:
#ifdef GCC_HACK
public: // work around for the template friend problem
#endif
		double x[R][C];
	public:
		// Constructor which generates an uninitialized matrix: 
		matrix()     { }
		// Constructor which generates the identity-matrix: 
		//   1 0 0 
		//   0 1 0
		//   0 0 1
		matrix(int)  {  _mFOR(r,c)  x[r][c] = (r==c) ? 1.0 : 0.0;  }
		// Copy-constructor: 
		matrix(const matrix<R,C> &m)  {  _mFOR(r,c)  x[r][c]=m.x[r][c];  }
		
		// Assignment operator: 
		matrix<R,C> &operator=(const matrix<R,C> &m)
			{  _mFOR(r,c) x[r][c]=m.x[r][c];  return(*this);  }
		
		// Set this to a null-matrix / to a identity-matrix. 
		void set_null()   {  _mFOR(r,c)  x[r][c]=0.0;  }
		void set_ident()  {  _mFOR(r,c)  x[r][c] = (r==c) ? 1.0 : 0.0;  }
		
		// Get matrix_row indexed r: 
		// Do not use matrix_row in your code; only use its operator[] 
		// to be able to access and modify any element of the matrix: 
		//   matrix<4,4> mat;
		//   double val = mat[r][c];
		// NO RANGE CHECK IS PER_mFORMED ON r. 
		matrix_row<C> operator[](int r)
			{  return(matrix_row<C>(x[r]));  }
		
		
		/**************************************************************/
		/* Functions which can be applied to non-initialized matrices */
		/* as they overwrite the content of *this:                    */
		
		matrix<R,C> &mul(const matrix<R,C> &a,double b)
			{            _mFOR(r,c)  x[r][c]=a.x[r][c]*b;  return(*this);  }
		matrix<R,C> &div(const matrix<R,C> &a,double b)
			{  b=1.0/b;  _mFOR(r,c)  x[r][c]=a.x[r][c]*b;  return(*this);  }
		
		matrix<R,C> &add(const matrix<R,C> &a,const matrix<R,C> &b)
			{  _mFOR(r,c)  x[r][c]=a.x[r][c]+b.x[r][c];  return(*this);  }
		matrix<R,C> &sub(const matrix<R,C> &a,const matrix<R,C> &b)
			{  _mFOR(r,c)  x[r][c]=a.x[r][c]-b.x[r][c];  return(*this);  }
		
		matrix<R,C> &neg(const matrix<R,C> &a)
			{  _mFOR(r,c)  x[r][c]=-a.x[r][c];  return(*this);  }
		
		// Inverts the matrix m and stores the inverted matrix in *this. 
		// NOTE: Only quadratic matrices may be inverted; if R!=C, 
		//       EX_Matrix_Illegal_Invert is thrown. 
		// (The identity-matrix initialisation is important.) 
		matrix<R,R> &invert(const matrix<R,R> &m)
			{  set_ident();  internal::matrix_invert_copy(x[0],R,C,m.x[0],R,C);
			   return(*this);  }
		
		// Transposes the matrix and assign it to *this: 
		matrix<R,C> &transpose(const matrix<C,R> &m)
			{  _mFOR(r,c)  x[r][c]=m.x[c][r];  return(*this);  }
		
		/**************************************************************/
		/* Functions taking *this as argument a and overwriting *this */
		/* with the result:                                           */
		
		matrix<R,C> &mul(double b)
			{            _mFOR(r,c)  x[r][c]*=b;  return(*this);  }
		matrix<R,C> &div(double b)
			{  b=1.0/b;  _mFOR(r,c)  x[r][c]*=b;  return(*this);  }
		
		matrix<R,C> &add(const matrix<R,C> &b)
			{  _mFOR(r,c)  x[r][c]+=b.x[r][c];  return(*this);  }
		matrix<R,C> &sub(const matrix<R,C> &b)
			{  _mFOR(r,c)  x[r][c]-=b.x[r][c];  return(*this);  }
		
		matrix<R,C> &neg()
			{  _mFOR(r,c)  x[r][c]*=-1.0;  return(*this);  }
		
		// NOTE: This function will throw EX_Matrix_Illegal_Mult, if R!=C. 
		// There is also a more general multiplication function available; 
		// see below. 
		matrix<R,C> &mul(const matrix<R,C> &b)
			{  internal::matrix_mul(x[0],R,C,b.x[0],R,C);  return(*this);  }
		
		// Inverts the matrix *this. 
		// NOTE: Only quadratic matrices may be inverted; if R!=C, 
		//       EX_Matrix_Illegal_Invert is thrown. 
		matrix<R,R> &invert()
			{  internal::matrix_invert_copy(x[0],R,C);  return(*this);  }
		
		// Transposes the matrix *this. 
		// Obviously, this only works on quadratic matrices. 
		matrix<R,R> &transpose()
			{  internal::matrix_transpose(x[0],R,C);  return(*this);  }
		
#ifndef GCC_HACK
		// Function to multiply the vector v with matrix m, storing the 
		// resulting vector in r. 
		template<int r,int c>friend void mult(vector<r> &r,const matrix<r,c> &m,const vector<c> &v);
		friend void mult(vector<3> &r,const matrix<4,4> &m,const vector<3> &v);
#endif
		
		// Matrix multiplication friend: 
		template<int M,int L,int N>friend void mult(matrix<M,L> &r,
			const matrix<L,N> &a,const matrix<M,N> &b);
		
		template<int r,int c>friend std::ostream& operator<<(std::ostream &s,const matrix<r,c> &m);
		
		// Comparing matrices: 
		// Returns 1, if a is equal to *this (or each component pair does not 
		// differ more than epsilon). 
		int compare_to(const matrix<R,C> &a,double epsilon) const 
			{  _mFOR(r,c)  if(fabs(x[r][c]-a.x[r][c])>epsilon) return(0);  return(1);  }
		
		// Returns 1, if this is a null-matrix (no component > epsilon). 
		int is_null(double epsilon) const 
			{  _mFOR(r,c)  if(fabs(x[r][c])>epsilon) return(0);  return(1);  }
		
		// Returns 1, if this is an identity-matrix. 
		int is_ident(double epsilon) const  {
			_mFOR(r,c)
				if(fabs((r==c) ? (x[r][c]-1.0) : (x[r][c])) > epsilon)
					return(0);
			return(1);
		}
};


// Function to multiply the two matrices a and b storing the result in r. 
template<int M,int L,int N> inline 
	void mult(matrix<M,L> &r,const matrix<L,N> &a,const matrix<M,N> &b)
	{  internal::matrix_mul(r.x[0],M,L,a.x[0],L,N,b.x[0],M,N);  }

template<int R,int C> inline 
	std::ostream& operator<<(std::ostream &s,const matrix<R,C> &m)
	{  return(internal::stream_write_array2(s,m.x[0],R,C));  }

}  /* end of namespace */

#undef _mFOR
#undef _mFORN

#endif  /* _NS_internal_vect_imatrix_HPP_  */

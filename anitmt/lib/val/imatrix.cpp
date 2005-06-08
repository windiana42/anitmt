/*
 * imatrix.cpp
 * 
 * Implementation on internally used matrix functions. 
 * 
 * Copyright (c) 2000--2002 by Wolfgang Wieser > wwieser -a- gmx -*- de < 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "internals.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Most is done inline for speed increase. 

namespace internal_vect
{
namespace internal
{

// For 2d-array subscript. 
// Array array with R rows; returns array[r][c]. 
// I decided to use a macro so that it expands to an expression 
// usable as lvalue as well. 
#define SUB(array,R,r,c)  array[r*R+c]

// Suffix 2 for 2-dim array (matrix). 
std::ostream& stream_write_array2(std::ostream& s,const double *x,int R,int C)
{
	s << "[ ";
	for(int r=0; r<R; r++)
	{
		s << "[";
		for(int c=0; c<C; c++)
		{
			s << SUB(x,R,r,c) << (c==C-1 ? "" : ",");
		}
		s << "]" << (r==R-1 ? "" : ", ");
	}
	s << " ]";

	return(s);
}


// Multiplication of matrix a (pointer am; size ar, ac) with 
// matrix b (pointer bm; size br, bc). 
// Result is stored in matrix r (pointer rm; size rr, rc). 
// NOTE: Matrix multiplication is only defined if ac==br and 
//       ar==rr and bc==rc. If this is not the case, an 
//       exception is thrown. 
static inline void 
	_matrix_mul(double *rm,int rr,int rc,
			    const double *am,int ar,int ac,
			    const double *bm,int br,int bc)
{
	if(ac!=br || ar!=rr || bc!=rc)
	{  throw internal_vect::EX_Matrix_Illegal_Mult();  }

	for(int c=0; c<rc; c++)
	{
		for(int r=0; r<rr; r++)
		{
			double s=0.0;
			for(int i=0; i<ac; i++)
			{  s += SUB(am,ar,r,i) * SUB(bm,br,i,c);  }
			SUB(rm,rr,r,c)=s;  // store
		}
	}
}

void matrix_mul(double *rm,int rr,int rc,
		        const double *am,int ar,int ac,
		        const double *bm,int br,int bc)
		throw(internal_vect::EX_Matrix_Illegal_Mult)
{
	_matrix_mul(rm,rr,rc, am,ar,ac, bm,br,bc);
}

// Version where a is multiplied with r and the result is stored in r. 
// Requires a temporary. [ r = a * r; }
void matrix_lmul(double *rm,int rr,int rc,
		  const double *am,int ar,int ac)
		throw(internal_vect::EX_Matrix_Illegal_Mult)
{
	double bm[rr*rc];
	for(int i=rr*rc-1; i>=0; i--)  // copy rm to bm 
	{  bm[i]=rm[i];  }
	_matrix_mul(rm,rr,rc, am,ar,ac, bm,rr,rc);
}

// Version where r is multiplied with b and the result is stored in r. 
// Requires a temporary. [ r = r * b; }
void matrix_rmul(double *rm,int rr,int rc,
		  const double *bm,int br,int bc)
		throw(internal_vect::EX_Matrix_Illegal_Mult)
{
	double am[rr*rc];
	for(int i=rr*rc-1; i>=0; i--)  // copy rm to am 
	{  am[i]=rm[i];  }
	_matrix_mul(rm,rr,rc, am,rr,rc, bm,br,bc);
}

// Inverse a matrix. 
// Matrix m (pointer m; size r, c) is inversed; the result is 
// stored in matrix r (pointer rm; size rr, rc).
// NOTE: The result matrix r MUST BE SET TO THE IDENTITY MATRIX 
//       BEFORE CALLING THIS FUNCTION. 
// NOTE: Matrix inversion can only be done with quadratic matrices. 
//       If r!=c or rr!=rc or c!=rc, an exception is thrown. 
// Returns number of divisions by zero. 
inline int _matrix_inverse(double *rm,int rr,int rc,
		double *m, int r, int c)
		throw(internal_vect::EX_Matrix_Illegal_Inverse)
{
	int nzd=0;  // number of zero divisions
	
	// Implementing the Gauss-Jordan algorithm. 
	if(r!=c || rr!=rc || c!=rc)
	{  throw internal_vect::EX_Matrix_Illegal_Inverse();  }

	// m is the identiy matrix. 

	int n=rr;  // = rc = r = c
	for(int k=0; k<n; k++)
	{
		double tmp=SUB(m,n,k,k);
		if(fabs(tmp)<=1e-11)  // See if tmp is null 
		{  ++nzd;  }
		for(int j=0; j<n; j++)
		{
			SUB(m, n,k,j)/=tmp;
			SUB(rm,n,k,j)/=tmp;
		}
		for(int i=0; i<n; i++)  if(i!=k)
		{
			double tmp=SUB(m,n,i,k);
			for(int j=0; j<n; j++)
			{
				SUB(m, n,i,j) -= tmp * SUB(m, n,k,j);
				SUB(rm,n,i,j) -= tmp * SUB(rm,n,k,j);
			}
		}
	}
	
	return(nzd);
}

// NOTE: m IS MODIFIED!! 
int matrix_inverse(double *rm,int rr,int rc,
		            double *m, int r, int c)
		throw(internal_vect::EX_Matrix_Illegal_Inverse)
{
	return(_matrix_inverse(rm,rr,rc,m,r,c));  // inline
}

// m is copied; not modified. 
int matrix_inverse_copy(double *rm,int rr,int rc,
		           const double *m, int r, int c)
		throw(internal_vect::EX_Matrix_Illegal_Inverse)
{
	double tmp[r*c];
	for(int i=r*c-1; i>=0; i--)  // copy m to tmp
	{  tmp[i]=m[i];  }
	
	return(_matrix_inverse(rm,rr,rc,tmp,r,c));  // inline 
}

// m is argument and return value. 
int matrix_inverse_copy(double *m,int r,int c)
		throw(internal_vect::EX_Matrix_Illegal_Inverse)
{
	double tmp[r*c];
	// copy m to tmp and set m to the identity matrix. 
	double *src=m,*dest=tmp;
	for(int ir=0; ir<r; ir++)
	{
		for(int ic=0; ic<c; ic++)
		{
			*(dest++)=*(src++);
			SUB(m,r,ir,ic) = (ir==ic) ? 1.0 : 0.0;
		}
	}
	
	return(_matrix_inverse(m,r,c,tmp,r,c));  // inline 
}


// Transpose matrix: 
void matrix_transpose(double *m,int mr,int mc)
{
	for(int r=1; r<mr; r++)
	{
		for(int c=0; c<r; c++)
		{
			register double tmp=SUB(m,mr,r,c);
			SUB(m,mr,r,c)=SUB(m,mr,c,r);
			SUB(m,mr,c,r)=tmp;
		}
	}
}


// Calculate determinant: 
double matrix_det(const double *m,int n)
{
	#warning remove this assert: 
	assert(n>2);  // n<=2 done inline 
	
	// Copy it: 
	double tmp[n*n];
	for(int i=0,nn=n*n; i<nn; i++)
	{  tmp[i]=m[i];  }
	
	for(int r=0,n1=n-1; r<n1; r++)
	{
		//stream_write_array2(std::cerr,tmp,n,n);  std::cerr << std::endl;
		double fn=SUB(tmp,n,r,r);
		if(fabs(fn)<=1e-11)  // See if fn is null before dividing. 
		{
			// Find next row (below) which is non-null at this position: 
			int fr=r+1;
			for(; fr<n; fr++)
			{  if(fabs(SUB(tmp,n,fr,r))>1e-11)  goto found;  }
			return(0.0);  // det is 0. 
			found:;
			// Add row (only the last n-r elems since the other ones are 0): 
			for(int i=r; i<n; i++)
			{  SUB(tmp,n,r,i)+=SUB(tmp,n,fr,i);  }
			fn=SUB(tmp,n,r,r);
		}
		for(int cr=r+1; cr<n; cr++)
		{
			double f=SUB(tmp,n,cr,r)/fn;
			for(int cc=r; cc<n; cc++)
			{  SUB(tmp,n,cr,cc)-=f*SUB(tmp,n,r,cc);  }
		}
	}
	//stream_write_array2(std::cerr,tmp,n,n);  std::cerr << std::endl;
	
	double det=1.0;
	for(int i=0; i<n; i++)
	{  det*=SUB(tmp,n,i,i);  }
	return(det);
}


// Multiplication of matrix m (size mr,mc) with vector v (vn 
// dimensions); result is stored in rv (rn elements). 
// NOTE: If mc!=vn or mr!=rn, EX_Matrix_Illegal_VectMult is thrown. 
// MATRIX * VECTOR: 
void matrix_mul_vect_mv(
	double *rv,int rn,
	const double *m,int mr,int mc,
	const double *v,int vn)  throw(internal_vect::EX_Matrix_Illegal_VectMult)
{
	if(mc!=vn || mr!=rn)
	{  throw internal_vect::EX_Matrix_Illegal_VectMult();  }

	for(int r=0; r<mr; r++)
	{
		double s=0.0;
		for(int i=0; i<mc; i++)
		{  s += SUB(m,mr,r,i) * v[i];  }
		rv[r]=s;
	}
}

#ifndef LIBVAL_DISABLE__VEC_MUL_MAT
// VECTOR * MATRIX: 
void matrix_mul_vect_vm(
	double *rv,int rn,
	const double *m,int mr,int mc,
	const double *v,int vn)  throw(internal_vect::EX_Matrix_Illegal_VectMult)
{
	if(mr!=vn || mc!=rn)
	{  throw internal_vect::EX_Matrix_Illegal_VectMult();  }

	for(int c=0; c<mc; c++)
	{
		double s=0.0;
		for(int i=0; i<mr; i++)
		{  s += SUB(m,mr,i,c) * v[i];  }
		rv[c]=s;
	}
}
#else
#warning "**** LIBVAL_DISABLE__VEC_MUL_MAT is defined. ****"
#endif


// Special function: multiply a 3d vector with a 4x4 matrix and 
// get a 3d vector. 
// The exception is thrown if the 4th element of the result is 
// different from 1. 
// MATRIX * VECTOR: 
void matrix_mul_vect343mv(double *rv,const double *m,const double *v)
{
	for(int r=0; r<3; r++)
	{
		rv[r] = SUB(m,4,r,0)*v[0] + SUB(m,4,r,1)*v[1] + 
				SUB(m,4,r,2)*v[2] + SUB(m,4,r,3) /* *1 */;
	}

	#warning This needs thinking...
	// tmp stores the 4th component of the result vector. 
	double tmp = SUB(m,4,3,0)*v[0] + SUB(m,4,3,1)*v[1] + 
				 SUB(m,4,3,2)*v[2] + SUB(m,4,3,3) /* *1 */;
	if(fabs(tmp-1.0)>0.00001)  // gonna throw the exception, damn!!
	{
		#if 1
		std::cerr << "*** Matrix*Vector problem??? " << (tmp-1.0) << std::endl;
		#else
		matrix<4,4> e_mat;
		vector<3> e_v3;
		vector<4> e_v4;
		for(int r=0; r<4; r++)
			for(int c=0; c<4; c++)
				e_mat(r,c,SUB(m,4,r,c));
		for(int i=0; i<3; i++)  e_v3(i,v[i]);
		for(int i=0; i<3; i++)  e_v4(i,rv[i]);  e_v4(3,tmp);
		std::cerr << "OOPS: matrix<4,4> * vector<3> multiplication problem:" << std::endl;
		std::cerr << "Vector: " << e_v3 << ";  "
		        "Result: " << e_v4 << ";  "
				"Matrix: " << e_mat << std::endl;
		abort();
		#endif
	}
}

#ifndef LIBVAL_DISABLE__VEC_MUL_MAT
// VECTOR * MATRIX: 
void matrix_mul_vect343vm(double *rv,const double *m,const double *v)
{
	for(int c=0; c<3; c++)
	{
		rv[c] = SUB(m,4,0,c)*v[0] + SUB(m,4,1,c)*v[1] + 
				SUB(m,4,2,c)*v[2] + SUB(m,4,3,c) /* *1 */;
	}
}
#endif

}  // end of namespace internal
}  // namespace end 


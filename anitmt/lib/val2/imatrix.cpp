#include "internals.hpp"

#include <stdio.h>
#include <stdlib.h>

// Most is done inline for speed increase. 

namespace internal_vect
{
namespace internal
{

// For 2d-array subscript. 
// Array array with C columns; returns array[c][r]. 
// I decided to use a macro so that it expands to an expression 
// usable as lvalue as well. 
#define SUB(array,C,c,r)  array[c*C+r]

// Suffix 2 for 2-dim array (matrix). 
ostream& stream_write_array2(ostream& s,const double *x,int C,int R)
{
	static int warned=0;
	if(!warned)
	{
		s << "[please implement vect::internal::stream_write_array2()]\n";
		++warned;
	}

	for(int r=0; r<R; r++)
	{
		s << "| ";
		for(int c=0; c<C; c++)
		{
			s << SUB(x,C,c,r) << " ";
		}
		s << "|\n";
	}

	return(s);
}


// Multiplication of matrix a (pointer am; size ac, ar) with 
// matrix b (pointer bm; size bc, br). 
// Result is stored in matrix r (pointer rm; size rc, rr). 
// NOTE: Matrix multiplication is only defined if ac==br and 
//       ar==rr and bc==rc. If this is not the case, an 
//       exception is thrown. 
static inline void 
	_matrix_mul(double *rm,int rc,int rr,
			    const double *am,int ac,int ar,
			    const double *bm,int bc,int br)
{
	if(ac!=br || ar!=rr || bc!=rc)
	{  throw internal_vect::EX_Matrix_Illegal_Mult();  }

	for(int r=0; r<rr; r++)
	{
		for(int c=0; c<rc; c++)
		{
			double s=0.0;
			for(int i=0; i<ac; i++)
			{  s += SUB(am,ac,i,r) * SUB(bm,bc,c,i);  }
			SUB(rm,rc,c,r)=s;  // store
		}
	}
}

void matrix_mul(double *rm,int rc,int rr,
		        const double *am,int ac,int ar,
		        const double *bm,int bc,int br)
		throw(internal_vect::EX_Matrix_Illegal_Mult)
{
	_matrix_mul(rm,rc,rr, am,ac,ar, bm,bc,br);
}

// Version where r is multiplied with b and the result is stored in r. 
// Requires a temporary. 
void matrix_mul(double *rm,int rc,int rr,
		        const double *bm,int bc,int br)
		throw(internal_vect::EX_Matrix_Illegal_Mult)
{
	double am[rc*rr];
	for(int i=rc*rr-1; i>=0; i--)  // copy rm to am 
	{  am[i]=rm[i];  }
	_matrix_mul(rm,rc,rr, am,rc,rr, bm,bc,br);
}

// Invert a matrix. 
// Matrix m (pointer m; size c, r) is inverted; the result is 
// stored in matrix r (pointer rm; size rc, rr).
// NOTE: The result matrix r MUST BE SET TO THE IDENTITY MATRIX 
//       BEFORE CALLING THIS FUNCTION. 
// NOTE: Matrix inversion can only be done with quadratic matrices. 
//       If c!=r or rc!=rr or c!=rc, an exception is thrown. 
inline void _matrix_invert(double *rm,int rc,int rr,
		           double *m, int c, int r)
		throw(internal_vect::EX_Matrix_Illegal_Invert)
{
	// Implementing the Gauss-Jordan algorithm. 
	if(c!=r || rc!=rr || c!=rc)
	{  throw internal_vect::EX_Matrix_Illegal_Invert();  }

	// m is the identiy matrix. 

	int n=rc;  // = rr = c = r
	for(int k=0; k<n; k++)
	{
		double tmp=SUB(m,n,k,k);
		for(int j=0; j<n; j++)
		{
			SUB(m, n,j,k)/=tmp;
			SUB(rm,n,j,k)/=tmp;
		}
		for(int i=0; i<n; i++)  if(i!=k)
		{
			double tmp=SUB(m,n,k,i);
			for(int j=0; j<n; j++)
			{
				SUB(m, n,j,i) -= tmp * SUB(m, n,j,k);
				SUB(rm,n,j,i) -= tmp * SUB(rm,n,j,k);
			}
		}
	}
}

// NOTE: m IS MODIFIED!! 
void matrix_invert(double *rm,int rc,int rr,
		           double *m, int c, int r)
		throw(internal_vect::EX_Matrix_Illegal_Invert)
{
	_matrix_invert(rm,rc,rr,m,c,r);  // inline
}

// m is copied; not modified. 
void matrix_invert_copy(double *rm,int rc,int rr,
		          const double *m, int c, int r)
		throw(internal_vect::EX_Matrix_Illegal_Invert)
{

	double tmp[c*r];
	for(int i=c*r-1; i>=0; i--)  // copy m to tmp
	{  tmp[i]=m[i];  }
	_matrix_invert(rm,rc,rr,tmp,c,r);  // inline 
}

// m is argument and return value. 
void matrix_invert_copy(double *m,int c,int r)
		throw(internal_vect::EX_Matrix_Illegal_Invert)
{

	double tmp[c*r];
	// copy m to tmp and set m to the identity matrix. 
	double *src=m,*dest=tmp;
	for(int ic=0; ic<c; ic++)
	{
		for(int ir=0; ir<r; ir++)
		{
			*(dest++)=*(src++);
			SUB(m,c,ic,ir) = (ic==ir) ? 1.0 : 0.0;
		}
	}
	_matrix_invert(m,c,r,tmp,c,r);  // inline 
}


// Multiplication of matrix m (size mc,mr) with vector v (vn 
// dimensions); result is stored in rv (rn elements). 
// NOTE: If mc!=vn or mr!=rn, EX_Matrix_Illegal_VectMult is thrown. 
void matrix_mul_vect(
	double *rv,int rn,
	const double *m,int mc,int mr,
	const double *v,int vn)  throw(internal_vect::EX_Matrix_Illegal_VectMult)
{
	if(mc!=vn || mr!=rn)
	{  throw internal_vect::EX_Matrix_Illegal_VectMult();  }

	for(int r=0; r<mr; r++)
	{
		double s=0.0;
		for(int c=0; c<mc; c++)
		{  s += SUB(m,mc,c,r) * v[c];  }
		rv[r]=s;
	}
}

// Special function: multiply a 3d vector with a 4x4 matrix and 
// get a 3d vector. 
// The exception is thrown if the 4th element of the result is 
// different from 1. 
void matrix_mul_vect343(double *rv,const double *m,const double *v)
{
	for(int r=0; r<3; r++)
	{
		rv[r] = SUB(m,4,0,r)*v[0] + SUB(m,4,1,r)*v[1] + 
				SUB(m,4,2,r)*v[2] + SUB(m,4,3,r) /* *1 */;
	}
	
	#warning This needs thinking...
	// tmp stores the 4th component of the result vector. 
	double tmp = SUB(m,4,0,3)*v[0] + SUB(m,4,1,3)*v[1] + 
				 SUB(m,4,2,3)*v[2] + SUB(m,4,3,3) /* *1 */;
	if(fabs(tmp-1.0)>0.00001)  // gonna throw the exception, damn!!
	{
		#if 1
		cerr << "*** Matrix*Vector problem??? " << (tmp-1.0) << std::endl;
		#else
		matrix<4,4> e_mat;
		vector<3> e_v3;
		vector<4> e_v4;
		for(int c=0; c<4; c++)
			for(int r=0; r<4; r++)
				e_mat(c,r,SUB(m,4,c,r));
		for(int i=0; i<3; i++)  e_v3(i,v[i]);
		for(int i=0; i<3; i++)  e_v4(i,rv[i]);  e_v4(3,tmp);
		cerr << "OOPS: matrix<4,4> * vector<3> multiplication problem:" << std::endl;
		cerr << "Vector: " << e_v3 << ";  "
		        "Result: " << e_v4 << ";  "
				"Matrix: " << e_mat << std::endl;
		abort();
		#endif
	}
}

}  // end of namespace internal
}  // namespace end 


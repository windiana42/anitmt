/*
 * vect.cpp
 * 
 * Implementation of vector functions (namespaces vect, vect::internal). 
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

#include <iostream>

#include <vect.hpp>

// Most is done inline for speed increase. 

namespace vect
{
// Special functions: 
  void mult(vector<3> &r,const matrix<4,4> &m,const vector<3> &v)
  {  internal::matrix_mul_vect343(r._get_ptr(),m.x[0],v._get_ptr());  }


	namespace internal
	{
		// Suffix 1 for 1-dim array (vector). 
		std::ostream& stream_write_array1(std::ostream& s,const double *x,int n)
		{
			s << "<";
			if(n>0)
			{
				s << *x;
				for(int i=1; i<n; i++)
				{  s << "," << x[i] ;  }
			}
			s << ">";
			return(s);
		}
		
		// For 2d-array subscript. 
		// Array array with C columns; returns array[c][r]. 
		// I decided to use a macro so that it expands to an expression 
		// usable as lvalue as well. 
		#define SUB(array,C,c,r)  array[c*C+r]
		
		// Suffix 2 for 2-dim array (matrix). 
		std::ostream& stream_write_array2(std::ostream& s,const double *x,int C,int R)
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
			{  throw vect::EX_Matrix_Illegal_Mult();  }
			
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
				throw(vect::EX_Matrix_Illegal_Mult)
		{
			_matrix_mul(rm,rc,rr, am,ac,ar, bm,bc,br);
		}
		
		// Version where r is multiplied with b and the result is stored in r. 
		// Requires a temporary. 
		void matrix_mul(double *rm,int rc,int rr,
		                const double *bm,int bc,int br)
				throw(vect::EX_Matrix_Illegal_Mult)
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
				throw(vect::EX_Matrix_Illegal_Invert)
		{
			// Implementing the Gauss-Jordan algorithm. 
			if(c!=r || rc!=rr || c!=rc)
			{  throw vect::EX_Matrix_Illegal_Invert();  }
			
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
				throw(vect::EX_Matrix_Illegal_Invert)
		{
			_matrix_invert(rm,rc,rr,m,c,r);  // inline
		}
		
		// m is copied; not modified. 
		void matrix_invert_copy(double *rm,int rc,int rr,
		                  const double *m, int c, int r)
				throw(vect::EX_Matrix_Illegal_Invert)
		{
			
			double tmp[c*r];
			for(int i=c*r-1; i>=0; i--)  // copy m to tmp
			{  tmp[i]=m[i];  }
			_matrix_invert(rm,rc,rr,tmp,c,r);  // inline 
		}
		
		// m is argument and return value. 
		void matrix_invert_copy(double *m,int c,int r)
				throw(vect::EX_Matrix_Illegal_Invert)
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
			const double *v,int vn)  throw(vect::EX_Matrix_Illegal_VectMult)
		{
			if(mc!=vn || mr!=rn)
			{  throw vect::EX_Matrix_Illegal_VectMult();  }
			
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
			
			double tmp = SUB(m,4,0,3)*v[0] + SUB(m,4,1,3)*v[1] + 
				         SUB(m,4,2,3)*v[2] + SUB(m,4,3,3) /* *1 */;
			if(fabs(tmp-1.0)>0.00001)  // gonna throw the exception, damn!!
			{
				#if 0
				EX_Matrix_34_Problem exc;
				for(int c=0; c<4; c++)
					for(int r=0; r<4; r++)
						exc.mat(c,r,SUB(m,4,c,r));
				for(int i=0; i<3; i++)  exc.v3(i,v[i]);
				for(int i=0; i<3; i++)  exc.v4(i,rv[i]);  exc.v4(3,tmp);
				throw(exc);
				#endif
				std::cerr << "*** Matrix<4,4>*Vector<3> multiplication problem?!?!"
					" (Please tell Wolfgang when you see that.)" << std::endl;
			}
		}
		
	}
}


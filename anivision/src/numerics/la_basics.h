/*
 * numerics/la_basics.h
 * 
 * Numerics library linear algebra basics. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _WW_NUMERICS_LINALG_BASICS_H_
#define _WW_NUMERICS_LINALG_BASICS_H_ 1

#include <numerics/num_math.h>

namespace NUM  // numerics
{

// Numerics vector array: 
template<typename T=double>class VectorArray
{
	public:
		enum _Transfer { Transfer };
	private:
		int dim;
		int nvectors;
		T *array;   // [nvectors*dim]
		
		// NOT C++ safe: 
		VectorArray<T> &operator=(const VectorArray<T> &)  { nc_assert(0); }
		VectorArray(const VectorArray<T> &)  { nc_assert(0); }
	public:  _CPP_OPERATORS
		VectorArray()
			{  dim=0; nvectors=0;  array=NULL;  }
		VectorArray(int _nvectors,int _dim)
			{  dim=_dim;  nvectors=_nvectors;
				array=ALLOC<T>(nvectors*dim);  }
		// Transfer points from passed array deleting them in passed array: 
		VectorArray(_Transfer,VectorArray<T> &src)
			{  dim=src.dim; nvectors=src.nvectors; array=src.array;
				src.dim=0; src.nvectors=0; src.array=NULL;  }
		~VectorArray()
			{  array=FREE(array);  nvectors=0;  dim=0;  }
		
		// Return vector dimension: (3 for 3dim vectors)
		inline int Dim() const
			{  return(dim);  }
		
		// Return number of vectors (or points): 
		inline int NVectors() const
			{  return(nvectors);  }
		
		// Return array base pointer; use with care: 
		inline T *GetArrayBase() const
			{  return(array);  }
		
		// Index operator: return vector (array of size dim) 
		// or NULL for out of range (i.e. i<0 or i>=nvectors). 
		// Returns pointer to internal data, so may be modified 
		// by dereferencing the returned pointer.  
		inline T *operator[](int i)
			{  return((i<0 || i>=nvectors) ? NULL : (array+dim*i));  }
		inline const T *operator[](int i) const
			{  return((i<0 || i>=nvectors) ? NULL : (array+dim*i));  }
		
		// Resize vector array; last elements are lost on downsize. 
		// All elements will be lost when changing the dimension. 
		// Default dim=-1 will keep the current dimension. 
		void Resize(int _nvectors,int _dim=-1)
		{
			if(_dim<0) _dim=dim;
			int nelem=_nvectors*_dim;
			if(nelem==nvectors*dim) return;
			nvectors=_nvectors;  dim=_dim;
			array=REALLOC(array,nelem);
		}
};


template<typename T=double>class SMatrix
{
	public:
		enum _NullMat { NullMat };
	private:
		// Matrix elements: 
		T *m;
		// Matrix size: 
		int r,c;
		
		T &_m(int r,int c)
			{  return(operator[](r)[c]);  }
	public:  _CPP_OPERATORS
		// Copy constructor: 
		SMatrix(const SMatrix &s)
			{  r=s.r;  c=s.c;  m=ALLOC<T>(r*c);
				for(int i=0,n=r*c; i<n; i++)  m[i]=s.m[i];  }
		// NOTE: Creates uninitialized matrix: 
		SMatrix(int _r,int _c)
			{  r=_r;   c=_c;  m=ALLOC<T>(r*c);  }
		// Create null matrix:
		SMatrix(_NullMat,int _r,int _c)
			{
				r=_r;   c=_c;  m=ALLOC<T>(r*c);
				for(int i=0,n=r*c;i<n;i++) m[i]=0.0;
			}
		// Create rotation matrix: 
		SMatrix(int n,int p,int q,T sin,T cos)
			{
				r=c=n; m=ALLOC<T>(r*c);
				for(int i=0;i<r;i++)for(int j=0;j<c;j++)
					m[i*c+j] = i==j ? 1.0 : 0;
				_m(p,p)=_m(q,q)=cos;
				_m(p,q)=sin;  _m(q,p)=-sin;
			}
		~SMatrix()
			{  m=FREE(m);  }
		
		// Assignment operator: 
		SMatrix<T> &operator=(const SMatrix<T> &s)
		{
			if(r*c!=s.r*s.c)
			{  FREE(m);  m=ALLOC<T>(s.r*s.c);  }
			r=s.r;  c=s.c;  for(int i=0,n=r*c; i<n; i++)  m[i]=s.m[i];
			return(*this);
		}
		
		// Get size: 
		int R() const  {  return(r);  }
		int C() const  {  return(c);  }
		
		// Get element: Use SMatrix[r][c]; no range check.
		T *operator[](int _r)  {  return(m+_r*c);  }
		const T *operator[](int _r) const  {  return(m+_r*c);  }
		
		// Transpose this matrix: 
		SMatrix<T> &transpose()
			{  for(int i=0;i<r;i++) for(int j=i+1;j<c;j++)
				{  T tmp=_m(i,j);  _m(i,j)=_m(j,i);  _m(j,i)=tmp;  }
				return(*this);  }
		
		// Multiply: 
		template<typename t>friend SMatrix<t> operator*(
			const SMatrix<t> &a,const SMatrix<t> &b);
};


// Matrix multiplication: 
extern int MatMult(SMatrix<double> &res,
	const SMatrix<double> &a1,const SMatrix<double> &a2);
extern int MatMult(SMatrix<float> &res,
	const SMatrix<float> &a1,const SMatrix<float> &a2);

template<typename T>inline SMatrix<T> operator*(
	const SMatrix<T> &a,const SMatrix<T> &b)
{
	SMatrix<T> tmp(a.R(),b.C());
	int rv=MatMult(tmp,a,b); nc_assert(!rv);
	return(tmp);
}

// Multiplication of matrix with diagonal matrix. 
extern int MatMultMD(SMatrix<double> &res,
	const SMatrix<double> &a,const double *diag);
extern int MatMultMD(SMatrix<float> &res,
	const SMatrix<float> &a,const float *diag);

// Multiplication of matrix with vector. 
extern int MatMultMV(double *res,const SMatrix<double> &a,const double *vec);
extern int MatMultMV(float *res,const SMatrix<float> &a,const float *vec);

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_LINALG_BASICS_H_ */

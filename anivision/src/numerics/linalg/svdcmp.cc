/*
 * numerics/linalg/svdcmp.cc
 * 
 * Singular value decomposition. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de)
 * 
 * The SVD algorithm is based on a routine by G.E. Forsythe, 
 * M.A. Malcolm, and C.B. Moler which in turn is based on 
 * the original algorithm of G.H. Golub and C. Reinsch. 
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */ 

#include "linalg.h"

namespace NUM
{

// Note that fabs() is overloaded, too. 
template<typename T>static inline T SignOfNeg(T a,T b)
	{  return(b<0.0 ? fabs(a) : -fabs(a));  }

// Singular value decomposition. 
// Decompose the m x n - matrix A into U, V, W so that 
// A = U * W * transpose(V). 
// Thereby, U is an column-orthonormal m x n - matrix and replaces 
//              the input matrix A once the function returns,
//          W is a n x n diagonal matrix stored in the array w and 
//          V is an orthogonal n x n - matrix. 
// The elements W[] are called the "singular values" of A. 
// maxiter: Limit number of iterations. 
// Return value: 
//   0 -> OK
//   1 -> bailed out (no convergence after maxiter-many iterations)
//  -2 -> illegally-sized matrices 
template<typename T>int _SVDecompose(SMatrix<T> &A,T *W,SMatrix<T> &V,
	int maxiter)
{
	// Size of input matrix for convenience: 
	int m=A.R(),n=A.C();
	
	// Sanity check: 
	if(V.R()!=n || V.C()!=n)  return(-2);
	
	T anorm, c, f, g, h, s, scale, x, y, z;
	T tmp[n];
	g = scale = anorm = 0.0;
	
	// First, we need the Householder reduction ( -> bidiag matrix). 
	for(int i=0; i<n; i++)
	{
		int l=i+1;
		tmp[i]=scale*g;
		g=0.0;
		scale=0.0;
		if(i<m)
		{
			for(int k=i; k<m; k++)
				scale+=fabs(A[k][i]);
			if(scale)
			{
				T sum=0.0;
				for(int k=i; k<m; k++)
				{
					A[k][i]/=scale;
					sum += A[k][i] * A[k][i];
				}
				f=A[i][i];
				g=SignOfNeg(sqrt(sum),f);
				h=f*g-sum;
				A[i][i]=f-g;
				for(int j=l; j<n; j++)
				{
					T sum=0.0;
					for(int k=i; k<m; k++)
						sum += A[k][i] * A[k][j];
					f=sum/h;
					for(int k=i; k<m; k++)
						A[k][j] += f * A[k][i];
				}
				for(int k=i; k<m; k++)
					A[k][i] *= scale;
			}
		}
		W[i]=scale*g;
		g=0.0;
		scale=0.0;
		if(i<m && i!=n)
		{
			for(int k=l; k<n; k++)
				scale += fabs(A[i][k]);
			if(scale)
			{
				T sum=0.0;
				for(int k=l; k<n; k++)
				{
					A[i][k] /= scale;
					sum += A[i][k] * A[i][k];
				}
				f=A[i][l];
				g=SignOfNeg(sqrt(sum), f);
				h=f*g-sum;
				A[i][l]=f-g;
				for(int k=l; k<n; k++)
					tmp[k] = A[i][k] / h;
				for(int j=l; j<m; j++)
				{
					T sum=0.0;
					for(int k=l; k<n; k++)
						sum += A[j][k] * A[i][k];
					for(int k=l; k<n; k++)
						A[j][k] += sum * tmp[k];
				}
				for(int k=l; k<n; k++)
					A[i][k] *= scale;
			}
		}
		anorm=MAX( anorm, fabs(W[i])+fabs(tmp[i]) );
	}
	
	// The next two loops will accumulate the transformations 
	// on the right and left (in this order). 
	for(int i=n-1,l=n; i>=0; i--)
	{
		if(i<n-1)
		{
			if(g)
			{
				for(int j=l; j<n; j++)
					V[j][i] = (A[i][j] / A[i][l]) / g;
				for(int j=l; j<n; j++)
				{
					T sum=0.0;
					for(int k=l; k<n; k++)
						sum += A[i][k] * V[k][j];
					for(int k=l; k<n; k++)
						V[k][j] += sum * V[k][i];
				}
			}
			for(int j=l; j<n; j++)
				V[i][j] = V[j][i] = 0.0;
		}
		V[i][i]=1.0;
		g=tmp[i];
		l=i;
	}
	
	for(int i=MIN(m,n)-1; i>=0; i--)
	{
		int l=i+1;
		g=W[i];
		for(int j=l; j<n; j++)
			A[i][j] = 0.0;
		if(g)
		{
			g=1.0/g;
			for(int j=l; j<n; j++)
			{
				T sum=0.0;
				for(int k=l; k<m; k++)
					sum += A[k][i] * A[k][j];
				f=sum/A[i][i]*g;
				for(int k=i; k<m; k++)
					A[k][j]+=f*A[k][i];
			}
			for(int j=i; j<m; j++)
				A[j][i] *= g;
		}
		else for(int j=i; j<m; j++)
			A[j][i]=0.0;
		++A[i][i];
	}
	
	for(int k=n-1; k>=0; k--)
	{
		for(int its=1,l; its<=maxiter; its++)
		{
			int nm;
			for(l=k; l>=0; l--)
			{
				nm=l-1;
				// Theoretically, tmp[1] must be 0...
				if( fabs(tmp[l]) + anorm == anorm )  goto skipit;
				if( fabs(W[nm]) + anorm == anorm )  break;
			}
			c=0.0;
			s=1.0;
			for(int i=l; i<=k; i++)
			{
				f=s*tmp[i];
				tmp[i]=c*tmp[i];
				if( fabs (f) + anorm == anorm) break;
				g=W[i];
				h=HYPOT(f,g);
				W[i]=h;
				h=1.0/h;
				c=g*h;
				s=-f*h;
				for(int j=0; j<m; j++)
				{
					y=A[j][nm];
					z=A[j][i];
					A[j][nm] = y*c + z*s;
					A[j][i] = z*c - y*s;
				}
			}
			skipit:;
			z=W[k];
			if(l==k)
			{
				if(z<0.0)
				{
					// Make sure the singular value is >=0. 
					W[k]=-z;
					for(int j=0; j<n; j++)
						V[j][k]=-V[j][k];
				}
				break;
			}
			if(its==maxiter)
				return(1);  // No convergence; bail out. 
			x=W[l];
			
			nm=k-1;
			y=W[nm];
			g=tmp[nm];
			h=tmp[k];
			f=((y-z)*(y+z) + (g-h)*(g+h)) / (2.0*h*y);
			g=HYPOT(f,(__typeof__(f))1.0);
			f=((x-z)*(x+z) + h*((y / (f - SignOfNeg(g,f))) - h)) / x;
			c=s=1.0;
			
			// Basically a QR transformation: 
			for(int j=l; j<=nm; j++)
			{
				int i=j+1;
				g=tmp[i];
				y=W[i];
				h=s*g;
				g=c*g;
				z=HYPOT(f,h);
				tmp[j]=z;
				c=f/z;
				s=h/z;
				f=x*c + g*s;
				g=g*c - x*s;
				h=y*s;
				y*=c;
				for(int ii=0; ii<n; ii++)
				{
					x=V[ii][j];
					z=V[ii][i];
					V[ii][j]=x*c + z*s;
					V[ii][i]=z*c - x*s;
				}
				z=HYPOT(f,h);
				W[j]=z;
				if(z)
				{
					z=1.0/z;
					c=f*z;
					s=h*z;
				}
				f=c*g + s*y;
				x=c*y - s*g;
				for(int ii=0; ii<m; ii++)
				{
					y=A[ii][j];
					z=A[ii][i];
					A[ii][j]=y*c + z*s;
					A[ii][i]=z*c - y*s;
				}
			}
			tmp[l]=0.0;
			tmp[k]=f;
			W[k]=x;
		}
	}
	
	return(0);

}

int SVDecompose(SMatrix<double> &a,double *w,SMatrix<double> &v,int maxiter)
	{  return(_SVDecompose(a,w,v,maxiter));  }
int SVDecompose(SMatrix<float> &a,float *w,SMatrix<float> &v,int maxiter)
	{  return(_SVDecompose(a,w,v,maxiter));  }


// Perform SVD (forward/) back substitution. 
// The SCD solution for the (under/overdetermined) equation A * x = b 
// can be obtained by first SVDecomposing A into U,W,V and then 
// using the back substitution to calculate x from b. 
// Return value: 
//   0 -> OK
//  -2 -> illegally-sized matrices 
template<typename T>int _SVDFwBackSubst(const SMatrix<T> &U,const T *W,
	const SMatrix<T> &V,const T *b,T *x)
{
	// Size of input matrix for convenience: 
	int m=U.R(),n=U.C();
	
	// Sanity check: 
	if(V.R()!=n || V.C()!=n)  return(-2);
	
	// First, compute diag(W)^-1 * transpose(U) * b. 
	T tmp[n];
	for(int i=0; i<n; i++)
	{
		double h=0.0;
		if(W[i]!=0.0)
		{
			for(int j=0; j<m; j++)
				h+=U[j][i]*b[j];
			h/=W[i];
		}
		tmp[i]=h;
		
		fprintf(stderr,"tmp[%d]=%g;   w[%d]=%g\n",i,tmp[i],i,W[i]);
	}
	
	// Matrix multiplication V * tmp: 
	MatMultMV(x,V,tmp);
	
	return(0);
}

int SVDFwBackSubst(const SMatrix<double> &u,const double *w,
	const SMatrix<double> &v,const double *b,double *x)
	{  return(_SVDFwBackSubst(u,w,v,b,x));  }
int SVDFwBackSubst(const SMatrix<float> &u,const float *w,
	const SMatrix<float> &v,const float *b,float *x)
	{  return(_SVDFwBackSubst(u,w,v,b,x));  }

}  // end of namespace NUM

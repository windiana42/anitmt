/*
 * numerics/linalg/gauss-seidel.h
 * 
 * Gauss-Seidel iterative algorithm: Iterative sover for sparse matrix 
 * equations. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _WW_NUMERICS_LINALG_GAUSS_SEIDEL_H_
#define _WW_NUMERICS_LINALG_GAUSS_SEIDEL_H_ 1

// 
// The Gauss-Seidel - Algorithm is an iterative algorithm for solving 
// linear equations. It is primarily useful for sparse, diagonally-dominant 
// systems. Note that convergence of the Gauss-Seidel algorithm is 
// not guaranteed unless the system is diagonally dominant. 
// 
// To solve the equation A * x = y with A and y known and x unknown, 
// procede as follows: 
// Each row in the system is represented by a GaussSeidelRow structure. 
// This contains the non-zero coefficients of the matrix and _pointers_ 
// to the associated unknown x-values which must be allocated externally. 
// Note that this has the advantage of allowing a system to be solved 
// in-place, i.e. without having to "extract and write back" the solved 
// x[]-values from the GaussSeidelRow array. 
// Of couse, you need one GaussSeidelRow entry for every row in the 
// linear system to be solved. 
// NOTE also that A[0] MUST BE the diagonal element, i.e. 
//   row[i].A[0] corresponds to A[i][i] in the original system and then 
//   *row[i].x[0] corresponds to x[i]. 
// 

// This represents a row of the linear system to be solved. 
// Note: A[0..nent-1] and y are not modified by the algorithm. 
template<typename T>struct GaussSeidelRow
{
	int nent;   // Number of non-zero entries in this row. 
	T *A;       // Array of [nent]-many (normally non-zero) coefficients 
	            // in this row. A[0] MUST BE the diagonal element. 
	T **x;      // Array of [nent]-many _pointers_to_ the unknowns 
	            // (allocated somewhere else). 
	T y;        // Known y-value in this row. 
};

// This is a special version in case all matrix elements in one row are -1 
// except the first one. In this case, arrange the indices so that 
// A[0]=<value> and A[1]..A[nent-1]=-1. (This is trivially possible by 
// permutating the x[] pointers.) 
// And then, store A[0] in A0 in the following struct: 
// Note: A0 and y are not modified by the algorithm. 
template<typename T>struct GaussSeidelRow_M1
{
	int nent; // Number of non-zero entries in this row. 
	T A0;     // Matrix coefficient on the diagonal corresponding to x[0], 
	          // all others in this row are assumed to be -1. 
	T **x;    // Array of [nent]-many _pointers_to_ the unknowns 
	          // (allocated somewhere else). 
	T y;      // Known y-value in this row. 
};


// Actually solve the system; see gauss-seidel.cc: 
extern double GaussSeidelIterate(int n,const GaussSeidelRow<double> *row,
	double eps,int maxiter,double omega);
extern float GaussSeidelIterate(int n,const GaussSeidelRow<float> *row,
	float eps,int maxiter,float omega);

// Special version; see gauss-seidel.cc: 
extern double GaussSeidelIterate_M1(int n,const GaussSeidelRow_M1<double> *row,
	double eps,int maxiter,double omega);
extern float GaussSeidelIterate_M1(int n,const GaussSeidelRow_M1<float> *row,
	float eps,int maxiter,float omega);

#endif  /* _WW_NUMERICS_LINALG_GAUSS_SEIDEL_H_ */

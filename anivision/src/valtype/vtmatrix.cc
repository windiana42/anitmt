/*
 * valtype/vtmatrix.cc
 * 
 * Matrix value implementation. 
 * This is part of the AniVision project. 
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

#include "valtypes.h"

#warning determinant, inverse

Vector operator*(const Matrix &a,const Vector &b)
{
	mat_assert(a.nc==b.n);
	
	Vector res(Vector::NoInit,a.nr);
	
	for(short int r=0; r<a.nr; r++)
	{
		double sum=0.0;
		for(short int c=0; c<a.nc; c++)
		{  sum+=a[r][c]*b[c];  }
		res[r]=sum;
	}
	
	return(res);
}


Matrix operator*(const Matrix &a,const Matrix &b)
{
	mat_assert(a.nc==b.nr);
	
	Matrix res(Matrix::NoInit,a.nr,b.nc);
	
	for(short int r=0; r<a.nr; r++)
	{
		for(short int c=0; c<b.nc; c++)
		{
			double sum=0.0;
			for(short int i=0; i<a.nc; i++)
			{  sum+=a.x(r,i)*b.x(i,c);  }
			res[r][c]=sum;
		}
	}
	
	return(res);
}


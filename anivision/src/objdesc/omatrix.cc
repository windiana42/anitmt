/*
 * objdesc/omatrix.h
 * 
 * Object matrix class describes location and orientation of object 
 * in space. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "omatrix.h"
#include <numerics/num_math.h>

#include <assert.h>


/*******************************************************************************
 * 
 * Translation matrix: 
 *    / 1 0 0  tx \
 *    | 0 1 0  ty |
 *    \ 0 0 1  tz /
 * 
 * Scale matrix:
 *    / sx 0  0   0 \
 *    | 0  sy 0   0 |
 *    \ 0  0  sz  0 /
 * 
 * Rotation matrix:
 *    /  cos  0  sin  0 \
 *    |   0   1   0   0 |
 *    \ -sin  0  cos  0 /
 * 
*******************************************************************************/


namespace NUM
{

// INIT STATIC VARIABLES: 
// Defaults for object front, up, right vectors (normally x,y,z): 
double ObjectMatrix::obj_front[3]={ 1.0, 0.0, 0.0 };
double ObjectMatrix::obj_up[3]=   { 0.0, 1.0, 0.0 };
double ObjectMatrix::obj_right[3]={ 0.0, 0.0, 1.0 };


ObjectMatrix operator*(const ObjectMatrix &a,const ObjectMatrix &b)
{
	ObjectMatrix r;
	for(short int i=0; i<3; i++) for(short int j=0; j<3; j++)
	{	r[i][j] = a.m[i][0]*b.m[0][j]
		        + a.m[i][1]*b.m[1][j]
    	        + a.m[i][2]*b.m[2][j];  }
	for(short int i=0; i<3; i++)
	{	r[i][3] = a.m[i][0]*b.m[0][3]
		        + a.m[i][1]*b.m[1][3]
		        + a.m[i][2]*b.m[2][3]
		        + a.m[i][3];  }
	return(r);
}


void ObjectMatrix::CalcTrafo_Rel(double *res,const double *orv) const
{
	for(short int i=0; i<3; i++)
	{	res[i] = m[i][0]*orv[0]
		       + m[i][1]*orv[1]
		       + m[i][2]*orv[2];  }
}


void ObjectMatrix::CalcTrafo(double *res,const double *vect) const
{
	for(short int i=0; i<3; i++)
	{	res[i] = m[i][0]*vect[0]
		       + m[i][1]*vect[1]
		       + m[i][2]*vect[2]
		       + m[1][3];  }
}


// Special helper function: Invert 3x3 matrix which is specified as 
// three column vectors ( a | b | c ). 
// Store result in 3x3 matrix res[r][c]. 
// Returns determinant of original matrix. 
double InvertMatrix3x3CV(double res[3][3],
	const double *a,const double *b,const double *c)
{
	double det = a[0]*b[1]*c[2] - c[0]*b[1]*a[2]
	           + a[2]*b[0]*c[1] - c[2]*b[0]*a[1]
	           + a[1]*b[2]*c[0] - c[1]*b[2]*a[0];
	double fact=1.0/det;
	
	// First line of res: 
	res[0][0] = (b[1]*c[2] - b[2]*c[1])*fact;
	res[0][1] = (b[2]*c[0] - b[0]*c[2])*fact;
	res[0][2] = (b[0]*c[1] - b[1]*c[0])*fact;
	
	// Second line of res: 
	res[1][0] = (a[2]*c[1] - a[1]*c[2])*fact;
	res[1][1] = (a[0]*c[2] - a[2]*c[0])*fact;
	res[1][2] = (a[1]*c[0] - a[0]*c[1])*fact;
	
	// Third line of res: 
	res[2][0] = (a[1]*b[2] - a[2]*b[1])*fact;
	res[2][1] = (a[2]*b[0] - a[0]*b[2])*fact;
	res[2][2] = (a[0]*b[1] - a[1]*b[0])*fact;
	
	return(det);
}


int ObjectMatrix::SetObjPos(
	const double *pos,
	const double *front,const double *up,const double *right,
	const double *obj_front,const double *obj_up,const double *obj_right)
{
	double tmpR[3];
	if(!right)
	{
		CrossProduct(tmpR,front,up);
		right=tmpR;
	}
	
	// See ObjectMatrix(_ObjPosMat opm,...) for some comments about what 
	// I am doing here. 
	
	double O_1[3][3];
	double det=InvertMatrix3x3CV(O_1,obj_front,obj_up,obj_right);
	
	for(short int r=0; r<3; r++)
	{
		for(short int c=0; c<3; c++)
		{	m[r][c] = front[r] * O_1[0][c]
			        +    up[r] * O_1[1][c]
			        + right[r] * O_1[2][c];  }
		// Finally, insert the translation component: 
		m[r][3]=pos[r];
	}
	
	return(det==0 ? 1 : 0);
}


ObjectMatrix::ObjectMatrix(_ObjPosMat opm,const double *pos,const double *front,
	const double *up)
{
	double tmpA[3],tmpB[3];
	if(opm==ObjPosMatNrm)
	{
		NormalizeVector(tmpA,front,3);  front=tmpA;
		NormalizeVector(tmpB,up,3);     up=tmpB;
	}
	
	// Calculate right vector as front \cross up: 
	// It would also be possible to allow the user to supply 
	// an arbitrary value. 
	double right[3];
	CrossProduct(right,front,up);
	
	// The object's coordinate system is assumed to be: 
	//   up=y, front=x, right=z
	// You may specify any coordinate system here. 
	const double *obj_front=ObjectMatrix::obj_front;
	const double *obj_up=   ObjectMatrix::obj_up;
	const double *obj_right=ObjectMatrix::obj_right;
	
	// Now, the rotation part (R, 3x3) of the matrix needs to fulfil 
	// the following equations: 
	//   R * obj_front = front
	//   R * obj_up = up
	//   R * obj_right = right
	// We can put that into a matrix equation: 
	//   R * O = P
	//   with O = ( obj_front | obj_up | obj_right ) 
	//   and  P = ( front | up | right )
	// (with (a|b|c) meaning matrix with vectors a,b,c a 1.,2.,3. column) 
	// Hence, the desired rotation part R can be calculated as follows: 
	//   R = P * O^-1. 
	double O_1[3][3];
	InvertMatrix3x3CV(O_1,obj_front,obj_up,obj_right);
	
	// Calculate multiplication P * O_1 storing the result in m[][]. 
	// Also insert the translation in the translation column of 
	// the 3x4 m[][] matrix. 
	for(short int r=0; r<3; r++)
	{
		for(short int c=0; c<3; c++)
		{	m[r][c] = front[r] * O_1[0][c]
			        +    up[r] * O_1[1][c]
			        + right[r] * O_1[2][c];  }
		// Finally, insert the translation component: 
		m[r][3]=pos[r];
	}
}

}  // end of namespace NUM

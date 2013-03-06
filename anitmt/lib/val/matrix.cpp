/*
 * matrix.cpp
 * 
 * Implementation of non-inline matrix functions. 
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser > wwieser -a- gmx -*- de < 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "matrix.hpp"
#include <assert.h>

namespace vect
{

#ifndef GCC_HACK
template< >
Matrix<4,4>::Matrix(enum MatRotX,double angle) : x(0)
{
	double sina=sin(angle);
	double cosa=cos(angle);
	x[1][1]=cosa;  x[1][2]=-sina;
	x[2][1]=sina;  x[2][2]= cosa;
}

template< >
Matrix<4,4>::Matrix(enum MatRotY,double angle) : x(0)
{
	double sina=sin(angle);
	double cosa=cos(angle);
	x[0][0]= cosa;  x[0][2]= sina;
	x[2][0]=-sina;  x[2][2]= cosa;
}

template< >
Matrix<4,4>::Matrix(enum MatRotZ,double angle) : x(0)
{
	double sina=sin(angle);
	double cosa=cos(angle);
	x[0][0]= cosa;  x[0][1]=-sina;
	x[1][0]= sina;  x[1][1]= cosa;
}

template< >
Matrix<4,4>::Matrix(enum MatScale,double fact,int idx) : x(0)
{
	x[idx][idx]=fact;
}

template< >
Matrix<4,4>::Matrix(enum MatScale,double fact) : x(0)
{
	x[0][0]=fact;
	x[1][1]=fact;
	x[2][2]=fact;
}

template< >
Matrix<4,4>::Matrix(enum MatScale,const Vector<3> &v) : x(0)
{
	x[0][0]=v[0];
	x[1][1]=v[1];
	x[2][2]=v[2];
}

template< >
Matrix<4,4>::Matrix(enum MatTrans,double delta,int idx) : x(0)
{
	x[idx][3]=delta;
}

template< >
Matrix<4,4>::Matrix(enum MatTrans,const Vector<3> &v) : x(0)
{
	x[0][3]=v[0];
	x[1][3]=v[1];
	x[2][3]=v[2];
}

template< >
Matrix<4,4>::Matrix(enum MatColVec,const Vector<3> &c1,
	const Vector<3> &c2,const Vector<3> &c3) : x()
{
	for(int i=0; i<3; i++)
	{
		x[i][0]=c1[i];  x[i][1]=c2[i];  x[i][2]=c3[i];  x[i][3]=0.0;
		x[3][i]=0.0;
	}
	x[3][3]=1.0;
}

template< >
Matrix<4,4>::Matrix(enum MatRowVec,const Vector<3> &r1,
	const Vector<3> &r2,const Vector<3> &r3) : x()
{
	for(int i=0; i<3; i++)
	{
		x[0][i]=r1[i];  x[1][i]=r2[i];  x[2][i]=r3[i];  x[3][i]=0.0;
		x[i][3]=0.0;
	}
	x[3][3]=1.0;
}

template< >
Matrix<4,4>::Matrix(MatColVec,const Vector<4> &c0,
	const Vector<4> &c1,const Vector<4> &c2,const Vector<4> &c3) : x()
{
	for(int i=0; i<4; i++)
	{  x[i][0]=c0[i];  x[i][1]=c1[i];  x[i][2]=c2[i];  x[i][3]=c3[i];  }
}

template< >
Matrix<4,4>::Matrix(MatRowVec,const Vector<4> &r0,
	const Vector<4> &r1,const Vector<4> &r2,const Vector<4> &r3) : x()
{
	for(int i=0; i<4; i++)
	{  x[0][i]=r0[i];  x[1][i]=r1[i];  x[2][i]=r2[i];  x[3][i]=r3[i];  }
}
#endif

// rotates a specified angle around v 
Matrix<4,4> mat_rotate_around(const Vector<3> &v,double angle)
{
	Matrix<4,4> rv;
	Matrix<4,4> rot = mat_rotate_vect_vect(v,Vector<3>(0.0,0.0,1.0));

	// rotate v to z
	rv=rot;

	// z-rotation (around v)
	rv.lmul(mat_rotate_z(angle));

	// rotate z back to v
	rv.lmul(rot.inverse());

	return(rv);
}

// rotates a vector to another
Matrix<4,4> mat_rotate_vect_vect(const Vector<3> &from,const Vector<3> &to)
{
	Matrix<4,4> rv;

	Vector<3> vf=from,vt=to,tmp;
	double angle;

	// ******** rotation from v1 to <1,0,0> (save to matrix)
	// z-rotation 
	angle = -atan2(vf[1],vf[0]);
	vf.rotate_z(angle);
	rv=mat_rotate_z(angle);
	tmp = rv * from;

	// y-rotation 
	angle = atan2(vf[2],vf[0]);
	/**/vf.rotate_y(angle);
	rv=mat_rotate_y(angle) * rv;
	tmp = rv * from;

	// ******** rotation from v2 to <1,0,0>
	// z-rotation 
	double z_angle = -atan2(vt[1],vt[0]);
	vt.rotate_z(z_angle);

	// y-rotation 
	double y_angle = atan2(vt[2],vt[0]);
	/**/vt.rotate_y(y_angle);

	// ******** save rotation from <1,0,0> to vt
	rv.lmul(mat_rotate_y(-y_angle));  // y-rotation
	rv.lmul(mat_rotate_z(-z_angle));  // z-rotation 

	return(rv);
}

// rotates a vector to another by using a sperical rotation with the
// horizontal plane defined by the normal vector "up"
Matrix<4,4> mat_rotate_vect_vect_up(const Vector<3> &from,const Vector<3> &to,
	const Vector<3> &up)
{
	Matrix<4,4> rv;

	// ******** rotation of from-vector to <1,0,0> 

	// rotate up to z and from to the x-z plain
	Matrix<4,4> up_from_2_z_x = mat_rotate_pair_pair(up,from,
		Vector<3>(0.0,0.0,1.0),Vector<3>(1.0,0.0,0.0) );

	// rotate to map the up-rotation to a z-rotation
	rv=up_from_2_z_x;

	Vector<3> v1 = up_from_2_z_x * from;
	// y-rotation 
	double angle = atan2(v1[2],v1[0]);
	// rotation is made by other y-rotation

	Vector<3> v2 = up_from_2_z_x * to;
	// ******** rotation from v2 to <1,0,0> by a 
	//          rotation around z (up) and y 

	// z-rotation (up is z now)
	double z_angle = -atan2(v2[1],v2[0]);
	v2.rotate_z(z_angle);

	// y-rotation 
	double y_angle = atan2(v2[2],v2[0]);
	v2.rotate_y(y_angle);

	// additional rotation around y-axis
	angle -= y_angle;
	rv.lmul(mat_rotate_y(angle));

	// rotate around z-axis
	angle = -z_angle;
	rv.lmul(mat_rotate_z(angle));

	// rotate back
	rv=mat_rotate_pair_pair(Vector<3>(0.0,0.0,1.0),Vector<3>(1.0,0.0,0.0),
				up,from)*rv;

	return(rv);
}

// rotates a vector pair to another
// the first vectors of each pair will mach exactly afterwards but the second
// may differ in the angle to the first one. They will be in the same plane
// then. 
Matrix<4,4> mat_rotate_pair_pair(
	const Vector<3> &vect1f,const Vector<3> &vect1u,
	const Vector<3> &vect2f,const Vector<3> &vect2u)
{
	Matrix<4,4> rv;
	Vector<3> x(vect1f),y(vect1u);
	double angle;

	// ****** get rotation from <1,0,0> and <0,1,0> to vector pair 1

	// get the vectors
	//x=vect1f; (done above)
	//y=vect1u;
	// rotate x to <1,0,0> and y to <0,1,0> now

	// get z-rotation
	double z_angle1 = -atan2(x[1],x[0]);
	x.rotate_z(z_angle1);
	y.rotate_z(z_angle1);

	// get y-rotation
	double y_angle1 = atan2(x[2],x[0]);
	//x.rotate_y(y_angle1);
	y.rotate_y(y_angle1);

	// get x-rotation
	double x_angle1 = -atan2(y[2],y[1]);
	//x.rotate_x(x_angle1);
	//y.rotate_x(x_angle1);

	//******
	// get rotation from <1,0,0> and <0,1,0> to vector pair 2

	// get the vectors
	x=vect2f; 
	y=vect2u;
	// rotate x to <1,0,0> and y to <0,1,0> now

	// get z-rotation
	double z_angle2 = -atan2(x[1],x[0]);
	x.rotate_z(z_angle2);
	y.rotate_z(z_angle2);

	// get y-rotation
	double y_angle2 = atan2(x[2],x[0]);
	//x.rotate_y(y_angle2);
	y.rotate_y(y_angle2);

	// get x-rotation
	double x_angle2 = -atan2(y[2],y[1]);
	//x.rotate_x(x_angle2);
	//y.rotate_x(x_angle2);

	// ****** save the rotation from pair 1 to origin 

	// rotate around z-axis
	angle=z_angle1;
	rv=mat_rotate_z(angle);

	// rotate around y-axis
	angle=y_angle1;
	rv.lmul(mat_rotate_y(angle));

	// rotate around x-axis
	angle=x_angle1;
	rv.lmul(mat_rotate_x(angle));

	// ****** save the rotation from origin to pair2 

	// rotate around x-axis
	angle=-x_angle2;
	rv.lmul(mat_rotate_x(angle));

	// rotate around y-axis
	angle=-y_angle2;
	rv.lmul(mat_rotate_y(angle));

	// rotate around z-axis
	angle=-z_angle2;
	rv.lmul(mat_rotate_z(angle));

	return(rv);
}

// spherical rotation with the horizontal plane defined through
// the normal vector up and the front vector 
// the x-coordinate of angles is used for the rotation around front
// the y-coordinate of angles is used for the rotation around up
// and the z-coordiante specifies the angle to go up from the plane
Matrix<4,4> mat_rotate_spherical_pair(
	const Vector<3> &front,const Vector<3> &up,const Vector<3> &angles)
{
	// first rotate around front 
	Matrix<4,4> rv(mat_rotate_around(front,angles[0]));

	Vector<3> u(rv*up);       // get the new up vector

	// then around up
	Matrix<4,4> rv2(mat_rotate_around(up,angles[1]));
	rv.lmul(rv2);

	Vector<3> f(rv2*front);   // get the new front vector

	// and last around u x f
	rv.lmul(mat_rotate_around(f.cross(u),angles[2]));

	return(rv);
}

//! returns the scale factors of each axis as vector, caused by Matrix
Vector<3> get_scale_component(const Matrix<4,4> &mat)
{
	#warning make faster...
	
	Vector<3> x(1,0,0);
	Vector<3> y(0,1,0);
	Vector<3> z(0,0,1);
	Vector<3> trans = get_translate_component( mat );

	x = mat * x - trans;
	y = mat * y - trans;
	z = mat * z - trans;

	return(Vector<3>(x.abs(),y.abs(),z.abs()));
}

//! returns the translation caused by transformation Matrix
Vector<3> get_translate_component(const Matrix<4,4> &mat)
{
	return(mat * Vector<3>(0,0,0));
}

/*! returns the rotations about X,Y and Z axes in radians as vector, 
that is caused by Matrix */
Vector<3> get_rotate_component(const Matrix<4,4> &mat)
{
	Vector<3> x(1,0,0);
	Vector<3> y(0,1,0);
	Vector<3> trans = get_translate_component( mat );

	// get rotated vectors
	Vector<3> x_rot = mat * x - trans; 
	Vector<3> y_rot = mat * y - trans;

	// calc rotation by rerotating them to x and y

	// get z-rotation by rotating x_rot in x-z plain
	double z_angle = atan2( x_rot[1], x_rot[0] ); 
	x_rot.rotate_z(-z_angle);
	y_rot.rotate_z(-z_angle);

	// get y-rotation by rotating x_rot in x-direction
	double y_angle = -atan2( x_rot[2], x_rot[0] ); // get angle 
	/**/x_rot.rotate_y(-y_angle);
	y_rot.rotate_y(-y_angle);

	// get x-rotation by rotating y_rot in y-direction
	double x_angle = atan2( y_rot[2], y_rot[1] );
	/**/x_rot.rotate_x(-x_angle);
	/**/y_rot.rotate_x(-x_angle);

	// (vector::operator==() uses epsilon)
	/**/assert( x_rot == x * (x_rot*x) );assert( y_rot == y * (y_rot*y) );

	return(Vector<3>(x_angle,y_angle,z_angle));
}  

}  // end of namespace vect 

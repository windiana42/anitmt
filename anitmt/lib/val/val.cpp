/*
 * val.cpp
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
 *   Dec 2000      started writing (Wolfgang Wieser)
 *   Mar 3, 2001   added get_xxx_component() (Matrin Trautmann)
 * 
 */

#include <iostream>

#include "val.hpp"

namespace values
{
  const double epsilon=0.000000001;  // Max. difference for comparisons. 
  
  //*******
  // Scalar
  //*******
  // (everything inline)
  
  //*******
  // Vector
  //*******
	// Vector rotation functions: 
	Vector rotateX(const Vector &v,double theta)
	{
		double sinval=sin(theta),cosval=cos(theta);
		Vector r(v);
		// change Y and Z coordinate: 
		r(1,v[1]*cosval-v[2]*sinval);
		r(2,v[1]*sinval+v[2]*cosval);
		return(r);
	}
	
	Vector rotateY(const Vector &v,double theta)
	{
		double sinval=sin(theta),cosval=cos(theta);
		Vector r(v);
		// change X and Z coordinate: 
		r(0,v[0]*cosval+v[2]*sinval);
		r(2,v[2]*cosval-v[0]*sinval);
		return(r);
	}
	
	Vector rotateZ(const Vector &v,double theta)
	{
		double sinval=sin(theta),cosval=cos(theta);
		Vector r(v);
		// change X and Y coordinate: 
		r(0,v[0]*cosval-v[1]*sinval);
		r(1,v[1]*cosval+v[0]*sinval);
		return(r);
	}
	
	// Rotation member functions: faster than the functions above, 
	// but they change *this. 
	Vector &Vector::rotateX(double theta)
	{
		double sinval=sin(theta),cosval=cos(theta);
		double tmpy=x[1],tmpz=x[2];
		x(1,tmpy*cosval-tmpz*sinval);
		x(2,tmpy*sinval+tmpz*cosval);
		return(*this);
	}
	
	Vector &Vector::rotateY(double theta)
	{
		double sinval=sin(theta),cosval=cos(theta);
		double tmpx=x[0],tmpz=x[2];
		x(0,tmpx*cosval+tmpz*sinval);
		x(2,tmpz*cosval-tmpx*sinval);
		return(*this);
	}
	
	Vector &Vector::rotateZ(double theta)
	{
		double sinval=sin(theta),cosval=cos(theta);
		double tmpx=x[0],tmpy=x[1];
		x(0,tmpx*cosval-tmpy*sinval);
		x(1,tmpy*cosval+tmpx*sinval);
		return(*this);
	}
	
	// Coordinate system conversion functions: 
	Vector to_spherical(const Vector &v)
	{
		// x <- r     = v.abs();
		// y <- phi   = atan(y/x);
		// z <- theta = acos(z/r);
		Vector r(v);   // initialized, so there are no problems with 4d-vectors. 
		r(0,v.abs());
		r(1,atan2(v[1],v[0]));   // correct??
		r(2,acos(v[2]/r[0]));
		return(r);
	}
	
	Vector to_rectangular(const Vector &v)
	{
		// x = r * sin(theta) * cos(phi)
		// y = r * sin(theta) * sin(phi)
		// z = r * cos(theta)
		Vector r(v);   // initialized, so there are no problems with 4d-vectors. 
		double tmp = v[0]*sin(v[2]);  // r*sin(theta)
		r(0,tmp*cos(v[1]));
		r(1,tmp*sin(v[1]));
		r(2,v[0]*cos(v[2]));
		return(r);
	}
	
	Vector &Vector::to_spherical()
	{
		double tmpx=abs();
		double tmpy=atan2(x[1],x[0]);   // correct??
		x(2,acos(x[2]/tmpx));
		x(1,tmpy);
		x(0,tmpx);
		return(*this);
	}
	
	Vector &Vector::to_rectangular()
	{
		double tmpy=x[0]*sin(x[2]);  // r*sin(theta)
		double tmpx=tmpy*cos(x[1]);
		tmpy*=sin(x[1]);
		x(2,x[0]*cos(x[2]));
		x(0,tmpx);
		x(1,tmpy);
		return(*this);
	}
	
  //*******
  // Matrix
  //*******
	
	Matrix::Matrix(enum MatRotX,double angle) : 
		Valtype(Valtype::matrix),x(0)
	{
		double sina=sin(angle);
		double cosa=cos(angle);
		x(1,1, cosa);  x(2,1,-sina);
		x(1,2, sina);  x(2,2, cosa);
	}
	
	Matrix::Matrix(enum MatRotY,double angle) : 
		Valtype(Valtype::matrix),x(0)
	{
		double sina=sin(angle);
		double cosa=cos(angle);
		x(0,0, cosa);  x(2,0, sina);
		x(0,2,-sina);  x(2,2, cosa);
	}
	
	Matrix::Matrix(enum MatRotZ,double angle) : 
		Valtype(Valtype::matrix),x(0)
	{
		double sina=sin(angle);
		double cosa=cos(angle);
		x(0,0, cosa);  x(1,0,-sina);
		x(0,1, sina);  x(1,1, cosa);
	}
	
	Matrix::Matrix(enum MatScale,double fact,int idx) : 
		Valtype(Valtype::matrix),x(0)
	{
		x(idx,idx,fact);
	}
	
	Matrix::Matrix(enum MatScale,const Vector &v) : 
		Valtype(Valtype::matrix),x(0)
	{
		x(0,0,v[0]);
		x(1,1,v[1]);
		x(2,2,v[2]);
	}
	
	Matrix::Matrix(enum MatTrans,double delta,int idx) : 
		Valtype(Valtype::matrix),x(0)
	{
		x(3,idx,delta);
	}
	
	Matrix::Matrix(enum MatTrans,const Vector &v) : 
		Valtype(Valtype::matrix),x(0)
	{
		x(3,0,v[0]);
		x(3,1,v[1]);
		x(3,2,v[2]);
	}
	
	// rotates a specified angle around v 
	Matrix Mrotate_around(const Vector &v,double angle)
	{
		Matrix rv;
		
		// rotate v to z
		rv*=Mrotate_vect_vect(v,Vector(0.0,0.0,1.0));
		
		// z-rotation (around v)
		rv*=MrotateZ(angle);
		
		// rotate z back to v
		rv*=Mrotate_vect_vect(Vector(0.0,0.0,1.0),v);
		
		return(rv);
	}
	
	// rotates a vector to another
	Matrix Mrotate_vect_vect(const Vector &from,const Vector &to)
	{
		Matrix rv;
		
		Vector vf=from,vt=to;
		double angle;
		
		// ******** rotation from v1 to <1,0,0> (save to matrix)
		// z-rotation 
		angle = -atan2(vf[1],vf[0]);
		vf.rotateZ(angle);
		rv*=MrotateZ(angle);
		
		// y-rotation 
		angle = atan2(vf[2],vf[0]);
		//vf.rotateY(angle);
		rv*=MrotateY(angle);
		
		// ******** rotation from v2 to <1,0,0>
		// z-rotation 
		double z_angle = -atan2(vt[1],vt[0]);
		vt.rotateZ(z_angle);
		
		// y-rotation 
		double y_angle = atan2(vt[2],vt[0]);
		//vt.rotateY(y_angle);
		
		// ******** save rotation from <1,0,0> to vt
		rv*=MrotateY(-y_angle);  // y-rotation
		rv*=MrotateZ(-z_angle);  // z-rotation 
		
		return(rv);
	}
	
	// rotates a vector to another by using a sperical rotation with the
	// horizontal plane defined by the normal vector "up"
	Matrix Mrotate_vect_vect_up(const Vector &from,const Vector &to,
		const Vector &up)
	{
		Matrix rv;
		
		// ******** rotation of from-vector to <1,0,0> 
		
		// rotate up to z and from to the x-z plain
		Matrix up_from_2_z_x = Mrotate_pair_pair(up,from,
			Vector(0.0,0.0,1.0),Vector(1.0,0.0,0.0) );
		
		// rotate to map the up-rotation to a z-rotation
		rv*=up_from_2_z_x;
		
		Vector v1 = up_from_2_z_x * from;
		// y-rotation 
		double angle = atan2(v1[2],v1[0]);
		// rotation is made by other y-rotation
		
		Vector v2 = up_from_2_z_x * to;
		// ******** rotation from v2 to <1,0,0> by a 
		//          rotation around z (up) and y 
		
		// z-rotation (up is z now)
		double z_angle = -atan2(v2[1],v2[0]);
		v2.rotateZ(z_angle);
		
		// y-rotation 
		double y_angle = atan2(v2[2],v2[0]);
		v2.rotateY(y_angle);
		
		// additional rotation around y-axis
		angle -= y_angle;
		rv*=MrotateY(angle);
		
		// rotate around z-axis
		angle = -z_angle;
		rv*=MrotateZ(angle);
		
		// rotate back
		rv*=Mrotate_pair_pair(Vector(0.0,0.0,1.0),Vector(1.0,0.0,0.0),
			up,from);
		
		return(rv);
	}
	
	// rotates a vector pair to another
	// the first vectors of each pair will mach exactly afterwards but the second
	// may differ in the angle to the first one. They will be in the same plane
	// then. 
	Matrix Mrotate_pair_pair(
		const Vector &vect1f,const Vector &vect1u,
		const Vector &vect2f,const Vector &vect2u)
	{
		Matrix rv;
		Vector x(vect1f),y(vect1u);
		double angle;
		
		// ****** get rotation from <1,0,0> and <0,1,0> to vector pair 1
		
		// get the vectors
		//x=vect1f; (done above)
		//y=vect1u;
		// rotate x to <1,0,0> and y to <0,1,0> now
		
		// get z-rotation
		double z_angle1 = -atan2(x[1],x[0]);
		x.rotateZ(z_angle1);
		y.rotateZ(z_angle1);
		
		// get y-rotation
		double y_angle1 = atan2(x[2],x[0]);
		//x.rotateY(y_angle1);
		y.rotateY(y_angle1);
		
		// get x-rotation
		double x_angle1 = -atan2(y[2],y[1]);
		//x.rotateX(x_angle1);
		//y.rotateX(x_angle1);
		
		//******
		// get rotation from <1,0,0> and <0,1,0> to vector pair 2
		
		// get the vectors
		x=vect2f; 
		y=vect2u;
		// rotate x to <1,0,0> and y to <0,1,0> now
		
		// get z-rotation
		double z_angle2 = -atan2(x[1],x[0]);
		x.rotateZ(z_angle2);
		y.rotateZ(z_angle2);
		
		// get y-rotation
		double y_angle2 = atan2(x[2],x[0]);
		//x.rotateY(y_angle2);
		y.rotateY(y_angle2);
		
		// get x-rotation
		double x_angle2 = -atan2(y[2],y[1]);
		//x.rotateX(x_angle2);
		//y.rotateX(x_angle2);
		
		// ****** save the rotation from pair 1 to origin 
		
		// rotate around z-axis
		angle=z_angle1;
		rv*=MrotateZ(angle);
		
		// rotate around y-axis
		angle=y_angle1;
		rv*=MrotateY(angle);
		
		// rotate around x-axis
		angle=x_angle1;
		rv*=MrotateX(angle);
		
		// ****** save the rotation from origin to pair2 
		
		// rotate around x-axis
		angle=-x_angle2;
		rv*=MrotateX(angle);
		
		// rotate around y-axis
		angle=-y_angle2;
		rv*=MrotateY(angle);
		
		// rotate around z-axis
		angle=-z_angle2;
		rv*=MrotateZ(angle);
		
		return(rv);
	}
	
	// spherical rotation with the horizontal plane defined through
	// the normal vector up and the front vector 
	// the x-coordinate of angles is used for the rotation around front
	// the y-coordinate of angles is used for the rotation around up
	// and the z-coordiante specifies the angle to go up from the plane
	Matrix Mrotate_spherical_pair(
		const Vector &front,const Vector &up,const Vector &angles)
	{
		// first rotate around front 
		Matrix rv(Mrotate_around(front,angles[0]));
		
		Vector u(rv*up);       // get the new up vector
		
		// then around up
		Matrix rv2(Mrotate_around(up,angles[1]));
		rv*=rv2;

		Vector f(rv2*front);   // get the new front vector
		
		// and last around u x f
		rv*=Mrotate_around(f.cross(u),angles[2]);
		
		return(rv);
	}
	
	// get the rotation from v1 to v2 around axis 
	double get_rotation_around(
		const Vector &v1,const Vector &v2,const Vector &axis)
	{
		// rotate both vectors so that axis maches z and v1 is 
		// in the x-z-plain
		Matrix rot_easy=Mrotate_pair_pair(axis,v1,
			Vector(0.0,0.0,1.0),Vector(1.0,0.0,0.0));
		Vector easy_v2=rot_easy*v2;
		
		// get rotation from easy_v2 to x-z-plain around z
		double z_angle = atan2(easy_v2[1],easy_v2[0]);
		
		return(z_angle);
	}
	
	//! returns the scale factors of each axis as vector, caused by Matrix
	Vector get_scale_component( const Matrix &mat )
	{
		Vector x(1,0,0);
		Vector y(0,1,0);
		Vector z(0,0,1);
		
		x *= mat;
		y *= mat;
		z *= mat;
		
		return Vector( abs(x), abs(y), abs(z) );
	}

	//! returns the translation caused by transformation Matrix
	Vector get_translation_component( const Matrix &mat )
	{
		return mat * Vector(0,0,0);
	}

	/*! returns the rotations about X,Y and Z axes in radians as vector, 
	that is caused by Matrix */
	Vector get_rotation_component( const Matrix &mat )
	{
		Vector x(1,0,0);
		Vector y(0,1,0);
		Vector trans = get_translation_component( mat );
		
		// get rotated vectors
		Vector x_rot = mat * x - trans; 
		Vector y_rot = mat * y - trans;
		
		// calc rotation by rerotating them to x and y
		
		// get z-rotation by rotating x_rot in x-z plain
		double z_angle = atan2( x_rot[1], x_rot[0] ); 
		x_rot.rotateZ(-z_angle);
		y_rot.rotateZ(-z_angle);
		
		// get y-rotation by rotating x_rot in x-direction
		double y_angle = -atan2( x_rot[2], x_rot[0] ); // get angle 
		/**/x_rot.rotateY(-y_angle);
		y_rot.rotateY(-y_angle);
		
		// get x-rotation by rotating y_rot in y-direction
		double x_angle = atan2( y_rot[2], y_rot[1] );
		/**/x_rot.rotateX(-x_angle);
		/**/y_rot.rotateX(-x_angle);
		
		// (vector::operator==() uses epsilon)
		/**/assert( x_rot == x );assert( y_rot == y );
		
		return Vector( x_angle, y_angle, z_angle );
	}  

}

/*
 * vector.cpp
 * 
 * Vector template implementation. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
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

namespace vect
{

// Vector rotation functions: 
Vector<3> rotateX(const Vector<3> &v,double theta)
{
	double sinval=sin(theta),cosval=cos(theta);
	Vector<3> r(v);
	// change Y and Z coordinate: 
	r(1,v[1]*cosval-v[2]*sinval);
	r(2,v[1]*sinval+v[2]*cosval);
	return(r);
}

Vector<3> rotateY(const Vector<3> &v,double theta)
{
	double sinval=sin(theta),cosval=cos(theta);
	Vector<3> r(v);
	// change X and Z coordinate: 
	r(0,v[0]*cosval+v[2]*sinval);
	r(2,v[2]*cosval-v[0]*sinval);
	return(r);
}

Vector<3> rotateZ(const Vector<3> &v,double theta)
{
	double sinval=sin(theta),cosval=cos(theta);
	Vector<3> r(v);
	// change X and Y coordinate: 
	r(0,v[0]*cosval-v[1]*sinval);
	r(1,v[1]*cosval+v[0]*sinval);
	return(r);
}

// Rotation member functions: faster than the functions above, 
// but they change *this. 
Vector<3> &Vector<3>::rotateX(double theta)
{
	double sinval=sin(theta),cosval=cos(theta);
	double tmpy=x[1],tmpz=x[2];
	x(1,tmpy*cosval-tmpz*sinval);
	x(2,tmpy*sinval+tmpz*cosval);
	return(*this);
}

Vector<3> &Vector<3>::rotateY(double theta)
{
	double sinval=sin(theta),cosval=cos(theta);
	double tmpx=x[0],tmpz=x[2];
	x(0,tmpx*cosval+tmpz*sinval);
	x(2,tmpz*cosval-tmpx*sinval);
	return(*this);
}

Vector<3> &Vector<3>::rotateZ(double theta)
{
	double sinval=sin(theta),cosval=cos(theta);
	double tmpx=x[0],tmpy=x[1];
	x(0,tmpx*cosval-tmpy*sinval);
	x(1,tmpy*cosval+tmpx*sinval);
	return(*this);
}

// Coordinate system conversion functions: 
Vector<3> to_spherical(const Vector<3> &v)
{
	// x <- r     = v.abs();
	// y <- phi   = atan(y/x);
	// z <- theta = acos(z/r);
	Vector<3> r(v);   // initialized, so there are no problems with 4d-Vector<3>s. 
	r(0,v.abs());
	r(1,atan2(v[1],v[0]));   // correct??
	r(2,acos(v[2]/r[0]));
	return(r);
}

Vector<3> to_rectangular(const Vector<3> &v)
{
	// x = r * sin(theta) * cos(phi)
	// y = r * sin(theta) * sin(phi)
	// z = r * cos(theta)
	Vector<3> r(v);   // initialized, so there are no problems with 4d-Vector<3>s. 
	double tmp = v[0]*sin(v[2]);  // r*sin(theta)
	r(0,tmp*cos(v[1]));
	r(1,tmp*sin(v[1]));
	r(2,v[0]*cos(v[2]));
	return(r);
}

Vector<3> &Vector<3>::to_spherical()
{
	double tmpx=abs();
	double tmpy=atan2(x[1],x[0]);   // correct??
	x(2,acos(x[2]/tmpx));
	x(1,tmpy);
	x(0,tmpx);
	return(*this);
}

Vector<3> &Vector<3>::to_rectangular()
{
	double tmpy=x[0]*sin(x[2]);  // r*sin(theta)
	double tmpx=tmpy*cos(x[1]);
	tmpy*=sin(x[1]);
	x(2,x[0]*cos(x[2]));
	x(0,tmpx);
	x(1,tmpy);
	return(*this);
}


// get the rotation from v1 to v2 around axis 
double get_rotation_around(
	const Vector<3> &v1,const Vector<3> &v2,const Vector<3> &axis)
{
	// rotate both vectors so that axis maches z and v1 is 
	// in the x-z-plain
	Matrix<4,4> rot_easy=Mrotate_pair_pair(axis,v1,
		Vector<3>(0.0,0.0,1.0),Vector<3>(1.0,0.0,0.0));
	Vector<3> easy_v2=rot_easy*v2;

	// get rotation from easy_v2 to x-z-plain around z
	double z_angle = atan2(easy_v2[1],easy_v2[0]);

	return(z_angle);
}

// rotates a vector pair to another
// the first vectors of each pair will mach exactly afterwards but the second
// may differ in the angle to the first one. They will be in the same plane
// then. The result are rotations about x-,y- and z-axis as a vector 
Vector<3> Vrotate_pair_pair(
	const Vector<3> &vect1f,const Vector<3> &vect1u,
	const Vector<3> &vect2f,const Vector<3> &vect2u)
{
  //!!! lazy implementation !!!
  Matrix<4,4> m = Mrotate_pair_pair( vect1f, vect1u, vect2f, vect2u );
  Vector<3> res = get_rotate_component( m );
  return res;
}

}  // namespace end

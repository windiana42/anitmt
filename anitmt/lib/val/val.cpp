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
 *   Dec 2000   started writing
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
  
}

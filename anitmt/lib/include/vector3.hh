/*  
Copyright (c) 1999  Roberto Javier Peon


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Maintainer: Roberto Javier Peon
Contact: fenix@acm.org
*/

#ifndef __VECTOR3__
#define __VECTOR3__

#include <iostream.h>

// NOTE all function declared within the class that return 
// a vector3 are DESTRUCTIVE functions.
// i.e. a.proj(b) == a=proj(a,b)
// only use them if you want the result stored in  
// *this

// all the functions which are declared outside the class
// are completely safe.

class vector3 {
public:
  // [x,y,z] => [3] 
  double coord[3];
  vector3();
  vector3(double, double, double);
  vector3(double*);
  virtual ~vector3();

  friend ostream& operator<<(ostream& , const vector3& ) ;
  vector3& operator=(const vector3&);
  friend istream& operator>>(istream& , vector3& );

  // add each element of a by constant
  // and store it in that element.
  vector3& operator+=(const double);
  // subtract each element of a by constant 
  // and store it in that element.
  vector3& operator-=(const double);
  // multiply each element of a by constant
  // and store it in that element.
  vector3& operator*=(const double);
  // divide each element of a by constant 
  // and store it in that element.
  vector3& operator/=(const double);

  // add each element of a by respective element in b
  // and store it in that element of a.
  vector3& operator+=(const vector3& );
  // subtract each element of a by respective element in b
  // and store it in that element of a.
  vector3& operator-=(const vector3& );
  // multiply each element of a by respective element in b
  // and store it in that element of a.
  vector3& operator*=(const vector3& );
  // divide each element of a by respective element in b
  // and store it in that element of a.
  vector3& operator/=(const vector3& );

  double dot(const vector3& )const ;
  // a.cross(b) == cross of a and b stored into a.
  vector3& cross(const vector3&);
  // a.normalize() == normalization of a stored into a.
  vector3& normalize();
  // a.length() == length of a
  double length() const;
  // a.length2() == length of a squared. this is LOTS FASTER
  double length2() const;

  // add each element of a by constant
  vector3  operator+(const double) const;
  // subtract each element of a by constant
  vector3  operator-(const double) const;
  // multiply each element of a by constant
  vector3  operator*(const double) const;
  // divide each element of a by constant
  vector3  operator/(const double) const;
  // add each element of a by respective element in b
  vector3  operator+(const vector3&) const;
  // subtract each element of a by respective element in b
  vector3  operator-(const vector3&) const;
  // multiply each element of a by respective element in b
  vector3  operator*(const vector3&) const;
  // divide each element of a by respective element in b
  vector3  operator/(const vector3&) const;

  // return true if both are not zero  
  int operator&&(const vector3&) const; 
  // return true if either are not zero
  int operator||(const vector3&) const; 

  int operator==(const vector3&) const;
  int operator!=(const vector3&) const;

  // a.rotate(c) == rotation of a about the x axis by c radians
  // and storing the result in a
  vector3& rotateX(const double );
  // a.rotate(c) == rotation of a about the y axis by c radians
  // and storing the result in a
  vector3& rotateY(const double );
  // a.rotate(c) == rotation of a about the z axis by c radians
  // and storing the result in a
  vector3& rotateZ(const double );
  // a.rotate(b) == rotation of a about the 
  // x axis by b.coord[0] radians
  // y axis by b.coord[1] radians
  // z axis by b.coord[2] radians (in that order)
  // and storing the result in a.
  vector3& rotateXYZ(const vector3& );
  // a.rotate(b,c) == rotation of a about the point c
  // x axis by b.coord[0] radians
  // y axis by b.coord[1] radians
  // z axis by b.coord[2] radians (in that order)
  // and storing the result in a.
  vector3& rotateXYZ(const vector3&, const  vector3& );
  
// interpret current contents as rectangular
// and convert to spherical. Stores result in *this
  vector3& toSpherical();
// interpret current contents as spherical
// and convert to rectangular. Stores result in *this
  vector3& toRectangular();
  // a.proj(b) == projection of b onto a, with result stored in a.
  vector3& proj(const vector3&);
  // component(a,b) == length of projection of a onto b.
  double   component(const vector3&) const;
};
double dot(const vector3& ,const  vector3&) ;
vector3 cross(const vector3&,const  vector3&) ;
vector3 normalize(const vector3&) ;
double length(const vector3&);
double length2(const vector3&);

  // a.rotate(c) == rotation of a about the x axis by c radians
  // and storing the result in a
vector3 rotateX(const vector3&,const double );
  // a.rotate(c) == rotation of a about the y axis by c radians
  // and storing the result in a
vector3 rotateY(const vector3&,const double );
  // a.rotate(c) == rotation of a about the z axis by c radians
  // and storing the result in a
vector3 rotateZ(const vector3&,const double );
  // rotate(a,b) == rotation of a by
  // x axis by b.coord[0] radians
  // y axis by b.coord[1] radians
  // z axis by b.coord[2] radians (in that order)
  // and storing the result in a.
vector3 rotateXYZ(const vector3&, const vector3& );
  // rotate(a,b,c) == rotation of a about the point c and the
  // x axis by b.coord[0] radians
  // y axis by b.coord[1] radians
  // z axis by b.coord[2] radians (in that order)
  // and storing the result in a.
vector3 rotateXYZ(const vector3&, const vector3&, const  vector3& );

  // c+a add each element of a by constant (c)
vector3  operator+(const double, const vector3&);
  // c+a subtract each element of a by constant (c)
vector3  operator-(const double, const vector3&);
  // c+a multiply each element of a by constant (c)
vector3  operator*(const double, const vector3&);
  // c+a invert each element of a and multiply times 1/c
vector3  operator/(const double, const vector3&);

// interpret current contents as rectangular
// and convert to spherical.
vector3 toSpherical(const vector3&); 
// interpret current contents as spherical
// and conver to rectangular.
vector3 toRectangular(const vector3&); 
// proj(b,a) == projection of a onto b.
vector3 proj(const vector3&, const vector3&); 
// component(b,a) == length of projection of a onto b.
double  component(const vector3&, const vector3&); 

#endif

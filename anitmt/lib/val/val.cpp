/*****************************************************************************/
/**   This file offers datatypes designed for AniTMT			    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/*****************************************************************************/

#include <iostream>

#include "val.hpp"

namespace values{
  //*******
  // Scalar
  //*******
  Scalar::operator double() const { return x; }

  Scalar::Scalar() : x(0) {}
  Scalar::Scalar( double _x ) : x(_x) {}

  //*******
  // Vector
  //*******
  Vector::operator vect::vector3() const { return x; }

  Vector::Vector() : x(0) {}
  Vector::Vector( vect::vector3 _x ) : x(_x) {}

  //*******
  // Vector
  //*******

  Vector operator*( const Scalar s, const Vector v ) {
    return s.x * v.x;
  }
  Vector operator*( const Vector v, const Scalar s ) {
    return s.x * v.x;
  } 
  Vector operator+( const Vector v1, const Vector v2 ) {
    return v1.x + v2.x;
  }
  Scalar operator*( const Vector v1, const Vector v2 ) {
    return dot(v1.x,v2.x);
  }

  Scalar abs( const Vector v ){
    return v.x.length();
  }
}

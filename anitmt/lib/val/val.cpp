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
  Vector::Vector() : vect::vector3(0) {}
  Vector::Vector( vect::vector3 _x ) : vect::vector3(_x) {}

  //*******
  // String
  //*******

  String::String( std::string s ) : std::string( s ) {}
  String::String() : std::string() {}
  
  //*******
  // Flag
  //*******

  Flag::Flag( bool b ) : x( b ) {}
  Flag::Flag() {}
  
  //**********
  // Operators
  //**********

  Vector operator*( const Scalar s, const Vector v ) {
    return s.x * v;
  }
  Vector operator*( const Vector v, const Scalar s ) {
    return s.x * v;
  } 

  Scalar abs( const Vector v ){
    return v.length();
  }
}

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

#ifndef __values__
#define __values__

#include <string>

#include "vector.hpp"

namespace values{

  class Scalar;
  class Vector;
  class Matrix;
  class String;
  class Flag;

  class Valtype{
    enum Types{ scalar, vector, matrix, string, flag };

    Types type;
  public:
    Types get_type();
  };

  class Scalar : public Valtype{
    double x;
  public:
    operator double() const;
    friend Vector operator*( const Scalar, const Vector );
    friend Vector operator*( const Vector, const Scalar );

    Scalar( double i );
    Scalar();
  };
  
  class Vector : public Valtype, public vect::vector3 {
  public:
    Vector( vect::vector3 v );
    Vector();
  };

  class Matrix : public Valtype{
  public:
  };

  class String : public Valtype, public std::string{
  public:
    String( std::string );
    String();
  };

  class Flag : public Valtype {
    bool x;
  public:
    Flag( bool b );
    Flag();
  };

  //**********
  // Operators
  //**********

  Vector operator*( const Scalar, const Vector );
  Vector operator*( const Vector, const Scalar );
  Scalar abs( const Vector );
}
#endif

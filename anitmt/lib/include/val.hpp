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
  
  class Vector : public Valtype{
    vect::vector3 x;
  public:
    operator vect::vector3() const;
    friend Vector operator*( const Scalar, const Vector );
    friend Vector operator*( const Vector, const Scalar );
    friend Vector operator+( const Vector, const Vector );
    friend Scalar operator*( const Vector, const Vector );
    friend Scalar abs( const Vector );

    Vector( vect::vector3 v );
    Vector();
  };

  class Matrix : public Valtype{
  public:
  };

  class String : public Valtype{
    std::string x;
  public:
  };

  class Flag : public Valtype{
    bool x;
  public:
  };

  //**********
  // Operators
  //**********

  Vector operator*( const Scalar, const Vector );
  Vector operator*( const Vector, const Scalar );
  Vector operator+( const Vector, const Vector );
  Scalar operator*( const Vector, const Vector );
  Scalar abs( const Vector );
}
#endif

/*****************************************************************************/
/**   Offers all specialized basic AniTMT types                 	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#ifndef __AniTMT_types__
#define __AniTMT_types__

#include "val.hpp"
#include "vector.hpp"

namespace anitmt{

  //*************
  // Scalar types
  //*************

  class Angle : public values::Scalar {
  public:
    Angle() : values::Scalar() {}
    Angle( values::Scalar v ) : values::Scalar(v) {}
  };

  //*************
  // Vector types
  //*************

  class Position  : public values::Vector {
  public:
    Position() : values::Vector() {}
    Position( values::Vector v ) : values::Vector(v) {}
  };
  class Direction : public values::Vector {
  public:
    Direction() : values::Vector() {}
    Direction( values::Vector v ) : values::Vector(v) {}
  };
  class Up_Vector : public values::Vector {
  public:
    Up_Vector() : values::Vector() {}
    Up_Vector( values::Vector v ) : values::Vector(v) {}
  };

  //************
  // State types
  //************

  class Scene_State {};

  class Object_State : public values::Matrix {
  public:
    Object_State() : values::Matrix() {}
    Object_State( values::Matrix v ) : values::Matrix(v) {}
  };

  class Scalar_State : public values::Scalar {
  public:
    Scalar_State() : values::Scalar() {}
    Scalar_State( double v ) : values::Scalar(v) {}
    Scalar_State( values::Scalar v ) : values::Scalar(v) {}
  };

}

#endif

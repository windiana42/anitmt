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

#include <val/val.hpp>
#include "return.hpp"

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

  //*************
  // Object_State

  class Object_State 
  {
    values::Vector translate,rotate,scale; // transformations
    bool active;
  public:
    inline values::Matrix get_matrix();
    inline values::Vector get_translate();
    inline values::Vector get_rotate();
    inline values::Vector get_scale();
    inline bool is_active();

    Object_State() : active(false) {}
    Object_State( values::Vector trans, values::Vector rot, 
		  values::Vector scale );
    Object_State( values::Vector trans, values::Vector rot, 
		  values::Vector scale, bool active );
    Object_State( values::Matrix v );
    Object_State( values::Matrix v, bool a );
  };

  //*************
  // Scalar_State

  class Scalar_State 
  {
    values::Scalar val;
    bool active;
  public:
    inline values::Scalar get_value();
    inline bool is_active();

    Scalar_State();
    Scalar_State( double v );
    Scalar_State( double v, bool a );
    Scalar_State( values::Scalar v );
    Scalar_State( values::Scalar v, bool a );
  };

  //*****************
  // Reference types
  //*****************
  /*
  class Pos_Reference {
    Return<Position> *node;
  public:
    Return<Position> *get_node();

    Node_Reference( Prop_Tree_Node *node )
  };
  */
}

#include "tmttype_inline.cpp"

#endif

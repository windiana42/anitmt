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

#include "tmttype.hpp"

namespace anitmt{

  //************
  // State types
  //************

  //*************
  // Object_State

  Object_State::Object_State( values::Vector trans, values::Vector rot, 
			      values::Vector scal )
    : translate(trans), rotate(rot), scale(scal), active(true) {}
  Object_State::Object_State( values::Vector trans, values::Vector rot, 
			      values::Vector scal, bool a )
    : translate(trans), rotate(rot), scale(scal), active(a) {}

  Object_State::Object_State( values::Matrix v ) 
    : translate( get_translate_component(v) ),
      rotate( get_rotate_component(v) ),
      scale( get_scale_component(v) ),
      active(true) {}

  Object_State::Object_State( values::Matrix v, bool a )
    : translate( get_translate_component(v) ),
      rotate( get_rotate_component(v) ),
      scale( get_scale_component(v) ),
      active(a) {}

  //*************
  // Scalar_State

  Scalar_State::Scalar_State() : active(false) {}
  Scalar_State::Scalar_State( double v ) : val(v), active(true) {}
  Scalar_State::Scalar_State( double v, bool a ) : val(v), active(a) {}
  Scalar_State::Scalar_State( values::Scalar v ) : val(v), active(true) {}
  Scalar_State::Scalar_State( values::Scalar v, bool a ) : val(v), active(a) {}

  //*****************
  // Reference types
  //*****************
}


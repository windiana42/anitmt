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

#ifndef __AniTMT_types_inline_implementation__
#define __AniTMT_types_inline_implementation__

#include "tmttype.hpp"

namespace anitmt{

  //************
  // State types
  //************

  //*************
  // Object_State

  values::Matrix Object_State::get_matrix()
  {
    return Mtranslate( translate ) * Mrotate( rotate ) * Mscale( scale );
  }
  values::Vector Object_State::get_translate() { return translate; }
  values::Vector Object_State::get_rotate() { return rotate; }
  values::Vector Object_State::get_scale() { return scale; }
  bool Object_State::is_active() { return active; }

  //*************
  // Scalar_State

  values::Scalar Scalar_State::get_value() { return val; }  
  bool Scalar_State::is_active() { return active; }

  //*****************
  // Reference types
  //*****************
}

#endif

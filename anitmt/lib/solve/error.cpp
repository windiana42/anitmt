/*****************************************************************************/
/**   Offers Exception classes for error handling               	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#include "error.hpp"

namespace solve {

  //******************************
  // EX: general solve exception
  //******************************

  EX::EX(const std::string n ) : name(n) {}
  EX::~EX() {}

  //****************************************
  // Error_Position: general error position
  //****************************************

  Error_Position::~Error_Position() {}

  //******************************
  // EX: general user error
  //******************************

  EX_user_error::EX_user_error(const std::string name, Error_Position *p )
    : EX(name), pos(p) {}
}


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
}

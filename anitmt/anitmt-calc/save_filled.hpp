/*****************************************************************************/
/**   This file offers a save function in the filled ADL format		    **/
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

#ifndef __AniTMT_Save_Filled__
#define __AniTMT_Save_Filled__

#include <string>
#include <proptree/proptree.hpp>
#include "animation.hpp"

namespace anitmt {
  static const std::string WN_INDENT_STRING = "  ";
  void save_filled( std::string filename, Animation *root );
}

#endif


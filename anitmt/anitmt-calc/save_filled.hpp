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
#include "proptree.hpp"

namespace anitmt {
  static const int WN_INDENT_WIDTH = 2;
  void save_filled( std::string filename, Prop_Tree_Node *root );
}

#endif


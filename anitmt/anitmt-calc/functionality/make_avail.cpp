/*****************************************************************************/
/**   This file makes the generated functionality availible		    **/
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

#include "base_func.hpp"
#include "functionality.hpp"
#include "scalar.hpp"
#include "object.hpp"

namespace functionality 
{
  void make_nodes_availible()
  {
    make_base_func_nodes_availible();
    make_functionality_nodes_availible();
    make_scalar_nodes_availible();
    make_object_nodes_availible();
  }
}

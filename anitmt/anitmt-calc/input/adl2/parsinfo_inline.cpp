/*****************************************************************************/
/**   This file offers a class where the parser stores information         **/
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

#ifndef __anitmt_input_adl_parsinfo_inline_implementation__
#define __anitmt_input_adl_parsinfo_inline_implementation__

#include "parsinfo.hpp"

namespace anitmt
{
  namespace adlparser
  {
    //*********************************************************************
    // adlparser_info: stores information needed by the parser and scanner
    //*********************************************************************

    inline void adlparser_info::set_pass( pass_type p )
    {
      pass = p;
    }

    //*********************************************
    // tree node access/modify functions functions

    Prop_Tree_Node *adlparser_info::get_current_tree_node()
    {
      return tree_node.top();
    }

    void adlparser_info::set_new_tree_node( Prop_Tree_Node *node )
    {
      tree_node.push( node );
    }
    void adlparser_info::tree_node_done()
    {
      assert( !tree_node.empty() );
      tree_node.pop();
    }

  }
}
#endif

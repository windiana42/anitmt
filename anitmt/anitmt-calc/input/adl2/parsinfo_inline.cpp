/*****************************************************************************/
/**   This file offers a class where the parser stores information         **/
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

#ifndef __anitmt_input_adl_parsinfo_inline_implementation__
#define __anitmt_input_adl_parsinfo_inline_implementation__

#include "parsinfo.hpp"
#include <assert.h>

#ifdef EXTREME_INLINE
#define _INLINE_ inline
#else
#define _INLINE_
#endif

namespace anitmt
{
  namespace adlparser
  {
    //*********************************************************************
    // adlparser_info: stores information needed by the parser and scanner
    //*********************************************************************

    _INLINE_ void adlparser_info::set_pass( pass_type p )
    {
      pass = p;
    }

    //*********************************************
    // tree node access/modify functions functions

    proptree::Prop_Tree_Node *adlparser_info::get_current_tree_node()
    {
      return tree_node.top();
    }

    void adlparser_info::set_new_tree_node( proptree::Prop_Tree_Node *node )
    {
      tree_node.push( node );
    }
    void adlparser_info::tree_node_done()
    {
      assert( !tree_node.empty() );
      tree_node.pop();
    }

    //! store position for later access
    _INLINE_ void adlparser_info::store_pos()
    {
      while( old_positions.size() >= max_old_positions )
      {
	delete old_positions.back();
	old_positions.pop_back();
      }
      old_positions.push_front( get_pos() );
    }
    //! get current position (must be deleted!)
    _INLINE_ message::Abstract_Position *adlparser_info::get_pos()
    {
      return file_pos.duplicate();
    }
    //! get stored position n (n=0: last) (must be deleted!)
    _INLINE_ message::Abstract_Position *adlparser_info::get_old_pos( unsigned n)
    {
      // too few elements availible?
      if( old_positions.size() <= n ) return 0;

      return old_positions[n];
    }
    //! set maximum number of stored positions
    _INLINE_ void adlparser_info::set_max_old_positions( unsigned n )
    {
      max_old_positions = n;
      while( old_positions.size() > max_old_positions )
      {
	delete old_positions.back();
	old_positions.pop_back();
      }
    }
  }
}
#undef _INLINE_

#endif

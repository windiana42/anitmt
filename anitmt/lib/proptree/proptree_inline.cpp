/*****************************************************************************/
/**   This file offers a tree structure for groups of properties	    **/
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

#ifndef __Proptree_Prop_Tree_inline_implementation__
#define __Proptree_Prop_Tree_inline_implementation__

#include "proptree.hpp"

#include "property.hpp"

namespace proptree{

  //************************************************************
  // Prop_Tree_Node: provides tree structure for property groups
  //************************************************************

  inline void Prop_Tree_Node::set_position( message::Abstract_Position* p ) 
  { 
    if( (pos != 0) && (pos != message::GLOB::no_position) )
      delete pos;
    pos = p; 
  }

  inline void Prop_Tree_Node::add_property( std::string name, Property *prop )
  {
    //prop->set_name(name);
    //prop->set_node(this);
    properties[ name ] = prop;
  }

}
#endif
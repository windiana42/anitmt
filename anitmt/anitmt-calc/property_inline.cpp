/*****************************************************************************/
/**   This file offers a container type for properties			    **/
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

#ifndef __AniTMT_Property_inline_implementation__
#define __AniTMT_Property_inline_implementation__

#include "property.hpp"

namespace anitmt
{
  //****************************************
  // Property: container for property values
  //****************************************

  //***********
  // modifiers

  inline void Property::set_name( std::string n ) { name = n; }
  inline void Property::set_node( Prop_Tree_Node* n ) { node = n; }	
  inline void Property::set_position( message::Abstract_Position* p ) 
  { pos = p; }
  inline void Property::set_type( values::Valtype::Types t ) { type = t; }

  //******************
  // access functions
  
  std::string			Property::get_name() { return name; }
  message::Abstract_Position*	Property::get_position() { return pos; }
  values::Valtype::Types	Property::get_type() { return type; }
  Prop_Tree_Node*		Property::get_node() 
  { 
    assert( node != 0 ); return node; 
  }	

}

#endif

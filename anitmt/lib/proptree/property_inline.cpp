/*****************************************************************************/
/**   This file offers a container type for properties			    **/
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

#ifndef __Proptree_Property_inline_implementation__
#define __Proptree_Property_inline_implementation__

#include "property.hpp"

#include <assert.h>

namespace proptree
{
  //****************************************
  // Property: container for property values
  //****************************************

  //***********
  // modifiers

  inline void Property::set_name( std::string n ) { name = n; }
  inline void Property::set_node( Prop_Tree_Node* n ) { node = n; }	
  inline void Property::set_position( message::Abstract_Position* p ) 
  { 
    if( (pos != 0) && (pos != message::GLOB::no_position) )
      delete pos;
    pos = p; 
  }
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

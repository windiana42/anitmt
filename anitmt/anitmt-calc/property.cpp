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

#include "property.hpp"

#include <algorithm>

namespace anitmt
{
  //****************************************
  // Property: container for property values
  //****************************************

  //***********
  // modifiers
  
  // tells the property where it occurs in user's inputs
  void Property::set_input_position( message::Abstract_Position *p )
  {
    if( pos != 0 )
      ;//!!! warn that there is already another position
    pos = p;
  }
  
  //******************
  // access functions
  
  std::string			Property::get_name() { return name; }
  Prop_Tree_Node*		Property::get_node() { return node; }	
  message::Abstract_Position*	Property::get_position() { return pos; }
  values::Valtype::Types	Property::get_type() { return type; }

  //************************
  // Constructor/Destructor

  Property::Property( std::string n, Prop_Tree_Node *nod, 
		      values::Valtype::Types t ) 
    : name(n), node(nod), pos(0), type(t) {}

  //***********
  // operators

  // operator for output on standard out streams
  std::ostream &operator << ( std::ostream& os, Property &prop ){
    return prop.write2stream( os );
  }

  //******************************************************
  // Scalar_Property: container for scalar property values
  //******************************************************
  
  Scalar_Property::Scalar_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::Scalar>( name, node, values::Valtype::scalar );

  //******************************************************
  // Vector_Property: container for vector property values
  //******************************************************
  
  Vector_Property::Vector_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::Vector>( name, node, values::Valtype::vector );
    
  //******************************************************
  // Matrix_Property: container for matrix property values
  //******************************************************
  
  Matrix_Property::Matrix_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::Matrix>( name, node, values::Valtype::matrix );
    
  //******************************************************
  // String_Property: container for string property values
  //******************************************************
  
  String_Property::String_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::String>( name, node, values::Valtype::string );
    
  //******************************************************
  // Flag_Property: container for flag property values
  //******************************************************
  
  Flag_Property::Flag_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::Flag>( name, node, values::Valtype::flag );
    
  
}

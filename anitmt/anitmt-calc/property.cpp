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

#include <val/val.hpp>

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
    : Type_Property<values::Scalar>( name, node, values::Valtype::scalar ) {}

  Scalar_Property::Scalar_Property()
    : Type_Property<values::Scalar>( "none", 0, values::Valtype::scalar ) {}

  //******************************************************
  // Vector_Property: container for vector property values
  //******************************************************
  
  Vector_Property::Vector_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::Vector>( name, node, values::Valtype::vector ) {}
    
  Vector_Property::Vector_Property()
    : Type_Property<values::Vector>( "none", 0, values::Valtype::vector ) {}

  //******************************************************
  // Matrix_Property: container for matrix property values
  //******************************************************
  
  Matrix_Property::Matrix_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::Matrix>( name, node, values::Valtype::matrix ) {}
    
  Matrix_Property::Matrix_Property()
    : Type_Property<values::Matrix>( "none", 0, values::Valtype::matrix ) {}

  //******************************************************
  // String_Property: container for string property values
  //******************************************************
  
  String_Property::String_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::String>( name, node, values::Valtype::string ) {}
    
  String_Property::String_Property()
    : Type_Property<values::String>( "none", 0, values::Valtype::string ) {}

  //******************************************************
  // Flag_Property: container for flag property values
  //******************************************************
  
  Flag_Property::Flag_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::Flag>( name, node, values::Valtype::flag ) {}
    
  Flag_Property::Flag_Property()
    : Type_Property<values::Flag>( "none", 0, values::Valtype::flag ) {}

  
}

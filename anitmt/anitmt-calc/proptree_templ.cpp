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

#ifndef __AniTMT_Prop_Tree_Templateimplementaion__
#define __AniTMT_Prop_Tree_Templateimplementaion__

#include "proptree.hpp"

namespace anitmt{

  //************************************************************
  // Prop_Tree_Node: provides tree structure for property groups
  //************************************************************

  //************
  // exceptions

  class Prop_Tree_Node::EX_property_rejected : public EX {
  public:
    EX_property_rejected() : EX( "property rejected" ) {}
  };
  class Prop_Tree_Node::EX_property_type_rejected : public EX {
  public:
    EX_property_type_rejected(/* std::string wrong_type, std::string right_type*/ )
      : EX( /*right_type + " expected. " + wrong_type + " found instead"*/ "wrong property type" ) {}
  };
  class Prop_Tree_Node::EX_property_unknown : public EX {
  public:
    EX_property_unknown( /*std::string prop_name*/ ) 
      : EX( "unknown property" /*+ prop_name*/ ) {}
  };

  //*******************
  // template functions
  
  // set property
  template< class T >  
  void Prop_Tree_Node::set_property( std::string name, T val ) 
    throw( EX_property_unknown, EX_property_type_rejected, 
	   EX_property_rejected ) {
    
    if( get_property( name ) != 0 )
      {
	Type_Property<T> *p = 
	    dynamic_cast< Type_Property<T>* >( properties[ name ] );
	
	if( !p )
	  throw EX_property_type_rejected();
	
	bool accepted;
	try{
	  accepted = p->set_if_ok( val );
	}
	catch( EX ){
	  throw EX_property_rejected();
	}

	if( !accepted )
	  throw EX_property_rejected();
      }
    else
      throw EX_property_unknown();
  }

  //**************************************************************************
  // Node_Factory<NT>: provides a factory template for objects that may create 
  //                   any node objects of type NT
  //**************************************************************************

  //**********
  // functions

  template< class NT>
  Prop_Tree_Node *Node_Factory<NT>::create( std::string name, Animation *ani ){
    return new NT( name, ani );
  }

}

#endif

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

  //*******************
  // template functions
  
  // set property
  template< class T >  
  void Prop_Tree_Node::set_property( std::string name, T val ) 
    throw( EX_unknown_property,EX_wrong_property_type ) {
    
    if( get_property( name ) != 0 )
      {
	Type_Property<T> *p = 
	    dynamic_cast< Type_Property<T>* >( properties[ name ] );
	
	if( !p )
	  throw EX_wrong_property_type();
	
	bool accepted = p->set_if_ok( val );
	if( !accepted )
	  throw EX_property_rejected();
      }
    else
      throw EX_unknown_property();
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

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
    throw( EX_property_unknown, EX_property_type_rejected, 
	   EX_property_rejected ) {
    
    Property *property = get_property( name );
    if( property != 0 )
      {
	Type_Property<T> *p = dynamic_cast< Type_Property<T>* >( property );
	
	if( !p ) throw EX_property_type_rejected();
	
	bool accepted;
	try{
	  accepted = p->set_value( val );
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

namespace solve
{
  //***************************
  // Push to another tree node 
  //***************************

  // establishes push connection to property of foreign tree node
  // ( returnvalue false means: unknown property )
  template<class T>
  bool establish_Push_Connection( Priority_System *sys, 
				  Priority_System::level_type level,
				  Operand<T> &src, 
				  anitmt::Prop_Tree_Node *dest_node,
				  std::string dest_prop ) {
  
    if( !dest_node ) return false;

#ifdef __DEBUG__
    std::cout << "try to establish push" << std::endl;
#endif
    Operand<T> *dest 
      = dynamic_cast< Operand<T>* >
      ( dest_node->get_property( dest_prop ) );

    if( !dest ) return false;

    establish_Push_Connection( sys, level, src, *dest );

#ifdef __DEBUG__
    std::cout << "push established on level " << level << std::endl;
#endif

    return true;
  }
}
#endif

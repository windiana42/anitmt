/*****************************************************************************/
/**   This file offers a tree structure for groups of properties	    **/
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

#ifndef __Proptree_Prop_Tree_Templateimplementaion__
#define __Proptree_Prop_Tree_Templateimplementaion__

#include "proptree.hpp"

namespace proptree
{
  //************************************************************
  // Prop_Tree_Node: provides tree structure for property groups
  //************************************************************

  //*******************
  // template functions
  
  // set property (return value false: set failed)
  template< class T >  
  Prop_Tree_Node::setp_error Prop_Tree_Node::set_property( std::string name, 
							   T val ) 
    throw() 
  {
    Property *property = get_property( name );
    if( property != 0 )
      {
	Type_Property<T> *p = dynamic_cast< Type_Property<T>* >( property );

	if( !p ) return SP_wrong_property_type;
	
 	if( !p->set_value( val ) )
	  return SP_value_rejected;
      }
    else
      return SP_property_unknown;

    return SP_no_err;
  }

  // **************************************************************************
  // Node_Factory: provides a factory template for objects that may create 
  //               any node objects of type NT that provides a special type
  // **************************************************************************

  template< class Provider_Type, class NT >
  Node_Factory<Provider_Type,NT>::node_return_type 
  Node_Factory<Provider_Type,NT>::
  create( std::string name, tree_info *info, message::Message_Consultant *msg )
    const
  {
    NT *node = new NT( name, info, msg );
    return node_return_type(node,node);	// return as different types
  }

  template< class Provider_Type, class NT >
  Node_Factory<Provider_Type,NT>::node_return_type 
  Node_Factory<Provider_Type,NT>::cast( Prop_Tree_Node *prop ) const
  {
    return node_return_type(dynamic_cast<Provider_Type*>(prop), prop);
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
				  proptree::Prop_Tree_Node *dest_node,
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

  // establishes push connection to property of foreign tree node
  // ( returnvalue false means: unknown property )
  template<class T>
  bool establish_Push_Connection( Priority_System *sys, 
				  Priority_System::level_type level,
				  Operand<T> &src, 
				  proptree::Property *dest_prop ) 
  {
#ifdef __DEBUG__
    std::cout << "try to establish push" << std::endl;
#endif
    Operand<T> *dest 
      = dynamic_cast< Operand<T>* >( dest_prop );

    if( !dest ) return false;

    establish_Push_Connection( sys, level, src, *dest );

#ifdef __DEBUG__
    std::cout << "push established on level " << level << std::endl;
#endif

    return true;
  }

  // establishes push connection to property of foreign tree node
  // ( returnvalue false means: unknown property )
  template<class T>
  bool establish_Push_Connection( Priority_System *sys, 
				  Priority_System::level_type level,
				  proptree::Property *src_prop, 
				  Operand<T> &dest ) 
  {
#ifdef __DEBUG__
    std::cout << "try to establish push" << std::endl;
#endif
    Operand<T> *src 
      = dynamic_cast< Operand<T>* >( src_prop );

    if( !src ) return false;

    establish_Push_Connection( sys, level, *src, dest );

#ifdef __DEBUG__
    std::cout << "push established on level " << level << std::endl;
#endif

    return true;
  }
  template<class T>
  bool equal_solver( Operand<T> &a, proptree::Property *prop )
  {
    Operand<T> *src = dynamic_cast<Operand<T>*>(prop);
  
    if( !src ) 
    {
      //!!! output better error message !!!
      a.error() << "incompatible arguments to equal solver";
      return false;
    }
    equal_solver( a, *src );
    return true;
  }
  template<class T>
  bool equal_solver( proptree::Property *prop, Operand<T> &a )
  {
    Operand<T> *src = dynamic_cast<T*>(prop);
  
    if( !src ) 
    {
      //!!! output better error message !!!
      a.error() << "incompatible arguments to equal solver";
      a.caused_error();
      return false;
    }
    equal_solver( *src, a );
    return true;
  }
}
#endif

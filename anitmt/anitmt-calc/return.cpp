/*****************************************************************************/
/**   Offers interfaces for defined return types                	    **/
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

#ifndef __AniTMT_Return_Implementation_
#define __AniTMT_Return_Implementation_

#include "return.hpp"

#include <solve/operator.hpp>
#include <solve/constraint.hpp>

namespace anitmt
{
  //************************************************************
  // << template <Return_Type> >>
  // Return: interface for tree nodes with a special return type
  //************************************************************

  template <class Return_Type>
  Return_Type Return<Return_Type>::type_id;

  template <class Return_Type>
  Return<Return_Type> *Return<Return_Type>::get_prev( Return_Type& )
  {
#ifdef __DEBUG__
    std::cout << "return prev node: " << prev << std::endl;
#endif
    return prev;
  }

  template <class Return_Type>
  Return<Return_Type> *Return<Return_Type>::get_next( Return_Type& )
  {
#ifdef __DEBUG__
    std::cout << "return next node: " << next << std::endl;
#endif
    return next;
  }

  template <class Return_Type>
  void Return<Return_Type>::set_prev( Return<Return_Type> *p )
  {
    prev = p;
#ifdef __DEBUG__
    std::cout << "setting previous node to: " << p << std::endl;
#endif
  }

  template <class Return_Type>
  void Return<Return_Type>::set_next( Return<Return_Type> *n )
  {
    next = n;
#ifdef __DEBUG__
    std::cout << "setting next node to: " << n << std::endl;
#endif
  }

  //! function that is called after hierarchy was set up for each node
  template <class Return_Type>
  void Return<Return_Type>::hierarchy_final_init( Return_Type& )
  {
    if( !next ) init_last( this );
  }

  // constructor
  template <class Return_Type>
  Return<Return_Type>::Return() : prev(0), next(0)
  {
  }

  //******************************************************************
  // << template <Return_Type> >>
  // Contain_Return: Contains Tree nodes of the same return type
  //******************************************************************

  //! tries to use the node as element for this container
  template <class Return_Type>
  bool Contain_Return<Return_Type>::try_add_child( Prop_Tree_Node 
						   *prop_node ) throw() 
  {  
    Return<Return_Type> *node 
      = dynamic_cast< Return<Return_Type>* >( prop_node );
  
    if( !node )
      return false;
    else
    {
      if( unique_child && ( num_children > 0 ) )
      {
	prop_node->error() << "There is already a child of the same type";
	return false;
      }

      if( !content.empty() )
      {
	Return<Return_Type> *last = *(--content.end()) ;
	last->set_next( node );
	last->init_next( node );
	node->set_prev( last );
	node->init_prev( last );
      }
      else
      {
	node->init_first( node );
      }

      content.push_back( node );
      num_children++;

      return true;
    }
  }

  //! returns the result according to children that are active at time t
  template <class Return_Type>
  Contain_Return<Return_Type>::Optional_Return_Type 
  Contain_Return<Return_Type>::get_return_value( values::Scalar t ) 
    throw() 
  {
    Optional_Return_Type ret;
    for( content_type::iterator i = content.begin(); i != content.end(); i++ )
    {
      ret = (*i)->get_return_value( t );

      if( ret.first ) return ret;
    }

    return ret;			// return ret.first = false
  }

  //! function that is called after hierarchy was set up for each node
  template <class Return_Type>
  void Contain_Return<Return_Type>::hierarchy_final_init()
  {
    if( essential_child && ( num_children == 0 ) )
    {
      //!!!#warning replace with new error system
      std::cerr << "essential child is missing!" << std::endl;
    }

    for( content_type::iterator i = content.begin(); i != content.end(); i++ )
    {
      (*i)->hierarchy_final_init( Return<Return_Type>::type_id );
    }    
  }

  template <class Return_Type>
  Contain_Return<Return_Type>::Contain_Return( bool essential, bool unique ) 
    : essential_child( essential ), unique_child( unique ), num_children(0) {}


  //******************************************************************
  // << template <Element_Type> >>
  // Contain: Contains Tree nodes of the same return type
  //******************************************************************

  //! tries to use the node as element for this container
  template <class Element_Type>
  bool Contain<Element_Type>::try_add_child( Prop_Tree_Node *prop_node )
    throw() 
  {  
    Element_Type *node = dynamic_cast< Element_Type* >( prop_node );
  
    if( !node )
      return false;
    else
    {
      if( unique_child && ( num_children > 0 ) )
      {
	prop_node->error() << "There is already a child of the same type";
	return false;
      }

      content.push_back( node );
      num_children++;

      return true;
    }
  }

  //! function that is called after hierarchy was set up for each node
  template <class Element_Type>
  void Contain<Element_Type>::hierarchy_final_init()
  {
    if( essential_child && ( num_children == 0 ) )
    {
      //!!! #warning replace with new error system
      std::cerr << "essential child is missing!" << std::endl;
    }


    /* content is invoked by proptree hierarchy
    for( content_type::iterator i = content.begin(); i != content.end(); i++ )
    {
      (*i)->hierarchy_final_init();
    } 
    */   
  }

  template <class Element_Type>
  Contain<Element_Type>::Contain( bool essential, bool unique ) 
    : essential_child( essential ), unique_child( unique ), num_children(0) {}

}

#endif

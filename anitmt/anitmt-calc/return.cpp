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

namespace anitmt{

  //************************************************************
  // << template <Return_Type> >>
  // Return: interface for tree nodes with a special return type
  //************************************************************


  template <class Return_Type>
  Return_Type *Return<Return_Type>::get_prev( Return_Type ){
    return prev;
  }

  template <class Return_Type>
  Return_Type *Return<Return_Type>::get_next( Return_Type ){
    return next;
  }

  template <class Return_Type>
  void Return<Return_Type>::set_prev( Return_Type* p ){
    prev = p;
  }

  template <class Return_Type>
  void Return<Return_Type>::set_next( Return_Type* n ){
    next = n;
  }

  // constructor
  template <class Return_Type>
  Return<Return_Type>::Return() : prev(0), next(0) {}

  //************************************************************
  // << template <Return_Type> >>
  // Contain_Return_Type: Contains Tree nodes of the same return type
  //************************************************************

  // tries to use the node as element for this container
  template <class Return_Type>
  bool Contain_Return<Return_Type>::try_add_child( Return<Return_Type> *node )
    throw( exception_more_than_one_child() ) {
      if( !node )
	return false;
      else
	{
	  if( unique_child && ( num_childs > 0 ) )
	    throw( more_than_one_child() );

	  content.last().set_next( node );
	  node.set_prev( content.last() );

	  content.push_back( node );
	  return true;
	}
    }

  // returns the result according to childs that are active at time t
  template <class Return_Type>
  Return_Type Contain_Return<Return_Type>::get_return_value( values::Scalar t,
							     Return_Type ) {

    for( content_type::iterator i = content.begin(); i != content.end(); i++ )
      {
	try{
	  return (*i)->get_return_value( t );
	}
	catch( exception_not_active_at_time ){
	  continue;
	}
      }
  }

  template <class Return_Type>
  Contain_Return<Return_Type>::Contain_Return( bool essential, bool unique ) 
    : essential_child( essential ), unique_child( unique ), num_childs(0) {}

}

#endif

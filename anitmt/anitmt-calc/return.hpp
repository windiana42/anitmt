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

#ifndef __AniTMT_Return__
#define __AniTMT_Return__

#include <list>
#include "val.hpp"

namespace anitmt{

  template <class Return_Type>
  class Contain_Return;  
  
  //************************************************************
  // << template <Return_Type> >>
  // Return: interface for tree nodes with a special return type
  //************************************************************

  // exception that this child is able to return a value at specified time
  class EX_not_active_at_time {};
  // exception that more than one child is added to a unique child container
  class EX_more_than_one_child {};
  // exception that no child was added but was essential
  class EX_essential_child_missing {};
  // exception that no child is able to return a value
  class EX_no_active_child {};

  template <class Return_Type>
  class Return {
  private:
    Return<Return_Type> *prev, *next;

    // friend functions for the according container class
    friend Contain_Return<Return_Type>;
    void set_prev( Return<Return_Type> *prev );
    void set_next( Return<Return_Type> *next );
  protected:
    Return<Return_Type> *get_prev( Return_Type type_ID = Return_Type() );
    Return<Return_Type> *get_next( Return_Type type_ID = Return_Type() );
  public:
    // returns the result at time t of defined return type
    virtual Return_Type get_return_value( values::Scalar t, 
					  Return_Type type_ID = Return_Type() )
      throw( EX_not_active_at_time ) = 0;

    Return();
  };

  //**************************************************************
  // << template <Return_Type> >>
  // Contain_Return: Contains tree nodes with the same return type
  //**************************************************************
  
  template <class Return_Type>
  class Contain_Return{
    bool essential_child, unique_child;
    int num_childs;

  protected:
    typedef std::list< Return<Return_Type> * > content_type;
    content_type content;
  public:

    // tries to use the node as element for this container
    bool try_add_child( Return<Return_Type> *node ) 
      throw( EX_more_than_one_child );

    // returns the result according to childs that are active at time t
    Return_Type get_return_value( values::Scalar t, 
				  Return_Type type_ID = Return_Type() )
      throw( EX_essential_child_missing, EX_no_active_child );

    Contain_Return( bool essential_child, bool is_unique_child );
  };
  

}

// templates have to enshure that all used variants are compiled
#include "return.cpp"

#endif


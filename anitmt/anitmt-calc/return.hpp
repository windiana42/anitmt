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
#include <val/val.hpp>
#include "error.hpp"
#include "proptree.hpp"

namespace anitmt{

  template <class Return_Type>
  class Contain_Return;  
  
  //************************************************************
  // << template <Return_Type> >>
  // Return: interface for tree nodes with a special return type
  //************************************************************

  // exception that more than one child is added to a unique child container
  class EX_more_than_one_child : public EX
  {
  public:
    EX_more_than_one_child() 
      : EX( "only one child of this type is allowed here") {}
  };
  // exception that no child was added but was essential
  class EX_essential_child_missing : public EX
  {
  public:
    EX_essential_child_missing() 
      : EX( "an essential child is missing" ) {}
  };

  template <class Return_Type>
  class Return {
  private:
    Return<Return_Type> *prev, *next;

    // friend functions for the according container class
    friend Contain_Return<Return_Type>;
    void set_prev( Return<Return_Type> *prev );
    void set_next( Return<Return_Type> *next );
    // initializes the connection to next/previous node
    virtual void init_next ( Return<Return_Type> *node ) {}
    virtual void init_prev ( Return<Return_Type> *node ) {}
    virtual void init_first( Return<Return_Type> *node ) {}
    virtual void init_last ( Return<Return_Type> *node ) {}

  protected:
    static Return_Type type_id;
    Return<Return_Type> *get_prev( Return_Type &type_ID = type_id );
    Return<Return_Type> *get_next( Return_Type &type_ID = type_id );
  public:
    //! consists of is_active and return_value
    typedef std::pair<bool,Return_Type> Optional_Return_Type;

    /*! returns the result at time t or false as first element, if not active
      in at time t */
    virtual Optional_Return_Type
    get_return_value( values::Scalar t, Return_Type &type_ID = type_id )
      throw() = 0;

    //! function that is called after hierarchy was set up for each node
    void hierarchy_final_init( Return_Type &type_ID );

    Return();
  };

  //**************************************************************
  // << template <Return_Type> >>
  // Contain_Return: Contains tree nodes with the same return type
  //**************************************************************
  
  template <class Return_Type>
  class Contain_Return{
    bool essential_child, unique_child;
    int num_children;

  protected:
    typedef std::list< Return<Return_Type> * > content_type;
    content_type content;
  public:
    //! returns content elements
    const content_type &get_content() const { return content; }

    //! tries to use the node as element for this container
    bool try_add_child( Prop_Tree_Node *node ) throw();

    //! consists of is_active and return_value
    typedef std::pair<bool,Return_Type> Optional_Return_Type;

    //! returns the result according to children that are active at time t
    Optional_Return_Type get_return_value
    ( values::Scalar t ) throw();

    //! function that is called after hierarchy was set up for each node
    void hierarchy_final_init();

    Contain_Return( bool essential_child, bool is_unique_child );
    virtual ~Contain_Return() {}
  };
  

  //**************************************************************
  // << template <Return_Type> >>
  // Contain: Contains tree node classes
  //**************************************************************
  
  //! contains tree node classes
  template <class Element_Type>
  class Contain
  {
    bool essential_child, unique_child;
    int num_children;

  protected:
    typedef std::list< Element_Type * > content_type;
    content_type content;
  public:
    //! returns content elements
    const content_type &get_content() const { return content; }

    //! tries to use the node as element for this container
    bool try_add_child( Prop_Tree_Node *node ) 
      throw();

    //! function that is called after hierarchy was set up for each node
    void hierarchy_final_init();

    Contain( bool essential_child, bool is_unique_child );
    virtual ~Contain() {}
  };
  

}

// templates have to enshure that all used variants are compiled
#include "return.cpp"

#endif


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

#ifndef __AniTMT_Prop_Tree__
#define __AniTMT_Prop_Tree__

#include <list>
#include <map>
#include <vector>

namespace anitmt{
  class Prop_Tree_Node;
  template< class NT> class Node_Factory;
}

#include <val/val.hpp>
#include <solve/priority.hpp>
#include <message/message.hpp>

#include "property.hpp"

//!!! should be removed!!!
#include "error.hpp"

#include "animation_classes.hpp"

namespace anitmt{

  //************************************************************
  // Prop_Tree_Node: provides tree structure for property groups
  //************************************************************

  class Prop_Tree_Node : public message::Message_Reporter
  {
  public:
    // error messages (for future use)
    enum child_err{ AC_no_err=0, AC_unique_child_err,
		    AC_child_type_rejected };	// for child creation
  private:

    // navigation in tree
    Prop_Tree_Node *parent;	// parent node
    Prop_Tree_Node *prev;	// previous node
    Prop_Tree_Node *next;	// next node
    Prop_Tree_Node *first_child;// first child node
    Prop_Tree_Node *last_child; // last child node
    int num_childs;		// number of childs

    std::string type;		// type of this node
    std::string name;		// name for reference

    message::Abstract_Position *pos;
				// last position where it is defined (by user)

    // properties
    typedef std::map< std::string, Property* > properties_type;
    properties_type properties;

    // children
    virtual bool try_add_child( Prop_Tree_Node *node ) = 0;
				
    // abstract child factory:
    class Child_Factory
    {
    public:
      virtual Prop_Tree_Node *create( std::string name, Animation* ani ) = 0;
    };

    // map of child fatories associated to a name as string 
    typedef std::map< std::string, Child_Factory* > child_factory_type;
    static child_factory_type child_factory;

    //! individual final init after hierarchy is set up (Has to call the 
    //! function of the return type container
    virtual void final_init() = 0;
  protected:
    Animation *ani;		// animation as root node

    // Access functions for hiding data structure
    inline void add_property( std::string name, Property *prop );

  public:
    //***********
    // functions

    virtual std::string	get_name();	// return name
    virtual std::string get_type();	// return type

    //! sets position where this tree node is defined by user
    inline void set_position( message::Abstract_Position* );

    //* property access
    Property *get_property( std::string name );	
				// return property (0 = unknown name)
    // set value of a property 
    enum setp_error{ SP_no_err=0, SP_wrong_property_type,
		     SP_value_rejected, SP_property_unknown };
    template< class T > 
    setp_error set_property( std::string name, T val ) throw();

    std::list<std::string> get_properties_names(); 
				// returns all property names

    //* node navigation
    inline Prop_Tree_Node* get_parent()		{ return parent; }
    inline Prop_Tree_Node* get_prev()		{ return prev; }
    inline Prop_Tree_Node* get_next()		{ return next; }
    inline Prop_Tree_Node* get_first_child()	{ return first_child; }
    inline Prop_Tree_Node* get_last_child()	{ return last_child; }
    //Prop_Tree_Node* get_child( int n );	

    //* child access

    //! returns child with name, or 0 for unknown name
    Prop_Tree_Node *get_child( std::string name );
				// return child with name
    std::list<Prop_Tree_Node*> get_all_children();
				// return all children
    Prop_Tree_Node *add_child( std::string type, std::string name )
      throw();
				// add child of type with name
    virtual Prop_Tree_Node *get_referenced_node( std::string ref, 
						 char separator='.' );
				// returns node by interpreting the hierachical
 				// reference string. 
    virtual Property *get_referenced_property( std::string ref, 
					       char separator='.' );
				// returns property by interpreting the 
 				// hierachical reference string. 

    //**************************
    // constructors / destructor

    Prop_Tree_Node( std::string type, std::string name, Animation *ani );
    virtual ~Prop_Tree_Node();

    //************************************
    // Don't call the following functions!

    static char add_child_factory( std::string name, Child_Factory* fac )
      throw();
				// adds a factory object for class generation

    //! function that is called after hierarchy was set up for each node
    void hierarchy_final_init();

  };

  //**************************************************************************
  // Node_Factory<NT>: provides a factory template for objects that may create 
  //                   any node objects of type NT
  //**************************************************************************
  template< class NT>
  class Node_Factory : public Prop_Tree_Node::Child_Factory{
  public:
    virtual Prop_Tree_Node *create( std::string name, Animation *ani );
  };


  //***************
  // test function
  //***************
  int property_tree_test();
}

namespace solve
{
  //***************************
  // Push to another tree node 
  //***************************

  // establishes push connection to operand of foreign tree node
  // ( returnvalue false means: unknown operand )
  template<class T>
  bool establish_Push_Connection( Priority_System *sys, 
				  Priority_System::level_type level,
				  Operand<T> &src, 
				  anitmt::Prop_Tree_Node   *dest_node,
				  std::string dest_prop );

}
// force template generation of all used types
#include "proptree_templ.cpp"
#include "proptree_inline.cpp"

#endif


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
/** Requires: class proptree::Semi_Global // may store semi global data	    **/
/*****************************************************************************/

#ifndef __Proptree_Prop_Tree__
#define __Proptree_Prop_Tree__

#include <list>
#include <map>
#include <vector>

namespace proptree
{
  // required!!!
  class Semi_Global;		// requires class for semi global variables
  // provided
  class Prop_Tree_Node;
  template< class Provider_Type, class NT> class Node_Factory;
}

#include <val/val.hpp>
#include <solve/priority.hpp>
#include <message/message.hpp>

#include "property.hpp"

namespace proptree
{
  //! stores information needed by any tree node
  class tree_info		
  {
    int id_counter;
  public:
    solve::Priority_System *priority_system;
    std::string get_unique_id(); // returns any unique id string

    Semi_Global *GLOB;		// stores semi global variables

    tree_info(solve::Priority_System *p_sys, Semi_Global *glob);
  };

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

    std::string type_name;	// type of this node
    std::string name;		// name for reference

    message::Abstract_Position *pos;
				// last position where it is defined (by user)

    // properties
    typedef std::map< std::string, Property* > properties_type;
    properties_type properties;

    // map of child fatories associated to a name as string 
    //typedef std::map< std::string, Child_Factory* > child_factory_type;
    //static child_factory_type child_factory;

    // ** constraint, solver and action establishing functions **
    //! establish general dependencies (common)
    virtual void common_init() = 0;
    virtual void first_init() {/*optional*/}
    virtual void last_init()  {/*optional*/}
  protected:
    tree_info *info;		// general informtion

    // Access functions for hiding data structure
    inline void add_property( std::string name, Property *prop );

    // children
    virtual Prop_Tree_Node *try_add_child( std::string type, std::string name )
      throw() = 0;

    //! custom initialization after hierarchy was set up
    virtual void custom_hierarchy_final_init() = 0;
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

    Prop_Tree_Node( std::string type, std::string name, tree_info *info,
		    message::Message_Consultant *msg_consultant );
    virtual ~Prop_Tree_Node();

    //************************************
    // Don't call the following functions!

    //static char add_child_factory( std::string name, Child_Factory* fac )
    //  throw();
				// adds a factory object for class generation

    //! function that is called after hierarchy was set up for each node
    void hierarchy_final_init();

  };

  // **************************************************************************
  // Basic_Node_Factory<NT>: provides a factory interface template for
  //                         any nodes of a special provider type
  // **************************************************************************
  template< class Provider_Type >
  class Basic_Node_Factory {
  public:
    typedef std::pair<Provider_Type*,Prop_Tree_Node*> node_return_type;
    virtual node_return_type create( std::string name, tree_info *info,
				      message::Message_Consultant *msg ) const
      = 0;
    virtual node_return_type cast( Prop_Tree_Node * ) const = 0;
  };

  // **************************************************************************
  // Node_Factory: provides a factory template for objects that may create 
  //               any node objects of type NT that provides a special type
  // **************************************************************************
  template< class Provider_Type, class NT >
  class Node_Factory : public Basic_Node_Factory<Provider_Type> {
  public:
    virtual node_return_type create( std::string name, tree_info *info,
				      message::Message_Consultant *msg ) const;
    virtual node_return_type cast( Prop_Tree_Node * ) const;
  };

  /*
  // ***************
  // test function
  // ***************
  int property_tree_test();
  */
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
				  proptree::Prop_Tree_Node   *dest_node,
				  std::string dest_prop );

}
// force template generation of all used types
#include "proptree_templ.cpp"
#include "proptree_inline.cpp"

#endif


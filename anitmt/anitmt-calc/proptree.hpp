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

#include "property.hpp"

//!!! should be replaced!!!
#include "error.hpp"

#include "animation_classes.hpp"

namespace anitmt{

  //************************************************************
  // Prop_Tree_Node: provides tree structure for property groups
  //************************************************************

  class Prop_Tree_Node{
    // navigation in tree
    Prop_Tree_Node *parent;	// parent node
    Prop_Tree_Node *prev;	// previous node
    Prop_Tree_Node *next;	// next node
    Prop_Tree_Node *first_child;// first child node
    Prop_Tree_Node *last_child; // last child node

    std::string type;		// type of this node
    std::string name;		// name for reference

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
    //*****************
    // known exceptions

    class EX_child_type_unknown : public EX 
    {
    public:
      EX_child_type_unknown() : EX( "child type unknown" ) {}
    };
    class EX_child_type_rejected : public EX 
    {
    public:
      EX_child_type_rejected() : EX( "child type rejected" ) {}
    };
    class EX_child_type_already_defined : public EX 
    {
    public:
      EX_child_type_already_defined() : EX( "child_type_already_defined" ) {}
    };
    class EX_invalid_reference : public EX 
    {
    public:
      EX_invalid_reference() : EX( "invalid reference" ) {}
    };
    class EX_property_rejected : public EX 
    {
    public:
      EX_property_rejected() : EX( "property rejected" ) {}
    };
    class EX_property_type_rejected : public EX 
    {
    public:
      EX_property_type_rejected(/* std::string wrong_type, 
				   std::string right_type*/ )
	: EX( /*right_type + " expected. " + wrong_type + " found instead"*/ 
	     "wrong property type" ) {}
    };
    class EX_property_unknown : public EX {
    public:
      EX_property_unknown( /*std::string prop_name*/ ) 
	: EX( "unknown property" /*+ prop_name*/ ) {}
    };


    //***********
    // functions

    virtual std::string	get_name();	// return name
    virtual std::string get_type();	// return type
    Property *get_property( std::string name );	
				// return property (0 = unknown name)
    template< class T > 
    void set_property( std::string name, T val )
      throw( EX_property_unknown, EX_property_type_rejected, 
	     EX_property_rejected );
				// set value of a property 
    std::list<std::string> get_properties_names(); 
				// returns all property names

    Prop_Tree_Node *get_child( std::string name );
				// return child with name
    std::list<Prop_Tree_Node*> get_all_children();
				// return all children
    Prop_Tree_Node *add_child( std::string type, std::string name )
      throw( EX_child_type_unknown, EX_child_type_rejected );
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

    static void add_child_factory( std::string name, Child_Factory* fac )
      throw( EX_child_type_already_defined );
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


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

#include "val.hpp"
#include "property.hpp"

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

    Animation *ani;		// animation as root node

    // properties
    typedef std::map< std::string, Property* > properties_type;
    properties_type properties;

    // childs
    virtual bool try_add_child( Prop_Tree_Node *node ) = 0;
				
    // abstract child factory:
    class Child_Factory{
    public:
      virtual Prop_Tree_Node *create( std::string name, Animation* ani ) = 0;
    };

    // map of child fatories associated to a name as string 
    typedef std::map< std::string, Child_Factory* > child_factory_type;
    static child_factory_type child_factory;

  protected:
    // Access functions for hiding data structure
    inline void add_property( std::string name, Property *prop ){
      properties[ name ] = prop;
    }

  public:
    // known exceptions
    class EX_child_type_unkown {};
    class EX_child_type_rejected {};
    class EX_child_type_already_defined {};
    class EX_unknown_property {};
    class EX_wrong_property_type {};
    class EX_property_rejected {};
    class EX_invalid_reference {}; //!!! should be more differentiated

    std::string	get_name();	// return name
    std::string get_type();	// return type
    Property *get_property( std::string name );	
				// return property
    template< class T > 
    void set_property( std::string name, T val )
      throw( EX_unknown_property,EX_wrong_property_type );
				// set value of a property 
    std::list<std::string> get_properties_names(); 
				// returns all property names

    Prop_Tree_Node *get_child( std::string name );
				// return child with name
    std::list<Prop_Tree_Node*> get_all_childs();
				// return all childs
    Prop_Tree_Node *add_child( std::string type, std::string name )
      throw( EX_child_type_unkown, EX_child_type_rejected );
				// add child of type with name
    virtual Prop_Tree_Node *get_referenced_node( std::string ref );
				// returns node by interpreting the hierachical
 				// reference string. 

    static void add_child_factory( std::string name, Child_Factory* fac )
      throw( EX_child_type_already_defined );
				// adds a factory object for class generation
    //**************************
    // constructors / destructor

    Prop_Tree_Node( std::string type, std::string name, Animation *ani );
    virtual ~Prop_Tree_Node();

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
}

// force template generation of all used types
#include "proptree_templ.cpp"

#endif


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

namespace anitmt{
  class Prop_Tree_Node;
  class Priority_Action;
}

#include "val.hpp"
#include "property.hpp"

namespace anitmt{

  //************************************************************
  // Prop_Tree_Node: provides tree structure for property groups
  //************************************************************

  // type for a list of involved nodes
  typedef std::list< Prop_Tree_Node* > involve_list_type; 

  class Prop_Tree_Node{
    // navigation in tree
    Prop_Tree_Node *parent;	// parent node
    Prop_Tree_Node *child;	// first child node
    Prop_Tree_Node *next;	// next node
    Prop_Tree_Node *prev;	// previous node

    // Prop_Tree_Nodes that were involved in a solve run
    static involve_list_type involve_list;

    // remove all entries from the involve list
    static void clear_involve_list();
    // return the actual involve list
    static const involve_list_type &get_involve_list();
    friend Priority_Action::do_it(); // uses the two functions above
  protected:

    // check each priority level until max_priority for allowed actions
    // and return a list of all involved Prop_Tree_Nodes
    //involve_list_type check_priorities( int max_priority );

    // properties
    typedef std::map<std::string, Property*> properties_type;
    properties_type properties;

    // priority levels for actions like a transmission of properties to 
    // neighbour nodes or like setting default values
    typedef vector< Priority_Action* > priority_levels_type;
    priority_levels_type priority_level;
  public:
  };

  class Priority_Action{
  protected:
    involve_list_type recheck_priorities( int max_priority, 
					  involve_list_type involved_nodes );
  public:
    // do the action which is on the given priority level 
    virtual involve_list_type do_it( int level ) = 0;
    
    virtual ~Priority_Action() {}
  };

  template<class T>
  class Default_Value : public Priority_Action{
    Type_Property<T> *prop;
    T val;
    Prop_Tree_Node *node;
  public:
    virtual involve_list_type do_it( int level ){ 
      Prop_Tree_Node::clear_involve_list(); // prepare an empty involve list
      // set will try to set the value and its results and additionally
      // it will fill the involve list with the involved nodes
      if( !prop->set_if_ok( val ) ) // set failed?
	return involve_list_type(); // return empty list
      
      return recheck_priorities( level, Prop_Tree_Node::get_involve_list() );
    }

    Default_Value( Type_Property<T> *p, T v, Prop_Tree_Node *n ) 
      : prop(p), val(v), node(n){}
  };
}
#endif


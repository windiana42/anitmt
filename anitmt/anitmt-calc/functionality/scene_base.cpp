// ********************************************
// generated file by funcgen (www.anitmt.org)
// requires:
//   - libmessage
//   - libval
//   - libsolve
//   - libproptree
// ********************************************

#include <solve/constraint.hpp>
#include "scene_base.hpp"

namespace functionality
{
  // ****************************
  // provider type implementation
  // ****************************

  // ** type specific node connection **
  _pt_scene_type *_pt_scene_type::get_prev_scene_type()
  {
    return _tc_prev_scene_type;
  }
  _pt_scene_type *_pt_scene_type::get_next_scene_type()
  {
    return _tc_next_scene_type;
  }
  void _pt_scene_type::set_prev_scene_type( _pt_scene_type*prev )
  {
    _tc_prev_scene_type = prev;
  }
  void _pt_scene_type::set_next_scene_type( _pt_scene_type*next )
  {
    _tc_next_scene_type = next;
  }

  // ** constructor **
  _pt_scene_type::_pt_scene_type( message::Message_Consultant *c )
    : proptree::Prop_Tree_Node("","",0,c), /* should never be used */
      _tc_next_scene_type(0),
      _tc_prev_scene_type(0)
  {
  }

  // *****************
  // container classes

  // ********************************************************************
  // serial container for nodes that provide scene_type:
  //   _serial_container_scene_type

  _container_scene_type::node_factories_type _container_scene_type::node_factories;
  void _container_scene_type::add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_scene_type>* nf )
  {
    node_factories[name] = nf;
  }
  proptree::Prop_Tree_Node *_container_scene_type::
  add_child( std::string type, std::string name, proptree::tree_info *info, 
             message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj )
  {
    node_factories_type::iterator i;
    i = node_factories.find(type);
    if( i == node_factories.end() ) return already_obj;
    proptree::Basic_Node_Factory<_pt_scene_type>* &nf = i->second;
    proptree::Basic_Node_Factory<_pt_scene_type>::node_return_type node;
    if( already_obj != 0 ) 
      node = nf->cast(already_obj);
    else
      node = nf->create(name,info,msg);
    elements.push_back(node.first); // store provided type pointer
    return node.second;                      // return general prop tree node pointer
  }

  _container_scene_type::elements_type::iterator _container_scene_type::elements_begin()
  {
    return elements.begin();
  }

  _container_scene_type::elements_type::iterator _container_scene_type::elements_end()
  {
    return elements.end();
  }

  bool _container_scene_type::elements_empty()
  {
    return elements.empty();
  }

  _container_scene_type::_container_scene_type(bool _max1, bool _min1, message::Message_Consultant * )
    : max1(_max1), min1(_min1)
  {
  }

    // ** Don't call the following functions! ** 
  //! function that is called after hierarchy was set up for each node
  void _container_scene_type::hierarchy_final_init()

  {
  }
  // *****************************
  // *****************************
  // tree node type implementation
  // *****************************
  // *****************************

  // *****************************
  // root: node type
  // *****************************

  // ** virtual tree node functions **
  proptree::Prop_Tree_Node *node_root::try_add_child( std::string type, std::string name ) throw()
  {
    proptree::Prop_Tree_Node *node = 0;
    node = _cn_scene_type.add_child( type, name, info, get_consultant(), node );
    return node;
  }
  void node_root::custom_hierarchy_final_init()
  {
    _cn_scene_type.hierarchy_final_init();
  }

  void node_root::common_init()
  {
    // ** invoke first_/last_init() for each child container **
    if( !_cn_scene_type.elements_empty() )
    {
      (*_cn_scene_type.elements_begin())->_rf_scene_type_first_init();
      (*(--_cn_scene_type.elements_end()))->_rf_scene_type_last_init();
    }
  }

  // ** result functions **
  // ** infrastructure functions **
  void node_root::make_availible()
  {
  }
  // ** constructor **
  node_root::node_root( std::string name, proptree::tree_info *info, 
    message::Message_Consultant *msg_consultant )
    : proptree::Prop_Tree_Node( "root", name, info, msg_consultant ),
      _cn_scene_type(false, false, get_consultant() )
  {
    // ********************
    // Register Properties 
  }

  // *****************************
  // scene: node type
  // *****************************

  // ** virtual tree node functions **
  proptree::Prop_Tree_Node *node_scene::try_add_child( std::string type, std::string name ) throw()
  {
    proptree::Prop_Tree_Node *node = 0;
    node = _cn_object_component.add_child( type, name, info, get_consultant(), node );
    node = _cn_scalar_component.add_child( type, name, info, get_consultant(), node );
    return node;
  }
  void node_scene::custom_hierarchy_final_init()
  {
    _cn_object_component.hierarchy_final_init();
    _cn_scalar_component.hierarchy_final_init();
  }

  void node_scene::common_init()
  {
    // ** invoke first_/last_init() for each child container **
    if( !_cn_object_component.elements_empty() )
    {
      (*_cn_object_component.elements_begin())->_rf_object_component_first_init();
      (*(--_cn_object_component.elements_end()))->_rf_object_component_last_init();
    }
    if( !_cn_scalar_component.elements_empty() )
    {
      (*_cn_scalar_component.elements_begin())->_rf_scalar_component_first_init();
      (*(--_cn_scalar_component.elements_end()))->_rf_scalar_component_last_init();
    }
  }

  // ** result functions **
  // ** infrastructure functions **
  void node_scene::make_availible()
  {
    _container_scene_type::add_node_factory( "scene", new proptree::Node_Factory<_pt_scene_type,node_scene > );
  }
  // ** constructor **
  node_scene::node_scene( std::string name, proptree::tree_info *info, 
    message::Message_Consultant *msg_consultant )
    : proptree::Prop_Tree_Node( "scene", name, info, msg_consultant ), 
      _pt_scene_type( msg_consultant ),
      _op_filename( "filename", this ),
      _op_scene_type( "scene_type", this ),
      _cn_object_component(false, false, get_consultant() ),
      _cn_scalar_component(false, false, get_consultant() )
  {
    // ********************
    // Register Properties 
    add_property( "filename", &_op_filename );
    add_property( "scene_type", &_op_scene_type );
  }

  // *****************************
  // make nodes availible 
  // *****************************

  void make_scene_base_nodes_availible()  {
    node_root::make_availible();
    node_scene::make_availible();
  }
}

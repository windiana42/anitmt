// ********************************************
// generated file by funcgen (www.anitmt.org)
// requires:
//   - libmessage
//   - libval
//   - libsolve
//   - libproptree
// ********************************************

#include <solve/constraint.hpp>
#include "base_func.hpp"

namespace functionality
{
  // ****************************
  // provider type implementation
  // ****************************

  // ** type specific node connection **
  _pt_object_component *_pt_object_component::get_prev_object_component()
  {
    return _tc_prev_object_component;
  }
  _pt_object_component *_pt_object_component::get_next_object_component()
  {
    return _tc_next_object_component;
  }
  void _pt_object_component::set_prev_object_component( _pt_object_component*prev )
  {
    _tc_prev_object_component = prev;
  }
  void _pt_object_component::set_next_object_component( _pt_object_component*next )
  {
    _tc_next_object_component = next;
  }

  // ** constructor **
  _pt_object_component::_pt_object_component( message::Message_Consultant *c )
    : proptree::Prop_Tree_Node("","",0,c) /* should never be used */,
      _av_object_component_orientation_time_is_avail(c),
      _av_object_component_position_time_is_avail(c)  {
  }

  // ** type specific node connection **
  _pt_scalar_component *_pt_scalar_component::get_prev_scalar_component()
  {
    return _tc_prev_scalar_component;
  }
  _pt_scalar_component *_pt_scalar_component::get_next_scalar_component()
  {
    return _tc_next_scalar_component;
  }
  void _pt_scalar_component::set_prev_scalar_component( _pt_scalar_component*prev )
  {
    _tc_prev_scalar_component = prev;
  }
  void _pt_scalar_component::set_next_scalar_component( _pt_scalar_component*next )
  {
    _tc_next_scalar_component = next;
  }

  // ** constructor **
  _pt_scalar_component::_pt_scalar_component( message::Message_Consultant *c )
    : proptree::Prop_Tree_Node("","",0,c) /* should never be used */,
      _av_scalar_component_value_time_is_avail(c)  {
  }

  // ** type specific node connection **
  _pt_scalar_vsa *_pt_scalar_vsa::get_prev_scalar_vsa()
  {
    return _tc_prev_scalar_vsa;
  }
  _pt_scalar_vsa *_pt_scalar_vsa::get_next_scalar_vsa()
  {
    return _tc_next_scalar_vsa;
  }
  void _pt_scalar_vsa::set_prev_scalar_vsa( _pt_scalar_vsa*prev )
  {
    _tc_prev_scalar_vsa = prev;
  }
  void _pt_scalar_vsa::set_next_scalar_vsa( _pt_scalar_vsa*next )
  {
    _tc_next_scalar_vsa = next;
  }

  // ** constructor **
  _pt_scalar_vsa::_pt_scalar_vsa( message::Message_Consultant *c )
    : proptree::Prop_Tree_Node("","",0,c) /* should never be used */,
      _pr_scalar_vsa_acceleration_time_start_param(c),
      _pr_scalar_vsa_acceleration_time_end_param(c),
      _pr_scalar_vsa_slope_time_start_param(c),
      _pr_scalar_vsa_slope_time_end_param(c),
      _pr_scalar_vsa_value_time_start_param(c),
      _pr_scalar_vsa_value_time_end_param(c),
      _av_scalar_vsa_acceleration_time_is_avail(c),
      _av_scalar_vsa_slope_time_is_avail(c),
      _av_scalar_vsa_value_time_is_avail(c)  {
  }

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
    : proptree::Prop_Tree_Node("","",0,c) /* should never be used */  {
  }

  // ** type specific node connection **
  _pt_space_state *_pt_space_state::get_prev_space_state()
  {
    return _tc_prev_space_state;
  }
  _pt_space_state *_pt_space_state::get_next_space_state()
  {
    return _tc_next_space_state;
  }
  void _pt_space_state::set_prev_space_state( _pt_space_state*prev )
  {
    _tc_prev_space_state = prev;
  }
  void _pt_space_state::set_next_space_state( _pt_space_state*next )
  {
    _tc_next_space_state = next;
  }

  // ** constructor **
  _pt_space_state::_pt_space_state( message::Message_Consultant *c )
    : proptree::Prop_Tree_Node("","",0,c) /* should never be used */,
      _pr_space_state_orientation_stretch_start_param(c),
      _pr_space_state_orientation_stretch_end_param(c),
      _pr_space_state_orientation_time_start_param(c),
      _pr_space_state_orientation_time_end_param(c),
      _pr_space_state_position_stretch_start_param(c),
      _pr_space_state_position_stretch_end_param(c),
      _pr_space_state_position_time_start_param(c),
      _pr_space_state_position_time_end_param(c),
      _av_space_state_orientation_stretch_is_avail(c),
      _av_space_state_orientation_time_is_avail(c),
      _av_space_state_position_stretch_is_avail(c),
      _av_space_state_position_time_is_avail(c)  {
  }

  // ** type specific node connection **
  _pt_track *_pt_track::get_prev_track()
  {
    return _tc_prev_track;
  }
  _pt_track *_pt_track::get_next_track()
  {
    return _tc_next_track;
  }
  void _pt_track::set_prev_track( _pt_track*prev )
  {
    _tc_prev_track = prev;
  }
  void _pt_track::set_next_track( _pt_track*next )
  {
    _tc_next_track = next;
  }

  // ** constructor **
  _pt_track::_pt_track( message::Message_Consultant *c )
    : proptree::Prop_Tree_Node("","",0,c) /* should never be used */,
      _pr_track_position_stretch_start_param(c),
      _pr_track_position_stretch_end_param(c),
      _av_track_position_stretch_is_avail(c)  {
  }

  // *****************
  // container classes

  // ********************************************************************
  // serial container for nodes that provide object_component:
  //   _serial_container_object_component

  _container_object_component::node_factories_type _container_object_component::node_factories;
  void _container_object_component::add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_object_component>* nf )
  {
    node_factories[name] = nf;
  }
  proptree::Prop_Tree_Node *_container_object_component::
  add_child( std::string type, std::string name, proptree::tree_info *info, 
             message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj )
  {
    node_factories_type::iterator i;
    i = node_factories.find(type);
    if( i == node_factories.end() ) return already_obj;
    proptree::Basic_Node_Factory<_pt_object_component>* &nf = i->second;
    proptree::Basic_Node_Factory<_pt_object_component>::node_return_type node;
    if( already_obj != 0 ) 
      node = nf->cast(already_obj);
    else
      node = nf->create(name,info,msg);
    elements.push_back(node.first); // store provided type pointer
    return node.second;                      // return general prop tree node pointer
  }

  _container_object_component::elements_type::iterator _container_object_component::elements_begin()
  {
    return elements.begin();
  }

  _container_object_component::elements_type::iterator _container_object_component::elements_end()
  {
    return elements.end();
  }

  bool _container_object_component::elements_empty()
  {
    return elements.empty();
  }

  _container_object_component::_container_object_component(bool _max1, bool _min1, message::Message_Consultant * )
    : max1(_max1), min1(_min1)
  {
  }

    // ** Don't call the following functions! ** 
  //! function that is called after hierarchy was set up for each node
  void _container_object_component::hierarchy_final_init()

  {
  }
  // ********************************************************************
  // serial container for nodes that provide scalar_component:
  //   _serial_container_scalar_component

  _container_scalar_component::node_factories_type _container_scalar_component::node_factories;
  void _container_scalar_component::add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_scalar_component>* nf )
  {
    node_factories[name] = nf;
  }
  proptree::Prop_Tree_Node *_container_scalar_component::
  add_child( std::string type, std::string name, proptree::tree_info *info, 
             message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj )
  {
    node_factories_type::iterator i;
    i = node_factories.find(type);
    if( i == node_factories.end() ) return already_obj;
    proptree::Basic_Node_Factory<_pt_scalar_component>* &nf = i->second;
    proptree::Basic_Node_Factory<_pt_scalar_component>::node_return_type node;
    if( already_obj != 0 ) 
      node = nf->cast(already_obj);
    else
      node = nf->create(name,info,msg);
    elements.push_back(node.first); // store provided type pointer
    return node.second;                      // return general prop tree node pointer
  }

  _container_scalar_component::elements_type::iterator _container_scalar_component::elements_begin()
  {
    return elements.begin();
  }

  _container_scalar_component::elements_type::iterator _container_scalar_component::elements_end()
  {
    return elements.end();
  }

  bool _container_scalar_component::elements_empty()
  {
    return elements.empty();
  }

  _container_scalar_component::_container_scalar_component(bool _max1, bool _min1, message::Message_Consultant * )
    : max1(_max1), min1(_min1)
  {
  }

    // ** Don't call the following functions! ** 
  //! function that is called after hierarchy was set up for each node
  void _container_scalar_component::hierarchy_final_init()

  {
  }
  // ********************************************************************
  // serial container for nodes that provide scalar_vsa:
  //   _serial_container_scalar_vsa

  _serial_container_scalar_vsa::node_factories_type _serial_container_scalar_vsa::node_factories;
  void _serial_container_scalar_vsa::add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_scalar_vsa>* nf )
  {
    node_factories[name] = nf;
  }

  proptree::Prop_Tree_Node *_serial_container_scalar_vsa::
  add_child( std::string type, std::string name, proptree::tree_info *info, 
             message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj )
  {
    node_factories_type::iterator i;
    i = node_factories.find(type);
    if( i == node_factories.end() ) return already_obj;
    proptree::Basic_Node_Factory<_pt_scalar_vsa>* &nf = i->second;
    proptree::Basic_Node_Factory<_pt_scalar_vsa>::node_return_type node;
    if( already_obj != 0 ) 
      node = nf->cast(already_obj);
    else
      node = nf->create(name,info,msg);
    if( !elements.empty() ) 
    { // link contained elements if not empty
      _pt_scalar_vsa *last = *(--elements.end());
      last->set_next_scalar_vsa( node.first );
      node.first->set_prev_scalar_vsa( last );
    }
    elements.push_back(node.first); 
    avail_operator_acceleration_time->add_operand( node.first->_av_scalar_vsa_acceleration_time_is_avail );
    avail_operator_slope_time->add_operand( node.first->_av_scalar_vsa_slope_time_is_avail );
    avail_operator_value_time->add_operand( node.first->_av_scalar_vsa_value_time_is_avail );
// store provided type pointer
    return node.second;                      // return general prop tree node pointer
  }

  std::pair<bool,acceleration> _serial_container_scalar_vsa::_rf_scalar_vsa_acceleration_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_scalar_vsa_acceleration_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_scalar_vsa_acceleration_time_start_param() )
        {
          return (*i)->_rf_scalar_vsa_acceleration_time( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,acceleration> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,slope> _serial_container_scalar_vsa::_rf_scalar_vsa_slope_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_scalar_vsa_slope_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_scalar_vsa_slope_time_start_param() )
        {
          return (*i)->_rf_scalar_vsa_slope_time( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,slope> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,value> _serial_container_scalar_vsa::_rf_scalar_vsa_value_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_scalar_vsa_value_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_scalar_vsa_value_time_start_param() )
        {
          return (*i)->_rf_scalar_vsa_value_time( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,value> ret; 
    ret.first = false;
    return ret;
  }

  _serial_container_scalar_vsa::elements_type::iterator _serial_container_scalar_vsa::elements_begin()
  {
    return elements.begin();
  }

  _serial_container_scalar_vsa::elements_type::iterator _serial_container_scalar_vsa::elements_end()
  {
    return elements.end();
  }

  bool _serial_container_scalar_vsa::elements_empty()
  {
    return elements.empty();
  }

  _serial_container_scalar_vsa::_serial_container_scalar_vsa(bool _max1, bool _min1, message::Message_Consultant *c)
    : max1(_max1), min1(_min1),
      _av_scalar_vsa_acceleration_time_is_avail(c),
      _av_scalar_vsa_slope_time_is_avail(c),
      _av_scalar_vsa_value_time_is_avail(c)
  {
    avail_operator_acceleration_time = new solve::Multi_And_Operator(c);
    _av_scalar_vsa_acceleration_time_is_avail = avail_operator_acceleration_time->get_result();
    avail_operator_slope_time = new solve::Multi_And_Operator(c);
    _av_scalar_vsa_slope_time_is_avail = avail_operator_slope_time->get_result();
    avail_operator_value_time = new solve::Multi_And_Operator(c);
    _av_scalar_vsa_value_time_is_avail = avail_operator_value_time->get_result();
  }
  //! function that is called after hierarchy was set up for each node
  void _serial_container_scalar_vsa::hierarchy_final_init()

  {
    avail_operator_acceleration_time->finish_adding();
    avail_operator_slope_time->finish_adding();
    avail_operator_value_time->finish_adding();
  }
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
  // ********************************************************************
  // serial container for nodes that provide space_state:
  //   _serial_container_space_state

  _serial_container_space_state::node_factories_type _serial_container_space_state::node_factories;
  void _serial_container_space_state::add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_space_state>* nf )
  {
    node_factories[name] = nf;
  }

  proptree::Prop_Tree_Node *_serial_container_space_state::
  add_child( std::string type, std::string name, proptree::tree_info *info, 
             message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj )
  {
    node_factories_type::iterator i;
    i = node_factories.find(type);
    if( i == node_factories.end() ) return already_obj;
    proptree::Basic_Node_Factory<_pt_space_state>* &nf = i->second;
    proptree::Basic_Node_Factory<_pt_space_state>::node_return_type node;
    if( already_obj != 0 ) 
      node = nf->cast(already_obj);
    else
      node = nf->create(name,info,msg);
    if( !elements.empty() ) 
    { // link contained elements if not empty
      _pt_space_state *last = *(--elements.end());
      last->set_next_space_state( node.first );
      node.first->set_prev_space_state( last );
    }
    elements.push_back(node.first); 
    avail_operator_orientation_stretch->add_operand( node.first->_av_space_state_orientation_stretch_is_avail );
    avail_operator_orientation_time->add_operand( node.first->_av_space_state_orientation_time_is_avail );
    avail_operator_position_stretch->add_operand( node.first->_av_space_state_position_stretch_is_avail );
    avail_operator_position_time->add_operand( node.first->_av_space_state_position_time_is_avail );
// store provided type pointer
    return node.second;                      // return general prop tree node pointer
  }

  std::pair<bool,orientation> _serial_container_space_state::_rf_space_state_orientation_stretch( stretch _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_space_state_orientation_stretch_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_space_state_orientation_stretch_start_param() )
        {
          return (*i)->_rf_space_state_orientation_stretch( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,orientation> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,orientation> _serial_container_space_state::_rf_space_state_orientation_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_space_state_orientation_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_space_state_orientation_time_start_param() )
        {
          return (*i)->_rf_space_state_orientation_time( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,orientation> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,position> _serial_container_space_state::_rf_space_state_position_stretch( stretch _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_space_state_position_stretch_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_space_state_position_stretch_start_param() )
        {
          return (*i)->_rf_space_state_position_stretch( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,position> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,position> _serial_container_space_state::_rf_space_state_position_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_space_state_position_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_space_state_position_time_start_param() )
        {
          return (*i)->_rf_space_state_position_time( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,position> ret; 
    ret.first = false;
    return ret;
  }

  _serial_container_space_state::elements_type::iterator _serial_container_space_state::elements_begin()
  {
    return elements.begin();
  }

  _serial_container_space_state::elements_type::iterator _serial_container_space_state::elements_end()
  {
    return elements.end();
  }

  bool _serial_container_space_state::elements_empty()
  {
    return elements.empty();
  }

  _serial_container_space_state::_serial_container_space_state(bool _max1, bool _min1, message::Message_Consultant *c)
    : max1(_max1), min1(_min1),
      _av_space_state_orientation_stretch_is_avail(c),
      _av_space_state_orientation_time_is_avail(c),
      _av_space_state_position_stretch_is_avail(c),
      _av_space_state_position_time_is_avail(c)
  {
    avail_operator_orientation_stretch = new solve::Multi_And_Operator(c);
    _av_space_state_orientation_stretch_is_avail = avail_operator_orientation_stretch->get_result();
    avail_operator_orientation_time = new solve::Multi_And_Operator(c);
    _av_space_state_orientation_time_is_avail = avail_operator_orientation_time->get_result();
    avail_operator_position_stretch = new solve::Multi_And_Operator(c);
    _av_space_state_position_stretch_is_avail = avail_operator_position_stretch->get_result();
    avail_operator_position_time = new solve::Multi_And_Operator(c);
    _av_space_state_position_time_is_avail = avail_operator_position_time->get_result();
  }
  //! function that is called after hierarchy was set up for each node
  void _serial_container_space_state::hierarchy_final_init()

  {
    avail_operator_orientation_stretch->finish_adding();
    avail_operator_orientation_time->finish_adding();
    avail_operator_position_stretch->finish_adding();
    avail_operator_position_time->finish_adding();
  }
  // ********************************************************************
  // serial container for nodes that provide track:
  //   _serial_container_track

  _serial_container_track::node_factories_type _serial_container_track::node_factories;
  void _serial_container_track::add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_track>* nf )
  {
    node_factories[name] = nf;
  }

  proptree::Prop_Tree_Node *_serial_container_track::
  add_child( std::string type, std::string name, proptree::tree_info *info, 
             message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj )
  {
    node_factories_type::iterator i;
    i = node_factories.find(type);
    if( i == node_factories.end() ) return already_obj;
    proptree::Basic_Node_Factory<_pt_track>* &nf = i->second;
    proptree::Basic_Node_Factory<_pt_track>::node_return_type node;
    if( already_obj != 0 ) 
      node = nf->cast(already_obj);
    else
      node = nf->create(name,info,msg);
    if( !elements.empty() ) 
    { // link contained elements if not empty
      _pt_track *last = *(--elements.end());
      last->set_next_track( node.first );
      node.first->set_prev_track( last );
    }
    elements.push_back(node.first); 
    avail_operator_position_stretch->add_operand( node.first->_av_track_position_stretch_is_avail );
// store provided type pointer
    return node.second;                      // return general prop tree node pointer
  }

  std::pair<bool,position> _serial_container_track::_rf_track_position_stretch( stretch _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_track_position_stretch_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_track_position_stretch_start_param() )
        {
          return (*i)->_rf_track_position_stretch( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,position> ret; 
    ret.first = false;
    return ret;
  }

  _serial_container_track::elements_type::iterator _serial_container_track::elements_begin()
  {
    return elements.begin();
  }

  _serial_container_track::elements_type::iterator _serial_container_track::elements_end()
  {
    return elements.end();
  }

  bool _serial_container_track::elements_empty()
  {
    return elements.empty();
  }

  _serial_container_track::_serial_container_track(bool _max1, bool _min1, message::Message_Consultant *c)
    : max1(_max1), min1(_min1),
      _av_track_position_stretch_is_avail(c)
  {
    avail_operator_position_stretch = new solve::Multi_And_Operator(c);
    _av_track_position_stretch_is_avail = avail_operator_position_stretch->get_result();
  }
  //! function that is called after hierarchy was set up for each node
  void _serial_container_track::hierarchy_final_init()

  {
    avail_operator_position_stretch->finish_adding();
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
  // scalar: node type
  // *****************************

  // ** virtual tree node functions **
  proptree::Prop_Tree_Node *node_scalar::try_add_child( std::string type, std::string name ) throw()
  {
    proptree::Prop_Tree_Node *node = 0;
    node = _cn_scalar_vsa.add_child( type, name, info, get_consultant(), node );
    return node;
  }
  void node_scalar::custom_hierarchy_final_init()
  {
    _cn_scalar_vsa.hierarchy_final_init();
  }

  void node_scalar::common_init()
  {
    solve::constraint( _op_duration>=0 );
    sum_solver(_op_end_time,_op_duration,_op_start_time);
    sum_solver(_op_end_value,_op_difference,_op_start_value);
    sum_solver(_op_end_slope,_op_slope_difference,_op_start_slope);
    solve::establish_Default_Value( info->priority_system,999, _op_slope_difference, 0);
    if( get_next() && get_next()->get_property( "start_time" ) )
    {
      solve::establish_Push_Connection( info->priority_system,10, _op_end_time, get_next()->get_property( "start_time" ));
    }
    if( get_prev() && get_prev()->get_property( "end_time" ) )
    {
      solve::establish_Push_Connection( info->priority_system,11, _op_start_time, get_prev()->get_property( "end_time" ));
    }
    if( get_next() && get_next()->get_property( "start_value" ) )
    {
      solve::establish_Push_Connection( info->priority_system,20, _op_end_value, get_next()->get_property( "start_value" ));
    }
    if( get_prev() && get_prev()->get_property( "end_value" ) )
    {
      solve::establish_Push_Connection( info->priority_system,21, _op_start_value, get_prev()->get_property( "end_value" ));
    }
    if( get_next() && get_next()->get_property( "start_slope" ) )
    {
      solve::establish_Push_Connection( info->priority_system,30, _op_end_slope, get_next()->get_property( "start_slope" ));
    }
    if( get_prev() && get_prev()->get_property( "end_slope" ) )
    {
      solve::establish_Push_Connection( info->priority_system,31, _op_start_slope, get_prev()->get_property( "end_slope" ));
    }
    // ** invoke first_/last_init() for each child container **
    if( !_cn_scalar_vsa.elements_empty() )
    {
      (*_cn_scalar_vsa.elements_begin())->_rf_scalar_vsa_first_init();
      (*(--_cn_scalar_vsa.elements_end()))->_rf_scalar_vsa_last_init();
    }
    // ** establish availability checks **
    solve::Multi_And_Operator *and;
    and = new solve::Multi_And_Operator(get_consultant());
    _av_scalar_component_value_time_is_avail = and->get_result();
    and->finish_adding();
    and->add_operand( _cn_scalar_vsa._av_scalar_vsa_value_time_is_avail );
    and->finish_adding();
    and = new solve::Multi_And_Operator(get_consultant());
    _av_scalar_vsa_acceleration_time_is_avail = and->get_result();
    and->finish_adding();
    and->add_operand( _cn_scalar_vsa._av_scalar_vsa_acceleration_time_is_avail );
    and->finish_adding();
    and = new solve::Multi_And_Operator(get_consultant());
    _av_scalar_vsa_slope_time_is_avail = and->get_result();
    and->finish_adding();
    and->add_operand( _cn_scalar_vsa._av_scalar_vsa_slope_time_is_avail );
    and->finish_adding();
    and = new solve::Multi_And_Operator(get_consultant());
    _av_scalar_vsa_value_time_is_avail = and->get_result();
    and->finish_adding();
    and->add_operand( _cn_scalar_vsa._av_scalar_vsa_value_time_is_avail );
    and->finish_adding();
  }

  void node_scalar::_rf_scalar_vsa_first_init()
  {
    solve::establish_Default_Value( info->priority_system,100, _op_start_time, 0);
    solve::establish_Default_Value( info->priority_system,1000, _op_start_value, 0);
  }

  // ** result functions **
  std::pair<bool,value> node_scalar::_rf_scalar_component_value_time( time t )
  {
    std::pair<bool,value> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_scalar_vsa._av_scalar_vsa_value_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:178 ***
       
      return(_cn_scalar_vsa._rf_scalar_vsa_value_time( t ));
    
  }

  std::pair<bool,acceleration> node_scalar::_rf_scalar_vsa_acceleration_time( time t )
  {
    std::pair<bool,acceleration> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_scalar_vsa._av_scalar_vsa_acceleration_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:171 ***
                                                                                     
      return(_cn_scalar_vsa._rf_scalar_vsa_acceleration_time( t ));
    
  }

  std::pair<bool,slope> node_scalar::_rf_scalar_vsa_slope_time( time t )
  {
    std::pair<bool,slope> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_scalar_vsa._av_scalar_vsa_slope_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:168 ***
                                                                       
      return(_cn_scalar_vsa._rf_scalar_vsa_slope_time( t ));
    
  }

  std::pair<bool,value> node_scalar::_rf_scalar_vsa_value_time( time t )
  {
    std::pair<bool,value> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_scalar_vsa._av_scalar_vsa_value_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:165 ***
       
      return(_cn_scalar_vsa._rf_scalar_vsa_value_time( t ));
    
  }

  // ** infrastructure functions **
  void node_scalar::make_availible()
  {
    _container_scalar_component::add_node_factory( "scalar", new proptree::Node_Factory<_pt_scalar_component,node_scalar > );
    _serial_container_scalar_vsa::add_node_factory( "scalar", new proptree::Node_Factory<_pt_scalar_vsa,node_scalar > );
  }
  // ** constructor **
  node_scalar::node_scalar( std::string name, proptree::tree_info *info, 
    message::Message_Consultant *msg_consultant )
    : proptree::Prop_Tree_Node( "scalar", name, info, msg_consultant ), 
      _pt_scalar_component( msg_consultant ), 
      _pt_scalar_vsa( msg_consultant ),
      _op_difference( "_op_difference", this ),
      _op_duration( "_op_duration", this ),
      _op_end_slope( "_op_end_slope", this ),
      _op_end_time( "_op_end_time", this ),
      _op_end_value( "_op_end_value", this ),
      _op_slope_difference( "_op_slope_difference", this ),
      _op_start_slope( "_op_start_slope", this ),
      _op_start_time( "_op_start_time", this ),
      _op_start_value( "_op_start_value", this ),
      _cn_scalar_vsa(false, true, get_consultant() )
  {
    // ********************
    // Register Properties 
    add_property( "difference", &_op_difference );
    add_property( "duration", &_op_duration );
    add_property( "end_slope", &_op_end_slope );
    add_property( "end_time", &_op_end_time );
    add_property( "end_value", &_op_end_value );
    add_property( "slope_difference", &_op_slope_difference );
    add_property( "start_slope", &_op_start_slope );
    add_property( "start_time", &_op_start_time );
    add_property( "start_value", &_op_start_value );
  }

  // *****************************
  // scalar_base: node type
  // *****************************

  // ** virtual tree node functions **
  proptree::Prop_Tree_Node *node_scalar_base::try_add_child( std::string type, std::string name ) throw()
  {
    proptree::Prop_Tree_Node *node = 0;
    return node;
  }
  void node_scalar_base::custom_hierarchy_final_init()
  {
  }

  void node_scalar_base::common_init()
  {
    solve::constraint( _op_duration>=0 );
    sum_solver(_op_end_time,_op_duration,_op_start_time);
    sum_solver(_op_end_value,_op_difference,_op_start_value);
    sum_solver(_op_end_slope,_op_slope_difference,_op_start_slope);
    solve::establish_Default_Value( info->priority_system,999, _op_slope_difference, 0);
    if( get_next() && get_next()->get_property( "start_time" ) )
    {
      solve::establish_Push_Connection( info->priority_system,10, _op_end_time, get_next()->get_property( "start_time" ));
    }
    if( get_prev() && get_prev()->get_property( "end_time" ) )
    {
      solve::establish_Push_Connection( info->priority_system,11, _op_start_time, get_prev()->get_property( "end_time" ));
    }
    if( get_next() && get_next()->get_property( "start_value" ) )
    {
      solve::establish_Push_Connection( info->priority_system,20, _op_end_value, get_next()->get_property( "start_value" ));
    }
    if( get_prev() && get_prev()->get_property( "end_value" ) )
    {
      solve::establish_Push_Connection( info->priority_system,21, _op_start_value, get_prev()->get_property( "end_value" ));
    }
    if( get_next() && get_next()->get_property( "start_slope" ) )
    {
      solve::establish_Push_Connection( info->priority_system,30, _op_end_slope, get_next()->get_property( "start_slope" ));
    }
    if( get_prev() && get_prev()->get_property( "end_slope" ) )
    {
      solve::establish_Push_Connection( info->priority_system,31, _op_start_slope, get_prev()->get_property( "end_slope" ));
    }
  }

  // ** result functions **
  // ** infrastructure functions **
  void node_scalar_base::make_availible()
  {
  }
  // ** constructor **
  node_scalar_base::node_scalar_base( std::string name, proptree::tree_info *info, 
    message::Message_Consultant *msg_consultant )
    : proptree::Prop_Tree_Node( "scalar_base", name, info, msg_consultant ),
      _op_difference( "_op_difference", this ),
      _op_duration( "_op_duration", this ),
      _op_end_slope( "_op_end_slope", this ),
      _op_end_time( "_op_end_time", this ),
      _op_end_value( "_op_end_value", this ),
      _op_slope_difference( "_op_slope_difference", this ),
      _op_start_slope( "_op_start_slope", this ),
      _op_start_time( "_op_start_time", this ),
      _op_start_value( "_op_start_value", this )
  {
    // ********************
    // Register Properties 
    add_property( "difference", &_op_difference );
    add_property( "duration", &_op_duration );
    add_property( "end_slope", &_op_end_slope );
    add_property( "end_time", &_op_end_time );
    add_property( "end_value", &_op_end_value );
    add_property( "slope_difference", &_op_slope_difference );
    add_property( "start_slope", &_op_start_slope );
    add_property( "start_time", &_op_start_time );
    add_property( "start_value", &_op_start_value );
  }

  // *****************************
  // scalar_subfunction: node type
  // *****************************

  // ** virtual tree node functions **
  proptree::Prop_Tree_Node *node_scalar_subfunction::try_add_child( std::string type, std::string name ) throw()
  {
    proptree::Prop_Tree_Node *node = 0;
    return node;
  }
  void node_scalar_subfunction::custom_hierarchy_final_init()
  {
  }

  void node_scalar_subfunction::common_init()
  {
    solve::constraint( _op_duration>=0 );
    sum_solver(_op_end_time,_op_duration,_op_start_time);
    sum_solver(_op_end_value,_op_difference,_op_start_value);
    sum_solver(_op_end_slope,_op_slope_difference,_op_start_slope);
    solve::establish_Default_Value( info->priority_system,999, _op_slope_difference, 0);
    if( get_next() && get_next()->get_property( "start_time" ) )
    {
      solve::establish_Push_Connection( info->priority_system,10, _op_end_time, get_next()->get_property( "start_time" ));
    }
    if( get_prev() && get_prev()->get_property( "end_time" ) )
    {
      solve::establish_Push_Connection( info->priority_system,11, _op_start_time, get_prev()->get_property( "end_time" ));
    }
    if( get_next() && get_next()->get_property( "start_value" ) )
    {
      solve::establish_Push_Connection( info->priority_system,20, _op_end_value, get_next()->get_property( "start_value" ));
    }
    if( get_prev() && get_prev()->get_property( "end_value" ) )
    {
      solve::establish_Push_Connection( info->priority_system,21, _op_start_value, get_prev()->get_property( "end_value" ));
    }
    if( get_next() && get_next()->get_property( "start_slope" ) )
    {
      solve::establish_Push_Connection( info->priority_system,30, _op_end_slope, get_next()->get_property( "start_slope" ));
    }
    if( get_prev() && get_prev()->get_property( "end_slope" ) )
    {
      solve::establish_Push_Connection( info->priority_system,31, _op_start_slope, get_prev()->get_property( "end_slope" ));
    }
  }

  // ** result functions **
  // ** infrastructure functions **
  void node_scalar_subfunction::make_availible()
  {
  }
  // ** constructor **
  node_scalar_subfunction::node_scalar_subfunction( std::string name, proptree::tree_info *info, 
    message::Message_Consultant *msg_consultant )
    : proptree::Prop_Tree_Node( "scalar_subfunction", name, info, msg_consultant ),
      _op_difference( "_op_difference", this ),
      _op_duration( "_op_duration", this ),
      _op_end_slope( "_op_end_slope", this ),
      _op_end_time( "_op_end_time", this ),
      _op_end_value( "_op_end_value", this ),
      _op_slope_difference( "_op_slope_difference", this ),
      _op_start_slope( "_op_start_slope", this ),
      _op_start_time( "_op_start_time", this ),
      _op_start_value( "_op_start_value", this )
  {
    // ********************
    // Register Properties 
    add_property( "difference", &_op_difference );
    add_property( "duration", &_op_duration );
    add_property( "end_slope", &_op_end_slope );
    add_property( "end_time", &_op_end_time );
    add_property( "end_value", &_op_end_value );
    add_property( "slope_difference", &_op_slope_difference );
    add_property( "start_slope", &_op_start_slope );
    add_property( "start_time", &_op_start_time );
    add_property( "start_value", &_op_start_value );
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
      _op_filename( "_op_filename", this ),
      _cn_object_component(false, false, get_consultant() ),
      _cn_scalar_component(false, false, get_consultant() )
  {
    // ********************
    // Register Properties 
    add_property( "filename", &_op_filename );
  }

  // *****************************
  // make nodes availible 
  // *****************************

  void make_base_func_nodes_availible()  {
    node_root::make_availible();
    node_scalar::make_availible();
    node_scalar_base::make_availible();
    node_scalar_subfunction::make_availible();
    node_scene::make_availible();
  }
}

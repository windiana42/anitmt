// ********************************************
// generated file by funcgen (www.anitmt.org)
// requires:
//   - libmessage
//   - libval
//   - libsolve
//   - libproptree
// ********************************************

#include <solve/constraint.hpp>
#include "object_base.hpp"

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
    : proptree::Prop_Tree_Node("","",0,c), /* should never be used */
      _tc_next_object_component(0),
      _tc_prev_object_component(0),
      _av_object_component_front_time_is_avail(c),
      _av_object_component_position_time_is_avail(c),
      _av_object_component_rotation_time_is_avail(c),
      _av_object_component_speed_time_is_avail(c),
      _av_object_component_translation_time_is_avail(c),
      _av_object_component_up_vector_time_is_avail(c)
  {
  }

  // ** type specific node connection **
  _pt_object_state *_pt_object_state::get_prev_object_state()
  {
    return _tc_prev_object_state;
  }
  _pt_object_state *_pt_object_state::get_next_object_state()
  {
    return _tc_next_object_state;
  }
  void _pt_object_state::set_prev_object_state( _pt_object_state*prev )
  {
    _tc_prev_object_state = prev;
  }
  void _pt_object_state::set_next_object_state( _pt_object_state*next )
  {
    _tc_next_object_state = next;
  }

  // ** constructor **
  _pt_object_state::_pt_object_state( message::Message_Consultant *c )
    : proptree::Prop_Tree_Node("","",0,c), /* should never be used */
      _tc_next_object_state(0),
      _tc_prev_object_state(0),
      _pr_object_state_acceleration_stretch_start_param(c),
      _pr_object_state_acceleration_stretch_end_param(c),
      _pr_object_state_acceleration_time_start_param(c),
      _pr_object_state_acceleration_time_end_param(c),
      _pr_object_state_direction_stretch_start_param(c),
      _pr_object_state_direction_stretch_end_param(c),
      _pr_object_state_direction_time_start_param(c),
      _pr_object_state_direction_time_end_param(c),
      _pr_object_state_front_stretch_start_param(c),
      _pr_object_state_front_stretch_end_param(c),
      _pr_object_state_front_time_start_param(c),
      _pr_object_state_front_time_end_param(c),
      _pr_object_state_position_stretch_start_param(c),
      _pr_object_state_position_stretch_end_param(c),
      _pr_object_state_position_time_start_param(c),
      _pr_object_state_position_time_end_param(c),
      _pr_object_state_speed_stretch_start_param(c),
      _pr_object_state_speed_stretch_end_param(c),
      _pr_object_state_speed_time_start_param(c),
      _pr_object_state_speed_time_end_param(c),
      _pr_object_state_stretch_time_start_param(c),
      _pr_object_state_stretch_time_end_param(c),
      _pr_object_state_up_vector_stretch_start_param(c),
      _pr_object_state_up_vector_stretch_end_param(c),
      _pr_object_state_up_vector_time_start_param(c),
      _pr_object_state_up_vector_time_end_param(c),
      _av_object_state_acceleration_stretch_is_avail(c),
      _av_object_state_acceleration_time_is_avail(c),
      _av_object_state_direction_stretch_is_avail(c),
      _av_object_state_direction_time_is_avail(c),
      _av_object_state_front_stretch_is_avail(c),
      _av_object_state_front_time_is_avail(c),
      _av_object_state_position_stretch_is_avail(c),
      _av_object_state_position_time_is_avail(c),
      _av_object_state_speed_stretch_is_avail(c),
      _av_object_state_speed_time_is_avail(c),
      _av_object_state_stretch_time_is_avail(c),
      _av_object_state_up_vector_stretch_is_avail(c),
      _av_object_state_up_vector_time_is_avail(c)
  {
  }

  // ** type specific node connection **
  _pt_timing *_pt_timing::get_prev_timing()
  {
    return _tc_prev_timing;
  }
  _pt_timing *_pt_timing::get_next_timing()
  {
    return _tc_next_timing;
  }
  void _pt_timing::set_prev_timing( _pt_timing*prev )
  {
    _tc_prev_timing = prev;
  }
  void _pt_timing::set_next_timing( _pt_timing*next )
  {
    _tc_next_timing = next;
  }

  // ** constructor **
  _pt_timing::_pt_timing( message::Message_Consultant *c )
    : proptree::Prop_Tree_Node("","",0,c), /* should never be used */
      _tc_next_timing(0),
      _tc_prev_timing(0),
      _pr_timing_acceleration_stretch_start_param(c),
      _pr_timing_acceleration_stretch_end_param(c),
      _pr_timing_acceleration_time_start_param(c),
      _pr_timing_acceleration_time_end_param(c),
      _pr_timing_speed_stretch_start_param(c),
      _pr_timing_speed_stretch_end_param(c),
      _pr_timing_speed_time_start_param(c),
      _pr_timing_speed_time_end_param(c),
      _pr_timing_stretch_time_start_param(c),
      _pr_timing_stretch_time_end_param(c),
      _av_timing_acceleration_stretch_is_avail(c),
      _av_timing_acceleration_time_is_avail(c),
      _av_timing_speed_stretch_is_avail(c),
      _av_timing_speed_time_is_avail(c),
      _av_timing_stretch_time_is_avail(c)
  {
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
    : proptree::Prop_Tree_Node("","",0,c), /* should never be used */
      _tc_next_track(0),
      _tc_prev_track(0),
      _pr_track_direction_stretch_start_param(c),
      _pr_track_direction_stretch_end_param(c),
      _pr_track_position_stretch_start_param(c),
      _pr_track_position_stretch_end_param(c),
      _av_track_direction_stretch_is_avail(c),
      _av_track_position_stretch_is_avail(c)
  {
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
  // serial container for nodes that provide object_state:
  //   _serial_container_object_state

  _serial_container_object_state::node_factories_type _serial_container_object_state::node_factories;
  void _serial_container_object_state::add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_object_state>* nf )
  {
    node_factories[name] = nf;
  }

  proptree::Prop_Tree_Node *_serial_container_object_state::
  add_child( std::string type, std::string name, proptree::tree_info *info, 
             message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj )
  {
    node_factories_type::iterator i;
    i = node_factories.find(type);
    if( i == node_factories.end() ) return already_obj;
    proptree::Basic_Node_Factory<_pt_object_state>* &nf = i->second;
    proptree::Basic_Node_Factory<_pt_object_state>::node_return_type node;
    if( already_obj != 0 ) 
      node = nf->cast(already_obj);
    else
      node = nf->create(name,info,msg);
    if( !elements.empty() ) 
    { // link contained elements if not empty
      _pt_object_state *last = *(--elements.end());
      last->set_next_object_state( node.first );
      node.first->set_prev_object_state( last );
    }
    elements.push_back(node.first); 
    avail_operator_acceleration_stretch->add_operand( node.first->_av_object_state_acceleration_stretch_is_avail );
    avail_operator_acceleration_time->add_operand( node.first->_av_object_state_acceleration_time_is_avail );
    avail_operator_direction_stretch->add_operand( node.first->_av_object_state_direction_stretch_is_avail );
    avail_operator_direction_time->add_operand( node.first->_av_object_state_direction_time_is_avail );
    avail_operator_front_stretch->add_operand( node.first->_av_object_state_front_stretch_is_avail );
    avail_operator_front_time->add_operand( node.first->_av_object_state_front_time_is_avail );
    avail_operator_position_stretch->add_operand( node.first->_av_object_state_position_stretch_is_avail );
    avail_operator_position_time->add_operand( node.first->_av_object_state_position_time_is_avail );
    avail_operator_speed_stretch->add_operand( node.first->_av_object_state_speed_stretch_is_avail );
    avail_operator_speed_time->add_operand( node.first->_av_object_state_speed_time_is_avail );
    avail_operator_stretch_time->add_operand( node.first->_av_object_state_stretch_time_is_avail );
    avail_operator_up_vector_stretch->add_operand( node.first->_av_object_state_up_vector_stretch_is_avail );
    avail_operator_up_vector_time->add_operand( node.first->_av_object_state_up_vector_time_is_avail );
// store provided type pointer
    return node.second;                      // return general prop tree node pointer
  }

  std::pair<bool,acceleration> _serial_container_object_state::_rf_object_state_acceleration_stretch( stretch _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_object_state_acceleration_stretch_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_object_state_acceleration_stretch_start_param() )
        {
          return (*i)->_rf_object_state_acceleration_stretch( _par_ ); 
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

  std::pair<bool,acceleration> _serial_container_object_state::_rf_object_state_acceleration_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_object_state_acceleration_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_object_state_acceleration_time_start_param() )
        {
          return (*i)->_rf_object_state_acceleration_time( _par_ ); 
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

  std::pair<bool,direction> _serial_container_object_state::_rf_object_state_direction_stretch( stretch _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_object_state_direction_stretch_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_object_state_direction_stretch_start_param() )
        {
          return (*i)->_rf_object_state_direction_stretch( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,direction> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,direction> _serial_container_object_state::_rf_object_state_direction_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_object_state_direction_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_object_state_direction_time_start_param() )
        {
          return (*i)->_rf_object_state_direction_time( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,direction> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,front> _serial_container_object_state::_rf_object_state_front_stretch( stretch _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_object_state_front_stretch_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_object_state_front_stretch_start_param() )
        {
          return (*i)->_rf_object_state_front_stretch( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,front> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,front> _serial_container_object_state::_rf_object_state_front_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_object_state_front_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_object_state_front_time_start_param() )
        {
          return (*i)->_rf_object_state_front_time( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,front> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,position> _serial_container_object_state::_rf_object_state_position_stretch( stretch _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_object_state_position_stretch_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_object_state_position_stretch_start_param() )
        {
          return (*i)->_rf_object_state_position_stretch( _par_ ); 
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

  std::pair<bool,position> _serial_container_object_state::_rf_object_state_position_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_object_state_position_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_object_state_position_time_start_param() )
        {
          return (*i)->_rf_object_state_position_time( _par_ ); 
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

  std::pair<bool,speed> _serial_container_object_state::_rf_object_state_speed_stretch( stretch _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_object_state_speed_stretch_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_object_state_speed_stretch_start_param() )
        {
          return (*i)->_rf_object_state_speed_stretch( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,speed> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,speed> _serial_container_object_state::_rf_object_state_speed_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_object_state_speed_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_object_state_speed_time_start_param() )
        {
          return (*i)->_rf_object_state_speed_time( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,speed> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,stretch> _serial_container_object_state::_rf_object_state_stretch_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_object_state_stretch_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_object_state_stretch_time_start_param() )
        {
          return (*i)->_rf_object_state_stretch_time( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,stretch> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,up_vector> _serial_container_object_state::_rf_object_state_up_vector_stretch( stretch _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_object_state_up_vector_stretch_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_object_state_up_vector_stretch_start_param() )
        {
          return (*i)->_rf_object_state_up_vector_stretch( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,up_vector> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,up_vector> _serial_container_object_state::_rf_object_state_up_vector_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_object_state_up_vector_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_object_state_up_vector_time_start_param() )
        {
          return (*i)->_rf_object_state_up_vector_time( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,up_vector> ret; 
    ret.first = false;
    return ret;
  }

  _serial_container_object_state::elements_type::iterator _serial_container_object_state::elements_begin()
  {
    return elements.begin();
  }

  _serial_container_object_state::elements_type::iterator _serial_container_object_state::elements_end()
  {
    return elements.end();
  }

  bool _serial_container_object_state::elements_empty()
  {
    return elements.empty();
  }

  _serial_container_object_state::_serial_container_object_state(bool _max1, bool _min1, message::Message_Consultant *c)
    : max1(_max1), min1(_min1),
      _av_object_state_acceleration_stretch_is_avail(c),
      _av_object_state_acceleration_time_is_avail(c),
      _av_object_state_direction_stretch_is_avail(c),
      _av_object_state_direction_time_is_avail(c),
      _av_object_state_front_stretch_is_avail(c),
      _av_object_state_front_time_is_avail(c),
      _av_object_state_position_stretch_is_avail(c),
      _av_object_state_position_time_is_avail(c),
      _av_object_state_speed_stretch_is_avail(c),
      _av_object_state_speed_time_is_avail(c),
      _av_object_state_stretch_time_is_avail(c),
      _av_object_state_up_vector_stretch_is_avail(c),
      _av_object_state_up_vector_time_is_avail(c)
  {
    avail_operator_acceleration_stretch = new solve::Multi_And_Operator(c);
    _av_object_state_acceleration_stretch_is_avail = avail_operator_acceleration_stretch->get_result();
    avail_operator_acceleration_time = new solve::Multi_And_Operator(c);
    _av_object_state_acceleration_time_is_avail = avail_operator_acceleration_time->get_result();
    avail_operator_direction_stretch = new solve::Multi_And_Operator(c);
    _av_object_state_direction_stretch_is_avail = avail_operator_direction_stretch->get_result();
    avail_operator_direction_time = new solve::Multi_And_Operator(c);
    _av_object_state_direction_time_is_avail = avail_operator_direction_time->get_result();
    avail_operator_front_stretch = new solve::Multi_And_Operator(c);
    _av_object_state_front_stretch_is_avail = avail_operator_front_stretch->get_result();
    avail_operator_front_time = new solve::Multi_And_Operator(c);
    _av_object_state_front_time_is_avail = avail_operator_front_time->get_result();
    avail_operator_position_stretch = new solve::Multi_And_Operator(c);
    _av_object_state_position_stretch_is_avail = avail_operator_position_stretch->get_result();
    avail_operator_position_time = new solve::Multi_And_Operator(c);
    _av_object_state_position_time_is_avail = avail_operator_position_time->get_result();
    avail_operator_speed_stretch = new solve::Multi_And_Operator(c);
    _av_object_state_speed_stretch_is_avail = avail_operator_speed_stretch->get_result();
    avail_operator_speed_time = new solve::Multi_And_Operator(c);
    _av_object_state_speed_time_is_avail = avail_operator_speed_time->get_result();
    avail_operator_stretch_time = new solve::Multi_And_Operator(c);
    _av_object_state_stretch_time_is_avail = avail_operator_stretch_time->get_result();
    avail_operator_up_vector_stretch = new solve::Multi_And_Operator(c);
    _av_object_state_up_vector_stretch_is_avail = avail_operator_up_vector_stretch->get_result();
    avail_operator_up_vector_time = new solve::Multi_And_Operator(c);
    _av_object_state_up_vector_time_is_avail = avail_operator_up_vector_time->get_result();
  }
  //! function that is called after hierarchy was set up for each node
  void _serial_container_object_state::hierarchy_final_init()

  {
    avail_operator_acceleration_stretch->finish_adding();
    avail_operator_acceleration_time->finish_adding();
    avail_operator_direction_stretch->finish_adding();
    avail_operator_direction_time->finish_adding();
    avail_operator_front_stretch->finish_adding();
    avail_operator_front_time->finish_adding();
    avail_operator_position_stretch->finish_adding();
    avail_operator_position_time->finish_adding();
    avail_operator_speed_stretch->finish_adding();
    avail_operator_speed_time->finish_adding();
    avail_operator_stretch_time->finish_adding();
    avail_operator_up_vector_stretch->finish_adding();
    avail_operator_up_vector_time->finish_adding();
  }
  // ********************************************************************
  // serial container for nodes that provide timing:
  //   _serial_container_timing

  _serial_container_timing::node_factories_type _serial_container_timing::node_factories;
  void _serial_container_timing::add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_timing>* nf )
  {
    node_factories[name] = nf;
  }

  proptree::Prop_Tree_Node *_serial_container_timing::
  add_child( std::string type, std::string name, proptree::tree_info *info, 
             message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj )
  {
    node_factories_type::iterator i;
    i = node_factories.find(type);
    if( i == node_factories.end() ) return already_obj;
    proptree::Basic_Node_Factory<_pt_timing>* &nf = i->second;
    proptree::Basic_Node_Factory<_pt_timing>::node_return_type node;
    if( already_obj != 0 ) 
      node = nf->cast(already_obj);
    else
      node = nf->create(name,info,msg);
    if( !elements.empty() ) 
    { // link contained elements if not empty
      _pt_timing *last = *(--elements.end());
      last->set_next_timing( node.first );
      node.first->set_prev_timing( last );
    }
    elements.push_back(node.first); 
    avail_operator_acceleration_stretch->add_operand( node.first->_av_timing_acceleration_stretch_is_avail );
    avail_operator_acceleration_time->add_operand( node.first->_av_timing_acceleration_time_is_avail );
    avail_operator_speed_stretch->add_operand( node.first->_av_timing_speed_stretch_is_avail );
    avail_operator_speed_time->add_operand( node.first->_av_timing_speed_time_is_avail );
    avail_operator_stretch_time->add_operand( node.first->_av_timing_stretch_time_is_avail );
// store provided type pointer
    return node.second;                      // return general prop tree node pointer
  }

  std::pair<bool,acceleration> _serial_container_timing::_rf_timing_acceleration_stretch( stretch _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_timing_acceleration_stretch_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_timing_acceleration_stretch_start_param() )
        {
          return (*i)->_rf_timing_acceleration_stretch( _par_ ); 
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

  std::pair<bool,acceleration> _serial_container_timing::_rf_timing_acceleration_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_timing_acceleration_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_timing_acceleration_time_start_param() )
        {
          return (*i)->_rf_timing_acceleration_time( _par_ ); 
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

  std::pair<bool,speed> _serial_container_timing::_rf_timing_speed_stretch( stretch _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_timing_speed_stretch_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_timing_speed_stretch_start_param() )
        {
          return (*i)->_rf_timing_speed_stretch( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,speed> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,speed> _serial_container_timing::_rf_timing_speed_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_timing_speed_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_timing_speed_time_start_param() )
        {
          return (*i)->_rf_timing_speed_time( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,speed> ret; 
    ret.first = false;
    return ret;
  }

  std::pair<bool,stretch> _serial_container_timing::_rf_timing_stretch_time( time _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_timing_stretch_time_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_timing_stretch_time_start_param() )
        {
          return (*i)->_rf_timing_stretch_time( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,stretch> ret; 
    ret.first = false;
    return ret;
  }

  _serial_container_timing::elements_type::iterator _serial_container_timing::elements_begin()
  {
    return elements.begin();
  }

  _serial_container_timing::elements_type::iterator _serial_container_timing::elements_end()
  {
    return elements.end();
  }

  bool _serial_container_timing::elements_empty()
  {
    return elements.empty();
  }

  _serial_container_timing::_serial_container_timing(bool _max1, bool _min1, message::Message_Consultant *c)
    : max1(_max1), min1(_min1),
      _av_timing_acceleration_stretch_is_avail(c),
      _av_timing_acceleration_time_is_avail(c),
      _av_timing_speed_stretch_is_avail(c),
      _av_timing_speed_time_is_avail(c),
      _av_timing_stretch_time_is_avail(c)
  {
    avail_operator_acceleration_stretch = new solve::Multi_And_Operator(c);
    _av_timing_acceleration_stretch_is_avail = avail_operator_acceleration_stretch->get_result();
    avail_operator_acceleration_time = new solve::Multi_And_Operator(c);
    _av_timing_acceleration_time_is_avail = avail_operator_acceleration_time->get_result();
    avail_operator_speed_stretch = new solve::Multi_And_Operator(c);
    _av_timing_speed_stretch_is_avail = avail_operator_speed_stretch->get_result();
    avail_operator_speed_time = new solve::Multi_And_Operator(c);
    _av_timing_speed_time_is_avail = avail_operator_speed_time->get_result();
    avail_operator_stretch_time = new solve::Multi_And_Operator(c);
    _av_timing_stretch_time_is_avail = avail_operator_stretch_time->get_result();
  }
  //! function that is called after hierarchy was set up for each node
  void _serial_container_timing::hierarchy_final_init()

  {
    avail_operator_acceleration_stretch->finish_adding();
    avail_operator_acceleration_time->finish_adding();
    avail_operator_speed_stretch->finish_adding();
    avail_operator_speed_time->finish_adding();
    avail_operator_stretch_time->finish_adding();
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
    avail_operator_direction_stretch->add_operand( node.first->_av_track_direction_stretch_is_avail );
    avail_operator_position_stretch->add_operand( node.first->_av_track_position_stretch_is_avail );
// store provided type pointer
    return node.second;                      // return general prop tree node pointer
  }

  std::pair<bool,direction> _serial_container_track::_rf_track_direction_stretch( stretch _par_ )
  {
    elements_type::iterator i;
    for( i = elements.begin(); i != elements.end(); ++i )
    {
      if( _par_ <= (*i)->_pr_track_direction_stretch_end_param() )
      {
        // does it match both limits?
        if( _par_ >= (*i)->_pr_track_direction_stretch_start_param() )
        {
          return (*i)->_rf_track_direction_stretch( _par_ ); 
        }
        else // must be undefined range
        {
          break;
        }
      }
    }
    // undefined range
    std::pair<bool,direction> ret; 
    ret.first = false;
    return ret;
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
      _av_track_direction_stretch_is_avail(c),
      _av_track_position_stretch_is_avail(c)
  {
    avail_operator_direction_stretch = new solve::Multi_And_Operator(c);
    _av_track_direction_stretch_is_avail = avail_operator_direction_stretch->get_result();
    avail_operator_position_stretch = new solve::Multi_And_Operator(c);
    _av_track_position_stretch_is_avail = avail_operator_position_stretch->get_result();
  }
  //! function that is called after hierarchy was set up for each node
  void _serial_container_track::hierarchy_final_init()

  {
    avail_operator_direction_stretch->finish_adding();
    avail_operator_position_stretch->finish_adding();
  }
  // *****************************
  // *****************************
  // tree node type implementation
  // *****************************
  // *****************************

  // *****************************
  // object: node type
  // *****************************

  // ** virtual tree node functions **
  proptree::Prop_Tree_Node *node_object::try_add_child( std::string type, std::string name ) throw()
  {
    proptree::Prop_Tree_Node *node = 0;
    node = _cn_object_state.add_child( type, name, info, get_consultant(), node );
    return node;
  }
  void node_object::custom_hierarchy_final_init()
  {
    _cn_object_state.hierarchy_final_init();
  }

  void node_object::common_init()
  {
    solve::establish_Default_Value( info->priority_system,3000, _op_center, vector((0),0,0));
    solve::establish_Default_Value( info->priority_system,3001, _op_front, vector((1),0,0));
    solve::establish_Default_Value( info->priority_system,3002, _op_up_vector, vector((0),1,0));
    // ** invoke first_/last_init() for each child container **
    if( !_cn_object_state.elements_empty() )
    {
      (*_cn_object_state.elements_begin())->_rf_object_state_first_init();
      (*(--_cn_object_state.elements_end()))->_rf_object_state_last_init();
    }
    // ** establish availability checks **
    solve::Multi_And_Operator *m_and;
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_component_front_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_front_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_component_position_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_position_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_component_rotation_time_is_avail = m_and->get_result();
    m_and->add_operand( solve::is_solved( _op_front ) );
    m_and->add_operand( solve::is_solved( _op_up_vector ) );
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_front_time_is_avail );
    m_and->add_operand( _cn_object_state._av_object_state_up_vector_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_component_speed_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_speed_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_component_translation_time_is_avail = m_and->get_result();
    m_and->add_operand( solve::is_solved( _op_center ) );
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_position_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_component_up_vector_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_up_vector_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_state_acceleration_stretch_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_acceleration_stretch_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_state_acceleration_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_acceleration_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_state_direction_stretch_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_direction_stretch_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_state_direction_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_direction_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_state_front_stretch_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_front_stretch_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_state_front_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_front_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_state_position_stretch_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_position_stretch_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_state_position_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_position_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_state_speed_stretch_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_speed_stretch_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_state_speed_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_speed_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_state_stretch_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_stretch_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_state_up_vector_stretch_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_up_vector_stretch_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_object_state_up_vector_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_object_state._av_object_state_up_vector_time_is_avail );
    m_and->finish_adding();
  }

  // ** result functions **
  std::pair<bool,front> node_object::_rf_object_component_front_time( time t )
  {
    std::pair<bool,front> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_front_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:104 ***
      
      return(_cn_object_state._rf_object_state_front_time( t ));
    
  }

  std::pair<bool,position> node_object::_rf_object_component_position_time( time t )
  {
    std::pair<bool,position> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_position_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:99 ***
      
      return(_cn_object_state._rf_object_state_position_time( t ));
    
  }

  std::pair<bool,rotation> node_object::_rf_object_component_rotation_time( time t )
  {
    std::pair<bool,rotation> no_res;
    no_res.first = false;
    // ** check for required properties 
    if( !_op_front.is_solved() || !_op_up_vector.is_solved() )
    {
      error() << "required property wasn't solved";
      return no_res;
    }
    // ** check for required children
    if( !_cn_object_state._av_object_state_front_time_is_avail.is_solved() || !_cn_object_state._av_object_state_up_vector_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:89 ***
      
      vector dest_front = _cn_object_state._rf_object_state_front_time( t ).second;
      vector dest_up_vector = _cn_object_state._rf_object_state_up_vector_time( t ).second;
      vector rotate = Vrotate_pair_pair( _op_front(), _op_up_vector(), 
           dest_front, dest_up_vector );
      return std::pair<bool,rotation>(true,rotate );
    
  }

  std::pair<bool,speed> node_object::_rf_object_component_speed_time( time t )
  {
    std::pair<bool,speed> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_speed_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:114 ***
      
      return(_cn_object_state._rf_object_state_speed_time( t ));
    
  }

  std::pair<bool,translation> node_object::_rf_object_component_translation_time( time t )
  {
    std::pair<bool,translation> no_res;
    no_res.first = false;
    // ** check for required properties 
    if( !_op_center.is_solved() )
    {
      error() << "required property wasn't solved";
      return no_res;
    }
    // ** check for required children
    if( !_cn_object_state._av_object_state_position_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:81 ***
      
      vector position = _cn_object_state._rf_object_state_position_time( t ).second;
      vector translate = position - _op_center();
      return std::pair<bool,translation>(true,translate );
    
  }

  std::pair<bool,up_vector> node_object::_rf_object_component_up_vector_time( time t )
  {
    std::pair<bool,up_vector> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_up_vector_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:109 ***
      
      return(_cn_object_state._rf_object_state_up_vector_time( t ));
    
  }

  std::pair<bool,acceleration> node_object::_rf_object_state_acceleration_stretch( stretch s )
  {
    std::pair<bool,acceleration> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_acceleration_stretch_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:165 ***
      
      return(_cn_object_state._rf_object_state_acceleration_stretch( s ));
    
  }

  std::pair<bool,acceleration> node_object::_rf_object_state_acceleration_time( time t )
  {
    std::pair<bool,acceleration> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_acceleration_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:160 ***
      
      return(_cn_object_state._rf_object_state_acceleration_time( t ));
    
  }

  std::pair<bool,direction> node_object::_rf_object_state_direction_stretch( stretch s )
  {
    std::pair<bool,direction> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_direction_stretch_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:139 ***
      
      return(_cn_object_state._rf_object_state_direction_stretch( s ));
    
  }

  std::pair<bool,direction> node_object::_rf_object_state_direction_time( time t )
  {
    std::pair<bool,direction> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_direction_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:134 ***
      
      return(_cn_object_state._rf_object_state_direction_time( t ));
    
  }

  std::pair<bool,front> node_object::_rf_object_state_front_stretch( stretch s )
  {
    std::pair<bool,front> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_front_stretch_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:176 ***
      
      return(_cn_object_state._rf_object_state_front_stretch( s ));
    
  }

  std::pair<bool,front> node_object::_rf_object_state_front_time( time t )
  {
    std::pair<bool,front> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_front_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:171 ***
      
      return(_cn_object_state._rf_object_state_front_time( t ));
    
  }

  std::pair<bool,position> node_object::_rf_object_state_position_stretch( stretch s )
  {
    std::pair<bool,position> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_position_stretch_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:129 ***
      
      return(_cn_object_state._rf_object_state_position_stretch( s ));
    
  }

  std::pair<bool,position> node_object::_rf_object_state_position_time( time t )
  {
    std::pair<bool,position> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_position_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:124 ***
      
      return(_cn_object_state._rf_object_state_position_time( t ));
    
  }

  std::pair<bool,speed> node_object::_rf_object_state_speed_stretch( stretch s )
  {
    std::pair<bool,speed> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_speed_stretch_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:155 ***
      
      return(_cn_object_state._rf_object_state_speed_stretch( s ));
    
  }

  std::pair<bool,speed> node_object::_rf_object_state_speed_time( time t )
  {
    std::pair<bool,speed> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_speed_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:150 ***
      
      return(_cn_object_state._rf_object_state_speed_time( t ));
    
  }

  std::pair<bool,stretch> node_object::_rf_object_state_stretch_time( time t )
  {
    std::pair<bool,stretch> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_stretch_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:145 ***
      
      return(_cn_object_state._rf_object_state_stretch_time( t ));
    
  }

  std::pair<bool,up_vector> node_object::_rf_object_state_up_vector_stretch( stretch s )
  {
    std::pair<bool,up_vector> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_up_vector_stretch_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:186 ***
      
      return(_cn_object_state._rf_object_state_up_vector_stretch( s ));
    
  }

  std::pair<bool,up_vector> node_object::_rf_object_state_up_vector_time( time t )
  {
    std::pair<bool,up_vector> no_res;
    no_res.first = false;
    // ** check for required children
    if( !_cn_object_state._av_object_state_up_vector_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:181 ***
      
      return(_cn_object_state._rf_object_state_up_vector_time( t ));
    
  }

  // ** infrastructure functions **
  void node_object::make_availible()
  {
    _container_object_component::add_node_factory( "object", new proptree::Node_Factory<_pt_object_component,node_object > );
    _serial_container_object_state::add_node_factory( "object", new proptree::Node_Factory<_pt_object_state,node_object > );
  }
  // ** constructor **
  node_object::node_object( std::string name, proptree::tree_info *info, 
    message::Message_Consultant *msg_consultant )
    : proptree::Prop_Tree_Node( "object", name, info, msg_consultant ), 
      _pt_object_component( msg_consultant ), 
      _pt_object_state( msg_consultant ),
      _op_center( "center", this ),
      _op_front( "front", this ),
      _op_up_vector( "up_vector", this ),
      _cn_object_state(false, true, get_consultant() )
  {
    // ********************
    // Register Properties 
    add_property( "center", &_op_center );
    add_property( "front", &_op_front );
    add_property( "up_vector", &_op_up_vector );
  }

  // *****************************
  // make nodes availible 
  // *****************************

  void make_object_base_nodes_availible()  {
    node_object::make_availible();
  }
}

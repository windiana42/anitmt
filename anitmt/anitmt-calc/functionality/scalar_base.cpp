// ********************************************
// generated file by funcgen (www.anitmt.org)
// requires:
//   - libmessage
//   - libval
//   - libsolve
//   - libproptree
// ********************************************

#include <solve/constraint.hpp>
#include "scalar_base.hpp"

namespace functionality
{
  // ****************************
  // help functions
  // ****************************

  template<class T>
  inline T extract_status( std::pair<bool,T> value, bool &status, 
			   bool &any_false )
  {
    status = value.first;
    any_false |= !value.first;
    return value.second;
  }
  // ****************************
  // provider type implementation
  // ****************************

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
    : proptree::Prop_Tree_Node("","",0,c), /* should never be used */
      _tc_prev_scalar_component(0),
      _tc_next_scalar_component(0),
      _av_scalar_component_value_time_is_avail(c)
  {
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
    : proptree::Prop_Tree_Node("","",0,c), /* should never be used */
      _tc_prev_scalar_vsa(0),
      _tc_next_scalar_vsa(0),
      _pr_scalar_vsa_acceleration_time_start_param(c),
      _pr_scalar_vsa_acceleration_time_end_param(c),
      _pr_scalar_vsa_slope_time_start_param(c),
      _pr_scalar_vsa_slope_time_end_param(c),
      _pr_scalar_vsa_value_time_start_param(c),
      _pr_scalar_vsa_value_time_end_param(c),
      _av_scalar_vsa_acceleration_time_is_avail(c),
      _av_scalar_vsa_slope_time_is_avail(c),
      _av_scalar_vsa_value_time_is_avail(c)
  {
  }

  // *****************
  // container classes

  // ********************************************************************
  // serial container for nodes that provide scalar_component:
  //   _serial_container_scalar_component

  _container_scalar_component::node_factories_type _container_scalar_component::node_factories;
  void _container_scalar_component::add_node_factory( std::string name, proptree::Basic_Node_Factory< _pt_scalar_component >* nf )
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
    proptree::Basic_Node_Factory< _pt_scalar_component >* &nf = i->second;
    proptree::Basic_Node_Factory< _pt_scalar_component >::node_return_type node;
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
  void _serial_container_scalar_vsa::add_node_factory( std::string name, proptree::Basic_Node_Factory< _pt_scalar_vsa >* nf )
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
    proptree::Basic_Node_Factory< _pt_scalar_vsa >* &nf = i->second;
    proptree::Basic_Node_Factory< _pt_scalar_vsa >::node_return_type node;
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

  std::pair< bool,acceleration > _serial_container_scalar_vsa::_rf_scalar_vsa_acceleration_time( time _par_ )
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
    std::pair< bool,acceleration > ret; 
    ret.first = false;
    return ret;
  }

  std::pair< bool,slope > _serial_container_scalar_vsa::_rf_scalar_vsa_slope_time( time _par_ )
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
    std::pair< bool,slope > ret; 
    ret.first = false;
    return ret;
  }

  std::pair< bool,value > _serial_container_scalar_vsa::_rf_scalar_vsa_value_time( time _par_ )
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
    std::pair< bool,value > ret; 
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
  // *****************************
  // *****************************
  // tree node type implementation
  // *****************************
  // *****************************

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
    if( get_first_child() && get_first_child()->get_property( "start_time" ) )
    {
      equal_solver(_op_start_time,get_first_child()->get_property( "start_time" ));
    }
    if( get_first_child() && get_first_child()->get_property( "start_value" ) )
    {
      equal_solver(_op_start_value,get_first_child()->get_property( "start_value" ));
    }
    if( get_first_child() && get_first_child()->get_property( "start_slope" ) )
    {
      equal_solver(_op_start_slope,get_first_child()->get_property( "start_slope" ));
    }
    if( get_last_child() && get_last_child()->get_property( "end_time" ) )
    {
      equal_solver(_op_end_time,get_last_child()->get_property( "end_time" ));
    }
    if( get_last_child() && get_last_child()->get_property( "end_value" ) )
    {
      equal_solver(_op_end_value,get_last_child()->get_property( "end_value" ));
    }
    if( get_last_child() && get_last_child()->get_property( "end_slope" ) )
    {
      equal_solver(_op_end_slope,get_last_child()->get_property( "end_slope" ));
    }
    solve::establish_Default_Value( info->priority_system,_pl_default_first_time, _op_start_time, 0);
    solve::establish_Default_Value( info->priority_system,_pl_default_first_state, _op_start_value, 0);
    // ** invoke first_/last_init() for each child container **
    if( !_cn_scalar_vsa.elements_empty() )
    {
      (*_cn_scalar_vsa.elements_begin())->_rf_scalar_vsa_first_init();
      (*(--_cn_scalar_vsa.elements_end()))->_rf_scalar_vsa_last_init();
    }
    // ** establish availability checks **
    solve::Multi_And_Operator *m_and;
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_scalar_component_value_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_scalar_vsa._av_scalar_vsa_value_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_scalar_vsa_acceleration_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_scalar_vsa._av_scalar_vsa_acceleration_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_scalar_vsa_slope_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_scalar_vsa._av_scalar_vsa_slope_time_is_avail );
    m_and->finish_adding();
    m_and = new solve::Multi_And_Operator(get_consultant());
    _av_scalar_vsa_value_time_is_avail = m_and->get_result();
    m_and->finish_adding();
    m_and->add_operand( _cn_scalar_vsa._av_scalar_vsa_value_time_is_avail );
    m_and->finish_adding();
  }

  // ** result functions **
  std::pair< bool,value > node_scalar::_rf_scalar_component_value_time( time t )
  {
    std::pair< bool,value > no_res;
    no_res.first = false;
    bool did_any_result_fail;
    bool did_result_fail;
    did_any_result_fail = false;
    did_result_fail = false;
    // ** check for required children
    if( !_cn_scalar_vsa._av_scalar_vsa_value_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:127 ***
                                                                         
      return(_cn_scalar_vsa._rf_scalar_vsa_value_time( t ));
    
  }

  std::pair< bool,acceleration > node_scalar::_rf_scalar_vsa_acceleration_time( time t )
  {
    std::pair< bool,acceleration > no_res;
    no_res.first = false;
    bool did_any_result_fail;
    bool did_result_fail;
    did_any_result_fail = false;
    did_result_fail = false;
    // ** check for required children
    if( !_cn_scalar_vsa._av_scalar_vsa_acceleration_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:120 ***
                                                                                        
      return(_cn_scalar_vsa._rf_scalar_vsa_acceleration_time( t ));
    
  }

  std::pair< bool,slope > node_scalar::_rf_scalar_vsa_slope_time( time t )
  {
    std::pair< bool,slope > no_res;
    no_res.first = false;
    bool did_any_result_fail;
    bool did_result_fail;
    did_any_result_fail = false;
    did_result_fail = false;
    // ** check for required children
    if( !_cn_scalar_vsa._av_scalar_vsa_slope_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:117 ***
                                                                          
      return(_cn_scalar_vsa._rf_scalar_vsa_slope_time( t ));
    
  }

  std::pair< bool,value > node_scalar::_rf_scalar_vsa_value_time( time t )
  {
    std::pair< bool,value > no_res;
    no_res.first = false;
    bool did_any_result_fail;
    bool did_result_fail;
    did_any_result_fail = false;
    did_result_fail = false;
    // ** check for required children
    if( !_cn_scalar_vsa._av_scalar_vsa_value_time_is_avail.is_solved() )
    {
      error() << "required child container wasn't solved";
      return no_res;
    }
    // *** user code following... line:114 ***
                                                                           
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
      _op_difference( "difference", this ),
      _op_duration( "duration", this ),
      _op_end_slope( "end_slope", this ),
      _op_end_time( "end_time", this ),
      _op_end_value( "end_value", this ),
      _op_slope_difference( "slope_difference", this ),
      _op_start_slope( "start_slope", this ),
      _op_start_time( "start_time", this ),
      _op_start_value( "start_value", this ),
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

    // *****************
    // Register Aliases 
    add_property( "diff_slope", &_op_slope_difference );
    add_property( "diff_value", &_op_difference );
  }

  // *****************************
  // make nodes availible 
  // *****************************

  void make_scalar_base_nodes_availible()  {
    node_scalar::make_availible();
  }
}

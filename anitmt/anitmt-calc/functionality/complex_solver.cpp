// ********************************************
// generated file by funcgen (www.anitmt.org)
// requires:
//   - libmessage
//   - libval
//   - libsolve
//   - libproptree
// ********************************************

#include <solve/constraint.hpp>
#include "complex_solver.hpp"

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

  // *****************
  // container classes

  // ********************
  // ********************
  // solver declartions
  // ********************
  // ********************

  _sc_accel_solver::_sc_accel_solver( solve::Operand< scalar >& __op_s, solve::Operand< scalar >& __op_t, solve::Operand< scalar >& __op_a, solve::Operand< scalar >& __op_v0, solve::Operand< scalar >& __op_ve,  message::Message_Consultant* consultant )
    : message::Message_Reporter(consultant),
      _op_s(__op_s),
      _op_t(__op_t),
      _op_a(__op_a),
      _op_v0(__op_v0),
      _op_ve(__op_ve),
      _op_at( get_consultant() ),
      _op_v0_ve( get_consultant() ),
      _op_vt( get_consultant() ),
      _op_ve2( get_consultant() ),
      _op_v02( get_consultant() ),
      _op_prod( get_consultant() ),
      _op_as( get_consultant() )
  {
    product_solver( _op_at, _op_a, _op_t, get_consultant() );
    sum_solver( _op_ve, _op_v0, _op_at, get_consultant() );
    _op_v0.assign( _op_s/_op_t-solve::const_op( values::Scalar( 0.5 ), get_consultant() )*_op_a*_op_t );
    sum_solver( _op_v0_ve, _op_v0, _op_ve, get_consultant() );
    product_solver( _op_vt, _op_v0_ve, _op_t, get_consultant() );
    product_solver( _op_s, solve::const_op( values::Scalar( 0.5 ), get_consultant() ), _op_vt, get_consultant() );
    product_solver( _op_as, _op_a, _op_s, get_consultant() );
    product_solver( _op_prod, solve::const_op( values::Scalar( 2 ), get_consultant() ), _op_as, get_consultant() );
    square_solver( _op_ve2, _op_ve, get_consultant() );
    square_solver( _op_v02, _op_v0, get_consultant() );
    sum_solver( _op_ve2, _op_v02, _op_prod, get_consultant() );
  }
  _sc_accel_solver* accel_solver( solve::Operand< scalar >& __op_s, solve::Operand< scalar >& __op_t, solve::Operand< scalar >& __op_a, solve::Operand< scalar >& __op_v0, solve::Operand< scalar >& __op_ve, message::Message_Consultant *msgc )
  {
    return new _sc_accel_solver( __op_s, __op_t, __op_a, __op_v0, __op_ve, msgc );
  }
  vector _sc_bezier_solver::get_pos( stretch s )
  {
    // *** user code following... line:78 ***
        
	int t = test->cooler_wert();
	return bezier->get_pos( s );
      
  }
  _sc_bezier_solver::_sc_bezier_solver( solve::Operand< vector >& __op_p1, solve::Operand< vector >& __op_p2, solve::Operand< vector >& __op_p3, solve::Operand< vector >& __op_p4, solve::Operand< scalar >& __op_length,  message::Message_Consultant* consultant )
    : message::Message_Reporter(consultant),
      _av_get_pos___is_avail( get_consultant() ),
      _op_p1(__op_p1),
      _op_p2(__op_p2),
      _op_p3(__op_p3),
      _op_p4(__op_p4),
      _op_length(__op_length)
  {
    solve::Multi_And_Operator *m_and;
    m_and = new solve::Multi_And_Operator( get_consultant() );
    _av_get_pos___is_avail = m_and->get_result();
    m_and->add_operand( solve::is_solved(_op_p1) );
    m_and->add_operand( solve::is_solved(_op_p2) );
    m_and->add_operand( solve::is_solved(_op_p3) );
    m_and->add_operand( solve::is_solved(_op_p4) );
    m_and->add_operand( test->_av_cooler_wert___is_avail );
    m_and->finish_adding();
    solve::constraint( _op_length>=solve::const_op( values::Scalar( 0 ), get_consultant() ) );
    test = test_solver( get_consultant() );
  }
  _sc_bezier_solver* bezier_solver( solve::Operand< vector >& __op_p1, solve::Operand< vector >& __op_p2, solve::Operand< vector >& __op_p3, solve::Operand< vector >& __op_p4, solve::Operand< scalar >& __op_length, message::Message_Consultant *msgc )
  {
    return new _sc_bezier_solver( __op_p1, __op_p2, __op_p3, __op_p4, __op_length, msgc );
  }
  int _sc_test_solver::cooler_wert(  )
  {
    // *** user code following... line:30 ***
        
	return 3;
      
  }
  _sc_test_solver::_sc_test_solver(  message::Message_Consultant* consultant )
    : message::Message_Reporter(consultant),
      _av_cooler_wert___is_avail( get_consultant() )
  {
    solve::Multi_And_Operator *m_and;
    m_and = new solve::Multi_And_Operator( get_consultant() );
    _av_cooler_wert___is_avail = m_and->get_result();
    m_and->finish_adding();
  }
  _sc_test_solver* test_solver( message::Message_Consultant *msgc )
  {
    return new _sc_test_solver( msgc );
  }
  // *****************************
  // *****************************
  // tree node type implementation
  // *****************************
  // *****************************

  // *****************************
  // make nodes availible 
  // *****************************

  void make_complex_solver_nodes_availible()  {
  }
}

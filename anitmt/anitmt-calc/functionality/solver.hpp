// ********************************************
// generated file by funcgen (www.anitmt.org)
// requires:
//   - libmessage
//   - libval
//   - libsolve
//   - libproptree
// ********************************************

#ifndef __functionality_solver__
#define __functionality_solver__

#include <list>
#include <string>
#include <map>
#include <math.h>

#include <message/message.hpp>
#include <val/val.hpp>
#include <solve/operand.hpp>
#include <solve/operator.hpp>
#include <solve/solver.hpp>
#include <proptree/property.hpp>
#include <proptree/proptree.hpp>

#include "base_func.hpp"
#include "val/val.hpp"
namespace functionality
{
  // **********************
  // base type declarations
  // **********************


  // **************************
  // provider type declarations
  // **************************

  // ****************
  // provider classes

  // *****************
  // container classes


  // ********************
  // ********************
  // operator declartions
  // ********************
  // ********************

}
namespace solve
{

  // *******************************************
  // one_operand_operator abs
  // *******************************************

  template< class T_Result, class T_Operand1 >
  class _oc_abs
    : public solve::Basic_Operator_for_1_Operand< T_Result, T_Operand1 >
  {
  public:
    _oc_abs( solve::Operand< T_Operand1 > &operand1 )
      : solve::Basic_Operator_for_1_Operand< T_Result, T_Operand1 > 
        ( operand1 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & val )
    { // user code: ./solver.afd:49:16: 
      
      return abs(val);
    
    }
  };

  inline solve::Operand< functionality::scalar >& 
  abs( solve::Operand< functionality::scalar > & operand1 )
  {
    return (new _oc_abs< functionality::scalar, functionality::scalar >( operand1 ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  abs( solve::Operand< functionality::vector > & operand1 )
  {
    return (new _oc_abs< functionality::scalar, functionality::vector >( operand1 ) )->get_result();
  }

  // *******************************************
  // two_operands_operator add
  // *******************************************

  template< class T_Result, class T_Operand1, class T_Operand2 >
  class _oc_add
    : public solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 >
  {
  public:
    _oc_add( solve::Operand< T_Operand1 > &operand1, solve::Operand< T_Operand2 > &operand2 )
      : solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 > 
        ( operand1, operand2 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & op1, const T_Operand2 & op2 )
    { // user code: ./solver.afd:127:16: 
      
      return op1 + op2;
    
    }
  };

  inline solve::Operand< functionality::scalar >& 
  operator+( solve::Operand< functionality::scalar > & operand1, const functionality::scalar & operand2 )
  {
    return (new _oc_add< functionality::scalar, functionality::scalar, functionality::scalar >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  operator+( const functionality::scalar & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_add< functionality::scalar, functionality::scalar, functionality::scalar >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  operator+( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_add< functionality::scalar, functionality::scalar, functionality::scalar >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator+( solve::Operand< functionality::vector > & operand1, const functionality::vector & operand2 )
  {
    return (new _oc_add< functionality::vector, functionality::vector, functionality::vector >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator+( const functionality::vector & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_add< functionality::vector, functionality::vector, functionality::vector >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator+( solve::Operand< functionality::vector > & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_add< functionality::vector, functionality::vector, functionality::vector >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator+( solve::Operand< functionality::matrix > & operand1, const functionality::matrix & operand2 )
  {
    return (new _oc_add< functionality::matrix, functionality::matrix, functionality::matrix >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator+( const functionality::matrix & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_add< functionality::matrix, functionality::matrix, functionality::matrix >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator+( solve::Operand< functionality::matrix > & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_add< functionality::matrix, functionality::matrix, functionality::matrix >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::string >& 
  operator+( solve::Operand< functionality::string > & operand1, const functionality::string & operand2 )
  {
    return (new _oc_add< functionality::string, functionality::string, functionality::string >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::string >& 
  operator+( const functionality::string & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_add< functionality::string, functionality::string, functionality::string >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::string >& 
  operator+( solve::Operand< functionality::string > & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_add< functionality::string, functionality::string, functionality::string >( operand1, operand2 ) )->get_result();
  }

  // *******************************************
  // two_operands_operator divide
  // *******************************************

  template< class T_Result, class T_Operand1, class T_Operand2 >
  class _oc_divide
    : public solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 >
  {
  public:
    _oc_divide( solve::Operand< T_Operand1 > &operand1, solve::Operand< T_Operand2 > &operand2 )
      : solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 > 
        ( operand1, operand2 )
    {
      init();
    }
  private:
    virtual bool are_operands_enough( const T_Operand1 & op1, const T_Operand2 & op2 )
    { // user code: ./solver.afd:217:24: 
      
      return (op2 != T_Operand2());
    
    }
    virtual bool are_operands_ok( const T_Operand1 & op1, const T_Operand2 & op2, solve::Solve_Run_Info* i )
    { // user code: ./solver.afd:212:20: 
      
      return (op1 == T_Operand1()) == (op2 == T_Operand2());	
					// both zero or both not zero
    
    }
    virtual T_Result calc_result( const T_Operand1 & op1, const T_Operand2 & op2 )
    { // user code: ./solver.afd:200:16: 
      
      return op1 / op2;
    
    }
    virtual T_Result calc_result_from_op1( const T_Operand1 & op )
    { // user code: ./solver.afd:208:25: 
      
      return T_Result();		// return 0 in result Type
    
    }
    virtual bool is_operand1_enough( const T_Operand1 & op1 )
    { // user code: ./solver.afd:204:23: 
      
      return op1 == T_Operand1();	//!!! replace by zero type
    
    }
  };

  inline solve::Operand< functionality::scalar >& 
  operator/( solve::Operand< functionality::scalar > & operand1, const functionality::scalar & operand2 )
  {
    return (new _oc_divide< functionality::scalar, functionality::scalar, functionality::scalar >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  operator/( const functionality::scalar & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_divide< functionality::scalar, functionality::scalar, functionality::scalar >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  operator/( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_divide< functionality::scalar, functionality::scalar, functionality::scalar >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator/( solve::Operand< functionality::vector > & operand1, const functionality::scalar & operand2 )
  {
    return (new _oc_divide< functionality::vector, functionality::vector, functionality::scalar >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator/( const functionality::vector & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_divide< functionality::vector, functionality::vector, functionality::scalar >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator/( solve::Operand< functionality::vector > & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_divide< functionality::vector, functionality::vector, functionality::scalar >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator/( solve::Operand< functionality::scalar > & operand1, const functionality::vector & operand2 )
  {
    return (new _oc_divide< functionality::vector, functionality::scalar, functionality::vector >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator/( const functionality::scalar & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_divide< functionality::vector, functionality::scalar, functionality::vector >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator/( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_divide< functionality::vector, functionality::scalar, functionality::vector >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator/( solve::Operand< functionality::matrix > & operand1, const functionality::scalar & operand2 )
  {
    return (new _oc_divide< functionality::matrix, functionality::matrix, functionality::scalar >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator/( const functionality::matrix & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_divide< functionality::matrix, functionality::matrix, functionality::scalar >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator/( solve::Operand< functionality::matrix > & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_divide< functionality::matrix, functionality::matrix, functionality::scalar >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator/( solve::Operand< functionality::scalar > & operand1, const functionality::matrix & operand2 )
  {
    return (new _oc_divide< functionality::matrix, functionality::scalar, functionality::matrix >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator/( const functionality::scalar & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_divide< functionality::matrix, functionality::scalar, functionality::matrix >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator/( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_divide< functionality::matrix, functionality::scalar, functionality::matrix >( operand1, operand2 ) )->get_result();
  }

  // *******************************************
  // two_operands_operator equal
  // *******************************************

  template< class T_Result, class T_Operand1, class T_Operand2 >
  class _oc_equal
    : public solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 >
  {
  public:
    _oc_equal( solve::Operand< T_Operand1 > &operand1, solve::Operand< T_Operand2 > &operand2 )
      : solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 > 
        ( operand1, operand2 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & op1, const T_Operand2 & op2 )
    { // user code: ./solver.afd:236:16: 
      
      return op1 == op2;
    
    }
  };

  inline solve::Operand< functionality::boolean >& 
  operator==( solve::Operand< functionality::boolean > & operand1, const functionality::boolean & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::boolean, functionality::boolean >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( const functionality::boolean & operand1, solve::Operand< functionality::boolean > & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::boolean, functionality::boolean >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( solve::Operand< functionality::boolean > & operand1, solve::Operand< functionality::boolean > & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::boolean, functionality::boolean >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( solve::Operand< functionality::scalar > & operand1, const functionality::scalar & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::scalar, functionality::scalar >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( const functionality::scalar & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::scalar, functionality::scalar >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::scalar, functionality::scalar >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( solve::Operand< functionality::vector > & operand1, const functionality::vector & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::vector, functionality::vector >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( const functionality::vector & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::vector, functionality::vector >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( solve::Operand< functionality::vector > & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::vector, functionality::vector >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( solve::Operand< functionality::matrix > & operand1, const functionality::matrix & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::matrix, functionality::matrix >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( const functionality::matrix & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::matrix, functionality::matrix >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( solve::Operand< functionality::matrix > & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::matrix, functionality::matrix >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( solve::Operand< functionality::string > & operand1, const functionality::string & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::string, functionality::string >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( const functionality::string & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::string, functionality::string >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator==( solve::Operand< functionality::string > & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_equal< functionality::boolean, functionality::string, functionality::string >( operand1, operand2 ) )->get_result();
  }

  // *******************************************
  // two_operands_operator greater
  // *******************************************

  template< class T_Result, class T_Operand1, class T_Operand2 >
  class _oc_greater
    : public solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 >
  {
  public:
    _oc_greater( solve::Operand< T_Operand1 > &operand1, solve::Operand< T_Operand2 > &operand2 )
      : solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 > 
        ( operand1, operand2 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & op1, const T_Operand2 & op2 )
    { // user code: ./solver.afd:293:16: 
      
      return op1 > op2;
    
    }
  };

  inline solve::Operand< functionality::boolean >& 
  operator>( solve::Operand< functionality::boolean > & operand1, const functionality::boolean & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::boolean, functionality::boolean >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( const functionality::boolean & operand1, solve::Operand< functionality::boolean > & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::boolean, functionality::boolean >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( solve::Operand< functionality::boolean > & operand1, solve::Operand< functionality::boolean > & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::boolean, functionality::boolean >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( solve::Operand< functionality::scalar > & operand1, const functionality::scalar & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::scalar, functionality::scalar >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( const functionality::scalar & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::scalar, functionality::scalar >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::scalar, functionality::scalar >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( solve::Operand< functionality::vector > & operand1, const functionality::vector & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::vector, functionality::vector >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( const functionality::vector & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::vector, functionality::vector >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( solve::Operand< functionality::vector > & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::vector, functionality::vector >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( solve::Operand< functionality::matrix > & operand1, const functionality::matrix & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::matrix, functionality::matrix >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( const functionality::matrix & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::matrix, functionality::matrix >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( solve::Operand< functionality::matrix > & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::matrix, functionality::matrix >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( solve::Operand< functionality::string > & operand1, const functionality::string & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::string, functionality::string >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( const functionality::string & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::string, functionality::string >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>( solve::Operand< functionality::string > & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_greater< functionality::boolean, functionality::string, functionality::string >( operand1, operand2 ) )->get_result();
  }

  // *******************************************
  // two_operands_operator greater_equal
  // *******************************************

  template< class T_Result, class T_Operand1, class T_Operand2 >
  class _oc_greater_equal
    : public solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 >
  {
  public:
    _oc_greater_equal( solve::Operand< T_Operand1 > &operand1, solve::Operand< T_Operand2 > &operand2 )
      : solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 > 
        ( operand1, operand2 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & op1, const T_Operand2 & op2 )
    { // user code: ./solver.afd:331:16: 
      
      return op1 >= op2;
    
    }
  };

  inline solve::Operand< functionality::boolean >& 
  operator>=( solve::Operand< functionality::boolean > & operand1, const functionality::boolean & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::boolean, functionality::boolean >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( const functionality::boolean & operand1, solve::Operand< functionality::boolean > & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::boolean, functionality::boolean >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( solve::Operand< functionality::boolean > & operand1, solve::Operand< functionality::boolean > & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::boolean, functionality::boolean >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( solve::Operand< functionality::scalar > & operand1, const functionality::scalar & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::scalar, functionality::scalar >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( const functionality::scalar & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::scalar, functionality::scalar >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::scalar, functionality::scalar >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( solve::Operand< functionality::vector > & operand1, const functionality::vector & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::vector, functionality::vector >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( const functionality::vector & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::vector, functionality::vector >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( solve::Operand< functionality::vector > & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::vector, functionality::vector >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( solve::Operand< functionality::matrix > & operand1, const functionality::matrix & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::matrix, functionality::matrix >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( const functionality::matrix & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::matrix, functionality::matrix >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( solve::Operand< functionality::matrix > & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::matrix, functionality::matrix >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( solve::Operand< functionality::string > & operand1, const functionality::string & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::string, functionality::string >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( const functionality::string & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::string, functionality::string >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator>=( solve::Operand< functionality::string > & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_greater_equal< functionality::boolean, functionality::string, functionality::string >( operand1, operand2 ) )->get_result();
  }

  // *******************************************
  // two_operands_operator less
  // *******************************************

  template< class T_Result, class T_Operand1, class T_Operand2 >
  class _oc_less
    : public solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 >
  {
  public:
    _oc_less( solve::Operand< T_Operand1 > &operand1, solve::Operand< T_Operand2 > &operand2 )
      : solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 > 
        ( operand1, operand2 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & op1, const T_Operand2 & op2 )
    { // user code: ./solver.afd:274:16: 
      
      return op1 < op2;
    
    }
  };

  inline solve::Operand< functionality::boolean >& 
  operator<( solve::Operand< functionality::boolean > & operand1, const functionality::boolean & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::boolean, functionality::boolean >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( const functionality::boolean & operand1, solve::Operand< functionality::boolean > & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::boolean, functionality::boolean >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( solve::Operand< functionality::boolean > & operand1, solve::Operand< functionality::boolean > & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::boolean, functionality::boolean >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( solve::Operand< functionality::scalar > & operand1, const functionality::scalar & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::scalar, functionality::scalar >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( const functionality::scalar & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::scalar, functionality::scalar >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::scalar, functionality::scalar >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( solve::Operand< functionality::vector > & operand1, const functionality::vector & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::vector, functionality::vector >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( const functionality::vector & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::vector, functionality::vector >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( solve::Operand< functionality::vector > & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::vector, functionality::vector >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( solve::Operand< functionality::matrix > & operand1, const functionality::matrix & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::matrix, functionality::matrix >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( const functionality::matrix & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::matrix, functionality::matrix >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( solve::Operand< functionality::matrix > & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::matrix, functionality::matrix >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( solve::Operand< functionality::string > & operand1, const functionality::string & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::string, functionality::string >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( const functionality::string & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::string, functionality::string >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<( solve::Operand< functionality::string > & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_less< functionality::boolean, functionality::string, functionality::string >( operand1, operand2 ) )->get_result();
  }

  // *******************************************
  // two_operands_operator less_equal
  // *******************************************

  template< class T_Result, class T_Operand1, class T_Operand2 >
  class _oc_less_equal
    : public solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 >
  {
  public:
    _oc_less_equal( solve::Operand< T_Operand1 > &operand1, solve::Operand< T_Operand2 > &operand2 )
      : solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 > 
        ( operand1, operand2 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & op1, const T_Operand2 & op2 )
    { // user code: ./solver.afd:312:16: 
      
      return op1 <= op2;
    
    }
  };

  inline solve::Operand< functionality::boolean >& 
  operator<=( solve::Operand< functionality::boolean > & operand1, const functionality::boolean & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::boolean, functionality::boolean >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( const functionality::boolean & operand1, solve::Operand< functionality::boolean > & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::boolean, functionality::boolean >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( solve::Operand< functionality::boolean > & operand1, solve::Operand< functionality::boolean > & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::boolean, functionality::boolean >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( solve::Operand< functionality::scalar > & operand1, const functionality::scalar & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::scalar, functionality::scalar >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( const functionality::scalar & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::scalar, functionality::scalar >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::scalar, functionality::scalar >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( solve::Operand< functionality::vector > & operand1, const functionality::vector & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::vector, functionality::vector >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( const functionality::vector & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::vector, functionality::vector >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( solve::Operand< functionality::vector > & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::vector, functionality::vector >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( solve::Operand< functionality::matrix > & operand1, const functionality::matrix & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::matrix, functionality::matrix >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( const functionality::matrix & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::matrix, functionality::matrix >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( solve::Operand< functionality::matrix > & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::matrix, functionality::matrix >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( solve::Operand< functionality::string > & operand1, const functionality::string & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::string, functionality::string >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( const functionality::string & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::string, functionality::string >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator<=( solve::Operand< functionality::string > & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_less_equal< functionality::boolean, functionality::string, functionality::string >( operand1, operand2 ) )->get_result();
  }

  // *******************************************
  // two_operands_operator mul
  // *******************************************

  template< class T_Result, class T_Operand1, class T_Operand2 >
  class _oc_mul
    : public solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 >
  {
  public:
    _oc_mul( solve::Operand< T_Operand1 > &operand1, solve::Operand< T_Operand2 > &operand2 )
      : solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 > 
        ( operand1, operand2 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & op1, const T_Operand2 & op2 )
    { // user code: ./solver.afd:162:16: 
      
      return op1 * op2;
    
    }
    virtual T_Result calc_result_from_op1( const T_Operand1 & op )
    { // user code: ./solver.afd:174:25: 
      
      return T_Result();		// return 0 in result Type
    
    }
    virtual T_Result calc_result_from_op2( const T_Operand2 & op )
    { // user code: ./solver.afd:178:25: 
      
      return T_Result();		// return 0 in result Type
    
    }
    virtual bool is_operand1_enough( const T_Operand1 & op )
    { // user code: ./solver.afd:166:23: 
      
      return !op;
    
    }
    virtual bool is_operand2_enough( const T_Operand2 & op )
    { // user code: ./solver.afd:170:23: 
      
      return !op;
    
    }
  };

  inline solve::Operand< functionality::scalar >& 
  operator*( solve::Operand< functionality::scalar > & operand1, const functionality::scalar & operand2 )
  {
    return (new _oc_mul< functionality::scalar, functionality::scalar, functionality::scalar >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  operator*( const functionality::scalar & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_mul< functionality::scalar, functionality::scalar, functionality::scalar >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  operator*( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_mul< functionality::scalar, functionality::scalar, functionality::scalar >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator*( solve::Operand< functionality::scalar > & operand1, const functionality::vector & operand2 )
  {
    return (new _oc_mul< functionality::vector, functionality::scalar, functionality::vector >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator*( const functionality::scalar & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_mul< functionality::vector, functionality::scalar, functionality::vector >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator*( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_mul< functionality::vector, functionality::scalar, functionality::vector >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator*( solve::Operand< functionality::scalar > & operand1, const functionality::matrix & operand2 )
  {
    return (new _oc_mul< functionality::matrix, functionality::scalar, functionality::matrix >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator*( const functionality::scalar & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_mul< functionality::matrix, functionality::scalar, functionality::matrix >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator*( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_mul< functionality::matrix, functionality::scalar, functionality::matrix >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  operator*( solve::Operand< functionality::vector > & operand1, const functionality::vector & operand2 )
  {
    return (new _oc_mul< functionality::scalar, functionality::vector, functionality::vector >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  operator*( const functionality::vector & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_mul< functionality::scalar, functionality::vector, functionality::vector >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  operator*( solve::Operand< functionality::vector > & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_mul< functionality::scalar, functionality::vector, functionality::vector >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  dot( solve::Operand< functionality::vector > & operand1, const functionality::vector & operand2 )
  {
    return (new _oc_mul< functionality::scalar, functionality::vector, functionality::vector >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  dot( const functionality::vector & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_mul< functionality::scalar, functionality::vector, functionality::vector >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  dot( solve::Operand< functionality::vector > & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_mul< functionality::scalar, functionality::vector, functionality::vector >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator*( solve::Operand< functionality::matrix > & operand1, const functionality::matrix & operand2 )
  {
    return (new _oc_mul< functionality::matrix, functionality::matrix, functionality::matrix >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator*( const functionality::matrix & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_mul< functionality::matrix, functionality::matrix, functionality::matrix >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator*( solve::Operand< functionality::matrix > & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_mul< functionality::matrix, functionality::matrix, functionality::matrix >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator*( solve::Operand< functionality::matrix > & operand1, const functionality::vector & operand2 )
  {
    return (new _oc_mul< functionality::vector, functionality::matrix, functionality::vector >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator*( const functionality::matrix & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_mul< functionality::vector, functionality::matrix, functionality::vector >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator*( solve::Operand< functionality::matrix > & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_mul< functionality::vector, functionality::matrix, functionality::vector >( operand1, operand2 ) )->get_result();
  }

  // *******************************************
  // one_operand_operator negative
  // *******************************************

  template< class T_Result, class T_Operand1 >
  class _oc_negative
    : public solve::Basic_Operator_for_1_Operand< T_Result, T_Operand1 >
  {
  public:
    _oc_negative( solve::Operand< T_Operand1 > &operand1 )
      : solve::Basic_Operator_for_1_Operand< T_Result, T_Operand1 > 
        ( operand1 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & val )
    { // user code: ./solver.afd:32:16: 
      
      return -val;
    
    }
  };

  inline solve::Operand< functionality::scalar >& 
  operator-( solve::Operand< functionality::scalar > & operand1 )
  {
    return (new _oc_negative< functionality::scalar, functionality::scalar >( operand1 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator-( solve::Operand< functionality::vector > & operand1 )
  {
    return (new _oc_negative< functionality::vector, functionality::vector >( operand1 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator-( solve::Operand< functionality::matrix > & operand1 )
  {
    return (new _oc_negative< functionality::matrix, functionality::matrix >( operand1 ) )->get_result();
  }

  // *******************************************
  // one_operand_operator normalize
  // *******************************************

  template< class T_Result, class T_Operand1 >
  class _oc_normalize
    : public solve::Basic_Operator_for_1_Operand< T_Result, T_Operand1 >
  {
  public:
    _oc_normalize( solve::Operand< T_Operand1 > &operand1 )
      : solve::Basic_Operator_for_1_Operand< T_Result, T_Operand1 > 
        ( operand1 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & vect )
    { // user code: ./solver.afd:112:16: 
      
      return normalize(vect);
    
    }
  };

  inline solve::Operand< functionality::vector >& 
  normalize( solve::Operand< functionality::vector > & operand1 )
  {
    return (new _oc_normalize< functionality::vector, functionality::vector >( operand1 ) )->get_result();
  }

  // *******************************************
  // one_operand_operator not
  // *******************************************

  template< class T_Result, class T_Operand1 >
  class _oc_not
    : public solve::Basic_Operator_for_1_Operand< T_Result, T_Operand1 >
  {
  public:
    _oc_not( solve::Operand< T_Operand1 > &operand1 )
      : solve::Basic_Operator_for_1_Operand< T_Result, T_Operand1 > 
        ( operand1 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & val )
    { // user code: ./solver.afd:13:16: 
      
      return !val;
    
    }
  };

  inline solve::Operand< functionality::boolean >& 
  operator!( solve::Operand< functionality::boolean > & operand1 )
  {
    return (new _oc_not< functionality::boolean, functionality::boolean >( operand1 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!( solve::Operand< functionality::scalar > & operand1 )
  {
    return (new _oc_not< functionality::boolean, functionality::scalar >( operand1 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!( solve::Operand< functionality::vector > & operand1 )
  {
    return (new _oc_not< functionality::boolean, functionality::vector >( operand1 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!( solve::Operand< functionality::matrix > & operand1 )
  {
    return (new _oc_not< functionality::boolean, functionality::matrix >( operand1 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!( solve::Operand< functionality::string > & operand1 )
  {
    return (new _oc_not< functionality::boolean, functionality::string >( operand1 ) )->get_result();
  }

  // *******************************************
  // two_operands_operator not_equal
  // *******************************************

  template< class T_Result, class T_Operand1, class T_Operand2 >
  class _oc_not_equal
    : public solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 >
  {
  public:
    _oc_not_equal( solve::Operand< T_Operand1 > &operand1, solve::Operand< T_Operand2 > &operand2 )
      : solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 > 
        ( operand1, operand2 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & op1, const T_Operand2 & op2 )
    { // user code: ./solver.afd:255:16: 
      
      return op1 != op2;
    
    }
  };

  inline solve::Operand< functionality::boolean >& 
  operator!=( solve::Operand< functionality::boolean > & operand1, const functionality::boolean & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::boolean, functionality::boolean >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( const functionality::boolean & operand1, solve::Operand< functionality::boolean > & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::boolean, functionality::boolean >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( solve::Operand< functionality::boolean > & operand1, solve::Operand< functionality::boolean > & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::boolean, functionality::boolean >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( solve::Operand< functionality::scalar > & operand1, const functionality::scalar & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::scalar, functionality::scalar >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( const functionality::scalar & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::scalar, functionality::scalar >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::scalar, functionality::scalar >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( solve::Operand< functionality::vector > & operand1, const functionality::vector & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::vector, functionality::vector >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( const functionality::vector & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::vector, functionality::vector >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( solve::Operand< functionality::vector > & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::vector, functionality::vector >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( solve::Operand< functionality::matrix > & operand1, const functionality::matrix & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::matrix, functionality::matrix >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( const functionality::matrix & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::matrix, functionality::matrix >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( solve::Operand< functionality::matrix > & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::matrix, functionality::matrix >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( solve::Operand< functionality::string > & operand1, const functionality::string & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::string, functionality::string >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( const functionality::string & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::string, functionality::string >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::boolean >& 
  operator!=( solve::Operand< functionality::string > & operand1, solve::Operand< functionality::string > & operand2 )
  {
    return (new _oc_not_equal< functionality::boolean, functionality::string, functionality::string >( operand1, operand2 ) )->get_result();
  }

  // *******************************************
  // one_operand_dual_solution_operator plus_minus
  // *******************************************

  template< class T_Result, class T_Operand1 >
  class _oc_plus_minus
    : public solve::Basic_Dual_Solution_Operator_for_1_Operand< T_Result, T_Operand1 >
  {
  public:
    _oc_plus_minus( solve::Operand< T_Operand1 > &operand1 )
      : solve::Basic_Dual_Solution_Operator_for_1_Operand< T_Result, T_Operand1 > 
        ( operand1 )
    {
      init();
    }
  private:
    virtual T_Result calc_result1( const T_Operand1 & val )
    { // user code: ./solver.afd:91:17: 
      
      return val;
    
    }
    virtual T_Result calc_result2( const T_Operand1 & val )
    { // user code: ./solver.afd:95:17: 
      
      return -val;
    
    }
  };

  inline solve::Operand< functionality::scalar >& 
  plus_minus( solve::Operand< functionality::scalar > & operand1 )
  {
    return (new _oc_plus_minus< functionality::scalar, functionality::scalar >( operand1 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  plus_minus( solve::Operand< functionality::vector > & operand1 )
  {
    return (new _oc_plus_minus< functionality::vector, functionality::vector >( operand1 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  plus_minus( solve::Operand< functionality::matrix > & operand1 )
  {
    return (new _oc_plus_minus< functionality::matrix, functionality::matrix >( operand1 ) )->get_result();
  }

  // *******************************************
  // one_operand_operator sqrt
  // *******************************************

  template< class T_Result, class T_Operand1 >
  class _oc_sqrt
    : public solve::Basic_Operator_for_1_Operand< T_Result, T_Operand1 >
  {
  public:
    _oc_sqrt( solve::Operand< T_Operand1 > &operand1 )
      : solve::Basic_Operator_for_1_Operand< T_Result, T_Operand1 > 
        ( operand1 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & val )
    { // user code: ./solver.afd:65:16: 
      
      return sqrt(val);
    
    }
    virtual bool is_operand_ok( const T_Operand1 & op, solve::Solve_Run_Info* info )
    { // user code: ./solver.afd:69:18: 
      
      // value under square root must be positive
      if( op < 0 )
      {
	if( !info->is_trial_run() )
	error() << "cannot calculate square root of " << op;
	return false;
      }
      return true;
    
    }
  };

  inline solve::Operand< functionality::scalar >& 
  sqrt( solve::Operand< functionality::scalar > & operand1 )
  {
    return (new _oc_sqrt< functionality::scalar, functionality::scalar >( operand1 ) )->get_result();
  }

  // *******************************************
  // two_operands_operator sub
  // *******************************************

  template< class T_Result, class T_Operand1, class T_Operand2 >
  class _oc_sub
    : public solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 >
  {
  public:
    _oc_sub( solve::Operand< T_Operand1 > &operand1, solve::Operand< T_Operand2 > &operand2 )
      : solve::Basic_Operator_for_2_Operands< T_Result, T_Operand1, T_Operand2 > 
        ( operand1, operand2 )
    {
      init();
    }
  private:
    virtual T_Result calc_result( const T_Operand1 & op1, const T_Operand2 & op2 )
    { // user code: ./solver.afd:145:16: 
      
      return op1 - op2;
    
    }
  };

  inline solve::Operand< functionality::scalar >& 
  operator-( solve::Operand< functionality::scalar > & operand1, const functionality::scalar & operand2 )
  {
    return (new _oc_sub< functionality::scalar, functionality::scalar, functionality::scalar >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  operator-( const functionality::scalar & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_sub< functionality::scalar, functionality::scalar, functionality::scalar >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::scalar >& 
  operator-( solve::Operand< functionality::scalar > & operand1, solve::Operand< functionality::scalar > & operand2 )
  {
    return (new _oc_sub< functionality::scalar, functionality::scalar, functionality::scalar >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator-( solve::Operand< functionality::vector > & operand1, const functionality::vector & operand2 )
  {
    return (new _oc_sub< functionality::vector, functionality::vector, functionality::vector >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator-( const functionality::vector & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_sub< functionality::vector, functionality::vector, functionality::vector >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::vector >& 
  operator-( solve::Operand< functionality::vector > & operand1, solve::Operand< functionality::vector > & operand2 )
  {
    return (new _oc_sub< functionality::vector, functionality::vector, functionality::vector >( operand1, operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator-( solve::Operand< functionality::matrix > & operand1, const functionality::matrix & operand2 )
  {
    return (new _oc_sub< functionality::matrix, functionality::matrix, functionality::matrix >( operand1, solve::const_op( operand2, operand1.get_consultant() ) ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator-( const functionality::matrix & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_sub< functionality::matrix, functionality::matrix, functionality::matrix >( solve::const_op( operand1, operand2.get_consultant() ), operand2 ) )->get_result();
  }

  inline solve::Operand< functionality::matrix >& 
  operator-( solve::Operand< functionality::matrix > & operand1, solve::Operand< functionality::matrix > & operand2 )
  {
    return (new _oc_sub< functionality::matrix, functionality::matrix, functionality::matrix >( operand1, operand2 ) )->get_result();
  }

}
namespace functionality
{

  // ***************************
  // tree node type declarations
  // ***************************

  // *****************************
  // make nodes availible 

  void make_solver_nodes_availible();
}
#endif

/*****************************************************************************/
/**   This file offers functions that tell availible basic operators	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#include "base_operators.hpp"

namespace funcgen
{
  std::string type_to_string( type t )
  {
    switch( t )
    {
    case none:		return "none";
    case boolean:	return "boolean";
    case result:	return "result";
    case operand:	return "operand";
    case operand1:	return "operand1";
    case operand2:	return "operand2";
    case operand3:	return "operand3";
    case operand4:	return "operand4";
    case operand5:	return "operand5";
    case operand6:	return "operand6";
    case operand7:	return "operand7";
    case operand8:	return "operand8";
    case operand9:	return "operand9";
    case info:		return "info";
    }
    assert(0);
    return "";
  }

  bool Basic_Operator::is_function( std::string function ) const
  {
    return functions.find( function ) != functions.end();
  }

  //! get parameter types of function (first is return type)
  const std::list<type> &Basic_Operator::get_types 
  ( std::string function ) const
  {
    Basic_Operator *new_this = const_cast<Basic_Operator*>(this);
    return new_this->functions[function];
  }
  const std::list<std::string> &Basic_Operator::get_required_functions()
    const
  {
    return required_functions;
  }
  std::string Basic_Operator::get_real_name()
    const
  {
    return real_name;
  }
  int Basic_Operator::get_num_operands() const
  {
    return num_operands;
  }
  std::string Basic_Operator::get_function_specification( std::string 
							  function ) const
  {
    Basic_Operator *new_this = const_cast<Basic_Operator*>(this);
    return new_this->function_specification[function];
  }

  void Basic_Operator::add_1_arg_fun( type return_type, std::string name, 
				 type arg1 )
  {
    std::list<type> &args = functions[name];
    args.push_back( return_type );
    args.push_back( arg1 );
    function_specification[name] 
      = type_to_string(return_type) + " " 
      + name + "( " + type_to_string(arg1) + " )";
  }
  void Basic_Operator::add_2_arg_fun( type return_type, std::string name, 
				 type arg1, type arg2 )
  {
    std::list<type> &args = functions[name];
    args.push_back( return_type );
    args.push_back( arg1 );
    args.push_back( arg2 );
    function_specification[name] 
      = type_to_string(return_type) + " " + name + "( " 
      + type_to_string(arg1) + ", " + type_to_string(arg2) + " )";
  }
  void Basic_Operator::add_3_arg_fun( type return_type, std::string name, 
				 type arg1, type arg2, type arg3 )
  {
    std::list<type> &args = functions[name];
    args.push_back( return_type );
    args.push_back( arg1 );
    args.push_back( arg2 );
    args.push_back( arg3 );
    function_specification[name] 
      = type_to_string(return_type) + " " + name + "( " + type_to_string(arg1)
      + ", " + type_to_string(arg2) + ", " + type_to_string(arg3) + " )";
  }
  void Basic_Operator::add_4_arg_fun( type return_type, std::string name, 
				 type arg1, type arg2, type arg3, type arg4 )
  {
    std::list<type> &args = functions[name];
    args.push_back( return_type );
    args.push_back( arg1 );
    args.push_back( arg2 );
    args.push_back( arg3 );
    args.push_back( arg4 );
    function_specification[name] 
      = type_to_string(return_type) + " " + name + "( " + type_to_string(arg1)
      + ", " + type_to_string(arg2) + ", " + type_to_string(arg3) + ", " 
      + type_to_string(arg4) + " )";
  }
  void Basic_Operator::add_5_arg_fun( type return_type, std::string name, 
				 type arg1, type arg2, type arg3, type arg4, type arg5 )
  {
    std::list<type> &args = functions[name];
    args.push_back( return_type );
    args.push_back( arg1 );
    args.push_back( arg2 );
    args.push_back( arg3 );
    args.push_back( arg4 );
    args.push_back( arg5 );
    function_specification[name] 
      = type_to_string(return_type) + " " + name + "( " + type_to_string(arg1)
      + ", " + type_to_string(arg2) + ", " + type_to_string(arg3) + ", " 
      + type_to_string(arg4) + ", " + type_to_string(arg5) + " )";
  }

  bool Available_Basic_Operators::is_operator( std::string operator_name ) 
    const
  {
    return basic_operators.find(operator_name) != basic_operators.end();
  }
  const Basic_Operator &Available_Basic_Operators::get_operator
  ( std::string basic_operator)
  {
    return basic_operators[basic_operator];
  }
  Available_Basic_Operators::Available_Basic_Operators()
  {
    {
      Basic_Operator &cur_operator = basic_operators["one_operand_operator"];
      cur_operator.real_name = "Basic_Operator_for_1_Operand";
      cur_operator.num_operands = 1;
      cur_operator.add_2_arg_fun( boolean, "is_operand_ok", operand, info );
      cur_operator.add_1_arg_fun( boolean, "is_operand_enough", operand );
      cur_operator.add_1_arg_fun( result, "calc_result", operand );
      cur_operator.required_functions.push_back("calc_result");
    }
    {
      Basic_Operator &cur_operator 
	= basic_operators["one_operand_dual_solution_operator"];
      cur_operator.real_name = "Basic_Dual_Solution_Operator_for_1_Operand";
      cur_operator.num_operands = 1;
      cur_operator.add_2_arg_fun( boolean, "is_operand_ok", operand, info );
      cur_operator.add_1_arg_fun( boolean, "is_operand_enough1", operand );
      cur_operator.add_1_arg_fun( boolean, "is_operand_enough2", operand );
      cur_operator.add_1_arg_fun( result, "calc_result1", operand );
      cur_operator.add_1_arg_fun( result, "calc_result2", operand );
      cur_operator.required_functions.push_back("calc_result1");
      cur_operator.required_functions.push_back("calc_result2");
    }
    {
      Basic_Operator &cur_operator = basic_operators["two_operands_operator"];
      cur_operator.real_name = "Basic_Operator_for_2_Operands";
      cur_operator.num_operands = 2;
      cur_operator.add_2_arg_fun( result, "calc_result", operand1, operand2 );
      cur_operator.add_2_arg_fun( boolean, "is_operand1_ok", operand1, info );
      cur_operator.add_2_arg_fun( boolean, "is_operand2_ok", operand2, info );
      cur_operator.add_1_arg_fun( boolean, "is_operand1_enough", operand1 );
      cur_operator.add_1_arg_fun( boolean, "is_operand2_enough", operand2 );
      cur_operator.add_3_arg_fun( boolean, "are_operands_ok", operand1, operand2, info );
      cur_operator.add_2_arg_fun( boolean, "are_operands_enough", operand1, operand2 );
      cur_operator.add_1_arg_fun( result, "calc_result_from_op1", operand1 );
      cur_operator.add_1_arg_fun( result, "calc_result_from_op2", operand2 );
      cur_operator.required_functions.push_back("calc_result");
    }
    {
      Basic_Operator &cur_operator 
	= basic_operators["simple_three_operands_operator"];
      cur_operator.real_name = "Basic_Simple_Operator_for_3_Operands";
      cur_operator.num_operands = 3;
      cur_operator.add_3_arg_fun( result, "calc_result", 
				  operand1, operand2, operand3 );
      cur_operator.add_2_arg_fun( boolean, "is_operand1_ok", operand1, info );
      cur_operator.add_2_arg_fun( boolean, "is_operand2_ok", operand2, info );
      cur_operator.add_2_arg_fun( boolean, "is_operand3_ok", operand3, info );
      cur_operator.add_4_arg_fun( boolean, "are_operands_ok", 
				  operand1, operand2, operand3, info );
      cur_operator.add_3_arg_fun( boolean, "are_operands_enough", 
				  operand1, operand2, operand3 );
      cur_operator.required_functions.push_back("calc_result");
    }
    {
      Basic_Operator &cur_operator 
	= basic_operators["simple_four_operands_operator"];
      cur_operator.real_name = "Basic_Simple_Operator_for_4_Operands";
      cur_operator.num_operands = 4;
      cur_operator.add_4_arg_fun( result, "calc_result", 
				  operand1, operand2, operand3, operand4 );
      cur_operator.add_2_arg_fun( boolean, "is_operand1_ok", operand1, info );
      cur_operator.add_2_arg_fun( boolean, "is_operand2_ok", operand2, info );
      cur_operator.add_2_arg_fun( boolean, "is_operand3_ok", operand3, info );
      cur_operator.add_2_arg_fun( boolean, "is_operand4_ok", operand4, info );
      cur_operator.add_5_arg_fun( boolean, "are_operands_ok", 
				  operand1, operand2, operand3, operand4, info);
      cur_operator.add_4_arg_fun( boolean, "are_operands_enough", 
				  operand1, operand2, operand3, operand4 );
      cur_operator.required_functions.push_back("calc_result");
    }
  }
}

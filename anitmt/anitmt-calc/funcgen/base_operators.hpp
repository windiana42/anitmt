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

#ifndef __Funcgen_Basic_Operators__
#define __Funcgen_Basic_Operators__

#include <map>
#include <set>
#include <list>
#include <string>

namespace funcgen
{
  enum op_par_type{ none, boolean, result, operand, operand1, operand2, 
		    operand3, operand4, operand5, operand6, operand7, 
		    operand8, operand9, info };
  
  class Basic_Operator
  {
  public:
    bool is_function( std::string function ) const;
    //! get parameter types of function (first is return type)
    const std::list<op_par_type> &get_types( std::string function ) const;
    //! get functions that are required
    const std::list<std::string> &get_required_functions() const;
    //! get real class name of the operator
    std::string get_real_name() const;
    //! get number of operands
    int get_num_operands() const;
    //! get type specifications of function
    std::string get_function_specification( std::string function ) const;
  private:
    friend class Available_Basic_Operators;
    std::map< std::string, std::list<op_par_type> > functions;
    std::list< std::string > required_functions;
    std::string real_name;
    std::map< std::string, std::string > function_specification;
    int num_operands;
    void add_1_arg_fun( op_par_type return_type, std::string name, 
			op_par_type arg1 );
    void add_2_arg_fun( op_par_type return_type, std::string name, 
			op_par_type arg1, 
			op_par_type arg2 );
    void add_3_arg_fun( op_par_type return_type, std::string name, 
			op_par_type arg1, op_par_type arg2, op_par_type arg3 );
    void add_4_arg_fun( op_par_type return_type, std::string name, 
			op_par_type arg1, op_par_type arg2, op_par_type arg3, 
			op_par_type arg4 );
    void add_5_arg_fun( op_par_type return_type, std::string name, 
			op_par_type arg1, op_par_type arg2, op_par_type arg3, 
			op_par_type arg4, op_par_type arg5 );
  };
  class Available_Basic_Operators
  {
  public:
    Available_Basic_Operators();

    bool is_operator( std::string basic_operator ) const;
    const Basic_Operator &get_operator( std::string basic_operator );
  private:
    std::map<std::string,Basic_Operator> basic_operators;
  };
}

#endif

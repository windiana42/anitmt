/*****************************************************************************/
/**   This file offers functions to handle expressions within the parser    **/
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

#ifndef __AniTMT_parser_expression__
#define __AniTMT_parser_expression__

#include <val/val.hpp>
#include <solve/operand.hpp>

#include <fstream>

namespace anitmt
{
  namespace adlparser
  {
    class Any_Type : public message::Message_Reporter
    {
    public:
      Any_Type( message::Message_Consultant* );
      Any_Type( values::Flag   x, message::Message_Consultant* );
      Any_Type( values::Scalar x, message::Message_Consultant* );
      Any_Type( values::Vector x, message::Message_Consultant* );
      Any_Type( values::Matrix x, message::Message_Consultant* );
      Any_Type( values::String x, message::Message_Consultant* );
      Any_Type( solve::Operand<values::Flag>   &x, 
		message::Message_Consultant* );
      Any_Type( solve::Operand<values::Scalar> &x, 
		message::Message_Consultant* );
      Any_Type( solve::Operand<values::Vector> &x, 
		message::Message_Consultant* );
      Any_Type( solve::Operand<values::Matrix> &x, 
		message::Message_Consultant* );
      Any_Type( solve::Operand<values::String> &x, 
		message::Message_Consultant* );

      Any_Type( const Any_Type& ); 
      Any_Type( Any_Type& ); // empties original value

      Any_Type &operator=( const Any_Type &op ); 
      Any_Type &operator=( Any_Type &op ); // empties original value

      inline bool has_value() const			{ return value; }
      inline values::Valtype::Types get_type() const	{ return type; }
      inline bool is_operand() const			{ return operand; }

      inline values::Flag   get_flag()   const;
      inline values::Scalar get_scalar() const;
      inline values::Vector get_vector() const;
      inline values::Matrix get_matrix() const;
      inline values::String get_string() const;
      inline solve::Operand<values::Flag>   &get_op_flag()   const;
      inline solve::Operand<values::Scalar> &get_op_scalar() const;
      inline solve::Operand<values::Vector> &get_op_vector() const;
      inline solve::Operand<values::Matrix> &get_op_matrix() const;
      inline solve::Operand<values::String> &get_op_string() const;

      ~Any_Type();
    private: 
      union
      {
        values::Flag 	*flag;
        values::Scalar 	*scalar;
        values::Vector 	*vector;
        values::Matrix	*matrix;
        values::String 	*string;
        solve::Operand<values::Flag>  	*op_flag;
        solve::Operand<values::Scalar>	*op_scalar;
        solve::Operand<values::Vector>	*op_vector;
        solve::Operand<values::Matrix>	*op_matrix;
        solve::Operand<values::String>	*op_string;
      } u;

      bool value;
      values::Valtype::Types type;
      bool operand;

      void empty();
    };

    Any_Type operator+( const Any_Type &op1, const Any_Type &op2 );
    Any_Type function( std::string name, std::list<Any_Type> ops );

    std::ostream &operator<<( std::ostream &os, const Any_Type &val );

    class Function_Instance
    {
    public:
      Function_Instance( std::string name,
			 std::list<values::Valtype::Types> parameter_types );
 
      bool is_instance( std::list<Any_Type> ops );

      virtual Any_Type call_function( std::list<Any_Type> ops,
				      message::Message_Reporter& msg );

      virtual ~Function_Instance();
    private:
      std::string name;
      std::list<values::Valtype::Types> parameter_types;

      friend std::ostream &operator<<( std::ostream &os, 
				       const Function_Instance & );
    };

    std::ostream &operator<<( std::ostream &os, const Function_Instance & );

    class Function_sqrt_scalar : public Function_Instance
    {
    public:
      Function_sqrt_scalar();
      virtual Any_Type call_function( std::list<Any_Type> ops,
				      message::Message_Reporter& msg );
    };

    class Function_dot_vecvec : public Function_Instance
    {
    public:
      Function_dot_vecvec();
      virtual Any_Type call_function( std::list<Any_Type> ops,
				      message::Message_Reporter& msg );
    };
    
    class Function_Handler
    {
    public:
      Function_Handler();
      Any_Type call_function( std::string function, 
			      std::list<Any_Type> ops,
			      message::Message_Reporter& );

      // instance will be deleted...
      void add_function_instance( std::string name, 
				  Function_Instance * );
      ~Function_Handler();
    private:
      std::map< std::string,std::list<Function_Instance*> > instances;
    };

  }
}

#include "expression_inline.cpp"

#endif

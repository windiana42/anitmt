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

#ifndef __AniTMT_parser_expression_inline__
#define __AniTMT_parser_expression_inline__

#include "expression.hpp"

#include <assert.h>

namespace anitmt
{
  namespace adlparser
  {
    values::Flag   Any_Type::get_flag() const
    {
      assert( value == true );
      assert( type == values::Valtype::flag );
      assert( operand == false );

      return *u.flag;
    }
    values::Scalar Any_Type::get_scalar() const
    {
      assert( value == true );
      assert( type == values::Valtype::scalar );
      assert( operand == false );

      return *u.scalar;
    }
    values::Vector Any_Type::get_vector() const
    {
      assert( value == true );
      assert( type == values::Valtype::vector );
      assert( operand == false );

      return *u.vector;
    }
    values::Matrix Any_Type::get_matrix() const
    {
      assert( value == true );
      assert( type == values::Valtype::matrix );
      assert( operand == false );

      return *u.matrix;
    }
    values::String Any_Type::get_string() const
    {
      assert( value == true );
      assert( type == values::Valtype::string );
      assert( operand == false );

      return *u.string;
    }
    solve::Operand<values::Flag>   &Any_Type::get_op_flag() const
    {
      assert( value == true );
      assert( type == values::Valtype::flag );
      assert( operand == true );

      return *u.op_flag;
    }
    solve::Operand<values::Scalar> &Any_Type::get_op_scalar() const
    {
      assert( value == true );
      assert( type == values::Valtype::scalar );
      assert( operand == true );

      return *u.op_scalar;
    }
    solve::Operand<values::Vector> &Any_Type::get_op_vector() const
    {
      assert( value == true );
      assert( type == values::Valtype::vector );
      assert( operand == true );

      return *u.op_vector;
    }
    solve::Operand<values::Matrix> &Any_Type::get_op_matrix() const
    {
      assert( value == true );
      assert( type == values::Valtype::matrix );
      assert( operand == true );

      return *u.op_matrix;
    }
    solve::Operand<values::String> &Any_Type::get_op_string() const
    {
      assert( value == true );
      assert( type == values::Valtype::string );
      assert( operand == true );

      return *u.op_string;
    }
  }
}

#endif

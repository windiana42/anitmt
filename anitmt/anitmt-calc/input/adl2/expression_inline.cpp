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

#ifdef EXTREME_INLINE
#define _INLINE_ inline
#else
#define _INLINE_
#endif

namespace anitmt
{
  namespace adlparser
  {
    _INLINE_ values::Flag   Any_Type::get_flag() const
    {
      assert( value == true );
      assert( type == values::Valtype::flag );
      assert( operand == false );

      return *u.flag;
    }
    _INLINE_ values::Scalar Any_Type::get_scalar() const
    {
      assert( value == true );
      assert( type == values::Valtype::scalar );
      assert( operand == false );

      return *u.scalar;
    }
    _INLINE_ values::Vector Any_Type::get_vector() const
    {
      assert( value == true );
      assert( type == values::Valtype::vector );
      assert( operand == false );

      return *u.vector;
    }
    _INLINE_ values::Matrix Any_Type::get_matrix() const
    {
      assert( value == true );
      assert( type == values::Valtype::matrix );
      assert( operand == false );

      return *u.matrix;
    }
    _INLINE_ values::String Any_Type::get_string() const
    {
      assert( value == true );
      assert( type == values::Valtype::string );
      assert( operand == false );

      return *u.string;
    }
    _INLINE_ solve::Operand<values::Flag>   &Any_Type::get_op_flag() const
    {
      assert( value == true );
      assert( type == values::Valtype::flag );
      assert( operand == true );

      return *u.op_flag;
    }
    _INLINE_ solve::Operand<values::Scalar> &Any_Type::get_op_scalar() const
    {
      assert( value == true );
      assert( type == values::Valtype::scalar );
      assert( operand == true );

      return *u.op_scalar;
    }
    _INLINE_ solve::Operand<values::Vector> &Any_Type::get_op_vector() const
    {
      assert( value == true );
      assert( type == values::Valtype::vector );
      assert( operand == true );

      return *u.op_vector;
    }
    _INLINE_ solve::Operand<values::Matrix> &Any_Type::get_op_matrix() const
    {
      assert( value == true );
      assert( type == values::Valtype::matrix );
      assert( operand == true );

      return *u.op_matrix;
    }
    _INLINE_ solve::Operand<values::String> &Any_Type::get_op_string() const
    {
      assert( value == true );
      assert( type == values::Valtype::string );
      assert( operand == true );

      return *u.op_string;
    }
  }
}
#undef _INLINE_

#endif

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

#include "expression.hpp"

#include <functionality/solver.hpp>
#include <assert.h>

namespace anitmt
{
  namespace adlparser
  {
    Any_Type::Any_Type( message::Message_Consultant* c )
      : message::Message_Reporter(c), value(false)
    {}

    Any_Type::Any_Type( values::Flag   x, message::Message_Consultant* c )
      : message::Message_Reporter(c), value(true), 
	type(values::Valtype::flag), operand(false)
    {
      u.flag = new values::Flag(x);
    }

    Any_Type::Any_Type( values::Scalar x, message::Message_Consultant* c )
      : message::Message_Reporter(c), value(true), 
	type(values::Valtype::scalar), operand(false)
    {
      u.scalar = new values::Scalar(x);
    }

    Any_Type::Any_Type( values::Vector x, message::Message_Consultant* c )
      : message::Message_Reporter(c), value(true), 
	type(values::Valtype::vector), operand(false)
    {
      u.vector = new values::Vector(x);
    }

    Any_Type::Any_Type( values::Matrix x, message::Message_Consultant* c )
      : message::Message_Reporter(c), value(true), 
	type(values::Valtype::matrix), operand(false)
    {
      u.matrix = new values::Matrix(x);
    }

    Any_Type::Any_Type( values::String x, message::Message_Consultant* c )
      : message::Message_Reporter(c), value(true), 
	type(values::Valtype::string), operand(false)
    {
      u.string = new values::String(x);
    }

    Any_Type::Any_Type( solve::Operand<values::Flag>   &x, 
			message::Message_Consultant* c )
      : message::Message_Reporter(c), value(true), 
	type(values::Valtype::flag), operand(true)
    {
      u.op_flag = &x;
    }

    Any_Type::Any_Type( solve::Operand<values::Scalar> &x, 
			message::Message_Consultant* c )
      : message::Message_Reporter(c), value(true), 
	type(values::Valtype::scalar), operand(true)
    {
      u.op_scalar = &x;
    }

    Any_Type::Any_Type( solve::Operand<values::Vector> &x, 
			message::Message_Consultant* c )
      : message::Message_Reporter(c), value(true), 
	type(values::Valtype::vector), operand(true)
    {
      u.op_vector = &x;
    }

    Any_Type::Any_Type( solve::Operand<values::Matrix> &x, 
			message::Message_Consultant* c )
      : message::Message_Reporter(c), value(true), 
	type(values::Valtype::matrix), operand(true)
    {
      u.op_matrix = &x;
    }

    Any_Type::Any_Type( solve::Operand<values::String> &x, 
			message::Message_Consultant* c )
      : message::Message_Reporter(c), value(true), 
	type(values::Valtype::string), operand(true)
    {
      u.op_string = &x;
    }

    Any_Type::Any_Type( const Any_Type& val )
      : message::Message_Reporter( val.get_consultant() )
    {
      empty();

      value = val.value;
      type = val.type;
      operand = val.operand;

      switch( val.get_type() )
      {
      case values::Valtype::flag:
	u.flag = new values::Flag( *val.u.flag );
	break;
      case values::Valtype::scalar:
	u.scalar = new values::Scalar( *val.u.scalar );
	break;
      case values::Valtype::vector:
	u.vector = new values::Vector( *val.u.vector );
	break;
      case values::Valtype::matrix:
	u.matrix = new values::Matrix( *val.u.matrix );
	break;
      case values::Valtype::string:
	u.string = new values::String( *val.u.string );
	break;
      case values::Valtype::neutral0: assert( false );
      case values::Valtype::neutral1: assert( false );
      }
    }

    Any_Type::Any_Type( Any_Type& val )
      : message::Message_Reporter( val.get_consultant() )
    {
      empty();

      value = val.value;
      type = val.type;
      operand = val.operand;

      val.empty();		// remove original value
    }

    Any_Type &Any_Type::operator=( const Any_Type &val )
    {
      empty();

      value = val.value;
      type = val.type;
      operand = val.operand;

      switch( val.get_type() )
      {
      case values::Valtype::flag:
	u.flag = new values::Flag( *val.u.flag );
	break;
      case values::Valtype::scalar:
	u.scalar = new values::Scalar( *val.u.scalar );
	break;
      case values::Valtype::vector:
	u.vector = new values::Vector( *val.u.vector );
	break;
      case values::Valtype::matrix:
	u.matrix = new values::Matrix( *val.u.matrix );
	break;
      case values::Valtype::string:
	u.string = new values::String( *val.u.string );
	break;
      case values::Valtype::neutral0: assert( false );
      case values::Valtype::neutral1: assert( false );
      }
      return *this;
    }
    Any_Type &Any_Type::operator=( Any_Type &val )
    {
      empty();

      value = val.value;
      type = val.type;
      operand = val.operand;

      val.empty();		// remove original value
      return *this;
    }

    Any_Type::~Any_Type()
    {
      empty();
    }

    void Any_Type::empty()
    {
      if( value )
      {
	if( !operand ) 
	{
	  switch( type )
	  {
	  case values::Valtype::flag:
	    delete u.flag;
	    break;
	  case values::Valtype::scalar:
	    delete u.scalar;
	    break;
	  case values::Valtype::vector:
	    delete u.vector;
	    break;
	  case values::Valtype::matrix:
	    delete u.matrix;
	    break;
	  case values::Valtype::string:
	    delete u.string;
	    break;
	  case values::Valtype::neutral0: assert( false );
	  case values::Valtype::neutral1: assert( false );
	  }
	  value = false;
	}
      }
    }

    Any_Type function( std::string name, std::list<Any_Type> ops )
    {
      assert( ops.size() >= 1 );
      message::Message_Consultant *c = ops.front().get_consultant();

      if( name == "sqrt" )
      {
	Any_Type &op1 = ops.front();
	switch( ops.size() )
	{
	case 1:
	  switch( op1.get_type() )
	  {
	  case values::Valtype::flag:
	    op1.error() 
	      << "scalar expected for call to \"sqrt(scalar)\", "
	      << "but flag found instead";
	    break;
	  case values::Valtype::scalar:
	    if( op1.is_operand() )
	      return Any_Type( sqrt( op1.get_op_scalar() ), c );
	    else
	      return Any_Type( values::Scalar( sqrt( op1.get_scalar() ) ), c );
	  case values::Valtype::vector:
	    op1.error() 
	      << "scalar expected for call to \"sqrt(scalar)\", "
	      << "but vector found instead";
	    break;
	  case values::Valtype::matrix:
	    op1.error() 
	      << "scalar expected for call to \"sqrt(scalar)\", "
	      << "but matrix found instead";
	    break;
	  case values::Valtype::string:
	    op1.error() 
	      << "scalar expected for call to \"sqrt(scalar)\", "
	      << "but string found instead";
	    break;
	  case values::Valtype::neutral0: assert( false );
	  case values::Valtype::neutral1: assert( false );
	  }
	default:
	  op1.error() 
	    << "Wrong number of arguments for call to \"sqrt(scalar)\"";
	}
      }
    }

    Any_Type operator+( const Any_Type &op1, const Any_Type &op2 )
    {
      message::Message_Consultant *c = op1.get_consultant();

      if( !op1.has_value() ) return Any_Type(c);
      if( !op2.has_value() ) return Any_Type(c);

      switch( op1.get_type() )
      {
      case values::Valtype::flag:
	op1.error() << "cannot add anything to a conditional value";
	op1.error() << "invalid first argument to operator '+'";
	break;
      case values::Valtype::scalar:
	switch( op2.get_type() )
	{
	case values::Valtype::flag:
	  op2.error() << "cannot add conditional value to scalar";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::scalar:
	  if(  op1.is_operand() &&  op2.is_operand() )
	    return Any_Type( op1.get_op_scalar() + op2.get_op_scalar(), c );
	  if( !op1.is_operand() &&  op2.is_operand() )
	    return Any_Type( op1.get_scalar() + op2.get_op_scalar(), c );
	  if(  op1.is_operand() && !op2.is_operand() )
	    return Any_Type( op1.get_op_scalar() + op2.get_scalar(), c );
	  if( !op1.is_operand() && !op2.is_operand() )
	    return 
	      Any_Type( values::Scalar(op1.get_scalar() + op2.get_scalar()), 
			c );
	  break;
	case values::Valtype::vector:
	  op2.error() << "cannot add vector to scalar";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::matrix:
	  op2.error() << "cannot add matrix to scalar";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::string:
	  op2.error() << "cannot add string to scalar";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::neutral0: assert( false );
	case values::Valtype::neutral1: assert( false );
	}
	break;
      case values::Valtype::vector:
	switch( op2.get_type() )
	{
	case values::Valtype::flag:
	  op2.error() << "cannot add conditional value to vector";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::scalar:
	  op2.error() << "cannot add scalar to vector";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::vector:
	  if(  op1.is_operand() &&  op2.is_operand() )
	    return Any_Type( op1.get_op_vector() + op2.get_op_vector(), c );
	  if( !op1.is_operand() &&  op2.is_operand() )
	    return Any_Type( op1.get_vector() + op2.get_op_vector(), c );
	  if(  op1.is_operand() && !op2.is_operand() )
	    return Any_Type( op1.get_op_vector() + op2.get_vector(), c );
	  if( !op1.is_operand() && !op2.is_operand() )
	    return Any_Type( op1.get_vector() + op2.get_vector(), c );
	  break;
	case values::Valtype::matrix:
	  op2.error() << "cannot add matrix to vector";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::string:
	  op2.error() << "cannot add string to vector";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::neutral0: assert( false );
	case values::Valtype::neutral1: assert( false );
	}
	break;
      case values::Valtype::matrix:
	switch( op2.get_type() )
	{
	case values::Valtype::flag:
	  op2.error() << "cannot add conditional value to matrix";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::scalar:
	  op2.error() << "cannot add scalar to matrix";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::vector:
	  op2.error() << "cannot add vector to matrix";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::matrix:
	  if(  op1.is_operand() &&  op2.is_operand() )
	    return Any_Type( op1.get_op_matrix() + op2.get_op_matrix(), c );
	  if( !op1.is_operand() &&  op2.is_operand() )
	    return Any_Type( op1.get_matrix() + op2.get_op_matrix(), c );
	  if(  op1.is_operand() && !op2.is_operand() )
	    return Any_Type( op1.get_op_matrix() + op2.get_matrix(), c );
	  if( !op1.is_operand() && !op2.is_operand() )
	    return Any_Type( op1.get_matrix() + op2.get_matrix(), c );
	  break;
	case values::Valtype::string:
	  op2.error() << "cannot add string to matrix";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::neutral0: assert( false );
	case values::Valtype::neutral1: assert( false );
	}
	break;
      case values::Valtype::string:
	switch( op2.get_type() )
	{
	case values::Valtype::flag:
	  op2.error() << "cannot add conditional value to string";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::scalar:
	  op2.error() << "cannot add scalar to string";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::vector:
	  op2.error() << "cannot add vector to string";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::matrix:
	  op2.error() << "cannot add matrix to string";
	  op2.error() << "invalid second argument to operator '+'";
	  break;
	case values::Valtype::string:
	  if(  op1.is_operand() &&  op2.is_operand() )
	    return Any_Type( op1.get_op_string() + op2.get_op_string(), c );
	  if( !op1.is_operand() &&  op2.is_operand() )
	    return Any_Type( op1.get_string() + op2.get_op_string(), c );
	  if(  op1.is_operand() && !op2.is_operand() )
	    return Any_Type( op1.get_op_string() + op2.get_string(), c );
	  if( !op1.is_operand() && !op2.is_operand() )
	    return Any_Type( op1.get_string() + op2.get_string(), c );
	  break;
	case values::Valtype::neutral0: assert( false );
	case values::Valtype::neutral1: assert( false );
	}
	break;
      case values::Valtype::neutral0: assert( false );
      case values::Valtype::neutral1: assert( false );
      }
      return Any_Type(c);
    }

    std::ostream &operator<<( std::ostream &os, const Any_Type &val )
    {
      if( val.has_value() )
      {
	if( !val.is_operand() )
	{
	  switch( val.get_type() )
	  {
	  case values::Valtype::flag:
	    os << val.get_flag();
	    break;
	  case values::Valtype::scalar:
	    os << val.get_scalar();
	    break;
	  case values::Valtype::vector:
	    os << val.get_vector();
	    break;
	  case values::Valtype::matrix:
	    os << val.get_matrix();
	    break;
	  case values::Valtype::string:
	    os << val.get_string();
	    break;
	  case values::Valtype::neutral0: assert( false );
	  case values::Valtype::neutral1: assert( false );
	  }
	}
	else
	{
	  switch( val.get_type() )
	  {
	  case values::Valtype::flag:
	    os << val.get_op_flag();
	    break;
	  case values::Valtype::scalar:
	    os << val.get_op_scalar();
	    break;
	  case values::Valtype::vector:
	    os << val.get_op_vector();
	    break;
	  case values::Valtype::matrix:
	    os << val.get_op_matrix();
	    break;
	  case values::Valtype::string:
	    os << val.get_op_string();
	    break;
	  case values::Valtype::neutral0: assert( false );
	  case values::Valtype::neutral1: assert( false );
	  }
	}
      }
      else
      {
	os << "<no value>";
      }
      return os;
    }

  }
}

/*****************************************************************************/
/**   This file offers a complex token class for storing all types          **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#ifndef __anitmt_input_adl_token_inline_implementation__
#define __anitmt_input_adl_token_inline_implementation__

#include "token.hpp"

#include "parsinfo.hpp"
#include "tokens.h"		// a lot of declarations TOK_...

namespace anitmt
{
  namespace adlparser
  {
    //*********************************************************************
    // Meta_Operand: stores operands and allows assigning operands without
    //               creating an Store_Operater object in between
    //*********************************************************************

    //*******
    // Flag
      
    inline Meta_Operand_Flag &Meta_Operand_Flag::operator=
    ( solve::Operand<values::Flag> &operand )
    { 
      if( tok->has_value() ) tok->consumed();
      tok->set_op_flag(operand);
      return *this;
    }
    inline solve::Operand<values::Flag> &Meta_Operand_Flag::operator()()
    { 
      return tok->op_flag(info); 
    }
	
    //*******
    // Scalar
      
    inline Meta_Operand_Scalar &Meta_Operand_Scalar::operator=
    ( solve::Operand<values::Scalar> &operand )
    { 
      if( tok->has_value() ) tok->consumed();
      tok->set_op_scalar(operand);
      return *this;
    }
    inline solve::Operand<values::Scalar> &Meta_Operand_Scalar::operator()()
    { 
      return tok->op_scalar(info); 
    }
	
    //*******
    // Vector
      
    inline Meta_Operand_Vector &Meta_Operand_Vector::operator=
    ( solve::Operand<values::Vector> &operand )
    { 
      if( tok->has_value() ) tok->consumed();
      tok->set_op_vector(operand);
      return *this;
    }
    inline solve::Operand<values::Vector> &Meta_Operand_Vector::operator()()
    { 
      return tok->op_vector(info); 
    }
	
    //*******
    // Matrix
      
    inline Meta_Operand_Matrix &Meta_Operand_Matrix::operator=
    ( solve::Operand<values::Matrix> &operand )
    { 
      if( tok->has_value() ) tok->consumed();
      tok->set_op_matrix(operand);
      return *this;
    }
    inline solve::Operand<values::Matrix> &Meta_Operand_Matrix::operator()()
    { 
      return tok->op_matrix(info); 
    }
	
    //*******
    // String
      
    inline Meta_Operand_String &Meta_Operand_String::operator=
    ( solve::Operand<values::String> &operand )
    { 
      if( tok->has_value() ) tok->consumed();
      tok->set_op_string(operand);
      return *this;
    }
    inline solve::Operand<values::String> &Meta_Operand_String::operator()()
    { 
      return tok->op_string(info); 
    }
	
    //******************************************
    // Token: to store token values of any type
    //******************************************

    int Token::get_type() const
    {
      return type;
    }

    //********************************
    // access functions by references

    std::string   &Token::identifier()
    {
      if( !has_value() )
      {
	u.identifier=new std::string; type=TOK_IDENTIFIER;
      }
      else
	if( type != TOK_IDENTIFIER )
	  consumed();
      return *u.identifier;
    }

    values::Flag   &Token::flag()
    {
      if( has_value() && type != TOK_FLAG )
	consumed();

      if( !has_value() )
      {
	u.flag=new values::Flag; type=TOK_FLAG;
      }
      return *u.flag;
    }

    values::Scalar &Token::scalar()
    {
      if( has_value() && type != TOK_SCALAR )
	consumed();

      if( !has_value() )
      {
	u.scalar=new values::Scalar; type=TOK_SCALAR;
      }
      return *u.scalar;
    }

    values::Vector &Token::vector()
    {
      if( has_value() && type != TOK_VECTOR )
	consumed();

      if( !has_value() )
      {
	u.vector=new values::Vector; type=TOK_VECTOR;
      }
      return *u.vector;
    }

    values::Matrix &Token::matrix()
    {
      if( has_value() && type != TOK_MATRIX )
	consumed();

      if( !has_value() )
      {
	u.matrix=new values::Matrix; type=TOK_MATRIX;
      }
      return *u.matrix;
    }

    values::String &Token::string()
    {
      if( has_value() && type != TOK_STRING )
	consumed();

      if( !has_value() )
      {
	u.string=new values::String; type=TOK_STRING;
      }
      return *u.string;
    }
    class adlparser_info;	// forward declaration
    solve::Operand<values::Flag>   &Token::op_flag( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      if( has_value() && type != TOK_OP_FLAG )
	consumed();

      if( !has_value() )
      {
	u.op_flag=new solve::Operand<values::Flag>
	  (info->msg.get_consultant());	// get's message consultant for operand
	type=TOK_OP_FLAG;
      }
      return *u.op_flag;
    }

    solve::Operand<values::Scalar> &Token::op_scalar( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      if( has_value() && type != TOK_OP_SCALAR )
	consumed();

      if( !has_value() )
      {
	u.op_scalar=new solve::Operand<values::Scalar>
	  (info->msg.get_consultant());	// get's message consultant for operand
	type=TOK_OP_SCALAR;
      }
      return *u.op_scalar;
    }

    solve::Operand<values::Vector> &Token::op_vector( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      if( has_value() && type != TOK_OP_VECTOR )
	consumed();

      if( !has_value() )
      {
	u.op_vector=new solve::Operand<values::Vector>
	  (info->msg.get_consultant());	// get's message consultant for operand
	type=TOK_OP_VECTOR;
      }
      return *u.op_vector;
    }

    solve::Operand<values::Matrix> &Token::op_matrix( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      if( has_value() && type != TOK_OP_MATRIX )
	consumed();

      if( !has_value() )
      {
	u.op_matrix=new solve::Operand<values::Matrix>
	  (info->msg.get_consultant());	// get's message consultant for operand
	type=TOK_OP_MATRIX;
      }
      return *u.op_matrix;
    }

    solve::Operand<values::String> &Token::op_string( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      if( has_value() && type != TOK_OP_STRING )
	consumed();

      if( !has_value() )
      {
	u.op_string=new solve::Operand<values::String>
	  (info->msg.get_consultant());	// get's message consultant for operand
	type=TOK_OP_STRING;
      }
      return *u.op_string;
    }

    Meta_Operand_Flag   Token::meta_op_flag  ( void* info )
    {
      return Meta_Operand_Flag( this, info );
    }

    Meta_Operand_Scalar Token::meta_op_scalar( void* info )
    {
      return Meta_Operand_Scalar( this, info );
    }

    Meta_Operand_Vector Token::meta_op_vector( void* info )
    {
      return Meta_Operand_Vector( this, info );
    }

    Meta_Operand_Matrix Token::meta_op_matrix( void* info )
    {
      return Meta_Operand_Matrix( this, info );
    }

    Meta_Operand_String Token::meta_op_string( void* info )
    {
      return Meta_Operand_String( this, info );
    }
    
    anitmt::Type_Property<values::Flag>   &Token::prop_flag()
    {
      assert(type==TOK_PROP_FLAG  );
      return *u.prop_flag;
    }

    anitmt::Type_Property<values::Scalar> &Token::prop_scalar()
    {
      assert(type==TOK_PROP_SCALAR  );
      return *u.prop_scalar;
    }

    anitmt::Type_Property<values::Vector> &Token::prop_vector()
    {
      assert(type==TOK_PROP_VECTOR  );
      return *u.prop_vector;
    }

    anitmt::Type_Property<values::Matrix> &Token::prop_matrix()
    {
      assert(type==TOK_PROP_MATRIX  );
      return *u.prop_matrix;
    }

    anitmt::Type_Property<values::String> &Token::prop_string()
    {
      assert(type==TOK_PROP_STRING  );
      return *u.prop_string;
    }

    //*********************************
    // readonly access functions

    std::string Token::get_identifier() const
    {
      assert(type==TOK_IDENTIFIER  );
      return *u.identifier;
    }

    values::Flag   Token::get_flag() const
    {
      assert(type==TOK_FLAG  );
      return *u.flag;
    }

    values::Scalar Token::get_scalar() const
    {
      assert(type==TOK_SCALAR  );
      return *u.scalar;
    }

    values::Vector Token::get_vector() const
    {
      assert(type==TOK_VECTOR  );
      return *u.vector;
    }

    values::Matrix Token::get_matrix() const
    {
      assert(type==TOK_MATRIX  );
      return *u.matrix;
    }

    values::String Token::get_string() const
    {
      assert(type==TOK_STRING  );
      return *u.string;
    }
    solve::Operand<values::Flag>   *Token::get_op_flag() const
    {
      assert(type==TOK_OP_FLAG  );
      return u.op_flag;
    }

    solve::Operand<values::Scalar> *Token::get_op_scalar() const
    {
      assert(type==TOK_OP_SCALAR  );
      return u.op_scalar;
    }

    solve::Operand<values::Vector> *Token::get_op_vector() const
    {
      assert(type==TOK_OP_VECTOR  );
      return u.op_vector;
    }

    solve::Operand<values::Matrix> *Token::get_op_matrix() const
    {
      assert(type==TOK_OP_MATRIX  );
      return u.op_matrix;
    }

    solve::Operand<values::String> *Token::get_op_string() const
    {
      assert(type==TOK_OP_STRING  );
      return u.op_string;
    }
    anitmt::Type_Property<values::Flag>   *Token::get_prop_flag() const
    {
      assert(type==TOK_PROP_FLAG  );
      return u.prop_flag;
    }

    anitmt::Type_Property<values::Scalar> *Token::get_prop_scalar() const
    {
      assert(type==TOK_PROP_SCALAR  );
      return u.prop_scalar;
    }

    anitmt::Type_Property<values::Vector> *Token::get_prop_vector() const
    {
      assert(type==TOK_PROP_VECTOR  );
      return u.prop_vector;
    }

    anitmt::Type_Property<values::Matrix> *Token::get_prop_matrix() const
    {
      assert(type==TOK_PROP_MATRIX  );
      return u.prop_matrix;
    }

    anitmt::Type_Property<values::String> *Token::get_prop_string() const
    {
      assert(type==TOK_PROP_STRING  );
      return u.prop_string;
    }

    //****************************************
    // set functions, that only copy pointers

    void Token::set_op_flag( solve::Operand<values::Flag> &f )
    {
      if( has_value() ) consumed();
      u.op_flag = &f;
      type = TOK_OP_FLAG;
    }

    void Token::set_op_scalar( solve::Operand<values::Scalar> &f )
    {
      if( has_value() ) consumed();
      u.op_scalar = &f;
      type = TOK_OP_SCALAR;
    }

    void Token::set_op_vector( solve::Operand<values::Vector> &f )
    {
      if( has_value() ) consumed();
      u.op_vector = &f;
      type = TOK_OP_VECTOR;
    }

    void Token::set_op_matrix( solve::Operand<values::Matrix> &f )
    {
      if( has_value() ) consumed();
      u.op_matrix = &f;
      type = TOK_OP_MATRIX;
    }

    void Token::set_op_string( solve::Operand<values::String> &f )
    {
      if( has_value() ) consumed();
      u.op_string = &f;
      type = TOK_OP_STRING;
    }

    void Token::set_prop_flag( anitmt::Type_Property<values::Flag> &f )
    {
      if( has_value() ) consumed();
      u.prop_flag = &f;
      type = TOK_PROP_FLAG;
    }

    void Token::set_prop_scalar( anitmt::Type_Property<values::Scalar> &f )
    {
      if( has_value() ) consumed();
      u.prop_scalar = &f;
      type = TOK_PROP_SCALAR;
    }

    void Token::set_prop_vector( anitmt::Type_Property<values::Vector> &f )
    {
      if( has_value() ) consumed();
      u.prop_vector = &f;
      type = TOK_PROP_VECTOR;
    }

    void Token::set_prop_matrix( anitmt::Type_Property<values::Matrix> &f )
    {
      if( has_value() ) consumed();
      u.prop_matrix = &f;
      type = TOK_PROP_MATRIX;
    }

    void Token::set_prop_string( anitmt::Type_Property<values::String> &f )
    {
      if( has_value() ) consumed();
      u.prop_string = &f;
      type = TOK_PROP_STRING;
    }

    bool Token::has_value() const
    {
      return type != TOK_INVALID_ID;
    }

    // delete stored token
    int Token::consumed()
    {
      if( type == TOK_INVALID_ID ) return -1; // no value stored ?!?

      switch( type )
      {
      case TOK_IDENTIFIER: assert( u.ptr != 0 ); delete u.identifier; break;
      case TOK_FLAG:   assert( u.ptr != 0 ); delete u.flag;   break;
      case TOK_SCALAR: assert( u.ptr != 0 ); delete u.scalar; break;
      case TOK_VECTOR: assert( u.ptr != 0 ); delete u.vector; break;
      case TOK_MATRIX: assert( u.ptr != 0 ); delete u.matrix; break;
      case TOK_STRING: assert( u.ptr != 0 ); delete u.string; break;
	// don't delete operands and properties
      case TOK_OP_FLAG:   assert( u.ptr != 0 ); break;
      case TOK_OP_SCALAR: assert( u.ptr != 0 ); break;
      case TOK_OP_VECTOR: assert( u.ptr != 0 ); break;
      case TOK_OP_MATRIX: assert( u.ptr != 0 ); break;
      case TOK_OP_STRING: assert( u.ptr != 0 ); break;
      case TOK_PROP_FLAG:   assert( u.ptr != 0 ); break;
      case TOK_PROP_SCALAR: assert( u.ptr != 0 ); break;
      case TOK_PROP_VECTOR: assert( u.ptr != 0 ); break;
      case TOK_PROP_MATRIX: assert( u.ptr != 0 ); break;
      case TOK_PROP_STRING: assert( u.ptr != 0 ); break;
      default: assert(0);
      };
      type = TOK_INVALID_ID;
      return 0;
    }

    // returns this token (needed by parser)
    Token &Token::tok() { return *this; }

  }
}
#endif
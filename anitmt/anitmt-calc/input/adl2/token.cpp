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

#include "token.hpp"

namespace anitmt
{
  namespace adlparser
  {
    //*********************************************************************
    // Meta_Operand: stores operands and allows assigning operands without
    //               creating an Store_Operater object in between
    //*********************************************************************

    Meta_Operand_Flag::Meta_Operand_Flag( Token *t, void *i ) 
      : tok(t), info(i) {}

    Meta_Operand_Scalar::Meta_Operand_Scalar( Token *t, void *i ) 
      : tok(t), info(i) {}

    Meta_Operand_Vector::Meta_Operand_Vector( Token *t, void *i ) 
      : tok(t), info(i) {}

    Meta_Operand_Matrix::Meta_Operand_Matrix( Token *t, void *i ) 
      : tok(t), info(i) {}

    Meta_Operand_String::Meta_Operand_String( Token *t, void *i ) 
      : tok(t), info(i) {}

    //******************************
    // Token: Type for lexer tokens
    //******************************

    //************
    // operators
    Token &Token::operator=(const Token &t)
    {
      // is current type different from type of t
      if( (type != TOK_INVALID_ID) && (type != t.get_type()) )
	consumed();		// delete old value

      switch( t.get_type() )
	{
	case TOK_INVALID_ID: break; // do nothing
	case TOK_IDENTIFIER: identifier() = t.get_identifier(); break;
	case TOK_FLAG:   flag()   = t.get_flag();   break;
	case TOK_SCALAR: scalar() = t.get_scalar(); break;
	case TOK_VECTOR: vector() = t.get_vector(); break;
	case TOK_MATRIX: matrix() = t.get_matrix(); break;
	case TOK_STRING: string() = t.get_string(); break;
	case TOK_OP_FLAG:   u.op_flag   = t.get_op_flag();   break;
	case TOK_OP_SCALAR: u.op_scalar = t.get_op_scalar(); break;
	case TOK_OP_VECTOR: u.op_vector = t.get_op_vector(); break;
	case TOK_OP_MATRIX: u.op_matrix = t.get_op_matrix(); break;
	case TOK_OP_STRING: u.op_string = t.get_op_string(); break;
	case TOK_PROP_FLAG:   u.prop_flag   = t.get_prop_flag();   break;
	case TOK_PROP_SCALAR: u.prop_scalar = t.get_prop_scalar(); break;
	case TOK_PROP_VECTOR: u.prop_vector = t.get_prop_vector(); break;
	case TOK_PROP_MATRIX: u.prop_matrix = t.get_prop_matrix(); break;
	case TOK_PROP_STRING: u.prop_string = t.get_prop_string(); break;
	default: assert(0);
	};
      type = t.get_type();
      return *this;
    }


    //***************************
    // constuctors/destructors

    Token::Token() : type(TOK_INVALID_ID)
    {
      u.ptr = 0;
    }
    // copy constructor
    Token::Token(const Token &t) : type(TOK_INVALID_ID)
    {
      u.ptr = 0;
      switch( t.get_type() )
	{
	case TOK_INVALID_ID: break; // do nothing
	case TOK_IDENTIFIER: identifier() = t.get_identifier(); break;
	case TOK_FLAG:   flag()   = t.get_flag();   break;
	case TOK_SCALAR: scalar() = t.get_scalar(); break;
	case TOK_VECTOR: vector() = t.get_vector(); break;
	case TOK_MATRIX: matrix() = t.get_matrix(); break;
	case TOK_STRING: string() = t.get_string(); break;
	case TOK_OP_FLAG:   u.op_flag   = t.get_op_flag();   break;
	case TOK_OP_SCALAR: u.op_scalar = t.get_op_scalar(); break;
	case TOK_OP_VECTOR: u.op_vector = t.get_op_vector(); break;
	case TOK_OP_MATRIX: u.op_matrix = t.get_op_matrix(); break;
	case TOK_OP_STRING: u.op_string = t.get_op_string(); break;
	case TOK_PROP_FLAG:   u.prop_flag   = t.get_prop_flag();   break;
	case TOK_PROP_SCALAR: u.prop_scalar = t.get_prop_scalar(); break;
	case TOK_PROP_VECTOR: u.prop_vector = t.get_prop_vector(); break;
	case TOK_PROP_MATRIX: u.prop_matrix = t.get_prop_matrix(); break;
	case TOK_PROP_STRING: u.prop_string = t.get_prop_string(); break;
	default: assert(0);
	};
      type = t.get_type();
    }

    Token::~Token()
    {
      consumed();
    }
  }   
}

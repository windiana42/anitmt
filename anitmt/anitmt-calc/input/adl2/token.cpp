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

    inline void _internal_assign( Token &dest, const Token &src )
    {
      switch( src.get_type() )
	{
	case TOK_INVALID_ID: break; // do nothing
	case TOK_IDENTIFIER: dest.identifier() = src.get_identifier(); break;
	case TOK_FLAG:   dest.flag()   = src.get_flag();   break;
	case TOK_SCALAR: dest.scalar() = src.get_scalar(); break;
	case TOK_VECTOR: dest.vector() = src.get_vector(); break;
	case TOK_MATRIX: dest.matrix() = src.get_matrix(); break;
	case TOK_STRING: dest.string() = src.get_string(); break;
	case TOK_OP_FLAG:   dest.u.op_flag   = src.get_op_flag();   break;
	case TOK_OP_SCALAR: dest.u.op_scalar = src.get_op_scalar(); break;
	case TOK_OP_VECTOR: dest.u.op_vector = src.get_op_vector(); break;
	case TOK_OP_MATRIX: dest.u.op_matrix = src.get_op_matrix(); break;
	case TOK_OP_STRING: dest.u.op_string = src.get_op_string(); break;
	case TOK_PROP_FLAG:   dest.u.prop_flag   = src.get_prop_flag();  break;
	case TOK_PROP_SCALAR: dest.u.prop_scalar = src.get_prop_scalar();break;
	case TOK_PROP_VECTOR: dest.u.prop_vector = src.get_prop_vector();break;
	case TOK_PROP_MATRIX: dest.u.prop_matrix = src.get_prop_matrix();break;
	case TOK_PROP_STRING: dest.u.prop_string = src.get_prop_string();break;
	default: assert(0);
	};
      dest.type = src.get_type();
    }

    //************
    // operators
    Token &Token::operator=(const Token &t)
    {
      // is current type different from type of t
      if( (type != TOK_INVALID_ID) && (type != t.get_type()) )
	consumed();		// delete old value

      _internal_assign(*this,t);
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
      _internal_assign(*this,t);
    }

    Token::~Token()
    {
      consumed();
    }
  }   
}

/*****************************************************************************/
/**   This file offers a parser for the Animation Description Language     **/
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

#include "adlparser.hpp"

namespace anitmt
{
  namespace adlparser
  {
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

    Token Reference_Resolver::get_identifier( std::string s )
    {
      anitmt::Property *prop = info->get_current_tree_node()->
	get_referenced_property(s);
      Token tok;
      if( prop != 0 )
	{
	  switch( prop->get_type() )
	    {
	    case values::Valtype::flag:
	      // type_property is converted to operand as needed by parser!
	      tok.set_op_flag  ( *dynamic_cast<Type_Property<values::Flag>*>
				   (prop) );
	      break;
	    case values::Valtype::scalar:
	      tok.set_op_scalar( *dynamic_cast<Type_Property<values::Scalar>*>
				   (prop) );
	      break;
	    case values::Valtype::vector:
	      tok.set_op_vector( *dynamic_cast<Type_Property<values::Vector>*>
				   (prop) );
	      break;
	    case values::Valtype::matrix:
	      tok.set_op_matrix( *dynamic_cast<Type_Property<values::Matrix>*>
				   (prop) );
	      break;
	    case values::Valtype::string:
	      tok.set_op_string( *dynamic_cast<Type_Property<values::String>*>
				   (prop) );
	      break;
	    }
	}
      return tok;
    }

    Token Property_Resolver::get_identifier( std::string s )
    {
      anitmt::Property *prop = info->get_current_tree_node()->
	get_property(s);
      Token tok;
      if( prop != 0 )
	{
	  switch( prop->get_type() )
	    {
	    case values::Valtype::flag:
	      tok.set_prop_flag  (*dynamic_cast<Type_Property<values::Flag>*>
				   (prop) );
	      break;
	    case values::Valtype::scalar:
	      tok.set_prop_scalar(*dynamic_cast<Type_Property<values::Scalar>*>
				   (prop) );
	      break;
	    case values::Valtype::vector:
	      tok.set_prop_vector(*dynamic_cast<Type_Property<values::Vector>*>
				   (prop) );
	      break;
	    case values::Valtype::matrix:
	      tok.set_prop_matrix(*dynamic_cast<Type_Property<values::Matrix>*>
				   (prop) );
	      break;
	    case values::Valtype::string:
	      tok.set_prop_string(*dynamic_cast<Type_Property<values::String>*>
				   (prop) );
	      break;
	    }
	}
      return tok;
    }
    
    //**********************************************************
    // adlparser_info: stores information for parser and lexer
    //**********************************************************

    // open file to be read by the lexer
    void adlparser_info::open_file( std::string filename )
    {
      file_pos.set_filename( filename );
      in_file.open( filename.c_str() );
      if( !lexer_uses_file_stream ) 
      {
	if( lexer ) delete lexer;
	lexer = new adlparser_FlexLexer(&in_file);      
	lexer->info = this;
      }
      lexer_uses_file_stream = true;
    }

    // open file to be read by the lexer
    void adlparser_info::open_stream( std::string filename, std::istream &in )
    {
      file_pos.set_filename( filename );
      if( lexer ) delete lexer;
      
      lexer = new adlparser_FlexLexer(&in);      
      lexer->info = this;
      lexer_uses_file_stream = false;
    }

    adlparser_info::adlparser_info( message::Message_Consultant *consultant )
      : msg(consultant), lexer(new adlparser_FlexLexer(&cin)),
	res_reference( this ), res_property( this ),
	lexer_uses_file_stream( false )
    {
      lexer->info = this;
      file_pos.set_filename("standard input");
    }
    adlparser_info::~adlparser_info()
    {
      delete lexer;
    }

  }
}


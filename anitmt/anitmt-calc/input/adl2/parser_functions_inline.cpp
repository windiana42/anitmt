/*****************************************************************************/
/**   This file offers functions and macros for the parser                  **/
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

#ifndef __inlineimplementation_anitmt_input_parser_functions__
#define __inlineimplementation_anitmt_input_parser_functions__

#include "parser_functions.hpp"

#include <solve/reference.hpp>

namespace anitmt
{
  namespace adlparser
  {
    //**********************
    // interfaces to lexer
    //**********************

    inline int yylex( Token *lvalp, void *vinfo )
    {
      adlparser_info *info = static_cast<adlparser_info*> (vinfo);
      info->lexer->yylval = lvalp; // lvalue variable to return token value
      return info->lexer->yylex();
    }

    //*************************
    // interfaces to messages
    //*************************

    inline message::Message_Stream yyerr( void* vinfo, int back )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vinfo);
      
      message::Abstract_Position *pos = 0;
      if( back >= 0 )		// shall I report an old position?
	pos = info->get_old_pos(back);
      if( !pos )		// still no position set
	pos = info->get_pos();	// use current position
      message::Message_Stream msg(message::noinit);
      info->msg.error( pos ).copy_to(msg);
      return msg;
    }

    inline message::Message_Stream yywarn( void* vinfo, int back )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vinfo);

      message::Abstract_Position *pos = 0;
      if( back >= 0 )		// shall I report an old position?
	pos = info->get_old_pos(back);
      if( !pos )		// still no position set
	pos = info->get_pos();	// use current position

      message::Message_Stream msg(message::noinit);
      info->msg.warn( pos ).copy_to(msg);
      return msg;
    }

    inline message::Message_Stream yyverbose( void* vinfo, int back, 
					      bool with_position, 
					      int vlevel, int detail )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vinfo);

      message::Message_Stream msg(message::noinit);
      if( with_position )
      {
	message::Abstract_Position *pos = 0;
	if( back >= 0 )		// shall I report an old position?
	  pos = info->get_old_pos(back);
	if( !pos )		// still no position set
	  pos = info->get_pos();	// use current position

	info->msg.verbose( vlevel, pos, detail ).copy_to(msg);
      }
      else
	info->msg.verbose( vlevel, message::GLOB::no_position, detail ).
	  copy_to(msg);
      
      return msg;
    }

    //******************************
    // functions used by the parser
    //******************************

    //! sets the position of a Property in the adl source
    inline void initialize_lexer( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->lexer->goto_initial_state();
    }

    // property declaration
    inline void prop_declaration_start( Property &prop, void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      
      switch( info->pass )
      {
      case pass1: // only parse structure?
	set_pos( &prop, vptr_info );
	// don't parse the expression yet
	info->lexer->dummy_statement_follows();	
	break;
      case pass2:
	resolve_references(info);
	break;
      default:
	assert(0);
      }
    }
    inline void flag_prop_declaration_finish
    ( Type_Property<values::Flag> &prop, Token &tok, void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      switch( info->pass )
      {
      case pass1:
	// do nothing
	break;
      case pass2:
	if( tok.get_type() == TOK_FLAG )
	{
	  if( !prop.set_value( tok.flag() ) )
	  {
	    yyerr(info) << "error while setting property " << prop.get_name() 
			<< " to " << tok.flag();
	  }
	}
	else 
	{
	  assert( tok.get_type() == TOK_OP_FLAG );
	  // establish reference
	  solve::explicite_reference( prop, tok.op_flag(vptr_info) ); 
	}
	resolve_properties(info); // resolve properties again
	break;
      default:
	assert(0);
      }
    }
    inline void scalar_prop_declaration_finish
    ( Type_Property<values::Scalar> &prop, Token &tok, void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      switch( info->pass )
      {
      case pass1:
	// do nothing
	break;
      case pass2:
	if( tok.get_type() == TOK_SCALAR )
	{
	  if( !prop.set_value( tok.scalar() ) )
	  {
	    yyerr(info) << "error while setting property " << prop.get_name() 
			<< " to " << tok.scalar();
	  }
	}
	else 
	{
	  assert( tok.get_type() == TOK_OP_SCALAR );
	  // establish reference
	  solve::explicite_reference( prop, tok.op_scalar(vptr_info) ); 
	}
	resolve_properties(info); // resolve properties again
	break;
      default:
	assert(0);
      }
    }
    inline void vector_prop_declaration_finish
    ( Type_Property<values::Vector> &prop, Token &tok, void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      switch( info->pass )
      {
      case pass1:
	// do nothing
	break;
      case pass2:
	if( tok.get_type() == TOK_VECTOR )
	{
	  if( !prop.set_value( tok.vector() ) )
	  {
	    yyerr(info) << "error while setting property " << prop.get_name() 
			<< " to " << tok.vector();
	  }
	}
	else 
	{
	  assert( tok.get_type() == TOK_OP_VECTOR );
	  // establish reference
	  solve::explicite_reference( prop, tok.op_vector(vptr_info) ); 
	}
	resolve_properties(info); // resolve properties again
	break;
      default:
	assert(0);
      }
    }
    inline void matrix_prop_declaration_finish
    ( Type_Property<values::Matrix> &prop, Token &tok, void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      switch( info->pass )
      {
      case pass1:
	// do nothing
	break;
      case pass2:
	if( tok.get_type() == TOK_MATRIX )
	{
	  if( !prop.set_value( tok.matrix() ) )
	  {
	    yyerr(info) << "error while setting property " << prop.get_name() 
			<< " to " << tok.matrix();
	  }
	}
	else 
	{
	  assert( tok.get_type() == TOK_OP_MATRIX );
	  // establish reference
	  solve::explicite_reference( prop, tok.op_matrix(vptr_info) ); 
	}
	resolve_properties(info); // resolve properties again
	break;
      default:
	assert(0);
      }
    }
    inline void string_prop_declaration_finish
    ( Type_Property<values::String> &prop, Token &tok, void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      switch( info->pass )
      {
      case pass1:
	// do nothing
	break;
      case pass2:
	if( tok.get_type() == TOK_STRING )
	{
	  if( !prop.set_value( tok.string() ) )
	  {
	    yyerr(info) << "error while setting property " << prop.get_name() 
			<< " to " << tok.string();
	  }
	}
	else 
	{
	  assert( tok.get_type() == TOK_OP_STRING );
	  // establish reference
	  solve::explicite_reference( prop, tok.op_string(vptr_info) ); 
	}
	resolve_properties(info); // resolve properties again
	break;
      default:
	assert(0);
      }
    }

    //! sets the position of a Property in the adl source
    inline void set_node_pos( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->get_current_tree_node()->set_position( info->file_pos.duplicate());
    }

    // tells the lexer to resolve identifiers as properties
    inline void resolve_properties( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->id_resolver = &info->res_property;
    }

    // tells the lexer to resolve identifiers as property references
    inline void resolve_references( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->id_resolver = &info->res_reference;
    }
  }
}

#endif

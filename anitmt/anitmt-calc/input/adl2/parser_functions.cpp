/*****************************************************************************/
/**   This file offers functions and macros for the parser                  **/
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

#include "parser_functions.hpp"

#include <proptree/proptree.hpp>
#include <utl/stdextend.hpp>
#include <solve/reference.hpp>
#include <val/val.hpp>

#include <stack>
#include <assert.h>

#include <tokens.h>

// forward declarations for scanner.cc/scanner.ll functions
void adlparser_goto_initial_state(/*in-out*/int &yy_start);
void adlparser_dummy_statement_follows(/*in-out*/int &yy_start);

namespace anitmt
{
  namespace adlparser
  {
    //******************************
    // functions used by the parser
    //******************************

    proptree::Child_Manager child_manager;

    //**************************
    // hierarchy move functions

    // creates new tree node and makes it the current one
    void change_current_child( void *vptr_info, std::string type, 
			       std::string name )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);

      proptree::Prop_Tree_Node *node = 0;
      switch( info->pass )
      {
      case pass1:
	if( name != "" )	// if there is a name given -> search child
	  node = info->get_current_tree_node()->get_child( name );
	if( node != 0 )		// if already found -> check type
	{
	  if( node->get_type() != type )
	    yyerr(vptr_info) << "child " << name 
			    << " already exists with different type " 
			    << node->get_type();
	}
	else			// else -> add new child
	  node = info->get_current_tree_node()->add_child( type, name );
	
	if( node == 0 )		// was it not possible to add child?
	{
	  yyerr(vptr_info) << "couldn't add tree node " << name << " as type " 
			   << type << " isn't allowed here";
	}
	else
	{
	  info->set_new_tree_node( node );
	}
	break;
      case pass2:
	// initialized child manager
	if( !child_manager.is_initialized() ) 
	  child_manager.set_root_node( info->get_current_tree_node() );

	info->set_new_tree_node( child_manager.get_child() );
	break;
      default:
	assert(0);
      }
      // also set the declaration position for the new tree node
      set_node_pos(vptr_info);
    }

    // changes back to the parent tree node
    void change_to_parent( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      switch( info->pass )
      {
      case pass1:
	break;
      case pass2:
	child_manager.child_finished();
	break;
      }
      info->tree_node_done();
    }

    //*************************
    // interfaces to messages
    //*************************

    message::Message_Stream yyerr( void* vinfo, int back )
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

    message::Message_Stream yywarn( void* vinfo, int back )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vinfo);

      message::Abstract_Position *pos = 0;
      if( back >= 0 )		// shall I report an old position?
	pos = info->get_old_pos(back);
      if( !pos )		// still no position set
	pos = info->get_pos();	// use current position

      message::Message_Stream msg(message::noinit);
      info->msg.warn( pos ).copy_to(msg);
      delete pos;
      return msg;
    }

    message::Message_Stream yyverbose( void* vinfo, int back, 
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

    // property declaration
    void prop_declaration_start( proptree::Property &prop, 
					void *vptr_info )
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
    void flag_prop_declaration_finish
    ( proptree::Type_Property<values::Flag> &prop, Token &tok, 
      void *vptr_info )
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
	  if( tok.get_type() == TOK_OP_FLAG )
	  {
	    // establish reference
	    solve::explicite_reference( prop, tok.op_flag(vptr_info) ); 
	  }
	  else			// were there an error in the expression
	    assert( tok.get_type() == TOK_INVALID_ID );
				// error was already in parser.yy
	}
	resolve_properties(info); // resolve properties again
	break;
      default:
	assert(0);
      }
    }
    void scalar_prop_declaration_finish
    ( proptree::Type_Property<values::Scalar> &prop, Token &tok, 
      void *vptr_info )
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
	  if( tok.get_type() == TOK_OP_SCALAR )
	  {
	    // establish reference
	    solve::explicite_reference( prop, tok.op_scalar(vptr_info) ); 
	  }
	  else			// were there an error in the expression
	    assert( tok.get_type() == TOK_INVALID_ID );
				// error was already in parser.yy
	}
	resolve_properties(info); // resolve properties again
	break;
      default:
	assert(0);
      }
    }
    void vector_prop_declaration_finish
    ( proptree::Type_Property<values::Vector> &prop, Token &tok, 
      void *vptr_info )
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
	  if( tok.get_type() == TOK_OP_VECTOR )
	  {
	    // establish reference
	    solve::explicite_reference( prop, tok.op_vector(vptr_info) ); 
	  }
	  else			// were there an error in the expression
	    assert( tok.get_type() == TOK_INVALID_ID );
				// error was already in parser.yy
	}
	resolve_properties(info); // resolve properties again
	break;
      default:
	assert(0);
      }
    }
    void matrix_prop_declaration_finish
    ( proptree::Type_Property<values::Matrix> &prop, Token &tok, 
      void *vptr_info )
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
	  if( tok.get_type() == TOK_OP_MATRIX )
	  {
	    // establish reference
	    solve::explicite_reference( prop, tok.op_matrix(vptr_info) ); 
	  }
	  else			// were there an error in the expression
	    assert( tok.get_type() == TOK_INVALID_ID );
				// error was already in parser.yy
	}
	resolve_properties(info); // resolve properties again
	break;
      default:
	assert(0);
      }
    }
    void string_prop_declaration_finish
    ( proptree::Type_Property<values::String> &prop, Token &tok, 
      void *vptr_info )
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
	  if( tok.get_type() == TOK_OP_STRING )
	  {
	    // establish reference
	    solve::explicite_reference( prop, tok.op_string(vptr_info) ); 
	  }
	  else			// were there an error in the expression
	    assert( tok.get_type() == TOK_INVALID_ID );
				// error was already in parser.yy
	}
	resolve_properties(info); // resolve properties again
	break;
      default:
	assert(0);
      }
    }

    // **********************
    // interfaces to lexer
    // **********************
    myFlex::myFlex(std::istream *in)
      : adlparser_FlexLexer(in)
    {
    }
    void myFlex::goto_initial_state() 
    {
      adlparser_goto_initial_state(/*in-out*/yy_start);
    }

    void myFlex::dummy_statement_follows() 
    {
      adlparser_dummy_statement_follows(/*in-out*/yy_start);
    }
    
    void myFlex::set_input_stream( std::istream &in ) 
    {
      yyrestart(in);
    }
  }
}

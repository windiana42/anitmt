/*****************************************************************************/
/**   This file offers a class where the parser stores information         **/
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

#include "parsinfo.hpp"

namespace anitmt
{
  namespace adlparser
  {

    //**********************************************************
    // adlparser_info: stores information for parser and lexer
    //**********************************************************

    // open file to be read by the lexer
    void adlparser_info::open_file( std::string filename )
    {
      file_pos.set_filename( filename );
      in_file.open( filename.c_str() );
      lexer->set_input_stream( in_file );
      lexer_uses_file_stream = true;
    }

    void adlparser_info::close_file()
    {
      lexer_uses_file_stream = false;
      lexer->set_input_stream( cin );
      in_file.close();
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
      : old_positions(10), max_old_positions(10),
	msg(consultant), lexer(new adlparser_FlexLexer(&cin)), pass(pass1),
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

    //**********************************************************
    // Identifier_Resolver: resolves indentifiers for the lexer
    //**********************************************************

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
	      tok.set_prop_flag  (*dynamic_cast<Type_Property<values::Flag>*>
				  (prop));
	      break;
	    case values::Valtype::scalar:
	      tok.set_prop_scalar(*dynamic_cast<Type_Property<values::Scalar>*>
				  (prop));
	      break;
	    case values::Valtype::vector:
	      tok.set_prop_vector(*dynamic_cast<Type_Property<values::Vector>*>
				  (prop) );
	      break;
	    case values::Valtype::matrix:
	      tok.set_prop_matrix(*dynamic_cast<Type_Property<values::Matrix>*>
				  (prop));
	      break;
	    case values::Valtype::string:
	      tok.set_prop_string(*dynamic_cast<Type_Property<values::String>*>
				  (prop));
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

  }
}
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

#ifndef __AniTMT_parser_functions__
#define __AniTMT_parser_functions__

#include <message/message.hpp>

#include "adlparser.hpp"
#include "token.hpp"
#include "parsinfo.hpp"

namespace anitmt
{
  namespace adlparser
  {
    //**********************
    // interfaces to lexer
    //**********************

    // define token type for parser
    #define YYSTYPE Token

    inline int yylex( Token *lvalp, void *info );

    //*************************
    // interfaces to messages
    //*************************

    inline message::Message_Stream yyerr( void* vinfo );
    inline message::Message_Stream yywarn( void* vinfo );
    inline message::Message_Stream yyverbose( void* vinfo, 
					      bool with_position=true, 
					      int vlevel=1, int detail=2 );


    // redefine error output
#define yyerror( s ) ( yyerr(info) << s, 1 )

    //******************************
    // functions used by the parser
    //******************************

    inline message::Message_Reporter &msg( void *info )
    {return static_cast<adlparser_info*>(info)->msg;}

    inline message::Message_Consultant *msg_consultant( void *info )
    {return static_cast<adlparser_info*>(info)->msg.get_consultant();}

    // creates new tree node and makes it the current one
    void change_current_child( void *vptr_info, std::string type, 
			       std::string name="" );

    // changes back to the parent tree node
    inline void change_to_parent( void *vptr_info );

    // property declaration
    inline void prop_declaration_start( Property &prop, void *vptr_info );
    inline void flag_prop_declaration_finish
    ( Type_Property<values::Flag> &prop, Token &tok, void *vptr_info );
    inline void scalar_prop_declaration_finish
    ( Type_Property<values::Scalar> &prop, Token &tok, void *vptr_info );
    inline void vector_prop_declaration_finish
    ( Type_Property<values::Vector> &prop, Token &tok, void *vptr_info );
    inline void matrix_prop_declaration_finish
    ( Type_Property<values::Matrix> &prop, Token &tok, void *vptr_info );
    inline void string_prop_declaration_finish
    ( Type_Property<values::String> &prop, Token &tok, void *vptr_info );


    inline void set_pos( Property *prop, void *info )
    {prop->set_position( static_cast<adlparser_info*>(info)->
			 file_pos.duplicate() );}

    // tells the lexer to resolve identifiers as properties
    inline void resolve_properties( void *vptr_info );

    // tells the lexer to resolve identifiers as property references
    inline void resolve_references( void *vptr_info );
  }
}

#include "parser_functions_inline.cpp"

#endif

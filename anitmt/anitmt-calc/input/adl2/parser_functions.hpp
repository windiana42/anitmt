/*****************************************************************************/
/**   This file offers functions and macros to shorten the parser           **/
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

#include "token.hpp"
#include "parsinfo.hpp"
#include "adlparser.hpp"

namespace anitmt
{
  namespace adlparser
  {

    //**********************
    // interfaces to lexer
    //**********************

    // define token type for parser
#define YYSTYPE Token

#define YYPARSE_PARAM info	// parameter to the parser (yyparser(void*))
#define YYLEX_PARAM info	// parameter to the lexer
#define YYLEX_PARAM_TYPE (parser_info&)	// -> yylex(parser_info &info)
    inline int yylex( Token *lvalp, void *info );

    //*************************
    // interfaces to messages
    //*************************

    //! returns error message stream
    //! \param vinfo 
    inline message::Message_Stream yyerr( void* vinfo, int back=-1 );
    inline message::Message_Stream yywarn( void* vinfo, int back=-1 );
    inline message::Message_Stream yyverbose( void* vinfo, int back=-1, 
					      bool with_position=true, 
					      int vlevel=1, int detail=2 );


    // redefine error output
#define yyerror( s ) ( yyerr(info) << s, 1 )

    //******************************
    // functions used by the parser
    //******************************

    //! returns a message object for reporting things to the user
    inline message::Message_Reporter &msg( void *info )
    {return static_cast<adlparser_info*>(info)->msg;}

    //! returns the used message consultant
    inline message::Message_Consultant *msg_consultant( void *info )
    {return static_cast<adlparser_info*>(info)->msg.get_consultant();}

    //! sets the position of a Property in the adl source
    inline void initialize_lexer( void *info );

    //! opens [new] child in hierarchy and makes it the current tree node
    void change_current_child( void *vptr_info, std::string type, 
			       std::string name="" );

    //! changes back to the parent tree node
    void change_to_parent( void *vptr_info );

    //! property declaration starts
    inline void prop_declaration_start( Property &prop, void *vptr_info );
    //! flag property declaration is ready
    inline void flag_prop_declaration_finish
    ( Type_Property<values::Flag> &prop, Token &tok, void *vptr_info );
    //! scalar property declaration is ready
    inline void scalar_prop_declaration_finish
    ( Type_Property<values::Scalar> &prop, Token &tok, void *vptr_info );
    //! vector property declaration is ready
    inline void vector_prop_declaration_finish
    ( Type_Property<values::Vector> &prop, Token &tok, void *vptr_info );
    //! matrix property declaration is ready
    inline void matrix_prop_declaration_finish
    ( Type_Property<values::Matrix> &prop, Token &tok, void *vptr_info );
    //! string property declaration is ready
    inline void string_prop_declaration_finish
    ( Type_Property<values::String> &prop, Token &tok, void *vptr_info );

    //! sets the position of a Property in the adl source
    inline void set_pos( Property *prop, void *info )
    {prop->set_position( static_cast<adlparser_info*>(info)->get_old_pos(1) );}

    //! sets the position of a Property in the adl source
    inline void set_node_pos( void *info );

    // tells the lexer to resolve identifiers as properties
    inline void resolve_properties( void *vptr_info );

    // tells the lexer to resolve identifiers as property references
    inline void resolve_references( void *vptr_info );
  }
}

#include "parser_functions_inline.cpp"

#endif

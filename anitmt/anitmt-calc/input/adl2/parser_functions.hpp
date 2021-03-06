/*****************************************************************************/
/**   This file offers functions and macros to shorten the parser           **/
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

#ifndef __AniTMT_parser_functions__
#define __AniTMT_parser_functions__

#include "token.hpp"
#include "parsinfo.hpp"
#include "adlparser.hpp"

#include <message/message.hpp>

namespace anitmt
{
  namespace adlparser
  {

    //**********************
    // interfaces to lexer
    //**********************

    // define token type for parser
#define YYSTYPE Token

    // replaced by %lex-param & co in parser.yy
    //#define YYPARSE_PARAM info	// parameter to the parser (yyparser(void*))
    //#define YYLEX_PARAM info	// parameter to the lexer
    //#define YYLEX_PARAM_TYPE (parser_info&)	// -> yylex(parser_info &info)
    int yylex( Token *lvalp, void *mode, adlparser_info *info );

    class myFlex : public adlparser_FlexLexer {
    public:
      myFlex( std::istream *in );
      int yylex( Token *yylval, void *mode, adlparser_info *info );

      void goto_initial_state();	/* forces the lexer to go to INITIAL state */
      void dummy_statement_follows();	/* tells the lexer to parse a dummy statement*/
      void set_input_stream( std::istream &in ); /* resets the input stream */
    };

    //*************************
    // interfaces to messages
    //*************************

    //! returns error message stream
    //! \param vinfo 
    message::Message_Stream yyerr( void* vinfo, int back=-1 );
    message::Message_Stream yywarn( void* vinfo, int back=-1 );
    message::Message_Stream yyverbose( void* vinfo, int back=-1, 
					      bool with_position=true, 
					      int vlevel=1, int detail=2 );


    // redefine error output
#define yyerror( mode, info, s ) ( yyerr(info) << s, 1 )

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
    void initialize_lexer( void *info );

    //! opens [new] child in hierarchy and makes it the current tree node
    void change_current_child( void *vptr_info, std::string type, 
			       std::string name="" );

    //! changes back to the parent tree node
    void change_to_parent( void *vptr_info );

    //! property declaration starts
    void prop_declaration_start( proptree::Property &prop, 
					void *vptr_info );
    //! flag property declaration is ready
    void flag_prop_declaration_finish
    ( proptree::Type_Property<values::Flag> &prop, Token &tok, 
      void *vptr_info );
    //! scalar property declaration is ready
    void scalar_prop_declaration_finish
    ( proptree::Type_Property<values::Scalar> &prop, Token &tok, 
      void *vptr_info );
    //! vector property declaration is ready
    void vector_prop_declaration_finish
    ( proptree::Type_Property<values::Vector> &prop, Token &tok, 
      void *vptr_info );
    //! matrix property declaration is ready
    void matrix_prop_declaration_finish
    ( proptree::Type_Property<values::Matrix> &prop, Token &tok, 
      void *vptr_info );
    //! string property declaration is ready
    void string_prop_declaration_finish
    ( proptree::Type_Property<values::String> &prop, Token &tok, 
      void *vptr_info );

    //! sets the position of a Property in the adl source
    inline void set_pos( proptree::Property *prop, void *info )
    {prop->set_position( static_cast<adlparser_info*>(info)->get_old_pos(1) );}

    //! sets the position of a Property in the adl source
    void set_node_pos( void *info );

    // tells the lexer to resolve identifiers as properties
    void resolve_properties( void *vptr_info );

    // tells the lexer to resolve identifiers as property references
    void resolve_references( void *vptr_info );
  }
}

#ifdef EXTREME_INLINE
#include "parser_functions_inline.cpp"
#endif

#endif

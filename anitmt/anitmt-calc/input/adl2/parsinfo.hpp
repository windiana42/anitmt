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

#ifndef __aiapi_pre_include
#define __aiapi_pre_include
#include "token.hpp"
#endif

#ifndef __anitmt_input_adl_parsinfo__
#define __anitmt_input_adl_parsinfo__

#include <stack>
#include <fstream>

#include <message/message.hpp>

#include "adlparser.hpp"
#include "property.hpp"
#include "proptree.hpp"

namespace anitmt
{
  namespace adlparser
  {
    //**********************************************************
    // Identifier_Resolver: resolves indentifiers for the lexer
    //**********************************************************

    // virtual class to resolve identifiers in expressions
    class Identifier_Resolver
    {
    public:
      virtual Token get_identifier( std::string s ) = 0;
      virtual ~Identifier_Resolver() {}
    };

    class adlparser_info;

    class Reference_Resolver : public Identifier_Resolver
    {
      adlparser_info *info;
    public:
      virtual Token get_identifier( std::string s );
      Reference_Resolver( adlparser_info *i ) : info(i) {}
    };

    class Property_Resolver : public Identifier_Resolver
    {
      adlparser_info *info;
    public:
      virtual Token get_identifier( std::string s );
      Property_Resolver( adlparser_info *i ) : info(i) {}
    };

    //**********************************************************
    // adlparser_info: stores information for parser and lexer
    //**********************************************************

    // Info opbject that is passed to the parser
    class adlparser_info
    {
      std::stack<Prop_Tree_Node*> tree_node;  // tree_node for the parser

    public:
      adlparser_info( message::Message_Consultant *consultant );
      ~adlparser_info();

      // sets the passage number (1 or 2)
      inline void set_pass( pass_type n );	

      // open file to be read by the lexer
      void open_file( std::string filename );
      void close_file();

      // open file to be read by the lexer
      void open_stream( std::string filename, std::istream &in );

      //*************************************************
      // only for manipulation from yylex() and yyparse()

      message::Message_Reporter msg;	// this offers message streams
      adlparser_FlexLexer *lexer;	// lexical analyzer / scanner
      pass_type pass;			// 1 : parse only structure
					// 2 : parse everything
      
      Identifier_Resolver* id_resolver;	// resolves an identifier string
      Reference_Resolver res_reference;	// resolver for references
      Property_Resolver res_property;	// resolver for property names

      int tab_len;
      message::File_Position file_pos;
      std::ifstream in_file;	// output file stream 
      bool lexer_uses_file_stream; // whether lexer is created for in_file

      // access/modify functions to tree node (for parser)
      inline Prop_Tree_Node *get_current_tree_node();
      inline void set_new_tree_node( Prop_Tree_Node *node );
      inline void tree_node_done();

    };

  }
}

#ifndef yyFlexLexer		// is FlexLexer not yet defined?
#define yyFlexLexer adlparser_FlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

//include inline implementation
#include "parsinfo_inline.cpp"

#endif


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


#ifndef __anitmt_input_adlparser__
#define __anitmt_input_adlparser__

#include <stack>
#include <fstream>

#include <val/val.hpp>
#include <solve/operand.hpp>
#include <message/message.hpp>

#include "property.hpp"
#include "proptree.hpp"

class adlparser_FlexLexer;	// declare prototype for lexer

namespace anitmt
{
  namespace adlparser
  {
#include "tokens.h" //include token defines TOK_...

    //******************************
    // Token: Type for lexer tokens
    //******************************

    // YACC/LEX token type
    class Token
    {
      int type;
      union{
	void *ptr;

	std::string *identifier;
	values::Flag   *flag;
	values::Scalar *scalar;
	values::Vector *vector;
	values::Matrix *matrix;
	values::String *string;
	solve::Operand<values::Flag>   *op_flag;
	solve::Operand<values::Scalar> *op_scalar;
	solve::Operand<values::Vector> *op_vector;
	solve::Operand<values::Matrix> *op_matrix;
	solve::Operand<values::String> *op_string;
	anitmt::Type_Property<values::Flag>   *prop_flag;
	anitmt::Type_Property<values::Scalar> *prop_scalar;
	anitmt::Type_Property<values::Vector> *prop_vector;
	anitmt::Type_Property<values::Matrix> *prop_matrix;
	anitmt::Type_Property<values::String> *prop_string;

	// these types make problems with the functions below
	//std::string *std_string; // might be useful for anything
	//double dval;
      }u;
    public:
      inline int get_type() const;

      // access functions by references
      inline std::string &identifier();
      inline values::Flag   &flag();
      inline values::Scalar &scalar();
      inline values::Vector &vector();
      inline values::Matrix &matrix();
      inline values::String &string();
      inline solve::Operand<values::Flag>   &op_flag();
      inline solve::Operand<values::Scalar> &op_scalar();
      inline solve::Operand<values::Vector> &op_vector();
      inline solve::Operand<values::Matrix> &op_matrix();
      inline solve::Operand<values::String> &op_string();
      inline anitmt::Type_Property<values::Flag>   &prop_flag();
      inline anitmt::Type_Property<values::Scalar> &prop_scalar();
      inline anitmt::Type_Property<values::Vector> &prop_vector();
      inline anitmt::Type_Property<values::Matrix> &prop_matrix();
      inline anitmt::Type_Property<values::String> &prop_string();

      // readonly access functions
      inline std::string get_identifier() const;
      inline values::Flag   get_flag() const;
      inline values::Scalar get_scalar() const;
      inline values::Vector get_vector() const;
      inline values::Matrix get_matrix() const;
      inline values::String get_string() const;
      inline solve::Operand<values::Flag>   *get_op_flag() const;
      inline solve::Operand<values::Scalar> *get_op_scalar() const;
      inline solve::Operand<values::Vector> *get_op_vector() const;
      inline solve::Operand<values::Matrix> *get_op_matrix() const;
      inline solve::Operand<values::String> *get_op_string() const;
      inline anitmt::Type_Property<values::Flag>   *get_prop_flag() const;
      inline anitmt::Type_Property<values::Scalar> *get_prop_scalar() const;
      inline anitmt::Type_Property<values::Vector> *get_prop_vector() const;
      inline anitmt::Type_Property<values::Matrix> *get_prop_matrix() const;
      inline anitmt::Type_Property<values::String> *get_prop_string() const;

      // set operand without creating a new object
      inline void set_op_flag  ( solve::Operand<values::Flag> &f );
      inline void set_op_scalar( solve::Operand<values::Scalar> &f );
      inline void set_op_vector( solve::Operand<values::Vector> &f );
      inline void set_op_matrix( solve::Operand<values::Matrix> &f );
      inline void set_op_string( solve::Operand<values::String> &f );
      inline void set_prop_flag  ( anitmt::Type_Property<values::Flag> &f );
      inline void set_prop_scalar( anitmt::Type_Property<values::Scalar> &f );
      inline void set_prop_vector( anitmt::Type_Property<values::Vector> &f );
      inline void set_prop_matrix( anitmt::Type_Property<values::Matrix> &f );
      inline void set_prop_string( anitmt::Type_Property<values::String> &f );

      // is a value stored in that token?
      inline bool has_value() const;

      // delete stored token
      inline int consumed();

      // returns this token (needed by parser)
      inline Token &tok();

      //************
      // operators
      Token &operator=(const Token &t);


      //***************************
      // constuctors/destructors

      Token();
      // copy constructor
      Token(const Token &t);

      ~Token();
    };

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

      //*************************************************
      // only for manipulation from yylex() and yyparse()

      message::Message_Reporter msg;	// this offers message streams

      adlparser_FlexLexer *lexer;	// lexical analyzer / scanner

      Identifier_Resolver* id_resolver;	// resolves an identifier string
      Reference_Resolver res_reference;	// resolver for references
      Property_Resolver res_property;	// resolver for property names

      int tab_len;
      message::File_Position file_pos;
      std::ifstream in_file;	// output file stream 
      bool lexer_uses_file_stream; // whether lexer is created for in_file

      // open file to be read by the lexer
      void open_file( std::string filename );

      // open file to be read by the lexer
      void open_stream( std::string filename, std::istream &in );

      // access/modify functions to tree node (for parser)
      inline Prop_Tree_Node *get_current_tree_node();
      inline void set_new_tree_node( Prop_Tree_Node *node );
      inline void tree_node_done();

    };

    // parse an adl file from instream and
    int parse_adl( Prop_Tree_Node *node, 
		   adlparser_info *info, std::string filename="" );
  }
}

#ifndef yyFlexLexer		// is FlexLexer not yet defined?
#define yyFlexLexer adlparser_FlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

//include template implementation
#include "adlparser_inline.cpp"

#endif

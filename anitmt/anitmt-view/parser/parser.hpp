/*****************************************************************************/
/**   This file belongs to a parser library and is used by AniTMT	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/*****************************************************************************/

#ifndef __PARSER__
#define __PARSER__

#include <list>
#include <string>

#include "vals.hpp"
#include "mystream.hpp"

namespace parser{

  /******************/
  /* Error messages */
  /******************/
  
  class Parser_Error : public err::Basic_Pos_Error{
    stream::Word_Stream *s;

  public:
    void print_Position( std::ostream &os);

    Parser_Error( int number, std::string message, stream::Word_Stream *ws );
  };

  void error( int number, std::string message, stream::Word_Stream *ws ) 
    throw(Parser_Error); 

  enum errornums{
    errExpected          = 300,
    errNumberExpected    = 301,
    errMulExpected       = 302,
    errCommaExpected     = 303,
    errVectAnfExpected   = 304,
    errVectEndExpected   = 305,
    errBracketRExpected  = 306,  /* right bracket expected */
    errBracketLExpected  = 307,  /* left bracket expected */
    errColonExpected     = 308,
    errQuotesExpected    = 309,

    errScalarExpected   = 310,
    errVectorExpected   = 311,
    errStringExpected   = 312,  
    errOpExpected       = 313,  /* operator expected */
    errTypeMismatch     = 314,  /* Operation not possible with type */

    errSyntax           = 320   /* don't know what to do with this */
  };

  
  /******************/
  /* Parser Classes */
  /******************/

  class Parser;

  class Parser_Info{
  public:
    class unknown{};		// word is unknown
    virtual values::Valtype get( Parser &p, std::string word ) 
      const throw(unknown, Parser_Error) = 0;
  };

  class Parser{
    enum items{ typeNumber, typeComma, 
		typeBracketLeft, typeBracketRight, typeAdd, typeSub, typeMul,
		typeDiv, typeMod, typePow, typeLess, typeGreater, 
		typeNotGreater, typeNotLess, typeEqual, typeNotEqual,
		typeOr, typeAnd, typeNot, typeQuest, typeColon, typeQuote,
		typeText, typeVectorAnf=typeLess, typeVectorEnd=typeGreater };

    stream::Word_Stream *stream;  

    int depth;         // depth of recursive function calls because of
    // Multiplications, Divisions, etc...
    int priority;      // Number which stands for the priority of the last 
    // calculation type

    std::string terminator; // this word terminates an expression

    items getType(std::string word);

    values::Valtype parseText( std::string word );

    values::Valtype read_Vector();
    values::Valtype read_Scalar();
    values::Valtype read_String();
    values::Valtype parseOperand( std::string word );

    values::Valtype parseTerm();

    typedef std::list< Parser_Info * > infostype;
    infostype infos;

  public:
    void error( int number, std::string message ) throw(Parser_Error); 

    std::string set_Terminator( std::string word );

    // functions of stream
    std::string get_Word();
    void unget_Word();
    std::string Parser::get_until( std::string terminator );
    std::string Parser::get_Line( std::string terminator = "" );
    // uses get_Word but throws an exception if there is another word returned
    void expect( std::string word, errornums errnum = errExpected ) 
      throw(Parser_Error);

    values::Valtype getTerm();
    values::Valtype getScalar();
    values::Valtype getVector();
    values::Valtype getString();

    stream::Pos_type *get_Pos();
    void set_Pos( stream::Pos_type *pos );

    void init( stream::Word_Stream *stream = 0 );

    void add_Parser_Info( Parser_Info *pi );

    Parser( stream::Word_Stream *s );

    ~Parser();
  };

  /*****************/
  /* Parser_Reader */
  /*****************/

  class Parser_Reader{
    Parser *parser;
    stream::Pos_type *pos;
    values::data_types type;
  public:
    values::Valtype get();
    Parser_Reader( Parser *p, values::data_types t );
    ~Parser_Reader();
  };
}

#endif

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

#include "parser.hpp"

/****************************************************************************/
/****************************** Implementation ******************************/
/****************************************************************************/

#include <functional>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>

#include "utils.hpp"

namespace parser{

  /******************/
  /* Error messages */
  /******************/
  
  Parser_Error::Parser_Error( int number, std::string message,
			      stream::Word_Stream *ws )
    : err::Basic_Pos_Error( number, message ), s(ws){
  }

  void Parser_Error::print_Position( std::ostream &os){
    s->print_Position(os);
  }

  void error( int number, std::string message, stream::Word_Stream *ws ) 
    throw( Parser_Error ){
    throw Parser_Error( number, message, ws );
  }


  /******************/
  /**** clParser ****/
  /******************/

  void Parser::error( int number, std::string message ) throw( Parser_Error ){
    throw Parser_Error( number, message, stream );
  }

  std::string Parser::get_Word(){
    return stream->get_Word();
  }

  void Parser::unget_Word(){
    stream->unget_Word();
  }

  std::string Parser::get_until( std::string terminator ){
    return stream->get_until( terminator );
  }

  std::string Parser::get_Line( std::string terminator ){
    return stream->get_Line( terminator );
  }

  void Parser::expect(std::string expected_word, errornums errnum /*=errExpected*/)
    throw( Parser_Error ){

    std::string word = get_Word();

    if( word != expected_word )
      {
	error( errnum, expected_word+" expected! "+word+" found instead");
      } 
  }

  Parser::items Parser::getType(std::string word){
    if( word == "(" )  return typeBracketLeft;
    if( word == ")" )  return typeBracketRight;
    if( word == "+" )  return typeAdd;
    if( word == "-" )  return typeSub;
    if( word == "*" )  return typeMul;
    if( word == "/" )  return typeDiv;
    if( word == "%" )  return typeMod;
    if( word == "^" )  return typePow;
    if( word == "," )  return typeComma;
    if( word == "<" )  return typeLess;
    if( word == ">" )  return typeGreater;
    if( word == "<=" ) return typeNotGreater;
    if( word == ">=" ) return typeNotLess;
    if( word == "=" )  return typeEqual;
    if( word == "!=" ) return typeNotEqual;
    if( word == "|" )  return typeOr;
    if( word == "&" )  return typeAnd;
    if( word == "!" )  return typeNot;
    if( word == "?" )  return typeQuest;
    if( word == ":" )  return typeColon;
    if( word == "\"" ) return typeQuote;

    if( isdigit( word[0] ) || (word[0] == '.') ) // ".25" is also a number
      return typeNumber;

    return typeText;
  }

  values::Valtype Parser::parseText( std::string word ){
    // ask "infos" whether they know the word
    infostype::const_iterator i;

    values::Valtype res;
    bool wrong_op = false;

    for( i = infos.begin(); i != infos.end(); i++ )
      {
	try{
	  res = (*i)->get( *this, word );
	  if( res.get_type() != values::undefined )
	    return res;

	  wrong_op = true;
	}
	catch( Parser_Info::unknown ){ // if word is unknown to Info
	  continue;
	}
      }

    if( wrong_op )
      {
	//error( errOpExpected, "Wrong operand \"" + word + "\"" );
	std::cerr << "warning: Unkwnown operand: " << word << std::endl;
      }
    
    error( errSyntax, "I don't know \"" + word + "\"" );
    return -1;
  }

  values::Valtype Parser::read_Vector(){
    expect( "<" , errVectAnfExpected );

    vect::vector3 res;

    std::string old_term = set_Terminator(","); // get old terminator
    res.coord[0] = getScalar().get_scalar();
    expect( "," , errCommaExpected );

    set_Terminator(","); 
    res.coord[1] = getScalar().get_scalar();
    expect( "," , errCommaExpected );

    set_Terminator(">"); 
    res.coord[2] = getScalar().get_scalar();
    expect( ">" , errVectEndExpected );

    set_Terminator( old_term ); // set old terminator
    
    return values::Valtype( res );
  }

  values::Valtype Parser::read_Scalar(){
    std::string word = get_Word();
    
    if( toupper(word[word.size()-1]) == 'E' )
      {
	std::string sign = get_Word();
	word += sign;
	if( (sign == "-") || (sign == "+") )
	  {
	    word += get_Word();
	  }
      }
    
    char *ret;

    double res = strtod( word.c_str(), &ret );

    if( *ret != 0 )		// did strtod find an error?
      error( errNumberExpected, "Number format error" );

    return values::Valtype( res );
  }

  values::Valtype Parser::read_String(){
    expect( "\"" , errQuotesExpected );
    std::string word, word2;

    int i,j;
    for(;;){
      word += get_Line("\"");

      // count backslashes at the end 
      for( i = word.size()-1, j=0; i >= 0; i--,j++ )
	if( word[i] != '\\' ) break;

      // if number is even the string is finished
      if( !(j & 1 ) ) break;

      // add quote that wasn't terminator
      word += '\"';
    }

    // check if string was terminated by quotes
    if( word.size() )
      if( word[ word.size() - 1 ] == '\n' )
	error( errQuotesExpected, "\" after string expected ");

    // remove escape backslashes
    for( int i=0; i<word.size(); i++ )
      {
	if( word[i] != '\\' )
	  word2 += word[i];
	else			// skip backslash
	  switch( word[++i] )
	    {
	    case 'a' : word2 += '\a'; break;
	    case 'b' : word2 += '\b'; break;
	    case 'f' : word2 += '\f'; break;
	    case 'n' : word2 += '\n'; break;
	    case 'r' : word2 += '\r'; break;
	    case 't' : word2 += '\t'; break;
	    case 'v' : word2 += '\v'; break;
	    case '0' : word2 += '\0'; break;
	    case '\\': word2 += '\\'; break;
	    case '\'': word2 += '\''; break;
	    case '"' : word2 += '\"'; break;
	    default:   word2 += word[i];
	    }
      }

    return values::Valtype( word2 );
  }

  values::Valtype Parser::parseOperand( std::string word ){
    values::Valtype Res;

    int savePri, saveDep;
    double x,y,z;

    switch( getType( word ) )
      {
      case typeVectorAnf:
	unget_Word();

	Res = read_Vector();
	break;
      case typeNumber:
	unget_Word();

	Res = read_Scalar();
	break;
      case typeQuote:
	unget_Word();

	Res = read_String();
	break;
      case typeText:
	Res = parseText( word );
	break;
      default:
	error( errOpExpected, "Operand expected");
      }

    return Res;
  }

  values::Valtype Parser::parseTerm(){
    values::Valtype res;

    std::string word = get_Word();

    int savePri;

    // a term has to be started by a value or a left bracket
    switch( getType(word) )
      {
      case typeBracketLeft:
	depth ++;

	savePri = priority;
	priority = -100; // at first there is no operation with higher priority

	res = parseTerm();

	expect( ")", errBracketRExpected );

	priority = savePri;

	depth --;
	break;

	// The factor can begin with a "not" ("!")
      case typeNot:
	savePri = priority;
	priority = 4; 

	res = parseTerm();

	priority = savePri;

	res = !res;
	break;

	// The vector might have an scalar factor or sign
      case typeSub:
	savePri = priority;
	priority = 4; 

	res = parseTerm();

	priority = savePri;

	res *= values::Valtype(-1);

	break;
      case typeAdd:
	savePri = priority;
	priority = 4; 

	res = parseTerm();

	priority = savePri;

	break;
      case typeText:
      case typeQuote:
      case typeVectorAnf:
      case typeNumber:
	res = parseOperand( word );
	break;
      default:
	error( errOpExpected, "Operand expected");
	unget_Word();
	return res;
      }

    values::Valtype op, op2;
    
    for(;;){
      word = get_Word();

      if( (word == terminator) && (depth == 0) )
	{
	  unget_Word();
	  return res;
	}

      // a term has to be started by a value or a left bracket
      switch( getType(word) )
	{
	case typeAdd:
	  if( priority >= 1 )      // if the last calculation is first
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = 1;

	  op = parseTerm();
	
	  res += op;

	  priority = savePri;
	  break;

	case typeSub:
	  if( priority >= 1 )      // if the last calculation is first
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = 1;

	  op = parseTerm();

	  res -= op;

	  priority = savePri;
	  break;

	case typeMul:
	  if( priority >= 2 )      // if the last calculation is first
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = 2;

	  op = parseTerm();

	  res *= op;

	  priority = savePri;
	  break;

	case typeDiv:
	  if( priority >= 2 )      // if the last calculation is first
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = 2;

	  op = parseTerm();

	  res /= op;

	  priority = savePri;
	  break;

	case typeMod:
	  if( priority >= 2 )      // if the last calculation is first
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = 2;

	  op = parseTerm();

	  res %= op;

	  priority = savePri;
	  break;

	case typePow:
	  if( priority >= 3 )      // if the last calculation is first
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = 3;

	  op = parseTerm();

	  res ^= op;

	  priority = savePri;
	  break;

	case typeEqual:
	  if( priority >= 0 )	
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = 0;

	  op = parseTerm();

	  res = (res == op);

	  priority = savePri;
	    
	  break;

	case typeNotEqual:
	  if( priority >= 0 )	
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = 0;

	  op = parseTerm();

	  res = !(res == op);

	  priority = savePri;
	    
	  break;

	case typeLess:
	  if( priority >= 0 )	
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = 0;

	  op = parseTerm();

	  res = (res < op);

	  priority = savePri;
	    
	  break;

	case typeGreater:
	  if( priority >= 0 )	
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = 0;

	  op = parseTerm();

	  res = (res > op);

	  priority = savePri;
	    
	  break;

	case typeNotGreater:
	  if( priority >= 0 )	
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = 0;

	  op = parseTerm();

	  res = !(res > op);

	  priority = savePri;
	    
	  break;

	case typeNotLess:
	  if( priority >= 0 )	
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = 0;

	  op = parseTerm();

	  res = !(res < op);

	  priority = savePri;
	    
	  break;

	case typeOr:
	  if( priority >= -2 )	
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = -2;

	  op = parseTerm();

	  res = (res || op);

	  priority = savePri;
	    
	  break;

	case typeAnd:
	  if( priority >= -2 )	
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = -2;

	  op = parseTerm();

	  res = (res && op);

	  priority = savePri;
	    
	  break;

	case typeQuest:
	  if( priority >= -3 )	
	    {
	      unget_Word();
	      return res;
	    }
	  savePri = priority;
	  priority = -3;

	  op = parseTerm();

	  expect( ":", errColonExpected );

	  op2 = parseTerm();

	  if( values::convert(res) )
	    {
	      res = op;
	    }
	  else
	    {
	      res = op2;
	    }

	  priority = savePri;
	    
	  break;

	case typeBracketRight:
	  unget_Word();
	  return res;

	default:
	  if( depth != 0 ) // if there are still brackets open
	    {
	      // work around for lack of real string streams
	      char depth_str[10];
	      sprintf( depth_str, "%d", depth );

	      error( errBracketRExpected, 
		     "Unknown item: " + word + 
		     " (there are still " + depth_str + " brackets open)");
	    }
	  unget_Word();
	  return res;
	}
    }
  }
  
  std::string Parser::set_Terminator( std::string word ){
    std::string old_term = terminator;
    terminator = word;
    return old_term;
  }

  values::Valtype Parser::getTerm(){
    int savepri = priority, savedep = depth;
    priority = -100;depth = 0;
  
    values::Valtype ret;
    try{
      ret = parseTerm();
    }
    catch( values::Type_Error &t ){
      error( t.get_number(), t.get_message() );
    }

    priority = savepri; depth = savedep;
  
    return ret;
  }

  values::Valtype Parser::getScalar(){
    values::Valtype res = getTerm();

    if( res.get_type() != values::scalar )
      {
	error( errScalarExpected, "Scalar expected" );
      }

    return res;
  }

  values::Valtype Parser::getVector(){
    values::Valtype res = getTerm();

    if( res.get_type() != values::vector )
      {
	error( errVectorExpected, "Vector expected" );
      }

    return res;
  }

  values::Valtype Parser::getString(){
    values::Valtype res = getTerm();

    if( res.get_type() != values::string )
      {
	error( errStringExpected, "Scalar expected" );
      }

    return res;
  }

  stream::Pos_type *Parser::get_Pos(){
    return stream->get_Pos();
  }

  void Parser::set_Pos( stream::Pos_type *pos ){
    stream = pos->get_Stream();
  }

  void Parser::init( stream::Word_Stream *ws ){
    depth = 0;
    priority = -100;

    if( ws != 0 ) stream = ws;
  }

  void Parser::add_Parser_Info( Parser_Info *pi ) {
    infos.push_back( pi );
  }

  Parser::Parser( stream::Word_Stream *s ){
    set_Terminator(" "); // no terminator!
    stream = s;
    init();
  }

  Parser::~Parser(){
    for_each( infos.begin(), infos.end(), utils::delete_ptr( infos ) );
  }

  /*****************/
  /* Parser_Reader */
  /*****************/

  values::Valtype Parser_Reader::get(){
    stream::Pos_type *save_pos = parser->get_Pos(); // save parser position
    parser->set_Pos( pos );	// change parser position to saved one

    try{
      values::Valtype x;
      switch( type )
	{
	case values::scalar:
	  x = parser->getScalar();
	  break;
	case values::vector:
	  x = parser->getVector();
	  break;
	case values::string:
	  x = parser->getString();
	  break;
	case values::undefined:
	  x = parser->getTerm();
	  break;
	}
      parser->set_Pos( save_pos ); // restore parser position
      delete save_pos;
      return x;
    }
    catch( parser::Parser_Error e){
      parser->set_Pos( save_pos ); // restore parser position
      
      // only syntax errors may be solved in later runs!
      if( e.get_number() != parser::errSyntax ) throw;
    }

    parser->set_Pos( save_pos );

    return values::Valtype();
  }

  Parser_Reader::Parser_Reader( Parser *p, values::data_types t ) 
    : parser( p ), type(t){
    pos = parser->get_Pos();
  }

  Parser_Reader::~Parser_Reader(){
    delete pos;
    delete parser;
  }
}

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

#include "mystream.hpp"

#include <functional>
#include <algorithm>
#include <strstream>

namespace stream{

  /******************/
  /* Error messages */
  /******************/

  End_Of_Stream::End_Of_Stream() 
    : err::Basic_Error( 0, "Unexpected end of Stream" ){
  }

  /***************/
  /* Char_Stream */
  /***************/

  char Char_Stream::get_ch() throw(End_Of_Stream){
    char ch;
    if( char_Buffer.size() )
      {
	ch = char_Buffer.top();
	char_Buffer.pop();
	return ch;
      }

    if( get(ch) )
      throw End_Of_Stream();
    return ch;
  }

  void Char_Stream::unget_ch(char ch){
    char_Buffer.push(ch);
  }

  void Char_Stream::unget_string(std::string str){
    for( off_type i = str.size(); i > 0; i-- )
      unget_ch( str[ i-1 ] );
  }

  void Char_Stream::clear_Buffer(){
    while(!char_Buffer.empty())
      char_Buffer.pop();
  }

  /******************/
  /* My_Word_Stream */
  /******************/

  void My_Word_Stream::add_comment( std::string beg, std::string end ){
    comment[ beg ] = end;
  }

  void My_Word_Stream::add_separator( std::string sep ){
    separator.insert( sep );
  }

  // clear Word buffer
  void My_Word_Stream::clear_Word_Buffer(){
    clear_Buffer();		// clear character buffer
    read_buffer.clear();
    read_buffer_pos = read_buffer.end();
  }


  std::string My_Word_Stream::read_Word(){
    std::string word;
    char ch;

    try{
      ch = get_ch();

      while( isspace(ch) ){
	word += ch;
	ch = get_ch();
      }

      while( !isspace(ch) ){
	word += ch;
	ch = get_ch();
      }

      unget_ch(ch);
    }
    catch( End_Of_Stream ){
      // no Problem! read_Word will return "" as EOS-indicator
    }

    return word;
  }

  std::string My_Word_Stream::check_seperator( std::string word ){
    // search separators in word
    off_type found_sep_pos = std::string::npos;
    off_type found_sep_len = 0;
    separatortype::const_iterator j, found_s;
    for( j = separator.begin(); j != separator.end(); j++ )
      {
	off_type p = word.find( *j );

	if( p != std::string::npos ) // was the separator found ?
	  {
	    // is this the first separator in word ...
	    if( found_sep_pos >= p )
	      // ... or is this the longest found separator
	      if( (found_sep_pos > p) ||
		  (found_sep_len < (j->size())) )
		{
		  found_s = j;
		  found_sep_len = j->size();
		  found_sep_pos = p;
		}
	  }
      }

    if( found_sep_len )
      {
	off_type p = word.find( *found_s );

	// is there a non space character before the seperator?
	for( off_type i = 0; i < p; i++ )
	  if( !isspace( word[i] ) )
	    {
	      // unget all from seperator to the end
	      unget_string( word.substr(p) );

	      return word.substr( 0, p );
	    }
	// was there no nonspace character before the seperator?

	// get position after seperator
	off_type endpos = p + found_s->size();

	// unget all after the seperator
	unget_string( word.substr(endpos) );

	return word.substr( 0, endpos );
      }
    return word;
  }

  std::string My_Word_Stream::check_comment( std::string word ){
    if( ignore_comments == false )
      return word;

    // search comments in word
    off_type found_comment_pos = std::string::npos;
    off_type found_comment_len = 0;
    commenttype::const_iterator i, found;
    for( i = comment.begin(); i != comment.end(); i++ )
      {
	off_type p = word.find( i->first );

	if( p != std::string::npos ) // was the comment begin found ?
	  {
	    // is this the first comment in word ...
	    if( found_comment_pos >= p )
	      // ... or is this the longest found comment in word
	      if( (found_comment_pos > p) ||
		  (found_comment_len < (i->first.size())) )
		{
		  found = i;
		  found_comment_len = i->first.size();
		  found_comment_pos = p;
		}
	  }
      }

    // was a commentbegin found?
    if( found_comment_len )
      {
	off_type p = word.find( found->first );

	// is there a non space character
	for( off_type i = 0; i < p; i++ )
	  if( !isspace( word[i] ) )
	    {
	      // unget all from seperator to the end
	      unget_string( word.substr(p) );

	      return word.substr( 0, p );
	    }

	// while end of comment isn't found
	while( ( p = word.find(found->second) ) == std::string::npos )
	  {
	    calc_pos_change( word );
	    word = read_Word();
	  }

	// get position after end of comment
	off_type endpos = p + found->second.size();

	calc_pos_change( word.substr(0,endpos) );

	// is there a non space character
	for( off_type k = endpos; k < word.size(); k++ )
	  if( !isspace( word[k] ) )
	    {
	      word = word.substr(endpos);

	      // check it also for comments!
	      word = check_comment( word );

	      return word;
	    }

	// add the next word to the spaces that might be after endpos
        word = word.substr(endpos) + read_Word();

	// check it also for comments!
	word = check_comment( word );

	return word;
      }
    return word;
  }


  std::string My_Word_Stream::read_Item(){
    std::string word;
    char ch;

    if( read_buffer_pos != read_buffer.end() )
      {
	word = read_buffer_pos->word;
	++read_buffer_pos;
      }
    else
      {
	word = read_Word();

	word = check_comment( word );	// check word for comments
	word = check_seperator( word ); // check word for seperators

	read_buffer.push_back( Word_Buffer( word, line, pos, offset ) );
	if( read_buffer.size() > max_read_buffer_size )
	  read_buffer.pop_front();

	read_buffer_pos = read_buffer.end();
      }

    calc_pos_change( word );

    return word;
  }

  // reads an Item and deletes the spaces
  std::string My_Word_Stream::get_Word(){
    std::string word = read_Item();

    for( std::string::size_type i = 0; i < word.size();  )
      {
	if( isspace( word[i] ) )
	  {
	    word.erase( i, i+1 );
	  }
	else
	  ++i;
      }

    return word;
  }

  void My_Word_Stream::unget_Word(){
    read_buffer_pos--;
    line   = read_buffer_pos->line;
    pos    = read_buffer_pos->pos;
    offset = read_buffer_pos->offset;
  }

  std::string My_Word_Stream::get_until( std::string terminator ){
    std::string res;
    std::string word;

    off_type p;

    for(;;){
      word = read_Item();

      p = word.find( terminator );

      if( p != std::string::npos )
	break;

      res += word;
    }

    res += word.substr(0,p);
    return res;
  }

  std::string My_Word_Stream::get_Line( std::string terminator ){
    std::string res;
    std::string word;

    off_type p;
    off_type q;
    for(;;){
      word = read_Item();

      if( word == "" )	
	{
	  if( res == "" )
	    throw End_Of_Stream();
	  else
	    word = "\n";
	}

      p = word.find( terminator );
      q = word.find( '\n' );

      if( (p != std::string::npos) || (q != std::string::npos) )
	break;

      res += word;
    }

    res += word.substr( 0, p<q ? p:q );
    
    if( q<p ) // newline as terminator is added to result
      res += '\n';

    return res;
  }

  void My_Word_Stream::calc_pos_change(std::string word){
    for( std::string::iterator i = word.begin(); i != word.end(); i++ )
      {
	offset++;

	switch( *i )
	  {
	  case '\n':
	    ++line;
	    pos = 1;
	    break;
	  case '\t':
	    pos = ((pos-1)/tab_len + 1)*tab_len + 1; // next tabulator?
	    break;
	  default:
	    ++pos;
	    break;
	  }
      }
  }

  void My_Word_Stream::print_Position(std::ostream &os){
    os << line << ":" << pos << ":";
  }

  void My_Word_Stream::disable_comments(){
    ignore_comments = false;
  }

  void My_Word_Stream::enable_comments(){
    ignore_comments = true;
  }

  My_Word_Stream::My_Word_Stream()
    : pos(1), line(1), offset(0), tab_len(8),
      max_read_buffer_size(10),read_buffer_pos(read_buffer.end()){ 

    enable_comments();
  }

  My_Word_Stream::~My_Word_Stream(){
  }

  /************************/
  /* additional functions */
  /************************/

  // sets comments and separators of an My_Word_Stream like C-Syntax
  void mk_C_Stream( My_Word_Stream &stream ){
    stream.add_separator( "{" );
    stream.add_separator( "}" );
    stream.add_separator( "(" );
    stream.add_separator( ")" );
    stream.add_separator( ";" );
    stream.add_separator( "," );

    stream.add_separator( "=" );
    stream.add_separator( "==" );
    stream.add_separator( "+" );
    stream.add_separator( "+=" );
    stream.add_separator( "-" );
    stream.add_separator( "-=" );
    stream.add_separator( "*" );
    stream.add_separator( "*=" );
    stream.add_separator( "/" );
    stream.add_separator( "/=" );
    stream.add_separator( "%" );
    stream.add_separator( "%=" );
    stream.add_separator( "<" );
    stream.add_separator( "<=" );
    stream.add_separator( ">" );
    stream.add_separator( ">=" );
    stream.add_separator( "^" );
    stream.add_separator( "^=" );
    stream.add_separator( "!" );
    stream.add_separator( "!=" );
    stream.add_separator( "&" );
    stream.add_separator( "&=" );
    stream.add_separator( "|" );
    stream.add_separator( "|=" );
    stream.add_separator( "~" );
    stream.add_separator( "?" );
    stream.add_separator( ":" );

    stream.add_separator( "&&" );
    stream.add_separator( "||" );
    stream.add_separator( "++" );
    stream.add_separator( "--" );

    stream.add_separator( "'" );
    stream.add_separator( "\"" );

    stream.add_separator( "/*" );
    stream.add_separator( "*/" );
    stream.add_separator( "//" );
    stream.add_comment( "/*", "*/" );
    stream.add_comment( "//", "\n" );
  }

  My_Word_Stream &operator ++( My_Word_Stream &stream ){
    mk_C_Stream( stream );
    return stream;
  }

  /*****************/
  /* String_Stream */
  /*****************/

  char String_Stream::get( char &ch ){
    if( pos >= str.size() )
      return -1;

    ch = str[ pos ];
    ++pos;
    return 0;
  }

  off_type String_Stream::get_pos(){
    return pos;
  }

  void String_Stream::set_pos( off_type p ){
    pos = p;
  }

  Pos_type *String_Stream::get_Pos(){
    return new String_Pos_type( this );
  }

  void String_Stream::print_Position(std::ostream &os){
    os << pos << ':';
  }

  String_Stream::String_Stream( std::string string ) : str( string ), pos(0) {}

  /*******************/
  /* String_Pos_type */
  /*******************/

  Word_Stream *String_Pos_type::get_Stream(){
    stream->set_pos( pos );
    return stream;
  }

  void String_Pos_type::print_Position(std::ostream &os){
    os << pos << ':';
  }

  String_Pos_type::String_Pos_type(String_Stream *ss) : stream(ss){
    pos = ss->get_pos();
  }
  
}

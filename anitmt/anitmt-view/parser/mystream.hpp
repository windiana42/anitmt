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

#ifndef __mystream__
#define __mystream__

namespace stream{
  class Pos_type;
  class Char_Stream;
  class Word_Stream;
}


#include <string>
#include <iostream>
#include <stack>
#include <list>
#include <map>
#include <set>

#include "error.hpp"

namespace stream{

  typedef std::string::size_type off_type;

  class Pos_type{
  public:
    virtual Word_Stream *get_Stream() = 0;
    virtual void print_Position(std::ostream &os) = 0;

    virtual ~Pos_type(){}
  };

  
  class Word_Stream{
  public:
    // gets the next word
    virtual std::string get_Word() = 0;
    // puts a word back to the buffer
    virtual void unget_Word() = 0;
    // reads until terminater
    virtual std::string get_until(std::string terminator) = 0;
    // reads until line end or terminator
    virtual std::string get_Line(std::string terminator = "") = 0; 
  
    virtual Pos_type *get_Pos() = 0;
    virtual void print_Position(std::ostream &os) = 0;
  };
  

  class End_Of_Stream : public err::Basic_Error{
  public:
    End_Of_Stream();
  }; // End of File Exception
  
  class Char_Stream{
    // ungets character
    std::stack<char> char_Buffer;

  protected:
    // gets character
    virtual char get( char &ch ) = 0;
  public:
    void clear_Buffer();

    char get_ch() throw(End_Of_Stream);
    void unget_ch(char ch);
    void unget_string(std::string str);
    virtual ~Char_Stream(){}
  };

  class My_Word_Stream : public Char_Stream, public Word_Stream{
    typedef std::map< std::string, std::string > commenttype;
    commenttype comment;
    typedef std::set< std::string > separatortype;
    separatortype separator;

    struct Word_Buffer{
      std::string word;
      off_type line;		// linenumber
      off_type pos;		// column 
      off_type offset;		// character offset in Stream
      Word_Buffer( std::string w,
		   off_type l,
		   off_type p,
		   off_type f)
	: word(w), line(l), pos( p ), offset( f ) {}
    };

    typedef std::list<Word_Buffer> buffertyp;
    buffertyp read_buffer;
    buffertyp::iterator read_buffer_pos;
    int max_read_buffer_size;

    // calculates the position that was changed by reading the word
    void calc_pos_change(std::string word);

    // reads one word
    std::string read_Word();
    // checks word for seperators
    std::string check_seperator( std::string word );
    // checks word for comments
    std::string check_comment( std::string word );
    // reads one Item that may be seperated by seperators like "="
    // it may ignore comments
    std::string read_Item();

    bool ignore_comments;

    int tab_len;		// length of tabulator (default: 8)
  protected:
    off_type line;		// current line
    off_type pos;		// current position
    off_type offset;

  public:
    void clear_Word_Buffer();

    void add_comment( std::string, std::string );
    void add_separator( std::string );

    // gets the next word
    std::string get_Word();
    // puts a word back to the buffer
    void unget_Word();
    // reads until terminater
    std::string get_until(std::string terminator);
    // reads until line end or terminator
    std::string get_Line(std::string terminator = "");
    // prints Position
    virtual void print_Position(std::ostream &os);
    // gets the current position
    virtual Pos_type *get_Pos() = 0;

    void disable_comments();
    void enable_comments();

    My_Word_Stream();
    virtual ~My_Word_Stream();
  };

  void mk_C_Stream( My_Word_Stream &stream ); // initializes for C syntax
  My_Word_Stream &operator ++( My_Word_Stream &stream ); // do the same

  class String_Stream : public My_Word_Stream{
    std::string str;
    std::string::size_type pos;
  protected:
    virtual char get( char &ch );
  public:
    off_type get_pos();
    void set_pos( off_type );

    Pos_type *get_Pos();
    void print_Position(std::ostream &os);
    String_Stream( std::string );
  };

  class String_Pos_type : public Pos_type{
    String_Stream *stream;
    std::string::size_type pos;
  public:
    virtual Word_Stream *get_Stream();
    virtual void print_Position(std::ostream &os);

    String_Pos_type(String_Stream *ss);
  };

}
#endif

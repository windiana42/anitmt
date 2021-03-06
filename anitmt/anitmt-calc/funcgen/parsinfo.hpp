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

#ifndef __anitmt_afd_parsinfo__
#define __anitmt_afd_parsinfo__

#include "token.hpp"

#include <stack>
#include <fstream>

#include <message/message.hpp>

#include "afdbase.hpp"

class funcgen_FlexLexer;	// lexer prototype

namespace funcgen
{
  class myFlex; // forward declaration (defined in parser_functions.hpp)

  //**********************************************************
  // afd_info: stores information for parser and lexer
  //**********************************************************

  // Info opbject that is passed to the parser
  class afd_info
  {
  private:
    std::deque<message::Abstract_Position*> old_positions;
    unsigned max_old_positions;
    std::stack<std::ifstream*> in_files;	// output file stream 
    std::stack<message::File_Position*> positions; // open file positions
  public:
    afd_info( AFD_Root *afd, message::Message_Consultant *consultant );
    ~afd_info();

    // open file to be read by the lexer (may be recursively)
    bool open_file( std::string filename, bool output_msg=true );
    bool close_file(); // false: still another file open

    //*************************************************
    // only for manipulation from yylex() and yyparse()

    message::Message_Reporter msg;	// this offers message streams
    myFlex *lexer;		// lexical analyzer / scanner
    AFD_Root *afd;			// manages all afd related data

    int tab_len;
    message::File_Position file_pos;
    std::ifstream &get_in_file();	// output file stream 
    bool lexer_uses_file_stream; // whether lexer is created for in_file

    int depth_counter;		// depth counter for the scanner

    //! store position for later access
    void store_pos();
    //! get current position (must be deleted!)
    message::Abstract_Position *get_pos();
    //! get stored position n (n=0: last) (must be deleted!)
    message::Abstract_Position *get_old_pos( unsigned n );
    //! set maximum number of stored positions
    void set_max_old_positions( unsigned n );
  };

}

#ifndef yyFlexLexer		// is FlexLexer not yet defined?
#define yyFlexLexer funcgen_FlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

//include inline implementation
#ifdef EXTREME_INLINE
#include "parsinfo_inline.cpp"
#endif

#endif


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
  //**********************************************************
  // afd_info: stores information for parser and lexer
  //**********************************************************

  // Info opbject that is passed to the parser
  class afd_info
  {
  private:
    std::deque<message::Abstract_Position*> old_positions;
    unsigned max_old_positions;
  public:
    afd_info( AFD_Manager *afd, message::Message_Consultant *consultant );
    ~afd_info();

    // open file to be read by the lexer
    void open_file( std::string filename );
    void close_file();

    // open file to be read by the lexer
    void open_stream( std::string filename, std::istream &in );

    //*************************************************
    // only for manipulation from yylex() and yyparse()

    message::Message_Reporter msg;	// this offers message streams
    funcgen_FlexLexer *lexer;		// lexical analyzer / scanner
    AFD_Manager *afd;			// manages all afd related data

    int tab_len;
    message::File_Position file_pos;
    std::ifstream in_file;	// output file stream 
    bool lexer_uses_file_stream; // whether lexer is created for in_file

    //! store position for later access
    inline void store_pos();
    //! get current position (must be deleted!)
    inline message::Abstract_Position *get_pos();
    //! get stored position n (n=0: last) (must be deleted!)
    inline message::Abstract_Position *get_old_pos( unsigned n );
    //! set maximum number of stored positions
    inline void set_max_old_positions( unsigned n );
  };

}

#ifndef yyFlexLexer		// is FlexLexer not yet defined?
#define yyFlexLexer funcgen_FlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

//include inline implementation
#include "parsinfo_inline.cpp"

#endif


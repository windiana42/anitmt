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

#include "parsinfo.hpp"

namespace funcgen
{
  //**********************************************************
  // adlparser_info: stores information for parser and lexer
  //**********************************************************

  // open file to be read by the lexer
  void afd_info::open_file( std::string filename )
  {
    file_pos.set_filename( filename );
    in_file.open( filename.c_str() );
    lexer->set_input_stream( in_file );
    lexer_uses_file_stream = true;
  }
  
  void afd_info::close_file()
  {
    lexer_uses_file_stream = false;
    lexer->set_input_stream( cin );
    in_file.close();
  }

  // open file to be read by the lexer
  void afd_info::open_stream( std::string filename, std::istream &in )
  {
    file_pos.set_filename( filename );
    if( lexer ) delete lexer;
      
    lexer = new funcgen_FlexLexer(&in);      
    lexer->info = this;
    lexer_uses_file_stream = false;
  }

  afd_info::afd_info( AFD_Manager *afd_manager, 
		      message::Message_Consultant *consultant )
    : old_positions(10), max_old_positions(10),
      msg(consultant), lexer(new funcgen_FlexLexer(&cin)), afd(afd_manager),
      lexer_uses_file_stream( false )
  {
    lexer->info = this;
    file_pos.set_filename("standard input");
  }
  afd_info::~afd_info()
  {
    delete lexer;
  }
}

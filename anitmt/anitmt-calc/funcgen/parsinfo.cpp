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
  bool afd_info::open_file( std::string filename, bool output_msg )
  {
    std::ifstream *is = new std::ifstream(filename.c_str());
    if( !(*is) )		// couldn't open file?
    {
      msg.error(message::GLOB::no_position) 
	<< "fatal error: couldn't open input file " << filename;
      return false;
    }
    in_files.push( is );
    positions.push( file_pos.duplicate() );

    file_pos.set_filename( filename );
    lexer->set_input_stream( *is );
    lexer_uses_file_stream = true;
    return true;
  }
  
  bool afd_info::close_file()
  {
    // remove current file from stack
    delete in_files.top();
    in_files.pop();
    delete positions.top();
    positions.pop();
    if( in_files.empty() )
    {
      lexer_uses_file_stream = false;
      lexer->set_input_stream( cin );
      return true;
    }
    else
    {
      lexer->set_input_stream( *in_files.top() );
      file_pos = *positions.top();
      return false;		// still another file open
    }
  }

  afd_info::afd_info( AFD_Root *AFD_Root, 
		      message::Message_Consultant *consultant )
    : old_positions(10), max_old_positions(10),
      msg(consultant), lexer(new funcgen_FlexLexer(&cin)), afd(AFD_Root),
      lexer_uses_file_stream( false )
  {
    lexer->info = this;
    file_pos.set_filename("standard input");
    msg.set_msg_default_position( &file_pos ); // set default message position
  }
  afd_info::~afd_info()
  {
    delete lexer;
  }
}

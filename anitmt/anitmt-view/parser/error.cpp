/*****************************************************************************/
/**   This file belongs to a parser library and is used by AniTMT	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/*****************************************************************************/

#include "error.hpp"

namespace err{

  /******************/
  /* Error messages */
  /******************/
  
  std::string My_Error::get_message(){
    return msg;
  }
  
  int My_Error::get_number(){
    return num;
  }

  My_Error::My_Error( int number, std::string message )
    : msg( message ), num( number ){
  }

  std::string Basic_Error::get_message(){
    return msg;
  }
  
  int Basic_Error::get_number(){
    return num;
  }

  Basic_Error::Basic_Error( int number, std::string message )
  : msg( message ), num( number ){
  }

  Basic_Error::~Basic_Error(){
  }

  Basic_Pos_Error::Basic_Pos_Error( int number, std::string message )
    : Basic_Error( number, message ){
  }

  void error( int number, std::string message ) throw( Basic_Error ){
    throw Basic_Error( number, message );
  }

  File_Open_Error::File_Open_Error( int number, std::string message, 
				    std::string file )
    : err::Basic_Pos_Error( number, message ), f(file){
  }

  void File_Open_Error::print_Position( std::ostream &os){
    os << f << ':';
  }

}

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

#ifndef __aniTMT_error__
#define __aniTMT_error__

#include <string>
#include <iostream>

namespace err{

  /******************/
  /* Error messages */
  /******************/
  
  // Internal fatal error that is caused by myself
  class My_Error{
    std::string msg;
    int num;
  public:
    std::string get_message();
    int get_number();

    My_Error( int number, std::string message );
  };

  // Animation Error, should be a fault of the user
  class Basic_Error{
    std::string msg;
    int num;
  public:
    std::string get_message();
    int get_number();

    Basic_Error( int number, std::string message );
    ~Basic_Error();
  };

  void error( int number, std::string message ) throw(Basic_Error); 

  class Basic_Pos_Error : public Basic_Error{
  public:
    virtual void print_Position( std::ostream &os) = 0;

    Basic_Pos_Error( int number, std::string message );
    virtual ~Basic_Pos_Error(){}
  };

  class File_Open_Error : public Basic_Pos_Error{
    std::string f;

  public:
    void print_Position( std::ostream &os);

    File_Open_Error( int number, std::string message, std::string file );
  };


}
#endif

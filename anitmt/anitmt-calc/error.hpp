/*****************************************************************************/
/**   Offers Exception classes for error handling               	    **/
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

#ifndef __AniTMT_Error__
#define __AniTMT_Error__

#include <string>

namespace anitmt {
  //******************************
  // EX: general anitmt exception
  //******************************

  class EX {
    std::string name;
  public:
    inline std::string get_name() { return name; }
    EX( std::string name );
    virtual ~EX();
  };

  //****************************************
  // Error_Position: general error position
  //****************************************

  class Error_Position {
  public:
    virtual ~Error_Position();
  };

  //******************************
  // EX: general user error
  //******************************

  class EX_user_error : public EX {
    Error_Position *pos;
  public:
    EX_user_error( std::string name, Error_Position *pos );
  };
}
#endif

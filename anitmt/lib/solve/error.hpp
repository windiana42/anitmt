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

#ifndef __Solve_Error__
#define __Solve_Error__

#include <string>

namespace solve {
  //******************************
  // EX: general solve exception
  //******************************

  class EX {
    std::string name;
  public:
    inline std::string get_name() { return name; }
    EX(const std::string name );
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
    EX_user_error(const std::string name, Error_Position *pos );
  };
}
#endif

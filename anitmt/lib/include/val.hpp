/*****************************************************************************/
/**   This file offers datatypes designed for AniTMT			    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/*****************************************************************************/

#ifndef __values__
#define __values__

#include <string>

namespace values{

  class Scalar;
  class Vector;
  class String;
  class Flag;

  class Valtype{
    enum Types{ scalar, vector, string, flag };

    Types type;
  public:
    Types get_type();
  };

  class Scalar : public Valtype{
    double x;
  public:
    operator double() const;

    Scalar( double i );
    Scalar();
  };
  
  class Vector : public Valtype{
  public:
  };

  class String : public Valtype{
    std::string x;
  public:
  };

  class Flag : public Valtype{
    bool x;
  public:
  };
}
#endif

/*****************************************************************************/
/**   This file offers extensions to the standard C++ library               **/
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

#include "stdextend.hpp"

//***********
// help stuff
//***********
namespace std
{
  // append number formats to strings
  std::string operator+( const std::string &s, int v )
  {
    char append[_max_digits];
    std::ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    return s + append;
  }
  std::string operator+( const std::string &s, long v )
  {
    char append[_max_digits];
    std::ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    return s + append;
  }
  std::string operator+( const std::string &s, double v )
  {
    char append[_max_digits];
    std::ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    return s + append;
  }
  // append number formats to strings 
  std::string &operator+=( std::string &s, int v )
  {
    char append[_max_digits];
    std::ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    s += append; return s;
  }
  std::string &operator+=( std::string &s, long v )
  {
    char append[_max_digits];
    std::ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    s += append; return s;
  }
  std::string &operator+=( std::string &s, double v )
  {
    char append[_max_digits];
    std::ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    s += append; return s;
  }
}


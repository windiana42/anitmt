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

#include <sstream>
#include <cctype>

//***********
// help stuff
//***********
namespace std
{
  // append number formats to strings
  std::string operator+( const std::string &s, int v )
  {
    std::ostringstream os;
    os << v;
    return s + os.str();
  }
  std::string operator+( const std::string &s, long v )
  {
    std::ostringstream os;
    os << v;
    return s + os.str();
  }
  std::string operator+( const std::string &s, double v )
  {
    std::ostringstream os;
    os << v;
    return s + os.str();
  }
  // os.str() number formats to strings 
  std::string &operator+=( std::string &s, int v )
  {
    std::ostringstream os;
    os << v;
    s += os.str(); 
    return s;
  }
  std::string &operator+=( std::string &s, long v )
  {
    std::ostringstream os;
    os << v;
    s += os.str(); 
    return s;
  }
  std::string &operator+=( std::string &s, double v )
  {
    std::ostringstream os;
    os << v;
    s += os.str(); 
    return s;
  }
/* not needed any more for C++11
  // convert number formats to strings
  string to_string( int v )
  {
    ostringstream os;
    os << v;
    return os.str();
  }
  string to_string( long v )
  {
    ostringstream os;
    os << v;
    return os.str();
  }
  string to_string( double v )
  {
    ostringstream os;
    os << v;
    return os.str();
  }
*/

  string to_upper( std::string str )
  {
    string ret;
    for( string::size_type i = 0; i < str.size(); ++i )
    {
      ret += char(toupper(str[i]));
    }
    return ret;
  }
  bool starts_with( std::string str, std::string cmp )
  {
    return str.find(cmp) == 0;
  }
}

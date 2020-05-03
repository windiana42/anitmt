/*****************************************************************************/
/**   Extends functions in the std namespace                    	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#include "stdextend.hpp"

#include <sstream>

/* not Needed for C++11 any more:
namespace std
{
  //*****************************************************************
  // string operators
  //*****************************************************************

  // append number formats to strings
  string operator+( const string &s, int v )
  {
    ostringstream os;
    os << v;
    return s + os.str();
  }
  string operator+( const string &s, long v )
  {
    ostringstream os;
    os << v;
    return s + os.str();
  }
  string operator+( const string &s, double v )
  {
    ostringstream os;
    os << v;
    return s + os.str();
  }
  string operator+( const string &s, values::Flag v )
  {
    ostringstream os;
    os << v;
    return s + os.str();
  }
  string operator+( const string &s, values::Scalar v )
  {
    ostringstream os;
    os << v;
    return s + os.str();
  }
  string operator+( const string &s, values::Vector v )
  {
    ostringstream os;
    os << v;
    return s + os.str();
  }
  string operator+( const string &s, values::Matrix v )
  {
    ostringstream os;
    os << v;
    return s + os.str();
  }
  //string &operator+( const string &s, values::String v ); // obsolete

  // os.str() number formats to strings 
  string &operator+=( string &s, int v )
  {
    ostringstream os;
    os << v;
    s += os.str(); 
    return s;
  }
  string &operator+=( string &s, long v )
  {
    ostringstream os;
    os << v;
    s += os.str(); 
    return s;
  }
  string &operator+=( string &s, double v )
  {
    ostringstream os;
    os << v;
    s += os.str(); 
    return s;
  }
  string &operator+=( string &s, values::Flag v )
  {
    ostringstream os;
    os << v;
    s += os.str(); 
    return s;
  }
  string &operator+=( string &s, values::Scalar v )
  {
    ostringstream os;
    os << v;
    s += os.str(); 
    return s;
  }
  string &operator+=( string &s, values::Vector v )
  {
    ostringstream os;
    os << v;
    s += os.str();
    return s;
  }
  string &operator+=( string &s, values::Matrix v )
  {
    ostringstream os;
    os << v;
    s += os.str();
    return s;
  }

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
  string to_string( values::Flag v )
  {
    ostringstream os;
    os << v;
    return os.str();
  }
  string to_string( values::Scalar v )
  {
    ostringstream os;
    os << v;
    return os.str();
  }
  string to_string( values::Vector v )
  {
    ostringstream os;
    os << v;
    return os.str();
  }
  string to_string( values::Matrix v )
  {
    ostringstream os;
    os << v;
    return os.str();
  }
}
*/

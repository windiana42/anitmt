/*****************************************************************************/
/**   Extends functions in the std namespace                    	    **/
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

#warning should be replaced by real stringstream (GCC 3.0)

#include <strstream>

#include "stdextend.hpp"

namespace std
{
  static const int _max_digits = 100;

  //*****************************************************************
  // string operators
  //*****************************************************************

  // append number formats to strings
  string operator+( const string &s, int v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    return s + append;
  }
  string operator+( const string &s, long v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    return s + append;
  }
  string operator+( const string &s, double v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    return s + append;
  }
  string operator+( const string &s, values::Flag v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    return s + append;
  }
  string operator+( const string &s, values::Scalar v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    return s + append;
  }
  string operator+( const string &s, values::Vector v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    return s + append;
  }
  string operator+( const string &s, values::Matrix v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    return s + append;
  }
  //string &operator+( const string &s, values::String v ); // obsolete

  // append number formats to strings 
  string &operator+=( string &s, int v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    s += append; return s;
  }
  string &operator+=( string &s, long v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    s += append; return s;
  }
  string &operator+=( string &s, double v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    s += append; return s;
  }
  string &operator+=( string &s, values::Flag v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    s += append; return s;
  }
  string &operator+=( string &s, values::Scalar v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    s += append; return s;
  }
  string &operator+=( string &s, values::Vector v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    s += append; return s;
  }
  string &operator+=( string &s, values::Matrix v )
  {
    char append[_max_digits];
    ostrstream os(append, _max_digits );
    os << v;
    os << '\0';
    s += append; return s;
  }
}


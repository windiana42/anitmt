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

#include <string>

//***********
// help stuff
//***********
namespace std
{
  // append number formats to strings
  std::string operator+( const std::string &s, int v );
  std::string operator+( const std::string &s, long v );
  std::string operator+( const std::string &s, double v );
  // append number formats to strings 
  std::string &operator+=( std::string &s, int v );
  std::string &operator+=( std::string &s, long v );
  std::string &operator+=( std::string &s, double v );

/* not needed any more for C++11
  string to_string( int );
  string to_string( long );
  string to_string( double );
*/

  string to_upper( std::string );
  bool starts_with( std::string str, std::string cmp );
}

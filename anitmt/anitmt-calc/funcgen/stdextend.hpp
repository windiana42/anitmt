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
#include <strstream>
#define _max_digits 20

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
}


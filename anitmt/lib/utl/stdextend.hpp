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

#ifndef __STD_Extended_functions__
#define __STD_Extended_functions__

#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <val/val.hpp>

namespace std
{

  //*****************************************************************
  // string operators
  //*****************************************************************

  // append number formats to strings
  string operator+( const string &s, int );
  string operator+( const string &s, long );
  string operator+( const string &s, double );
  string operator+( const string &s, values::Flag v );
  string operator+( const string &s, values::Scalar v );
  string operator+( const string &s, values::Vector v );
  string operator+( const string &s, values::Matrix v );
  //string &operator+( string, values::String v ); // obsolete

  // append number formats to strings
  string operator+=( const string &s, int );
  string operator+=( const string &s, long );
  string operator+=( const string &s, double );
  string operator+=( const string &s, values::Flag v );
  string operator+=( const string &s, values::Scalar v );
  string operator+=( const string &s, values::Vector v );
  string operator+=( const string &s, values::Matrix v );
  //string &operator+=( string, values::String v ); // obsolete

  string to_string( int );
  string to_string( long );
  string to_string( double );
  string to_string( values::Flag v );
  string to_string( values::Scalar v );
  string to_string( values::Vector v );
  string to_string( values::Matrix v );
}
#endif


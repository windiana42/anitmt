/*****************************************************************************/
/**   This file offers various utilities                        	    **/
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

#include "utl.hpp"

namespace utl{

  //*****************************************************************
  // maped_string: may used as string but stores one string only once
  //*****************************************************************

  maped_string::str2id_type maped_string::str2id;
  maped_string::id2str_type maped_string::id2str;
  maped_string::id_type maped_string::max_id = 0;

  maped_string::operator std::string() const{
    return id2str[id];
  }
  maped_string::maped_string( const std::string &s ){
    str2id_type::iterator i = str2id.find( s );
    if( i != str2id.end() )
      id = i->second;
    else
      {
	id = ++max_id;
	str2id[ s ] = id;
	id2str[ id ] = s;
      }
  }
  maped_string::maped_string( const char *s ){
    str2id_type::iterator i = str2id.find( std::string(s) );
    if( i != str2id.end() )
      id = i->second;
    else
      {
	id = ++max_id;
	str2id[ std::string(s) ] = id;
	id2str[ id ] = std::string(s);
      }
  }
  maped_string::~maped_string(){}


  bool operator == (const maped_string &s1, const maped_string &s2){
    return s1.id == s2.id;
  }
  bool operator == (const maped_string &s1, const std::string &s2){
    return std::string(s1) == s2;
  }
  bool operator == (const maped_string &s1, const char * s2){
    return std::string(s1) == std::string(s2);
  }

  // only for search in a map container
  bool operator <  (const maped_string &s1, const maped_string &s2){
    return s1.id < s2.id;
  }

  std::ostream &operator << (std::ostream &os, const maped_string &s){
    os << std::string(s);
    return os;
  }
}


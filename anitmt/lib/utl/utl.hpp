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

#ifndef __Utilities__
#define __Utilities__

#include <string>
#include <map>
#include <iostream>

namespace utl{

  //*****************************************************************
  // maped_string: may used as string but stores one string only once
  //*****************************************************************

  class maped_string{
    typedef int id_type;
    id_type id;

    // associates an id with the according string 
    typedef std::map< std::string, id_type > str2id_type;
    static str2id_type str2id;
    // associates a string with the according id
    typedef std::map< id_type, std::string > id2str_type;
    static id2str_type id2str;

    // current maximum id number
    static id_type max_id;
  public:
    operator std::string() const;
    friend bool operator == (const maped_string &s1, const maped_string &s2);
    friend bool operator == (const maped_string &s1, const std::string &s2);
    friend bool operator == (const maped_string &s1, const char * s2);
    // only for search in a map container !
    friend bool operator <  (const maped_string &s1, const maped_string &s2);

    maped_string( const std::string & );
    maped_string( const char * );
    ~maped_string();
  };

  bool operator <  (const maped_string &s1, const maped_string &s2);
  bool operator == (const maped_string &s1, const maped_string &s2);
  bool operator == (const maped_string &s1, const std::string &s2);
  bool operator == (const maped_string &s1, const char* s2);

  std::ostream &operator << (std::ostream &os, const maped_string &s);
}
#endif


/*****************************************************************************/
/**   This file offers various utilities                        	    **/
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

#ifndef __Utilities__
#define __Utilities__

#include <string>
#include <list>
#include <map>
#include <iostream>

namespace utl
{

  // ************************************************************************
  // assemble_list: generates a std::list in one expression with operator +
  // ************************************************************************

  template<class T>
  class Assemble_List
  {
  public:
    inline operator std::list<T>() { return stored_list; }
    std::list<T> get()		   { return stored_list; }
    inline Assemble_List &operator << ( const T &element ) 
    { stored_list.push_back(element); return *this; }
    Assemble_List &add( const T &element )
    { stored_list.push_back(element); return *this; }
  private:
    std::list<T> stored_list;
  };

  template<class T>
  inline Assemble_List<T> assemble_list()
  { return Assemble_List<T>(); }
  template<class T>
  inline Assemble_List<T> assemble_list(const T &element)
  { return Assemble_List<T>() << element; }
  template<class T>
  inline Assemble_List<T> assemble_list(const T &element1, 
					      const T &element2)
  { return Assemble_List<T>() << element1 << element2; }
  template<class T>
  inline Assemble_List<T> assemble_list(const T &element1,
					      const T &element2,
					      const T &element3)
  { return Assemble_List<T>() << element1 << element2 << element2; }

  // *****************************************************************
  // maped_string: may used as string but stores one string only once
  // *****************************************************************

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


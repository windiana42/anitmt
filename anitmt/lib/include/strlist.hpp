/*
 * strlist.hpp 
 * String list class header. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser
 * Bugs to wwieser@gmx.de
 * 
 * This is a part of the aniTMT animation project. 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Revision History:
 *   some time in 2001   started writing
 *
 */

#ifndef __stringlist_hpp__
#define __stringlist_hpp__

#include <string>
#include <list>

namespace anitmt
{

class stringlist : public std::list<std::string>
{
	private:
		void _addlist(const stringlist &sl);
	public:
		stringlist() : std::list<std::string>() { }
		stringlist(const stringlist &sl) : std::list<std::string>()
			{  _addlist(sl);  }
		stringlist(const char *str0,...);  // NULL-terminated!!!
		~stringlist()  {  clear();  }
		
		stringlist &operator=(const stringlist &sl)
			{  clear();  _addlist(sl);  return(*this);  }
		stringlist &operator+=(const stringlist &sl)
			{  _addlist(sl);  return(*this);  }
		
		bool is_empty()
			{  return((begin()==end()) ? true : false);  }
		
		// Add an entry at the end of the list: 
		void add(const std::string &str)  {  push_back(str);  }
		void add(const char *str)  {  add(std::string(str));  }
		
		// To write the string list to a stream: 
		friend ostream& operator<<(ostream& os,const stringlist &sl);
};

extern ostream& operator<<(ostream& os,const stringlist &sl);

}  /* end of namespace anitmt */

#endif  /* __stringlist_hpp__ */

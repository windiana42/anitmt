/*
 * searchpath.hpp
 * 
 * Header for search path class. 
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
 *   May 2001   started writing
 *
 */


#ifndef _Inc_IO_SearchPath_H_
#define _Inc_IO_SearchPath_H_ 1

#include "lproto.hpp"

#include <string>
#include <list>

namespace output_io
{

class Input_Stream;

class Search_Path
{
	private:
		std::list<std::string> spath;
	public:
		Search_Path()  { }
		~Search_Path()  {  clear();  }
		
		// `/' is appended if needed. 
		// Return value: 
		//  0 -> added 
		//  1 -> dir already in search path 
		//  2 -> dir empty 
		int add(const std::string &dir);
		
		void clear()  {  spath.clear();  }
		
		// Try opening the passed Input_Stream. 
		// The file name is passed to this function in is->Path(). 
		// search_current: if a file in a/b/c/file1 wants to include 
		//    "file2", then the value specifies what to do: 
		// -1 -> search for a/b/c/file2 before the search path
		// +1 -> search for a/b/c/file2 after search path test 
		//  0 -> do not search for a/b/c/file2 (unless a/b/c is in the 
		//       searchpath, of course)
		// If you set search_current, don't forget to pass the 
		// currently processed file (path and name; a/b/c/file1 in this 
		// example) in current_file otherwise it has no effect. 
		// * Files with absolute path (`/' at the beginning) are not 
		//   subject to path search. 
		// * You should add "." to the search path to allow the 
		//   inclusion of files in the cwd (which is not changed by 
		//   this class). If "." is not in the search path, files in 
		//   the cwd are only included if you set search_current 
		//   and the currently processed file is in the cwd (either 
		//   "./file" or "file"). 
		//   (cwd = current working directory; see chdir(2))
		// * If you set search_current=0 and honor_search_path=false, 
		//   only files with absolute paths will ever actually be 
		//   included. 
		// search_current is independent of honor_search_path. 
		// Return value: 
		//   true  -> success 
		//   false -> failure (no error message written) 
		bool try_open(Input_Stream *is,
			int search_current=0,
			const std::string &current_file=std::string(),
			bool honor_search_path=true);
};

}  // namespace end

#endif  /* _Inc_IO_SearchPath_H_ */

/*
 * findstr.hpp
 * 
 * Find strings in a buffer. 
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
 *   Apr 2001   started writing
 *
 */


#ifndef _Inc_IO_FindString_H_
#define _Inc_IO_FindString_H_ 1

// This includes the config: 
#include "../lib/lproto.hpp"

namespace output_io
{

class Find_String
{
	public:
		struct RV
		{
			const char *found;  // which string was found (=f[x])
			size_t found_len;   // strlen(found)
			int id;             // id of string which was found
			const void *hook;   // hook data pointer of the string 
			char *ptr;          // where it was found (&str[y])
		};
	private:
		struct String_Node
		{
			String_Node *prev,*next;
			const char *str;  // '\0'-terminated; not copied
			size_t len;       // length of string
			bool copied;
			bool enabled;
			int id;
			const void *hook;
			String_Node(const char *str,int id,const void *hook,bool enabled,bool copy);
			~String_Node();
		};
		String_Node *hash[256];
		
		inline int _Str2Hash(const char *str)
			{  return(int(*(const unsigned char *)str));  }
		
		inline void _DequeueNotTop(String_Node *n);
		inline void _Dequeue(String_Node *n,String_Node **top);
		inline void _QueueAfter(String_Node *n,String_Node *prev);
		
		String_Node *_Find_Node(const char *str);
		bool _Add_String(const char *str,int id,const void *hook,
			bool enabled,bool copy);
	public:
		Find_String();
		~Find_String();
		
		// You cannot add strings of zero length; this will fail 
		// with return value false. 
		// Add a string to search; str is NOT copied. 
		bool Add_String(const char *str,int id=-1,const void *hook=NULL,
			bool enabled=true)
			{  return(_Add_String(str,id,hook,enabled,false));  }
		// Add a string to search; str IS copied. 
		bool Copy_String(const char *str,int id=-1,const void *hook=NULL,
			bool enabled=true)
			{  return(_Add_String(str,id,hook,enabled,true));  }
		
		// Enable/Disable/Delete 
		// (Work fastest if you pass the same pointer as to Add_String.) 
		// Return value: 0 -> OK; 1 -> string not found 
		int Enable_String(const char *str);
		int Disable_String(const char *str);
		int Delete_String(const char *str);
		
		// Enable/Disable all strings with specified ID: 
		// Returns number of strings which were en/disabled 
		// (En/Disable will return 0 if all strings were already 
		// en/disabled.) 
		int Enable_ID(int id);
		int Disable_ID(int id);
		
		// Sort all strings by their length (longest first) to avoid 
		// that a shorter string is found where a longer one could 
		// also be matched. 
		// The sort algorithm is stable. 
		void Sort_By_Length();
		
		// Do the actual searching: 
		// Search in txt of size len; the result is returned in rv. 
		// rv->found: which string was found (=f[x])
		// rv->ptr: where it was found (&str[y])
		// NOTE: The strings are matched in the order they were specified, 
		//       so searching for "foo", "foobar" in "void foobar" will 
		//       return "foo" as matching. 
		// Return value:  number of bytes processed . 
		//   = (rv->ptr-str)+strlen(rv->found) if one of the strings was found 
		//   < len in case some string matches but the bytes str[len]... are 
		//         needed to check if all characters of the sound string match 
		//   = len if nothing matches 
		size_t Search(char *txt,size_t len,RV *rv);
		
		// Checks if one of the enabled strings matches txt of size len. 
		// This is like strncmp(txt,string[0,1,2...],string[0,1,2...].len). 
		// NOTE: If len is smaller than the length of the string to match, 
		//       the string does not match so make sure there is enough 
		//       input. 
		// Example: txt="blahblah": 
		//  will match: "blah", "b" "blahblah"
		//  no match:   "lah", "blahblahblah"
		// Return value: 
		//    true: a string matches (returned in rv). 
		//    false: no string matches
		// NOTE: rv->ptr has a different meaning (as it would otherwise 
		//       always be txt): rv->ptr is txt+strlen(rv->found). 
		bool Match(char *txt,size_t len,RV *rv);
};

}  // namespace end 

#endif  /* _Inc_IO_FindString_H_ */

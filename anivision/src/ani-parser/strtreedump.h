/*
 * ani-parser/strtreedump.h
 * 
 * Expandable, indent-capable, alloc-ahead string buffer for tree 
 * node dump. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _ANIVISION_ANI_STRINGTREEDUMP_H_
#define _ANIVISION_ANI_STRINGTREEDUMP_H_ 1

#include <hlib/linkedlist.h>


// This is used to get the string representation of the 
// complete tree (Node *this and everything below). 
// It supports nice indention. 
class StringTreeDump
{
	public:
		// We start a new entry each time the indent depth changes. 
		struct Entry : LinkedListBase<Entry>
		{
			size_t size;  // allocated len
			size_t len;   // actual length (without '\0')
			char *str;    // string data
			int indent;   // indent depth
			
			_CPP_OPERATORS
			Entry(int _indent);
			~Entry();
			
			// Append string to the entry: 
			void Append(const char *str,size_t len);
			void Append(const char *str)
				{  Append(str,str ? strlen(str) : 0);  }
			// Remove the passed character if that char is at 
			// the end of the entry: 
			void RemoveEnd(char c);
			
			// Instruct the entry to free all memory which was allocated 
			// too much in advance. Do this when no more data will be 
			// appended to this entry. 
			void TightSize();
			
			// Simply remove content without reallocation. 
			void Zap()
				{  if(len) { len=0;  *str='\0'; }  }
		};
	private:
		// The actual dump is stored here: 
		// We're currently appending to ents.last(). 
		LinkedList<Entry> ents;
		int curr_indent;
		bool indent_just_added;
		
	public: _CPP_OPERATORS
		StringTreeDump();
		~StringTreeDump();
		
		// Change indent: 
		void AddIndent(int n=1)
			{  curr_indent+=n;  indent_just_added=1;  }
		void SubIndent(int n=1)
			{  curr_indent-=n;  }
		bool IndentJustAdded() const  {  return(indent_just_added);  }
		
		// Append string to the dump: 
		void Append(const char *str);
		
		// Remove the passed character if that char is at the end of the dump: 
		void RemoveEnd(char c);
		
		// Write the complete string representation to a FILE *: 
		// Returns number of bytes written. 
		size_t Write(FILE *fp);
};

#endif  /* _ANIVISION_ANI_STRINGTREEDUMP_H_ */

/*
 * strlist.h 
 * 
 * Simple linked string list (based on linkedlist.h). 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_StringList_H_
#define _HLIB_StringList_H_ 1

#include <stddef.h>
#include "cplusplus.h"
#include "linkedlist.h"

// This implements a simple string list. 
// All the string elements get allocated by the list and freed 
// on destruction or removal. 
class StrList
{
	public:
		struct Node : LinkedListBase<Node>
		{  _CPP_OPERATORS_FF
			Node(int * /*failflag*/=NULL) : LinkedListBase<Node>()
				{  str=NULL; len=0;  }
			~Node()  {  StrList::free((char*)str);  len=0;  }
			
			const char *str;   // pointer to the string 
			size_t len;        // length of the string 
			
			private:  // Do not copy. 
				Node(const Node &) : LinkedListBase<Node>() { }
				void operator=(const Node &) { }
		};
	private:
		LinkedList<Node> list;
		
		// where: -1 -> beginning; +1 -> end. 
		int _insapp(const char *str,size_t len,int where);
	public:  _CPP_OPERATORS_FF
		StrList(int *failflag=NULL);
		~StrList();
		
		// These are the allocation and deallocation (free) functions 
		// used by StrList. Use them whenever you have to manually 
		// allocate/free a string of this list. 
		// Allocate len bytes; returns NULL if failed. 
		static char *alloc(size_t len)
			{  return((char*)LMalloc(len));  }
		// Deallocate pointer in *ptr. Always returns NULL. 
		// Does nothing if ptr=NULL. 
		static char *free(char *ptr)
			{  return((char*)LFree(ptr));  }
		
		// Delete (and free) all elements in the list. 
		void clear();
		
		// Insert string at the beginning of the list. 
		// String is (as always) copied. 
		// Returns 0 on success and -1 on (allocation) failure.
		int insert(const char *str,size_t len)
			{  return(str ? _insapp(str,len,-1) : 0);  }
		int insert(const char *str)
			{  return(str ? _insapp(str,strlen(str),-1) : 0);  }
		
		// Append a string at the end of the list. 
		// String is (as always) copied. 
		// Returns 0 on success and -1 on (allocation) failure.
		int append(const char *str,size_t len)
			{  return(str ? _insapp(str,len,+1) : 0);  }
		int append(const char *str)
			{  return(str ? _insapp(str,strlen(str),+1) : 0);  }
		
		// Insert/append a whole list. All elements of lst are 
		// copied. To copy a list, use clear() and append(). 
		// Return value: 0 -> OK; -1 -> allocation failed. 
		int insert(const StrList *lst);
		int append(const StrList *lst);
		
		// These return the first and the last node in the list. 
		// Be careful...
		const Node *first() const  {  return(list.first());  }
		const Node *last()  const  {  return(list.last());   }
		
		// NODE OPERATIONS: 
		// - Use the *next and *prev pointers to get the next and 
		//   previous nodes in the list. 
		// - Simply delete a node if it is no longer in the list 
		//   to free it and the string in it. 
		// - If you do not want the string to be freed, set Node::str. 
		//   to NULL before calling operator delete. 
		// - DO NOT MODIFY NODES WHICH ARE STILL IN THE LIST other 
		//   than changing the content of str (don't forget to keep 
		//   Node::len in sync). 
		
		// This function dequeues the passed node from the list and 
		// returns it. The node is NOT freed. Simply delete the node 
		// to free it and the string in it. 
		Node *dequeue(Node *n)  {  return(list.dequeue(n));  }
		
		// These work like dequeue(first()) and dequeue(last()): 
		// See dequeue() above. 
		Node *popfirst()  {  return(list.popfirst());  }
		Node *poplast()   {  return(list.poplast());   }
		
		// Check if element *n is in the list (only pointers are 
		// compared). Returns 1 if n was found and 0 if not. 
		int find(const Node *n) const  {  return(list.find(n) ? 1 : 0);  }
		
		// Finds the node containing the passed string. The strings 
		// are compared using len and strcmp(). 
		// Returns NULL if not found. 
		const Node *find(const char *str);
		
		// Counts the elements in the list: 
		int count() const  {  return(list.count());  }
};

#endif  /* _HLIB_StringList_H_ */

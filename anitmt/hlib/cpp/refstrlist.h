/*
 * refstrlist.h 
 * 
 * Simple linked string list (based on linkedlist.h) using RefStrings. 
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

#ifndef _HLIB_RefStringList_H_
#define _HLIB_RefStringList_H_ 1

#include <stddef.h>
#include "cplusplus.h"
#include "linkedlist.h"
#include "refstring.h"

class RefStrList
{
	public:
		struct Node : LinkedListBase<Node>, RefString
		{  _CPP_OPERATORS_FF
			Node(int *failflag=NULL) : 
				LinkedListBase<Node>(), RefString(failflag)  {}
			~Node()  {}
			
			private:  // Do not copy. 
				Node(const Node &) : 
					LinkedListBase<Node>(), RefString() { }
				void operator=(const Node &) { }
		};
	private:
		LinkedList<Node> list;
		
		// where: -1 -> beginning; +1 -> end. 
		int _insapp(const RefString &ref,int where);
		int _insapp(const char *str,int where);
	public:  _CPP_OPERATORS_FF
		RefStrList(int *failflag=NULL);
		~RefStrList();
		
		// Delete (and free) all elements in the list. 
		void clear();
		
		// List empty?
		bool is_empty() const  {  return(list.is_empty());  }
		
		// Insert string at the beginning of the list / append at the 
		// end of the list. 
		// Return value: 
		//  0 -> success 
		// -1 -> allocation failure 
		int insert(const RefString &str)  {  return(_insapp(str,-1));  }
		int append(const RefString &str)  {  return(_insapp(str,+1));  }
		int insert(const char *str)  {  return(_insapp(str,-1));  }
		int append(const char *str)  {  return(_insapp(str,+1));  }
		
		// See LinkedList for details: 
		// Return value: 
		//  0 -> success
		// -1 -> allocation failure 
		// -2 -> where=NULL or loc==0 
		int queuebefore(const RefString &str,Node *where)
			{  return(queue(str,where,-1));  }
		int queueafter(const RefString &str,Node *where)
			{  return(queue(str,where,+1));  }
		int queue(const RefString &str,Node *where,int loc);
		
		// To replace the content of a node, simply do 
		// node->set(new_content);
		
		// Insert/append a whole list. Only Nodes are allocated, the 
		// strings are (of course) only referenced. 
		// To copy a list, use clear() and append(). 
		// Return value: 
		//  0 -> OK
		// -1 -> allocation failure
		int insert(const RefStrList *lst);
		int append(const RefStrList *lst);
		
		// These return the first and the last node in the list. 
		// Be careful...
		const Node *first() const  {  return(list.first());  }
		const Node *last()  const  {  return(list.last());   }
		
		// NODE OPERATIONS: 
		// - Use the *next and *prev pointers to get the next and 
		//   previous nodes in the list. 
		// - Simply delete a node if it is no longer in the list 
		//   to free it and the string in it. 
		// - DO NOT MODIFY NODES WHICH ARE STILL IN THE LIST other 
		//   than changing the content of the string in the node. 
		
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
		// When comparing a data string to *str, the last '\0' of the 
		// data string (if any) is ignored, thus "abc" and "abc\0" will 
		// both match str="ABC". 
		// Returns NULL if not found. 
		// Passing str=NULL will find the first NULL-reference. 
		const Node *find(const char *str);
		
		// Counts the elements in the list: 
		int count() const  {  return(list.count());  }
};

#endif  /* _HLIB_StringList_H_ */

/*
 * searchpath.h
 * 
 * Implementation of class FileSearchPath, a simple search path 
 * file finder. 
 *
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_FileSearchPath_H_
#define _HLIB_FileSearchPath_H_ 1

#include <hlib/refstring.h>
#include <hlib/linkedlist.h>


// This class provides a file search path, i.e. a solution to the common 
// problem that you want to open a file which can be located at several 
// positions. 
// WORKS ONLY WITH '\0'-TERMINATED RefStrings. NO CHECKS. 
class FileSearchPath
{
	protected:
		struct Node : LinkedListBase<Node>
		{  _CPP_OPERATORS_FF
			RefString path;   // with ONE trailing "/" 
			
			Node(int *failflag=NULL) : path(failflag) { }
			~Node()  { }
		};
		struct _SCDptr   // used as dptr for internal _SearchCheck()
		{
			char mode;    // 'a' -> access; 'o' -> open
			int flags;    // access or open flags
			int ret_fd;   // return fd (for open(2) check)
		};
	private:
		LinkedList<Node> plist;   // path list
		
		// Returns node with passed path or NULL. 
		// If path is NULL or "", NULL is returned. 
		// Trailing "/" are ignored in a sane way. 
		Node *_FindNode(const char *path,size_t strlen_path);
		
		// Used by add(): If the path is already in the list, re-queue it at 
		// the proper pos and return 1, else 0. 
		// pos: +1 -> at the end; -1 -> at the beginning of the list
		int _RequeueIfInList(const char *path,size_t strlen_path,int pos);
		
		// Internally used by the Add() functions. 
		int _IntrnlAddNode(const char *path,size_t strlen_path,int pos);
		
		// Search kernel: 
		// SearchIterate calls the function _SearchCheck() (if call_virt=0) or 
		// SearchCheck() (if call_virt=1) for each complete path in question when 
		// searching for the passed file. 
		// If *result is non-NULL, and the return value is 0, then the complete 
		// path is returned to the caller in *result, OTHERWISE, *result is NOT 
		// modified. 
		// For current_file,current_order, see Search() below. 
		// *virt_ptr is passed to _SearchCheck()/SearchCheck() as *dptr argument 
		// and can be used as a hook for arbitrary data. So, you can e.g. make 
		// SearchCheck() use open(2) and pass the returned file descriptor back 
		// using *virt_ptr. 
		// Return value: 
		//  * -> what SearchCheck()/_SearchCheck() returns, especially: 
		//      0 -> found
		//      1 -> not found
		//     -4 -> SearchCheck() not implemented
		//  -1 -> alloc failure
		//  -2 -> *file is NULL or "" (or current_file is "")
		//  -3 -> *file is a directory (trailing "/")
		int _SearchIterate(const char *file,RefString *result,
			const char *current_file,int current_order,
			int call_virt,void *virt_ptr);
		
		// Internal implementation of _SearchCheck() for Access() and Open(): 
		int _SearchCheck(const char *file,void *dptr);
		
	public:  _CPP_OPERATORS_FF
		FileSearchPath(int *failflag=NULL);
		virtual ~FileSearchPath();
		
		// Add a path element: 
		// pos: +1 -> at the end; -1 -> at the beginning of the list
		// Return value: 
		//   0 -> okay, added (or just moved because it was alreay in 
		//        the list)
		//  -1 -> alloc failure
		//  -2 -> attempt to add NULL or "" as path
		// NOTE: The RefString - version just references the path if it 
		//       fits, i.e. if there is exactly one trailing "/". 
		// NOTE: You may add "./" to the search path if you want to search 
		//       for files in the CWD. 
		int Add(const RefString *path,int pos=+1);
		int Add(const char *path,int pos);
		
		// See Search(). 
		// Return value: 
		//  0 -> FOUND
		//  1 -> NOT found
		// -1 -> alloc failure
		// -4 -> not implemented
		//  * -> other error; will be returned by the search function 
		virtual int SearchCheck(const char * /*file*/,void * /*dptr*/)  {  return(-2);  }
		
		// Now, here are various search routines: 
		//  * File names beginning with "/" ("absolute path") are not subject 
		//    to path search; such a file path is checked as it is passed. 
		//  * If *result is non-NULL, and the return value is 0, then the complete 
		//    path is returned to the caller in *result, OTHERWISE, *result is NOT 
		//    modified. 
		//  * If you process #include statements, you may want to look in the 
		//    same directory as the file being currently read. For this reason, 
		//    the path in *current_file, if non-NULL, is also searched. (Yes, you 
		//    may pass a file name in current_file, becuase only the dirname will 
		//    be used and the basename cut off.) If current_file is non-NULL, 
		//    current_order specifies when to search the current path: 
		//    -1 -> before the search path
		//     0 -> do not search the search path (only current)
		//    +1 -> after the search path
		//  * Return values: Those of _SearchIterate(), see above. 
		// Search() is the most primitive one: It just calls the virtual 
		// function SearchCheck() for each file in question. 
		// dptr is passed as dptr to SearchCheck(). 
		int Search(const char *file,RefString *result=NULL,
			const char *current_file=NULL,int current_order=-1,
			void *dptr=NULL)
			{  return(_SearchIterate(file,result,current_file,current_order,1,dptr));  }
		// Do access(2) check for files in question until the first path 
		// matches. 
		// flags: R_OK, W_OK, X_OK, as passed to access(2)
		int Access(const char *file,int flags,RefString *result=NULL,
			const char *current_file=NULL,int current_order=-1);
		// Like Access(), but using open(2). 
		// The open flags (second arg) is the value of *flag_fd. 
		// When the function returns success (0), flag_fd holds the file 
		// descriptor as returned by open(2). 
		int Open(const char *file,int *flag_fd,RefString *result=NULL,
			const char *current_file=NULL,int current_order=-1);
};

#endif  /* _HLIB_FileSearchPath_H_ */

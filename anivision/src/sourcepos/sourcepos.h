/*
 * sourcepos/sourcepos.h
 * 
 * Classes for sophisticated source position (file/line/lpos, 
 * include hierarchy) handling. 
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


/*******************************************************************************

LITTLE DOCUMENTATION - USAGE INFO
---------------------------------

For each toplevel input (source) file you read in with your parser, 
create a SourcePositionArchive. Use the SourcePositionArchive to get 
SourceFileNodes and SourcePosition indicators. 
When parsing is done, you may safely delete the SourcePositionArchive; 
the SourceFileNodes and SourcePositions which are still in use will 
not get deleted (they clean up themselves when no longer in use). 

Note that SourcePositionArchive is not C++ - safe, create it once 
and only use pointers to it. 

SourcePosition represents a source position (line and possibly lpos 
in a file); it is meant for mass usage and kept as small as possible: 
SourcePosition contains only one pointer, so it is 4 or 8 bytes 
(depending on the architecture and padding). 
The pointer points to InternalSourcePosition. 
SourcePosition does reference counting and is C++ - safe 
(assignment and copy construction). 
Note that InternalSourcePosition are normally shared among all 
tokens with the same position - thus you can save a lot of memory 
when not using lpos (always set it to -1). 

(Internal) SourcePosition points to a SourceFileNode which represents 
a source file. SourceFileNodes are organized in a tree structure 
representing the include hierarchy. Note that if you include the same 
file twice, two SourceFileNodes will get allocated. 
SourceFileNode is C++ - safe (copy constructor and assignment). 
The SourceFileNodes can also save the include statement position and 
size -- information which is needed in case you want to concatenate 
all the included files into a single one. 
(Internally uses InternalSourceFileNode and reference counting.) 

For more docu, check the comments around the public functions/methods 
in this file. 

*******************************************************************************/

#ifndef _LIB_SOURCEPOS_H_
#define _LIB_SOURCEPOS_H_

#include <hlib/linkedlist.h>
#include <hlib/refstring.h>


typedef int64_t filepos_t;   // MUST BE SIGNED. 

class SourcePosition;
class InternalSourcePosition;
class SourceFileNode;
class InternalSourceFileNode;
class SourcePositionCache;    // YERY simple
class SourcePositionArchive;


// This struct merely serves as a namespace: 
struct SourcePositionNS
{
	// Prevent these from cluttering the namespace: 
	static void _forbidden();  // Simply calls assert(!"forbidden"). 
	static void _allocfail();  // Message and abort(). 
};


// NOT C++ - safe; use SourceFileNode. 
// Represents a read-in file; tree structure for include hierarchy. 
class InternalSourceFileNode : LinkedListBase<InternalSourceFileNode>
{
	friend class SourceFileNode;
	friend class SourcePositionArchive;
	friend class LinkedList<InternalSourceFileNode>;
	private:
		size_t refcnt;    // Number of references (by SourceFileNode)
		RefString path;   // Path to the file
		
		// Pointer to associated SourcePositionArchive: 
		// Must be set as long as archive exists. 
		SourcePositionArchive *pos_archive;
		
		// Tree structure: 
		InternalSourceFileNode *up;
		LinkedList<InternalSourceFileNode> down;
		
		// Start position of the inlcude statement in the parent node: 
		int parent_include_line;
		int parent_include_lpos;
		
		// Offset position & size of the include statement in the file: 
		filepos_t include_stmt_pos;
		size_t    include_stmt_len;
		
		// This is a SourcePosition cache caching the last few source 
		// positions in this file so that positions can be re-used 
		// when the same position is requested more than once. 
		// (Saves memory, especially when not using lpos.) 
		// (Gets deleted when no longer needed.)
		SourcePositionCache *pos_cache;
		
		// Called by subref(). 
		// If pos_archive is set, tell the pos_archive. 
		// Otherwise, delete ourselves. 
		void _destroy();
		
		// Reference counting. 
		// ONLY used by SourceFileNode and internal refs by children. 
		inline void addref()
			{  ++refcnt;  }
		inline void subref()
			{  if(--refcnt<=0) _destroy();  }
		
		// Destroy cache: (also done by _DetachArchive()). 
		void _DestroyPosCache()
			{  DELETE(pos_cache);  }
		// This is called by SourcePositionArchive upon its destruction. 
		// Will set our pointer to it (pos_archive) to NULL and 
		// destroy us if we're no longer needed. 
		// NOTE: It is vital that we delete the position cache here. 
		//       Firstly,  it is no longer used. 
		//       Secondly, if we do not, WE hold refs to our files 
		//                 preventing cleanup!
		void _DetachArchive()
			{  pos_archive=NULL; _DestroyPosCache(); if(!refcnt) _destroy();  }
		
		// NOT C++ - safe: 
		InternalSourceFileNode(const InternalSourceFileNode &) : 
			LinkedListBase<InternalSourceFileNode>()
			{  SourcePositionNS::_forbidden();  }
		InternalSourceFileNode &operator=(const InternalSourceFileNode &)
			{  SourcePositionNS::_forbidden();  return(*this);  }
		
		// Use these to add/delete children. Never directly access 
		// child list (down). 
		void _AddChild(InternalSourceFileNode *isf)
			{  down.append(isf);  addref();  }
		void _DelChild(InternalSourceFileNode *isf)
			{  down.dequeue(isf);  subref();  }
	public:  _CPP_OPERATORS_FF
		// Create an InternalSourceFileNode. 
		// Enqueues itself at the parent (if non-NULL). 
		// DO NOT FORGET to set the include position if needed. 
		InternalSourceFileNode(const RefString &file,
			InternalSourceFileNode *parent,
			SourcePositionArchive *pos_archive,int *failflag=NULL);
		// Destroy InternalSourceFileNode. No refs and no child 
		// nodes may be present. 
		// Will dequeue the node at parent (if any). 
		~InternalSourceFileNode();
};


// For public use. 
// C++ - safe (assignment and copy constructor). 
class SourceFileNode
{
	friend class SourcePositionArchive;
	public:
		enum _NullFile { NullFile };
	private:
		InternalSourceFileNode *isf;
		
		// Aquire ref and remove ref: 
		inline void _aqref()
			{  if(isf)  isf->addref();  }
		inline void _deref()
			{  if(isf)  isf->subref();  }
		
		SourceFileNode(InternalSourceFileNode *_isf)
			{  isf=_isf;  if(isf) isf->addref();  }
	public:  _CPP_OPERATORS_FF  // <-- Actually, we do not use it...
		SourceFileNode(_NullFile)                 // Create NULL ref
			{  isf=NULL;  }
		SourceFileNode(const SourceFileNode &sfn) // Copy
			{  isf=sfn.isf;  _aqref();  }
		~SourceFileNode()                         // Destroy
			{  _deref();  }
		
		SourceFileNode &operator=(const SourceFileNode &sfn)
			{  _deref();  isf=sfn.isf;  _aqref();  return(*this);  }
		SourceFileNode &operator=(_NullFile)
			{  _deref();  isf=NULL;  return(*this);  }
		
		// Check for NULL ref: 
		bool operator!() const
			{  return(!isf);  }
		bool operator==(_NullFile) const
			{  return(!isf);  }
		
		// Note: operator==() is true if this is the same 
		// file with the same position in the include tree. 
		// (I.e. same file read several times will yield to false 
		// if *this and sfn were read at different "times".) 
		bool operator==(const SourceFileNode &sfn) const
			{  return(isf==sfn.isf);  }
		bool operator!=(const SourceFileNode &sfn) const
			{  return(isf!=sfn.isf);  }
		
		// Get associated source position archive (or NULL): 
		SourcePositionArchive *GetArchive() const
			{  return(isf ? isf->pos_archive : NULL);  }
		
		// Get file path: 
		RefString GetPath() const
			{  return(isf ? isf->path : RefString());  }
};


// Internally used by SourcePosition and SourcePositionArchive. 
// NOT C++ - safe. 
class InternalSourcePosition
{
	friend class SourcePosition;
	friend class SourcePositionArchive;
	private:
		SourceFileNode file;   // File and all the include hierarchy
		int line;              // Line number
// These could theoretically use a union (if all the code is aware of that): 
		int lpos;              // Position on the line 
		int refcnt;
		
		// Called when last ref is deleted: 
		void _destroy();
		
		// Reference counting (used by class SourcePosition): 
		void addref()
			{  ++refcnt;  }
		void subref()
			{  if(--refcnt<=0)  _destroy();  }
		
		// NOT C++ - safe: 
		InternalSourcePosition(const InternalSourcePosition &) : 
			file(SourceFileNode::NullFile)
			{  SourcePositionNS::_forbidden();  }
		InternalSourcePosition &operator=(const InternalSourcePosition &)
			{  SourcePositionNS::_forbidden();  return(*this);  }
	public:  _CPP_OPERATORS_FF
		InternalSourcePosition(const SourceFileNode &fnode,
			int line,int lpos,int *failflag);
		~InternalSourcePosition();
};


// For public use. 
// C++ - safe (assignment and copy constructor). 
// This is the source position class to be used by other code. 
// This class is C++ - safe i.e. you can copy-construct/assign it, etc. 
// It uses reference counting. 
//    Construct empty NULL position: 
//       SourcePosition sp(SourcePosition::NullPos);
//       sp=SourcePosition::NullPos;
//    Copy possition:
//       SourcePosition sp2(sp);
//       sp2=sp;
// SIZE: 1*sizeof(void*)
class SourcePosition
{
	friend class SourcePositionArchive;
	public:
		enum _NullPos { NullPos };
	private:
		// This pointer may also be NULL. 
		InternalSourcePosition *isp;
		
		// Aquire and remove references: 
		inline void _aqref()
			{  if(isp) isp->addref();  }
		inline void _deref()
			{  if(isp)  isp->subref();  }
		
		SourcePosition(InternalSourcePosition *_isp)
			{  isp=_isp; _aqref();  }
		// This is only needed by source position cache to set up 
		// the cache array: 
		friend class SourcePositionCache;
		SourcePosition()
			{  isp=NULL;  }
	public: _CPP_OPERATORS
		// Create NULL position: 
		SourcePosition(_NullPos)
			{  isp=NULL;  }
		// Copy position: 
		SourcePosition(const SourcePosition &sp)
			{  isp=sp.isp;  _aqref();  }
		// Destroy position: 
		~SourcePosition()
			{  _deref();  }
		
		// Assign position: 
		SourcePosition &operator=(const SourcePosition &sp)
			{  _deref();  isp=sp.isp;  _aqref();  return(*this);  }
		SourcePosition &operator=(_NullPos)
			{  _deref();  isp=NULL;  return(*this);  }
		
		// Check for NULL position: 
		bool operator!() const
			{  return(!isp);  }
		bool operator==(_NullPos) const
			{  return(!isp);  }
		
		// operator==() returns true for same position in same file 
		// (see also SourceFileNode::operator==()). 
		bool test_equal(const SourcePosition &sp) const;
		bool operator==(const SourcePosition &sp) const
			{  return(test_equal(sp));  }
		bool operator!=(const SourcePosition &sp) const
			{  return(!test_equal(sp));  }
		
		// Get the associated file node or NullFile: 
		SourceFileNode GetFile() const
			{  return(isp ? isp->file : 
				SourceFileNode(SourceFileNode::NullFile));  }
		
		// Get line and lpos (curser position in line): 
		int GetLine() const  {  return(isp ? isp->line : (-1));  }
		int GetLPos() const  {  return(isp ? isp->lpos : (-1));  }
		// Get path: 
		RefString GetPath() const
			{  return(isp ? isp->file.GetPath() : RefString());  }
		
		// String representation of this position without 
		// include hierarchy. 
		// String looks like "file" or "file:line" or "file:line:lpos". 
		// file is omitted if with_file=0. 
		// Return value: 0 -> OK; -1 -> alloc failure
		int PosString(RefString *dest,bool with_file=1) const;
		// String representation of position range. 
		// Uses sinple position if the positions equal. 
		// Omits file completely if with_file=0. 
		// Return value: 0 -> OK; -1 -> alloc failure
		int PosRangeString(const SourcePosition &end,RefString *dest,
			bool with_file=1) const;
		
		// Write position relative to rpos. 
		// That means: leave away file if it is the same file. 
		// Return value: 0 -> OK; -1 -> alloc failure
		int PosStringRelative(const SourcePosition &rpos,RefString *dest) const;
		#warning String representation of include dump.
};


class SourcePositionCache
{
	private:
		// The actual cache is just an array; it is short because 
		// the parser reads the file top-to-bottom so there is no 
		// need to hold a lot of positions. Actually, 1 or 2 should 
		// be enough...
		static const int cache_size=6;
		SourcePosition cache[cache_size];
		
		// Index of the newest element (they are rotated): 
		int top;
		
		// Record some statistics: 
		int hits;
		int misses;
		
	public: _CPP_OPERATORS_FF
		SourcePositionCache(int *failflag);
		~SourcePositionCache();
		
		// Add this element to the cache (removing the oldest 
		// one): 
		void Store(const SourcePosition &sp)
		{  top=(top+1)%cache_size;  cache[top]=sp;  }
		
		// Find a SourcePosition in the cache. 
		// Returns NULL position if not found. 
		SourcePosition Find(int line,int lpos);
};


// You need one such object for each toplevel input file. 
// This class primarily holds the root of the include 
// hierarchy tree. 
// NOTE: NOT C++ - safe. 
class SourcePositionArchive
{
	private:
		// Head of the input (include) file hierarchy. 
		SourceFileNode head;
		// The currently used SourceFileNode: 
		SourceFileNode current;
		
		// Some statistics about SourceFileNode: 
		// Total amount of allocated file nodes: 
		int tot_file_nodes;
		// Max number of file nodes allocated simultaniously: 
		int max_file_nodes;
		// Current number of file nodes: 
		int curr_file_nodes;
		
		// Some statistics about InternalSourcePosition: 
		size_t tot_src_pos;
		size_t max_src_pos;
		size_t curr_src_pos;
		
		// Recursively call _DetachArchive() for all InternalSourceFileNodes. 
		void _RecursiveDetachArchive(InternalSourceFileNode *head);
		
		// Do not use these: 
		SourcePositionArchive(const SourcePositionArchive &) : 
			head(SourceFileNode::NullFile),current(head)
			{  SourcePositionNS::_forbidden();  }
		SourcePositionArchive &operator=(const SourcePositionArchive &)
			{  SourcePositionNS::_forbidden();  return(*this);  }
	public:  _CPP_OPERATORS_FF
		// Pass the primary input file as argument. failflag unused. 
		SourcePositionArchive(const RefString &file,int *failflag=NULL);
		~SourcePositionArchive();
		
		// Primary function: Request a SourcePosition 
		// for the passed location: 
		// line/lpos: In the current FILE; use the constructor and 
		// IncludeFile()/EndFile() to change the current file. 
		// Aborts on alloc failure. 
		// Use lpos=-1 if not using lpos. 
		//     line<0 will return a NullPos. 
		SourcePosition GetPos(int line,int lpos=-1);
		
		// IncludeFile(): call when an include directive is 
		// read and reading continues in passed file. 
		//   file      -> included file (path)
		//   line,lpos -> position of include directive in 
		//                current file
		//   include_stmt_pos, include_stmt_len -> offset 
		//          position and length of include directive
		//          (if needed)
		// Use EndFile() when EOF is reached to return to 
		// parent file. 
		// Both return the (new) current file node. 
		SourceFileNode IncludeFile(const RefString &file,
			int line,int lpos,
			filepos_t include_stmt_pos=-1,size_t include_stmt_len=0);
		SourceFileNode EndFile();
		
		// Get current file noce: 
		SourceFileNode CurrentFile() const
			{  return(current);  }
		
		// THESE ARE THOUGHT MAINLY FOR INTERNAL USE: 
		// Create/destroy an InternalSourceFileNode: 
		InternalSourceFileNode *AllocISF(const RefString &path,
			SourceFileNode &parent);
		void DestroyISF(InternalSourceFileNode *);  // READ FUNCTION!!!
		// Create/destroy InternalSourcePosition: 
		InternalSourcePosition *AllocISPNoCache(
			const SourceFileNode &fnode,int line,int lpos);
		void DestroyISP(InternalSourcePosition *isp);
		
};

#endif  /* _LIB_SOURCEPOS_H_ */

/*
 * calccore/sctparse.h
 * 
 * Scope tree parser. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _ANIVISION_CALCCORE_SCOPETREEPARSER_H_
#define _ANIVISION_CALCCORE_SCOPETREEPARSER_H_

#include <hlib/cplusplus.h>


namespace CC   // calculation core namespace
{

// Class to parse a scope tree, i.e. something like
//  entry_name = scope_name {
//    entry_name = expr
//    entry_name = scope_name2 {
//       ...
//    }
//  }
// There are different scope types; each scope type has a list of entries; 
// the default entry order (if the entry name is omitted during parsing) 
// is determined by the order of the entries in the scope def. 
// USAGE INFO: 
// First, you need to define the "grammar" by using DeclareScope() and 
// DefineScope(). Then call InitGrammar() to test the grammar. 
// Then, use ScopeTreeParser to parse the scopes via 
class ScopeTreeParser
{
	public:
		struct Scope;
		
		struct SCEnt
		{
			int name_tok;   // if this name is found...
			int scope_id;   // ...then it is this scope
			Scope *scope;   // initially NULL; ptr to avoid lookup
			
			_CPP_OPERATORS
			SCEnt(int _name_tok,int _scope_id) : name_tok(_name_tok),
				scope_id(_scope_id)  {  scope=NULL;  }
			~SCEnt() {}
		};
		
		struct Entry
		{
			int name_tok;    // token identifier for name string
			
			// Expression allowed after entry?
			bool has_expression;
			// Type of expression if has_expression is set: 
			int expr_id;   // this is an arbitrary ID for user level
			
			// List of allowed scopes: 
			int nents,alloc_ents;
			SCEnt *scent;
			
			_CPP_OPERATORS
			// Use these as "constructors": 
			static Entry Scope(int _name_tok)
				{  Entry tmp(_name_tok);  return(tmp);  }
			static Entry Expr(int _name_tok,int _expr_id)
				{  Entry tmp(_name_tok);  tmp.expr_id=_expr_id;
					tmp.has_expression=1;  return(tmp);  }
			~Entry() {  scent=(SCEnt*)LFree(scent);  }
			
			// Add a scope entry: 
			Entry &operator+(const SCEnt &se);
			
			// C++ stuff: 
			Entry(const Entry &e);
			Entry &operator=(const Entry &e);
			
			// Find (scope) entry with specified name: 
			SCEnt *FindSCEnt(int name_tok);
			
			private:
				Entry(int _name_tok) : name_tok(_name_tok)
					{  has_expression=0;  expr_id=-1;
						nents=0;  alloc_ents=0;  scent=NULL;  }
		};
		
		struct EntryList
		{
			int alloc_ents;  // number of entries allocated
			int nents;       // number of entries
			Entry **ent;     // entry array
			
			// Add new entry; content is copied. 
			void AddEntry(const Entry &ent);
			
			// Clear list removing all entries: 
			void Clear();
			
			_CPP_OPERATORS
			EntryList() : alloc_ents(0),nents(0),ent(NULL) {}
			EntryList(const Entry &first) : alloc_ents(0),nents(0),ent(NULL)
				{  AddEntry(first);  }
			~EntryList()
				{  Clear();  }
			
			// Transfer entries from list: 
			void TransferEntries(EntryList *from);
			
			// Sequencing operator: Add entries: 
			EntryList &operator,(const Entry &next)
				{  AddEntry(next);  return(*this);  }
			friend EntryList operator,(const Entry &first,const Entry &next)
				{  EntryList l(first);  return(l,next);  }
			
			// C++ stuff: 
			EntryList(const EntryList &l);
			EntryList &operator=(const EntryList &l);
		};
		
		struct Scope : EntryList
		{
			int scope_id;    // unique ID of the scope
			
			Scope(int _scope_id) : EntryList(),scope_id(_scope_id) {}
			~Scope()  {}
			
			// NOT C++-safe: 
			private:
				Scope(const Scope &) : EntryList() {}
				Scope &operator=(const Scope &) { return(*this); }
		};
		
	protected:
		// List of all scope types with their entries. 
		int nscopes,alloc_scopes;
		Scope **scope;
		
		struct SCEntry
		{
			// Scope or NULL in case of lookup failure. 
			Scope *scope;
			// Current entry index inside this scope. 
			int curr_eidx;
		};
		// Current scope stack; scs[top] is the scope we're currently in. 
		SCEntry *scs;  // size: [scs_alloc]
		int top;
		int scs_alloc;
		
		// Lookup scope by ID: 
		Scope *_LookupScope(int scope_id);
		Scope *_DeclareScope(int scope_id);
		
	public:  _CPP_OPERATORS
		ScopeTreeParser();
		~ScopeTreeParser();
		
		// Clear all stored information: 
		void Clear();
		
		// Define s scope. Specify the entries in their default order. 
		// (Ignore warnings about taking addresses of temporaries.) 
		// Return value: 
		//   0 -> OK
		//   1 -> nothing done, scope with passed ID already exists
		int DefineScope(int scope_id,EntryList *elist);
		
		// Call this after having set up the grammar using the DeclareScope()
		// and DefineScope() calls. Returns the number of errors encounterd. 
		int InitGrammar();
		
		// Finally, do the parsing: 
		// Set the current state. This means, tell the parser, we're 
		// currently inside the specified scope. 
		// Return value: 
		//   0 -> OK
		//   1 -> no such scope
		int SetCurrentScope(int scope_id);
		
		// Tell the parser that the scope entry with name name_tok and 
		// type scope_name_tok was found. 
		// (For "pos=cspline{...}", name_tok=pos and scope_name_tok=cspline.)
		// Use the non-N-version if no name was specified. 
		// The parser will enter this scope. 
		// Return value: 
		//   0 -> OK
		//   1 -> no such entry (inside current scope) (N-version only)
		//   4 -> no next entry / next entry is an expr (non-N-version only)
		//        NOTE: Just go on parsing and calling ParseEnterScope() and 
		//              ParseLeaveScope() and ignore return value 2. 
		//   2 -> error in previous level; ignored
		//   3 -> this entry has no subscope with specified scope_name
		//  -2 -> stack empty; use SetCurrentScope() first
		int ParseEnterScope(int scope_name_tok,int *return_name_tok=NULL);
		int ParseEnterScopeN(int scope_name_tok,int name_tok);
		
		// Tell parser that we just read expression entry; use N-version 
		// if the name is known, otherwise the second one. 
		// Stores expression ID in expr_id (as set in Entry). 
		// Return value: 
		//   0 -> OK
		//   1 -> no such entry in current scope (N-version only)
		//   4 -> no next entry / next entry is a scope (non-N-version only)
		//   2 -> error in previous level; ignored
		//   3 -> this entry does not allow an expression
		//  -2 -> stack empty; use SetCurrentScope() first
		int ParseEntry(int *expr_id,int *return_name_tok=NULL);
		int ParseEntryN(int name_tok,int *expr_id);
		
		// Call this to leave a scope again: 
		// Return value: 
		//   0 -> OK
		//  -2 -> stack empty; cannot leave anything
		int ParseLeaveScope();
};

}  // end of namespace CC

#endif  /* _ANIVISION_CALCCORE_SCOPETREEPARSER_H_ */

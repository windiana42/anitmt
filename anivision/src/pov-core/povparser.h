/*
 * pov-core/povparser.h
 * 
 * POV command comment parser. 
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

#ifndef _ANIVISION_POV_POVPARSER_H_
#define _ANIVISION_POV_POVPARSER_H_

#include <pov-core/objectspec.h>
#include <pov-scan/pov_scan.h>
#include <hlib/refstrlist.h>

// Provided by ani-parser code: 
namespace ANI { class Ani_Parser; }

namespace POV
{

// POV command comment parser. 
// Keeps a list of all commands (objects,...)
class CommandParser : private POVLexerScanner_Interface
{
	public:
		// This is the information which gets stored for each command 
		// comment [it may actually contain more than one command]: 
		struct CCommand : LinkedListBase<CCommand>
		{
			// Location of the command(s): 
			SourcePosition startpos;
			SourcePosition endpos;   // (may be semi-accurate)
			// The complete text of the command 
			// (NON-'\0'-term. RefString): 
			RefString text;
			// File sequence number; each time you call ParseFile(), 
			// this is increased by 1. 
			int file_seq;
			
			// This is the tree node which holds the parsed tree 
			// representation of the command: 
			// This is a PTN_None node holding the list. 
			// NOTE: This may contain more than one command. 
			POVTreeNode *ptn_root;
			
			_CPP_OPERATORS
			CCommand(const RefString &_text,const SourcePosition &_startpos,
				const SourcePosition &_endpos,int _fs);
			~CCommand();
		};
		
		
	private:
		// List of all stored commands. 
		LinkedList<CCommand> cmdlist;
		
		// List of all object specs (extracted from CCommands): 
		LinkedList<ObjectSpec> oslist;
		
		// "Our" Ani_Parser, or NULL: 
		ANI::Ani_Parser *ani_parser;
		// The Ani_Parser passed by ParseFile(): 
		ANI::Ani_Parser *passed_ani_parser;
		
		// Counts parse errors per ParseFile(). 
		int errors;
		
		// List of known file IDs: 
		RefStrList fileid_list;
		
		// Register the content...
		// Assumes that the passed command is already queued in 
		// cmdlist. Returns error count. 
		// Only call if ptn_root!=NULL. 
		int _RegisterCCommand(CCommand *cmd);
		// Helper for the above function: 
		int _Register_ObjectSpec(PTNObjectSpec *tn);
		int _Register_GeneralCmd(PTNGeneralCmd *tn);
		
		// [overriding a virtual from POVLexerScanner_Interface:]
		int CommandNotify(const RefString &text,
			SourcePosition startpos,SourcePosition endpos,int file_seq);
		
	public:  _CPP_OPERATORS
		CommandParser();
		~CommandParser();
		
		// Dump content to passed stream; useful for debugging purposes. 
		void DumpContent(FILE *out);
		
		// Have a file scanned and parsed. This reads in the file, 
		// parses the content and returns the error count. 
		// Pass an Ani_Parser which shall be used to parse the 
		// comand (only tree generation, nothing more); if you use 
		// NULL, an Ani_Parser will be created by the CommandParser. 
		int ParseFile(const RefString &file,ANI::Ani_Parser *ap_to_use);
		
		// Query some object by name: 
		// Return NULL if not found. 
		ObjectSpec *FindObject(const RefString &name);
};

}  // end of namespace POV

#endif  /* _ANIVISION_POV_POVPARSER_H_ */

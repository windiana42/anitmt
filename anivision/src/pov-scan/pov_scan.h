/*
 * pov-scan/pov_scan.h
 * 
 * POV scanner scans POVRay files for command comments. 
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

#ifndef _POV_SCANNER_INTERFACE_H_
#define _POV_SCANNER_INTERFACE_H_

// This class glues together the source position code and the 
// POVLexerScanner as defined in pov_scanner.h. 
// The interface is simple: You give it the toplevel POV file and 
// it will (recursively) read it in, extracting all command comments 
// and then returning the number of errors encountered as well 
// as call a virtual function for each command. 

#include "pov_scanner.h"
#include <sourcepos/sourcepos.h>

namespace POV
{

class POVLexerScanner_Interface : POVLexerScanner
{
	private:
		// The source position archive; it is deleted when parsing is 
		// done as the SourcePosition objects do ref counting and 
		// self-cleanup. 
		SourcePositionArchive *spa;
		
		// File sequence number counter; increased when calling ParseFile(). 
		int file_seq_cnt;
		
		// This is overriding a virtual from POVLexerScanner: 
		void IncludingFile(const RefString &file,int line,int lpos);
	protected:
		// Virtual function being called for each command comment 
		// encountered in the scanned file. Derived class must take care 
		// of it and store it or forget it...
		// NOTE: The complete text of the command is passed as 
		// NON-'\0'-term. RefString in text. 
		// file_seq is the file sequence number; each time you call 
		// ScanFile(), this is increased by 1. 
		// Return value: 
		//   0 -> go on scanning
		//   1 -> stop reading here; we don't need that file, 
		//        go one include hierarchy up
		//   2 -> stop scanning completely
		virtual int CommandNotify(const RefString &text,
			SourcePosition startpos,SourcePosition endpos,int file_seq);
		
	public: _CPP_OPERATORS
		POVLexerScanner_Interface();
		~POVLexerScanner_Interface();
		
		// Read in the passed file. 
		// Can be called more than once with different files 
		// to scan more than one file. All commands which belong to 
		// the same file have the same file_seq set. 
		// Return value: 
		//   0 -> OK
		//  >0 -> number of errors
		int ScanFile(const RefString &file);
};

}  // end of namespace POV

#endif  /* _POV_SCANNER_INTERFACE_H_ */

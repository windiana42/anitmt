/*
 * pov-scan/pov_scan.cc
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

#include "pov_scan.h"

#include <assert.h>

// Non-critical assert: 
#define nc_assert(x)  assert(x)

namespace POV
{

int POVLexerScanner_Interface::ScanFile(const RefString &file)
{
	// Get rid of old source position archive: 
	DELETE(spa);
	
	Reset();
	
	spa=NEW1<SourcePositionArchive>(file);
	
	if(SetInput(file,spa))
	{  DELETE(spa);  return(1);  }
	
	for(;;)
	{
		int rv=Parse();
		nc_assert(rv!=-1);
		if(rv==1)
		{
			const POVLexerScanner::CCommand *src=GetCommand();
			nc_assert(src);
			
			RefString text;
			text.set(src->text,src->text_len);
			
			// Call virtual function: 
			int rrv=CommandNotify(text,
				/*startpos=*/spa->GetPos(src->start_line,src->start_lpos),
				/*endpos=*/spa->GetPos(src->end_line,src->end_lpos),
				/*file_seq=*/file_seq_cnt);
			if(!rrv)  continue;
			if(rrv==1)
			{  SkipFile();  continue;  }
			if(rrv==2) break;  // Stop scanning completely. 
			assert(0);
		}
		if(rv==0)  break;   // EOF
		if(rv==-2)  break;  // aborted
		assert(0);  // Not reached. 
	}
	
	int nerr=Get_NErrors();
	Reset();
	++file_seq_cnt;
	return(nerr);
}


void POVLexerScanner_Interface::IncludingFile(const RefString &file,
	int line,int lpos)
{
	nc_assert(spa);
	
	if(!file)
	{  spa->EndFile();  }
	else
	{
		if(!spa->IncludeFile(file,line,lpos))
		{  CheckMalloc(NULL);  }
	}
}


int POVLexerScanner_Interface::CommandNotify(const RefString &/*text*/,
	SourcePosition /*startpos*/,SourcePosition /*endpos*/,int /*file_seq*/)
{
	// Default implementation of virtual function: 
	return(0);  // go on scanning but discart everything. 
}


POVLexerScanner_Interface::POVLexerScanner_Interface()
{
	spa=NULL;
	file_seq_cnt=0;
}

POVLexerScanner_Interface::~POVLexerScanner_Interface()
{
	DELETE(spa);
}

}  // end of namespace POV 

/*
 * pov-scan/test.cc
 * 
 * POV scanner test file. 
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

#include <hlib/prototypes.h>

#include "pov_scan.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>


struct MyScanner : POV::POVLexerScanner_Interface
{
	// [overriding virtual]
	int CommandNotify(const RefString &text,
		SourcePosition startpos,SourcePosition endpos,int file_seq)
	{
		RefString tmp;
		int rv=startpos.PosRangeString(endpos,&tmp);
		assert(!rv);
		fprintf(stdout,
			"----------<%s>----------\n"
			"%.*s\n",
			tmp.str(),
			text.len(),text.str());
		return(0);
	}
	
	MyScanner() : POV::POVLexerScanner_Interface() {}
	~MyScanner(){}
};

char *prg_name;

int main(int argc,char **arg)
{
	prg_name=GetPrgName(arg[0]);
	
	{
		MyScanner pls;
		int rv=pls.ScanFile(argc>=2 ? arg[1] : "test.pov");
		fprintf(stdout,"-----------------------------------------------\n");
		fprintf(stderr,"Parse()=%d\n",rv);
	}
	
	// Allocation debugging: 
	LMallocUsage lmu;
	LMallocGetUsage(&lmu);
	fprintf(stderr,"%s: %sAlloc: %u bytes in %d chunks; Peak: %u by,%d chks; "
		"(%u/%u/%u)%s\n",
		prg_name,
		lmu.curr_used ? "*** " : "",
		lmu.curr_used,lmu.used_chunks,lmu.max_used,lmu.max_used_chunks,
		lmu.malloc_calls,lmu.realloc_calls,lmu.free_calls,
		lmu.curr_used ? " ***" : "");
	
	return(0);
}

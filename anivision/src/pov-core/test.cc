/*
 * pov-core/test.cc
 * 
 * POV core test program. 
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

#include "povparser.h"
#include <ani-parser/ani_parser.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>


char *prg_name;

extern int hlib_allocdebug_trace;
extern size_t hlib_allocdebug_abort_seq;

int main(int argc,char **arg)
{
	prg_name=GetPrgName(arg[0]);
	
	// This only exists if allocation debugging is enabled in HLib: 
	// (Linker error otherwise.) 
	//hlib_allocdebug_trace=1;
	// Set this to find desired allocation call with debugger. 
	//hlib_allocdebug_abort_seq=632;
	
	do {
		// Static initialisation. IMPORTANT: 
		ANI::ExprValueType::_InitStaticVals();
		ANI::OpFunc_InitStaticVals();
		
		POV::CommandParser pcp;
		
		int rv=pcp.ParseFile(argc>=2 ? arg[1] : "test.pov",/*AniParser=*/NULL);
		fprintf(stderr,"Parse()=%d\n",rv);
		
		pcp.DumpContent(stdout);
		
		fprintf(stderr,"TreeNodes: "
			"%u bytes for %d tree nodes currently allocated.\n",
			ANI::TreeNode::tree_nodes_tot_size,
			ANI::TreeNode::n_alloc_tree_nodes);
		
	} while(0);
	
	fprintf(stderr,"EXITING: TreeNodes: "
			"%u bytes for %d tree nodes currently allocated.\n",
			ANI::TreeNode::tree_nodes_tot_size,
			ANI::TreeNode::n_alloc_tree_nodes);
	
	// CLEANUP static data: 
	ANI::OpFunc_CleanupStaticVals();
	ANI::ExprValueType::_CleanupStaticVals();
	
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

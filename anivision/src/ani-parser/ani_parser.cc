/*
 * ani-parser/ani_parser.cc
 * 
 * The AniVision language parser. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "tree.h"
#include "ani_parser.h"

#include "treereg.h"
#include "exprtf.h"

#include "ani_scanner.h"


namespace ANI
{


int Ani_Parser::ParseAnimation(RefString str_or_path,int is_file)
{
	int rv=-2;
	
	DELETE(lsb);
	
	AniLexerScanner *als = new AniLexerScanner(AniLexerScanner::SC_ANI,0);
	assert(!lsb);
	this->lsb=als;
	last_purpose="ANI";
	n_errors=0;
	assert(root==NULL);
	
	int siv=als->SetInput(str_or_path,
		is_file ? ALS_PF_File : ALS_PF_String,
		TSS_ANIMATON);
	if(!siv)
	{
		// Read in the file. 
		rv=_YYParse();
		
		fprintf(stderr,"ANI: Parsing done. Errors: %d lexer, %d parser. rv=%d\n",
			als->NErrors(),n_errors,rv);
		fprintf(stderr,"ANI:   Tree has %d nodes; "
			"%u bytes for %d tree nodes currently allocated.\n",
			root ? root->CountNodes() : 0,
			TreeNode::tree_nodes_tot_size,TreeNode::n_alloc_tree_nodes);
		// More memory is spent on: identifiers & "Value::"-values. 
	}	
	
	if(n_errors) assert(rv!=0);
	
	// Parsing done...
	DELETE(lsb);  als=NULL;
	
	return(rv);
}


int Ani_Parser::ParsePOVCommand(RefString buf,const SourcePosition &startpos,
	POV::POVTreeNode **ret_root)
{
	int rv=-2;
	
	AniLexerScanner *als = new AniLexerScanner(AniLexerScanner::SC_POV,1);
	assert(!lsb);
	last_purpose="POV";
	this->lsb=als;
	int save_errors=n_errors;
	n_errors=0;
	assert(pov_root==NULL);
	
	// The archive was created by POVLexerScanner_Interface::ScanFile() 
	// and is newly created for every (toplevel) file we scan (using 
	// e.g. $pov(scan=...)). 
	SourcePositionArchive *alt_spa=startpos.GetFile().GetArchive();
	assert(alt_spa);
	RefString _tmp(startpos.GetPath());
	int siv=als->SetInput(buf,ALS_PF_String,TSS_POV_ENTRY,
		&_tmp,startpos.GetLine(),startpos.GetLPos(),
		/*alt_pos_arch=*/alt_spa);
	if(!siv)
	{
		// Read in the file. 
		rv=_YYParse();
		
		fprintf(stderr,"POV:   Parsing done. Errors: %d lexer, %d parser. rv=%d\n",
			als->NErrors(),n_errors,rv);
	}	
	
	if(n_errors) assert(rv!=0);
	
	// Transfer root. 
	assert(ret_root);
	*ret_root=pov_root;
	pov_root=NULL;
	
	// Parsing done...
	DELETE(lsb);  als=NULL;
	n_errors=save_errors;
	
	return(rv);
}


void Ani_Parser::DumpTree(FILE *out)
{
	if(!root)
	{  fprintf(out,"DumpTree: empty root.\n");  }
	else
	{
		StringTreeDump strtd;
		root->DumpTree(&strtd);
		strtd.Append("\n");
		strtd.Write(out);
	}
}


int Ani_Parser::DoRegistration(AniAniSetObj *animation)
{
	if(!root)
	{  return(-2);  }
	if(n_errors)
	{  return(-3);  }
	
	fprintf(stderr,"%s: Registration step (%d substeps):\n",
		last_purpose,TRegistrationInfo::_TRL_LAST);
	
	for(TRegistrationInfo::TRegistrationLevel lvl=
		TRegistrationInfo::TRL_Optimize;
		lvl<TRegistrationInfo::_TRL_LAST; 
		lvl=(TRegistrationInfo::TRegistrationLevel)(lvl+1))
	{
		// We may not perform the TRL_AllIdentifiers step if there were 
		// errors in earlier steps. This is because we may deliberately 
		// attach a NULL ani scope to identifiers which have errors 
		// (e.g. name collisions) and these would now try to register 
		// again. 
		if(lvl==TRegistrationInfo::TRL_AllIdentifiers && n_errors)  break;
		
		TRegistrationInfo tri;
		tri.who=lvl;
		if(lvl==TRegistrationInfo::TRL_Animation)
		{  tri.animation=animation;  }
		root->DoRegistration(&tri);
		n_errors+=tri.n_errors;
		
		if(lvl==TRegistrationInfo::TRL_Optimize)
		{
			fprintf(stderr,"%s:   Stmt optimization removed %d tree nodes.\n",
				last_purpose,tri.tn_opt_deleted);
		}
		else
		{
			assert(tri.tn_opt_deleted==0);
			fprintf(stderr,"%s:   Registration step %d done: %d errors.\n",
				last_purpose,lvl,tri.n_errors);
		}
	}
	
	fprintf(stderr,"%s:   Registration done. n_errors=%d\n",
		last_purpose,n_errors);
	if(n_errors==0)
	{
		int n=root->CheckAniScopePresence();
		if(n)  ++n_errors;
		fprintf(stderr,"%s:   Left out ani scopes: %d%s\n",
			last_purpose,
			n,n ? " (INTERNAL ERROR)" : "");
	}
	
	return(n_errors ? 1 : 0);
}


int Ani_Parser::PerformExprTF()
{
	if(!root)
	{  return(-2);  }
	if(n_errors)
	{  return(-3);  }
	
	int prev_nnodes=root->CountNodes();
	size_t prev_tot_size=TreeNode::tree_nodes_tot_size;
	
	TFInfo ti;
	n_errors+=root->DoExprTF(&ti);
	
	#warning FIXME: 
	// FIXME:
	// Immediate constant folding is not clever enough to fold 
	// expressions like s+1+1 [ which is ((s+1)+1) ]. 
	
	int now_nnodes=root->CountNodes();
	
	fprintf(stderr,"%s:   TF: %d errors; %d node visitations\n",
		last_purpose,n_errors,ti.n_nodes_visited);
	fprintf(stderr,"%s:   TF: Immediate const folding removed %d nodes "
		"(%d node bytes)\n",
		last_purpose,
		prev_nnodes-now_nnodes,
		ssize_t(prev_tot_size)-ssize_t(TreeNode::tree_nodes_tot_size));
// NOTE: Incorrect: statistics is about ALL trees not just this one...
	fprintf(stderr,"%s:   Tree has %d nodes; "
		"%u bytes for %d tree nodes currently allocated.\n",
		last_purpose,now_nnodes,
		TreeNode::tree_nodes_tot_size,TreeNode::n_alloc_tree_nodes);
	
	if(!n_errors)
	{
		CheckIfEvalInfo cinfo;
		n_errors=root->CheckIfEvaluable(&cinfo);
		fprintf(stderr,"%s:   Evaluable check: %d errors "
			"(refs_to: this: %d, stack: %d, val: %d, static: %d)\n",
			last_purpose,n_errors,
			cinfo.nrefs_to_this_ptr,cinfo.nrefs_to_local_stack,
				cinfo.number_of_values,cinfo.nrefs_to_static);
	}
	
	return(n_errors ? 1 : 0);
}


Ani_Parser::Ani_Parser() : 
	identifier_hash(/*hash_size=*/10007/*...which is prime.*/)
{
	lsb=NULL;
	n_errors=0;
	root=NULL;
	pov_root=NULL;
	
	_set_root=NULL;
	
	hash_entries=0;
	hash_hits=0;
	hash_ents_size=0;
	hit_ents_size=0;
	
	last_purpose="none";
}

Ani_Parser::~Ani_Parser()
{
	_ClearHash();
	
	if(root)
	{
		fprintf(stderr,"ANI: Deleting main tree...\n");
		root->DeleteTree();
		root=NULL;
		fprintf(stderr,"ANI:   Main tree deleted: "
			"%u bytes for %d tree nodes still allocated.\n",
			TreeNode::tree_nodes_tot_size,TreeNode::n_alloc_tree_nodes);
	}
	DELETE(lsb);
}

}  // end of namespace ANI

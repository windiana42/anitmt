/*
 * ani-parser/.h
 * 
 * AniVision language parser. 
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

#ifndef _ANIVISION_ANI_PARSER_H_
#define _ANIVISION_ANI_PARSER_H_

#include <ani-parser/tree.h>
#include <pov-core/povtree.h>
#include <ani-parser/scannerbase.h>
#include <hlib/refstrhash.h>


namespace ANI
{

class Ani_Parser
{
	friend int Ani_parse(void *mode, Ani_Parser *_ap);
	friend int yylex(YYSTYPE *lvalp,YYLTYPE *llocp,void *mode, Ani_Parser *parser);
	friend void Ani_error(const char *s,void *_fp);
	private:
		LexerScannerBase *lsb;
		int n_errors;   // Accumulate all sorts of errors. 
		ANI::TreeNode *root;
		
		// This is just used temporarily to pass the root node 
		// from the POV parsing. It is then transferred to the 
		// caller. 
		// *pov_root is a POVTreeNode of type PTN_None with 
		// all the elements being PTNObjectSpec, PTNGeneralCmd or 
		// some other command-level node. 
		POV::POVTreeNode *pov_root;
		
		// Last purpose, "ANI" or "POV"; just for diagnostics. 
		const char *last_purpose;
		
		// Internally used between yyparse() and _YYParse(): 
		ANI::TreeNode *_set_root;
		
		// This is a hash of all identifiers, needed for more efficient 
		// memory use. 
		// The integer is used to count hash hits. 
		RefStringHash<int> identifier_hash;
		// Total counter: 
		int hash_entries;
		int hash_hits;
		size_t hash_ents_size;  // sum of entry sizes
		size_t hit_ents_size;   // sum of hit entry sizes
		
		// Clear the has and reset hit values. Also print statistics. 
		void _ClearHash();
		
		// Actually call the parser: 
		// Return value: 
		//  0 -> success
		//  1 -> there were parse errors
		//  2 -> critical parse error
		int _YYParse();
		// Internally used by the parser: 
		TNIdentifierSimple *_AllocIdentifier(const YYLTYPE &pos,char *str,
			bool const_char_arg=0);
		TNFunctionDef *_CreateFunctionDef(TNIdentifier *_name,
			TNTypeSpecifier *_ret_type,TNStatement *_func_body,
			bool is_static,const TNLocation _aloc0,
			bool is_const,const TNLocation _aloc1,
			TreeNode *arglist);
	public:  _CPP_OPERATORS
		Ani_Parser();
		~Ani_Parser();
		
		// Get the root node: 
		// Be careful with it. 
		TreeNode *GetRootNode()
			{  return(root);  }
		
		// Parse file or buffer as animation file. 
		// Return value: 
		//  -2  -> AniLexerScanner::SetInput() failed. 
		// else -> that of _YYParse(); see above. 
		int ParseAnimation(RefString str_or_path,int is_file);
		
		// Parse a POV command comment. 
		// Works similar to ParseAnimation(); returns same value. 
		// You MUST pass a pointer in ret_root where the tree root 
		// can be passed back. The tree root is not stored in this 
		// AniParser, so you CAN call ParsePOVCommand() immediately 
		// again to parse the next command(s) and you MUST take 
		// care to free the returned tree once it is no longer needed. 
		int ParsePOVCommand(RefString buf,const SourcePosition &startpos,
			POV::POVTreeNode **ret_root);
		
		// Dump the complete tree to a FILE: 
		// Uses indention. 
		void DumpTree(FILE *out);
		
		// Perform registration step. 
		// Must pass AniAniSetObj with AST_Animation (from core) 
		// for that. 
		// Return value: 
		//  1 -> there were errors during registration
		//  0 -> success
		// -2 -> root node empty; cannot do anything 
		//       (you need to call Parse() before)
		// -3 -> there were errors during parsing; nothing done 
		// Check coreglue.h for AniAniSetObj type. 
		int DoRegistration(AniAniSetObj *animation);
		
		// Perform type fixup step. 
		// Return value: 
		//  1 -> there were errors during TF/SE
		//  0 -> OK
		// -2 -> root node empty; cannot do anything 
		//       (you need to call Parse() before)
		// -3 -> there were errors during previous step(s) 
		//       nothing done
		int PerformExprTF();
};

}  // end of namespace ANI

#endif  /* _ANIVISION_ANI_PARSER_H_ */

/*
 * ani-parser/ani_scanner.h
 * 
 * AniVision language scanner. 
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

#ifndef _ANIVISION_ANI_SCANNER_H_
#define _ANIVISION_ANI_SCANNER_H_

#include <ani-parser/scannerbase.h>


extern "C"
{
	int AniS_lex(YYSTYPE *lvalp,/*YYLTYPE *llocp,*/void*);
	//int AniS_wrap(void*);
}

struct ani_scanner_private;

enum
{
	ALS_PF_File=  0x001,
	ALS_PF_String=0x002,
	ALS_PF_Push=  0x004
};

class AniLexerScanner : public LexerScannerBase
{
	friend int AniS_lex(YYSTYPE *lvalp,/*YYLTYPE *llocp,*/void*);
	//friend int AniS_wrap(void*);
	
	public:
		/* This is used by the token buffer. */
		struct TokenEntry : LinkedListBase<TokenEntry>
		{
			int token;
			YYLTYPE lloc;
			YYSTYPE lval;    /* Note: it's an [ugly] union... */
			
			/* Check if token may be deleted or cleared. */
			/* See zombie_list... */
			bool may_be_cleared() const;
			
			/* Clear up token; free str_val if needed. */
			/* Do not use force=1 unless you know what zombie_list is. */
			void clear(bool force=0);
			
			_CPP_OPERATORS
			TokenEntry();
			~TokenEntry()
				{  clear();  }
		};
		
		/* This is interally used instead of FILE* to feed the lexer. */
		class ALSInput
		{
			friend class AniLexerScanner;
			friend int AniS_lex(YYSTYPE *lvalp,/*YYLTYPE *llocp,*/void*);
			private:
				// The ALSInput one level below: 
				ALSInput *down;
				
				// Exactly one of these is non-null (!strbuf <--> is NULL): 
				RefString strbuf;   // reading from a string buffer 
				FILE *fp;           // reading from file
				
				// String buffer read position. 
				size_t strbuf_pos;
				// Stored path name for error diagnostics: 
				RefString fp_path;
				
				// Stored position when some other ALSInput is being 
				// read from (this is not on top of the stack). 
				int saved_line;
				int saved_lpos;
				// This is set when #including from the preprocessor 
				// because I then have to push initial state first which 
				// needs to get popped to be in PP state after file end. 
				int must_pop_state;
				// Tell pos_arch when reading file done. 
				int must_pop_pos_arch;
				
				// Special next token to be returned by next 
				// call to yylex (or -1): 
				int special_next_tok;
				
				// I hold a little token buffer for each input: 
				// NEW toks get added at the END of next_toks. 
				// OLDEST toks are at the beginning of prev_toks. 
				LinkedList<TokenEntry> next_toks;  // next tokens (read ahead)
				LinkedList<TokenEntry> prev_toks;  // already returned tokens
				int read_eof;
				
			public: _CPP_OPERATORS_FF
				ALSInput(ALSInput *down,int *failflag);
				~ALSInput();
				
				// Fill input content. Return value: 
				//  0 -> OK
				//  1 -> input already set
				// -2 -> opening file failed; see errno. 
				int SetFile(RefString path);
				int SetBuffer(RefString buf);
				
				// Perform read operation: 
				// Returns read bytes, 0 on EOF, -1 on error. 
				ssize_t read(char *buf,size_t len);
		};
	private:
		/* Private stuff... in lexer.ll becuase of prototypes... */
		struct ani_scanner_private *p;
		/* Count errors: */
		int n_errors;
		
		/* Current token    (while parsing)  */
		/* Start position   (previous position) */
		int line0,lpos0;
		/* End position     (current position) */
		int line1,lpos1;
		
		SourcePositionArchive *pos_arch;
		// If _we_ allocated the pos archive or if it was 
		// passed to us by SetInput(). 
		int pos_arch_allocated;
		
		// This needs some explanation: We allocate strings for 
		// TS_STRING and TS_IDENTIFIER but we do not know when 
		// these values are actually put into the tree by the 
		// bison parser. So, TokenEntries of that type get queued 
		// here until they are used. 
		LinkedList<TokenEntry> zombie_list;
		int zombie_list_nents;
		
		// Values for the token lists: 
		// Token look ahead cache size. 
		const int ahead_toks_wanted;   /* MINIMUM=1, NOT STATIC! */
		// NOTE: A larger value of old_toks_wanted enables better 
		//       re-use and keeps zombie_list smaller. 
		const int old_toks_wanted;
		
		// Main start condition / state. 
		// Can be IN_ANI for ani parsing or IN_POV for POV command 
		// comment parsing but _not_ INITIAL. 
		// Can be changed using SetMainStartCond(). 
		int main_start_cond;
		
		void __TidyUpZombieList(bool force);
		int last_zombie_list_nents;  // Number of entried after last tidy up. 
		inline void _TidyUpZombieList(bool force=0)
		{
			if(force || zombie_list_nents>=last_zombie_list_nents+16)
			{  __TidyUpZombieList(force);  }
		}
		
		/* Internally used for lex'ing; has no token buffer. */
		void _DoYYLex(TokenEntry *dest);
		
		/* Pop one buffer state from the stack. */
		/* Retval: 1 -> no more left; 0 -> go on */
		int _YYWrap();
		
		/* Use the current line0,lpos0..line1,lpos1 position and */
		/* create a TNLocation from it. Respects loc_use_lpos.   */
		ANI::TNLocation _MakeCurrLocation() const;
		/* ...using alternate start position: */
		ANI::TNLocation _MakeCurrLocation(int start_line,int start_lpos) const;
		
		int _ParseInt(char *yy_text,int yy_leng,YYSTYPE *yy_lval);
		int _ParseFloat(char *yy_text,int yy_leng,YYSTYPE *yy_lval);
		int _ParseString(char *yy_text,int yy_leng,YYSTYPE *yy_lval);
	public:  _CPP_OPERATORS
		/* Coonfig/tuning: */
		int allow_nested_comments;   /* 0 -> no; 1 -> yes; 2 -> yes and warn */
		int tab_width;               /* width of tab for correct lpos (8)    */
		int max_file_depth;          /* max include hierarchy depth          */
		int loc_use_lpos;            /* use lpos in location reporting (yes) */
		/* smart_lookahead tries to deambiguate ">" (larger <-> vector */
		/* end). It is somewhat flawed in that it cannot deambiguate   */
		/* in all situations and strange results may follow, so it is  */
		/* switched off by default. */
		int smart_lookahead;
		
		/* When YYLex() returns: position of the returned token: */
		YYLTYPE cloc;  /* "current location" */
		
		// [Overriding virtual:] 
		const YYLTYPE &GetCLoc() const
			{  return(cloc);  }
		
		// main_start_cond: main start condition, i.e. what sort of 
		//   tokens shall be recognized: 
		//   SC_ANI for animation parsing 
		//   SC_POV for POVRay command comment parsing 
		// Use SC_INITIAL if you do not know yet -- you may change 
		// the start condition at any time using SetMainStartCond(). 
		// YOU MUST SET runtime_change=1 if you ever intend to use 
		// SetMainStartCond(). (This will disable the token look 
		// ahead cache which is incompatible with start condition 
		// switching.) 
		AniLexerScanner(StartCond main_start_cond,bool runtime_change);
		~AniLexerScanner();
		
		// Get number of lexer/scanner errors: 
		// [Overriding virtual:] 
		int NErrors() const
			{  return(n_errors);  }
		
		// Reset scanner; like after construction but not changing 
		// config/tuning. 
		void Reset();
		
		// Change the main start condition, i.e. what tokens are 
		// recognized. NOTE that you must have set runtime_change=1 
		// in the constructor if you use this function. 
		// [overriding virtual]
		void SetMainStartCond(StartCond _main_start_cond);
		
		// This is the universal input function: 
		// str_or_file: The string (buffer) or file to be parsed; 
		//     may be both '\0'-terminated or not '\0'-terminated. 
		// flags: OR'ed together using:
		//   ALS_PF_File:   str_or_file is a path name to a file 
		//   ALS_PF_String: str_or_file is string buffer to be parsed
		//   ALS_PF_Push:   If set, treat passed input as "included", 
		//        i.e. stop reading the current input and parse this 
		//        inpur before continuing with the previous one. 
		// first_token: If >=0, this is the first token to be returned 
		//   by YYLex() before any tokens from the data to be lexically 
		//   analyzes. Useful to bring the parser into the correct 
		//   initial state. See the TSS_* tokens. 
		// You can pass the start position in the last 3 arguments if 
		// they are non-standard, e.g. if you pass a string to be 
		// read. A pos_file name may only be specified if ALS_PF_Push is 
		// not set; the file name will default to str_or_file if 
		// ALS_PF_File is set. 
		// In case you specify an alternate position, you may also 
		// pass a SourcePositionArchive to use. YOU THEN HAVE TO 
		// make sure that the current path is equal to the passed 
		// path. 
		// Return value: 
		//   0 -> OK; 
		//   1 -> must Reset() (another input is active and ALS_PF_Push
		//         was not set). 
		//  -2 -> failed to open file (see errno)
		//  -3 -> illegal flags (ALS_PF_File and ALS_PF_String set or 
		//        pos_file specified for ALS_PF_Push)
		//  -4 -> includes nested too deeply
		int SetInput(RefString str_or_file,int flags,int first_token,
			const RefString *pos_file=NULL,int pos_line=1,int pos_lpos=0,
			SourcePositionArchive *alt_pos_arch=NULL);
		
		// Guess what this will be...
		// [overriding virtual from LexerScannerBase.]
		int YYLex(YYSTYPE *lvalp,YYLTYPE *llocp);
};

#endif  /* _ANIVISION_ANI_SCANNER_H_ */

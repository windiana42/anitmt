/* 
 * pov-scan/pov_scanner.h
 * 
 * Lexical analyzer for POV special comment parser. 
 * Uses reentrant C scanner and thus needs flex-2.5.11 or higher. 
 * 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2002--2003 by Wolfgang Wieser (wwieser@gmx.de)
 * 
 * Flex code based on AniVision's language parser ani-parser/scanner.ll. 
 *
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _POV_SCANNER_H_
#define _POV_SCANNER_H_

// This file contains class POVLexerScanner. 
// Description what the class does: 
//  You give it a .pov file using SetInput(). Upon calling Parse(), 
//  the class will read the POV file until EOF, a sever error happens 
//  (unterminated string) or until it found and read in a comment 
//  which contained one or more special commands. The class does not 
//  know syntax of the commands, just reads them in literally conserving 
//  newlines and spacing so that correct error reporting can be done 
//  by higher level functions which read in the commands. 
//  The only syntax constraints for the command are: It has to begin 
//  with: 
//     //[whitespace]@IDENTIFIER( etc etc )       ..or..
//     /*[whitespace]@IDENTIFIER( blah... )
//  The brackets are important becuase the class checks for balanced 
//  brackets when deciding if a C++ style comment ends or not. 
//  More info further down. 
//    
//  When Parse() retuns value 1, the command has to be obtained using 
//  GetCommand(). The returned struct contains the file name and the 
//  position of the command and the content in *text of length text_len 
//  which is NOT '\0'-terminated. You must copy the contents of the 
//  returnes struct because the next time you call Parse(), the old 
//  contents will get lost. 
//  
//  The class will follow #include statements in they are of the 
//  following form: 
//     #include[whitespace]"FILE"
//  where at least one white space is required. Especially note that 
//  the statement will be ignored if there are spaces between the 
//  "#" and "include" and if it is not a single line. 
//  Thus, you can use "# include" if you would like that only POVRay 
//  follows the statement and that this class ignores it. 
//  
//  More info about the command comments: 
//  Two equivalent examples:
//  
//   /* @object(macro=bubble,
//      params={a,b, /*comment to be ignored*/ c}  // comment2
//      defs={pangle->angle} ) */
//
//   //@object(macro=bubble,  // comment2
//   //  params={a,b,c},defs={pangle->angle} )
//   // commentXX to be ignored
//
//   Note that when using C-style command comments, you may embed 
//   (nested) C-style and C++-style comments which get cut out as 
//   usual. However, when using the C++-style command comment, you 
//   may embed C++-style comments but NOT C-style comments. 
//   You may use any number of commands per comment. 
//   Note that C++-style comments are automatically resumed into 
//   the next line if 
//     - the next line begins with [whitespace]"//"  AND
//     - the brackets of the command read so far are all closed
//   Thus, the "commentXX" above will not be read in as command. 
//
//   You can manually terminate the command parsing inside a comment 
//   by using "@-", e.g. 
//   
//   /* @object(macro=bubble,
//      params={a,b,c},defs={pangle->angle})
//      @- further comments go here. */
//   // @object(macor=tired_of_bubble,
//   //  params={a,b,c},
//   // @- note that the object( directive above is not closed 
//   // @object(macro=this_will_be_interpreted_again)
// 

#include <hlib/cplusplus.h>
#include <hlib/refstring.h>
#include <ani-parser/location.h>


struct pov_scanner_private;

class POVLexerScanner
{
	friend int POVS_lex(void*);
	public:
		// This holds the special command comment. 
		struct CCommand
		{
			// Position of the command: 
			RefString file;
			int start_line;
			int start_lpos;
			int end_line;
			int end_lpos;
			// The actual command in a text buffer; size and alloc 
			// size below. Note that text is NOT '\0'-terminated. 
			char *text;
			size_t text_len;
			size_t text_asize;
			// Style of the command comment: 
			//   0 -> none; 1 -> C; 2 -> C++
			int style;
			// Counts number of open brackets. 
			int bracket_depth;
			
			// Set,append text in *text: 
			void SetText(const char *text,size_t len,bool check_brackets);
			void AppendText(const char *text,size_t len,bool check_brackets);
			
			// Clear content: 
			void Clear();
			
			// Returns 1, if the stored text has balanced brackets. 
			bool BalancedBrackets()
				{  return(bracket_depth==0);  }
			
			CCommand();
			~CCommand();
			_CPP_OPERATORS
		};
	private:
		// Private stuff... in lexer.ll becuase of prototypes... 
		struct pov_scanner_private *p;
		// Count errors: 
		int n_errors;
		
		// As passed to us with SetInput(): 
		SourcePositionArchive *pos_arch;
		
		// The currently parsed control command: 
		// cc.style=0 if not used 
		CCommand cc;
		
		// Pop one buffer state from the stack.
		// Retval: 1 -> no more left; 0 -> go on
		int _YYWrap();
		
		// Open input file, push it on the fhstack and continue 
		// scanning there. 
		// Return value: 
		//   0 -> OK; 
		//  -2 -> fopen() failed, see errno 
		//  -3 -> too deeply nested (see max_file_depth)
		int _OpenFile(const RefString &file);
		
		// Call to end a command; returns LEX_COMMAND. 
		int _EndCCommand();
	protected:
		// Virtual downcall: if an #include statement gets executed, 
		// all needed into is passed to keep include hierarchy stack in 
		// derived class up to date. 
		//   file: file name of included file
		//   line,lpos: position of the include statement
		// Note: This is also called by YYWrap at EOF with file being a 
		//       NULL reference (!file is true). 
		virtual void IncludingFile(const RefString &file,
			int line,int lpos);
		
		// Call this to stop reading this file and read on in 
		// the hierarchy up: 
		void SkipFile();
		
		/* Use the current line0,lpos0..line1,lpos1 position and */
		/* create a TNLocation from it. Respects loc_use_lpos.   */
		ANI::TNLocation _MakeCurrLocation() const;
		/* ...using alternate start position: */
		ANI::TNLocation _MakeCurrLocation(int start_line,int start_lpos) const;
		
		/* Start position   (previous position) */
		int line0,lpos0;   // see also line,lpos. 
		
	public:  _CPP_OPERATORS
		/* Config/tuning: */
		int allow_nested_comments;   /* 0 -> no; 1 -> yes; 2 -> yes and warn */
		int tab_width;               /* width of tab for correct lpos (8)    */
		int max_file_depth;          /* max include hierarchy depth          */
		int ignore_includes;         /* do not follow #include stmts         */
		int error_loc_use_lpos;      /* use lpos in locations for errors (y) */
		
		int line,lpos;   // current position; see also line0,lpos0. 
		
		//-----------------------------------------------------------------
		POVLexerScanner();
		virtual ~POVLexerScanner();
		
		// Get number of errors: 
		int Get_NErrors() const
			{  return(n_errors);  }
		
		// Reset scanner; like after construction but not changing 
		// config/tuning. 
		void Reset();
		
		// Read input from passed file: 
		// Pass a source position archive so that internally 
		// TNLocations (for the error reporting) can be created. 
		// Return value: 
		//   0 -> OK; 
		//   1 -> must Reset() (another input is active). 
		//  -2 -> failed to open file -- see errno
		int SetInput(const RefString &file,
			SourcePositionArchive *pos_arch_for_errors);
		
		// This will read in up to the next special comment block 
		// (This is like the yylex() call normally -- just that 
		// it does not return single tokens). 
		// Return value: 
		//    1 -> parsed next command block
		//         use GetCommand() to get access to it 
		//    0 -> EOF reached
		//   -1 -> no input was set
		//   -2 -> parsing aborted
		int Parse();
		
		// Get the command which was just parsed or NULL if 
		// there is none. 
		const CCommand *GetCommand() const
			{  return(cc.style ? &cc : NULL);  }
};

#endif  /* _POV_SCANNER_H_ */

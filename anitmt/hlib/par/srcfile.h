/*
 * srcfile.h
 * 
 * Header containing parameter source capable of reading parameters 
 * from a file. 
 * 
 * Copyright (c) 2001 -- 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "parsource.h"
#include "cmdline.h"

#ifndef _INC_PAR_ParSource_File_H_
#define _INC_PAR_ParSource_File_H_

namespace par
{

class ParameterSource_File : public ParameterSource
{
	public:
		enum PreprocessorErrcode
		{
			// warnings (<0)
			PPWarningIgnoringInclude=-100,  // allow_recursion=-1 and #include found
			PPWarningIgnoringGarbageAtEol,  // e.g. `*end blah'
			// errors (>0)
			PPUnknownSection=1,      // *section blah -> `blah' unknown
			PPTooManyEndStatements,  // *end occured too often
			PPArgOmitted,            // *section, *include without arg
			PPIncludeNestedTooOften, // allow_recursion exceeded
			PPUnterminatedString,    // <- after *section, *include...
			PPIllegalPPCommand       // e.g. *sgrfzz
		};
	private:
		Section *file_top_sect;
		Section *curr_sect;
		
		int _ParseString(char **str,size_t *retlen,char **end,
			const ParamArg::Origin *origin);
		
		int _ReadLine(FILE *fp,int *linecnt);
		int _ReadFile(const char *file,int allow_recursion);
		
		char *linebuf;
		size_t bufsize;
		size_t linelen;
		
		void _ResetLinebuf();
	protected:
		// (Called by the parse function.)
		// This notifies you of preprocessor errors when parsing a file. 
		// ppe -> tells you which error/warning was encountered; 
		//        NOTE: ppe>0 -> errors; ppe<0 -> warnings
		// pos -> current position; do not free pos->origin but copy 
		//        it if you need it lateron. 
		// sect -> current section pointer (all arg names in files are 
		//        relative to this section). 
		// arg -> this is normally NULL. 
		//        For PPUnknownSection this contains the secion name 
		//          which was not found. 
		// Return value: ignored; return 0. 
		virtual int PreprocessorError(
			PreprocessorErrcode ppe,
			const ParamArg::Origin *pos,
			const Section *sect,
			const char *arg);
		
		// Called when a file operation fails. 
		// path -> involved file 
		// line -> position inside file 
		// op -> operation which failed: 
		//        0 -> open() failed (line=-1)
		//        1 -> line buffer allocation failed
		//        2 -> read() failed
		//        3 -> ParamArg allocation failed 
		//       -1 -> failed to allocate RefString for path (line=-1) 
		// Return value: ignored; return 0. 
		virtual int FailedFileOp(
			const char *path,
			int line,
			int op);
		
	public:  _CPP_OPERATORS_FF
		ParameterSource_File(ParameterManager *manager,int *failflag=NULL);
		~ParameterSource_File();
		
		// Add all the params in file to the params here. 
		// All recognized params in file are copied as local copy 
		// and stored there. 
		// allow_recursion: allowed maximum recursion level. 
		//    -1 -> ignore #include 
		//    >=0 -> max. recursion level (0 -> only toplevel file) 
		// topsect: all parameters in the file are viewed relative to 
		//    topsect (NULL -> root section)
		// Return value: 
		//   0 -> success; 
		//  >0 -> errors	
		//  -1 -> aborted due to allocation failures 
		int ReadFile(const char *file,int allow_recursion,
			Section *topsect=NULL);
		
		// Parse in the param arg. This is called by ReadFile() for 
		// each parameter. 
		// pa is treated relative to topsect. 
		// Return value:
		//   0 -> success
		//  -1 -> error 
		//   1 -> pa->pdone set (nothing done)
		int Parse(ParamArg *pa,Section *topsect=NULL);
};

}  // namespace end 

#endif  /* _INC_PAR_ParSource_File_H_ */

/*
 * pov-core/povwriter.h
 * 
 * POV frame writer. 
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

#ifndef _ANIVISION_POV_POVWRITER_H_
#define _ANIVISION_POV_POVWRITER_H_

#include <hlib/cplusplus.h>

//#include <pov-core/objectspec.h>
//#include <pov-scan/pov_scan.h>
//#include <hlib/refstrlist.h>

#include <core/anicontext.h>


namespace POV
{

// POV frame writer. 
// Writes the POVRay output files. 
class FrameWriter
{
	public:
		// Class used to write the file. 
		class OFile
		{
			private:
				RefString fname;
				FILE *fp;
			public:
				// This counts the bytes written so far and is reset 
				// to zero by the Open() call (hence still valid after 
				// calling Close()). 
				size_t bytes_written;
			public:
				OFile();
				~OFile();
				
				// Return value: 
				//   0 -> OK;
				//  -1 -> error (message already written)
				int Open(RefString file);
				int Close();
				
				// Write content to file. 
				// Aborts on errors. 
				// Return value: 0. 
				int Printf(const char *fmt,...)
					__attribute__ ((__format__ (__printf__, 2, 3)));
		};
		
	private:
		// Current frame number: 
		int frame_no;
		
		// Current output file: 
		OFile ofile;
		
		// Some statistics: 
		int total_files_written;
		size_t total_bytes_written;
		
		// Stores the files already #included in this file. 
		RefStrList included_files;
		
		void _EvalExpr(String *dest,ExecThread *info,PTNExpression *expr,
			bool pass_null=0,bool remove_quotes=0);
		
		// Write POV code for an object: 
		int _WriteObject(ExecThread *eval_thread,
			Animation_Context::ObjectInstanceNode *oin,
			Animation_Context::POVAttachmentNode *pan);
	public:  _CPP_OPERATORS
		FrameWriter();
		~FrameWriter();
		
		// Actually write frame: 
		// Returns error count. 
		int WriteFrame(Animation_Context *ctx,ExecThread *eval_thread);
};

}  // end of namespace POV

#endif  /* _ANIVISION_POV_POVWRITER_H_ */

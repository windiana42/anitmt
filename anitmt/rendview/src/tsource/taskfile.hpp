/*
 * taskfile.hpp
 * 
 * Task file class; holding any file used by rendview. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _RNDV_TASKFILECLASS_HPP_
#define _RNDV_TASKFILECLASS_HPP_ 1

#include <lib/prototypes.hpp>
#include <hlib/refstring.h>
#include <hlib/htime.h>


// Any file used by rendview. 
class TaskFile
{
	public:
		enum FType
		{
			FTNone=0,
			FTImage,   // image like PNG,...  (f0001.png)
			FTFrame,   // render source file (the _frame_ scene file (f0001.inc))
			FTAdd,     // additional file (IOTRenderInput or IOTFilterInput)
		};
		enum IOType
		{
			IOTNone=0,
			IOTRenderInput,
			IOTRenderOutput,
			IOTFilterInput,
			IOTFilterOutput
		};
	private:
		// File type: 
		FType ftype;
		IOType iotype;
		
		// Info on what is to be done with the file on exit...
		//int delete_on_exit;
		
		// where to store
		
		// where it is from
		
		// where it is currently on the hd 
		RefString hdpath;
		//RefString basename
		//int64_t size;   // file size; (-1 if unset/nonexistant)
		//HTime mtime;    // modification time (invalid if unset/nonexistant)
		
		// Do not copy: 
		void _forbidden();
		void operator=(const TaskFile &)  {  _forbidden();  }
		TaskFile(TaskFile &)  {  _forbidden();  }
	public:  _CPP_OPERATORS_FF
		TaskFile(FType ft,IOType iot,int *failflag=NULL);
		~TaskFile();
		
		// Query type: 
		FType  GetFType()  const   {  return(ftype);  }
		IOType GetIOType() const   {  return(iotype);  }
		
		// Get path on hd (where the file actually is): 
		RefString HDPath() const
			{  return(hdpath);  }
		
		// Return the length of the base name (right of rightmost '/'). 
		// Returns 0 if hdpath is unset. 
		size_t BaseNameLength() const
			{  const char *tmp=BaseNamePtr();  return(tmp ? strlen(tmp) : 0);  }
		// Returns the pointer to the base name or NULL. 
		// ONLY USE TO COPY IT. Pointer is internal RefString-data. 
		const char *BaseNamePtr() const;
		
		// Return value: file length or -1 -> stat failed; -2 -> no hdpath
		int64_t FileLength() const
			{  return(FileLengthMTime(NULL));  }
		// Extended function: Return file length and modification time: 
		int64_t FileLengthMTime(HTime *mtime) const;
		
		// THESE FUNCTIONS MUST BE USED WITH CARE: 
		void SetHDPath(RefString r)   {  hdpath=r;  }
};

#endif  /* _RNDV_TASKFILECLASS_HPP_ */

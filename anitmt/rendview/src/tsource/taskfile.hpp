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
			FTScene,   // render source file (static part one for all scenes (anim.pov))
			FTFrame    // render source file (the _frame_ scene file (f0001.inc))
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
		
		
		// THESE FUNCTIONS MUST BE USED WITH CARE: 
		void SetHDPath(RefString r)   {  hdpath=r;  }
};

// THIS SHOULD BE MERGED WITH TaskFile. 
class AdditionalFile
{
	public:  _CPP_OPERATORS_FF
		AdditionalFile(int * /*failflag*/=NULL)  {  size=-1;  mtime.SetInvalid();  }
		~AdditionalFile()  {}
		
		int64_t size;   // file size; (-1 if unset/nonexistant)
		HTime mtime;    // modification time (invalid if unset/nonexistant)
		
		//RefString basename
		//RefString hdpath
		
		// Return the length of the base name (right of rightmost '/'). 
		size_t BaseNameLength() const  {  return(0);  }
};

#endif  /* _RNDV_TASKFILECLASS_HPP_ */

/*
 * taskfile.hpp
 * 
 * TaskFile - Task file class; holding any file used by rendview. 
 * TaskFileManager - Class managing all the task files. 
 * 
 * Copyright (c) 2002--2003 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include <hlib/linkedlist.h>
#include <hlib/refstring.h>
#include <hlib/htime.h>


class TaskFile;
class InternalTaskFile;
class TaskFileManager;

// This class only serves as a namespace for TaskFile and 
// TaskFileManager. 
struct TaskFileNamespace
{
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
	enum FCreator
	{
		FCGeneral=0,
		FCLocal,
		FCLDR,
		_FCLast
	};
	enum DeleteSpec
	{
		DontDel=0,  // DO NOT CHANGE VALUES OR ORDER. 
		DelExit,    // delete when exiting (TaskFileManager destructor)
		DelCycle,   // delete when recovering / before exiting
		DelFrame    // delete when frame done (i.e. TaskFile destructor)
	};
	
	static const char *FTypeString(FType ft);
	static const char *IOTypeString(IOType iot);
	static const char *FCreatorString(FCreator fc,bool with_colon=0);
	static const char *DeleteSpecString(DeleteSpec ds);
	
	static TaskFileManager *_tfmanager_ptr;
	static TaskFileManager *tfmanager()
		{  return(_tfmanager_ptr);  }
};


// This is the task file manager, a manager class just like most 
// other in this program: Exactly one instance must exist. 
// TaskFileManager owns all existing (Internal)TaskFiles. 
class TaskFileManager : public TaskFileNamespace
{
	private:
		// This is the hash of all existing (Internal)TaskFiles: 
		int tfhash_size;
		LinkedList<InternalTaskFile> *tfhash;  // LMalloc()'ed
		
		// Compute hash value out of path (rage: 0..tfhash_size)
		inline int _DoHash(const RefString *hdpath) const;
		
		// Find TaskFile with specified path in hash; 
		// return NULL if not found. 
		InternalTaskFile *_FindTaskFile(const RefString *hdpath);
		// Insert/Remove task file into/from hash: 
		void _InsertTaskFile(InternalTaskFile *itf);
		InternalTaskFile *_RemoveTaskFile(InternalTaskFile *iff);  // DOES NOT DELETE IT. 
		
	public:  _CPP_OPERATORS_FF
		TaskFileManager(int *failflag=NULL);
		~TaskFileManager();
		
		// Used by InternalTaskFile when last ref gets deleted: 
		void destroy(InternalTaskFile *itf);
		
		// Generate a TaskFile with specified name; if this task 
		// file is already in the database, return a reference. 
		// Check for failure using TaskFile::is_null(). 
		TaskFile GetTaskFile(RefString hdpath,FType ft,IOType iot,FCreator fc);
		
		// See TaskFile for these: 
		int SetFixedState(InternalTaskFile *itf,int64_t size,HTime *mtime,bool dld_ing);
		int64_t GetFixedState(InternalTaskFile *itf,HTime *mtime);
		int ClearFixedState(InternalTaskFile *itf);
		
		// See TaskFile:
		int SetIncompleteRename(InternalTaskFile *itf,
			RefString *new_name,bool may_not_exist,const char *error_prefix);
		
		// Returns the pointer to the base name or NULL. 
		// ONLY USE TO COPY IT. Pointer is internal RefString-data. 
		const char *BaseNamePtr(const InternalTaskFile *itf) const;
		
		// Return value: file length or -1 -> stat failed; -2 -> NULL ref
		// Store modification time in mtime if non-NULL. 
		// aware_of_fixed_state: if 1, then fixed_state may be set but 
		//   we call real stat(2) nevertheless. 
		int64_t FileLength(const InternalTaskFile *itf,HTime *mtime=NULL,
			bool aware_of_fixed_state=0);
		
		// Tell TaskFIleManager to delete files with DeleteSpec>=ds. 
		// ds must be >=DelExit. 
		// Return value: currently only 0. 
		int TidyUp(DeleteSpec ds);
		static int DoTidyUp(DeleteSpec ds)
			{  return(tfmanager()->TidyUp(ds));  }
};



// Any file used by rendview. 
class InternalTaskFile : 
	public TaskFileNamespace, 
	public LinkedListBase<InternalTaskFile>
{
	friend class TaskFileManager;
	friend class TaskFile;  // For member (read) access. 
	private:
		// Reference counter: 
		int refcnt;
		
		// File type: 
		FType ftype;
		IOType iotype;
		FCreator fcreator;
		
		// Where it is currently on the hd. 
		// MUST BE SET FOR EVERY FILE; CANNOT BE CHANGED (normally). 
		RefString hdpath;
		
		// When to delete this file (if ever): 
		DeleteSpec delspec;
		
		// Various flags: 
		int is_incomplete : 1;  // unfinished frame
		int was_downloaded : 1;
		int downloading : 1;   // see SetFixedState()
		int fixed_state : 1;   // see SetFixedState()
		int skip_flag : 1;   // special flag used by LDR server
		int : (sizeof(int)*8-5);
		
		// This is only valid if fixed_state: 
		HTime fix_mtime;
		int64_t fix_size;
		
		// Used by TaskFile: 
		void addref()  {  ++refcnt;  }
		void deref()
			{  --refcnt;  if(refcnt<=0)  tfmanager()->destroy(this);  }
		
	public:  _CPP_OPERATORS_FF
		InternalTaskFile(int *failflag=NULL);
		~InternalTaskFile();
		
		
};

// Task file is just a reference to a unique InternalTaskFile 
// managed by the TaskFileManager. You may use TaskFile just like 
// any ordinary POD type. 
class TaskFile : public TaskFileNamespace
{
	friend class TaskFileManager;
	private:
		InternalTaskFile *itf;
		
		inline void _deref()
			{  if(itf)  {  itf->deref(); itf=NULL;  }  }
		inline void _addref(InternalTaskFile *_itf)
			{  if(_itf)  {  itf=_itf;  itf->addref();  }  }
		
		// Internally used by TaskFileManager to construce a TaskFile. 
		TaskFile(InternalTaskFile *_itf)
			{  _addref(_itf);  }
	public:  _CPP_OPERATORS_FF
		TaskFile(int * =NULL)
			{  itf=NULL;  }
		TaskFile(const TaskFile &tf)
			{  itf=NULL;  _addref(tf.itf);  }
		~TaskFile()
			{  _deref();  }
		
		TaskFile &operator=(const TaskFile &tf)
			{  _deref();  _addref(tf.itf);  return(*this);  }
		
		// Returns 1 if this ia a NULL ref. 
		bool is_null() const    {  return(!itf);  }
		bool operator!() const  {  return(!itf);  }
		
		//**** TaskFile creation ****
		static TaskFile GetTaskFile(RefString hdpath,FType ft,IOType iot,FCreator fc)
			{  return(tfmanager()->GetTaskFile(hdpath,ft,iot,fc));  }
		
		//**** Access functions: ****
		
		// Query type: 
		FType  GetFType()  const   {  return(itf ? itf->ftype : FTNone);  }
		IOType GetIOType() const   {  return(itf ? itf->iotype : IOTNone);  }
		
		// Get path on hd (where the file actually is): 
		RefString HDPath() const
			{  return(itf ? itf->hdpath : RefString());  }
		
		// Returns the pointer to the base name or NULL. 
		// ONLY USE TO COPY IT. Pointer is internal RefString-data. 
		const char *BaseNamePtr() const
			{  return(tfmanager()->BaseNamePtr(itf));  }
		
		// Return value: file length or -1 -> stat failed; -2 -> NULL ref
		// Store modification time in mtime if non-NULL. 
		// aware_of_fixed_state: if 1, then fixed_state may be set but 
		//   we call real stat(2) nevertheless. 
		int64_t FileLength(HTime *mtime=NULL,bool aware_of_fixed_state=0) const
			{  return(tfmanager()->FileLength(itf,mtime,aware_of_fixed_state));  }
		
		// Set and get fixed file state. This can be used to 
		// temporarily store some value for size and mtime and 
		// retrieve it again. 
		// Used e.g. when downloading a file. 
		//   0 -> OK
		//  -2 -> TaskFile is NULL ref
		//  -3 -> downloading flag is set
		//  -4 -> size<0 or mtime invalid
		int SetFixedState(int64_t size,HTime *mtime,bool downloading)
			{  return(tfmanager()->SetFixedState(itf,size,mtime,downloading));  }
		// Return value: -1 -> no fixed state stored
		int64_t GetFixedState(HTime *mtime=NULL)
			{  return(tfmanager()->GetFixedState(itf,mtime));  }
		// Reset fixed state flag (e.g. when downloading done). 
		// Return vale: 
		//   0 -> OK
		//  -2 -> TaskFile is NULL ref
		//  -3 -> was not in fixed state 
		int ClearFixedState()
			{  return(tfmanager()->ClearFixedState(itf));  }
		
		// Set special skip_flag: 
		void SetSkipFlag(bool val)
			{  if(itf)  itf->skip_flag=val;  }
		int GetSkipFlag() const  // -2 -> NULL ref
			{  return(itf ? itf->skip_flag : -2);  }
		
		// Get/set incomplete flag: 
		void SetIncomplete(bool val)
			{  if(itf)  itf->is_incomplete=val;  }
		// Return value: 
		// hdpath is only chaned if renaming did not return error
		// +1 -> file did not exist and may_not_exist was set 
		//  0 -> OK
		// -2 -> null ref
		// -3 -> renaming failed; error written
		int SetIncompleteRename(RefString *new_name,bool may_not_exist,
			const char *error_prefix)
			{  return(tfmanager()->SetIncompleteRename(itf,new_name,may_not_exist,error_prefix));  }
		int IsIncomplete()
			{  return(itf ? itf->is_incomplete : -2);  }
		
		// Set delete spec:
		// Return value: 0 -> OK; -2 -> NULL ref
		int SetDelSpec(DeleteSpec ds)
			{  if(!itf)  return(-2);  itf->delspec=ds;  return(0);  }
};


#if 0
	private:
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
		
		
		
		// THESE FUNCTIONS MUST BE USED WITH CARE: 
		void SetHDPath(RefString r)   {  hdpath=r;  }
};
#endif

#endif  /* _RNDV_TASKFILECLASS_HPP_ */

/*
 * tfmanager.cpp
 * 
 * Task file manager class; knowing any file used by rendview. 
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


/*#include "taskfile.hpp"*/
#include "tasksource.hpp"

#include <assert.h>


#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif



// Static data setup: 
TaskFileManager *TaskFileNamespace::_tfmanager_ptr=NULL;


inline int TaskFileManager::_DoHash(const RefString *hdpath_rv) const
{
	int accu=0;
	for(const unsigned char *c=(const unsigned char*)hdpath_rv->str(); *c; c++)
	{  accu+=int(*c);  }
	return(accu % tfhash_size);
}


InternalTaskFile *TaskFileManager::_FindTaskFile(const RefString *hdpath_rv)
{
	if(!hdpath_rv)  return(NULL);
	LinkedList<InternalTaskFile> *head=&(tfhash[_DoHash(hdpath_rv)]);
	for(InternalTaskFile *i=head->first(); i; i=i->next)
	{  if(i->hdpath_rv==*hdpath_rv)  return(i);  }
	return(NULL);
}


void TaskFileManager::_InsertTaskFile(InternalTaskFile *itf)
{
	if(!itf)  return;
	LinkedList<InternalTaskFile> *head=&(tfhash[_DoHash(&itf->hdpath_rv)]);
	assert(!itf->next && head->last()!=itf);  // assert !queued
	head->insert(itf);
	_IncFileNumber();
}

InternalTaskFile *TaskFileManager::_RemoveTaskFile(InternalTaskFile *itf)
{
	if(!itf)  return(NULL);
	LinkedList<InternalTaskFile> *head=&(tfhash[_DoHash(&itf->hdpath_rv)]);
	assert(itf->next || head->last()==itf);  // assert !queued
	head->dequeue(itf);
	_DecFileNumber();
	return(itf);
}


void TaskFileManager::_DoUnlinkFile(InternalTaskFile *itf)
{
	if(!itf->was_downloaded && !itf->downloading && 
		itf->iotype!=IOTRenderOutput && itf->iotype!=IOTFilterInput)
	{
		// We may delete render/filter output when done. 
		Error("TF: OOPS?! Deleting non-downloaded (??) file \"%s\".\n",
			itf->hdpath_rv.str());
		Error("This is a bug. Aborting now to protect data.\n");
		assert(0);
	}
	DeleteFile(&itf->hdpath_rv,/*may_not_exist=*/1,
		/*error_prefix=*/FCreatorString(itf->fcreator,/*with_colon=*/1));
	itf->delspec=DontDel;
}


void TaskFileManager::_AssignJobPath(InternalTaskFile *itf,
	const RefString *hdpath_job,IOType iot,int reregister_error_occured)
{
	// If the hdpath for the job is not specified, 
	// keep the old value untouched. 
	if(!hdpath_job || !*hdpath_job)  return;
	
	RefString *dest=NULL;
	
	if(iot==IOTRenderInput || iot==IOTRenderOutput)
	{  dest=&(itf->hdpath_job[DTRender]);  }
	else if(iot==IOTFilterInput || iot==IOTFilterOutput)
	{  dest=&(itf->hdpath_job[DTFilter]);  }
	assert(dest);
	
	if(!(*dest))
	{  *dest=*hdpath_job;  }
	else if((*dest)!=(*hdpath_job))
	{
		// In this case, the reregister error must have been reported. 
		// If this assert fails, the bug is most likely not in 
		// TaskFileManager but in the task source (or who ever 
		// called GetTaskFile()). 
		assert(reregister_error_occured);
		*dest=*hdpath_job;
	}
}


TaskFile TaskFileManager::GetTaskFile(RefString hdpath_rv,
	FType ft,IOType iot,FCreator fc,const RefString *hdpath_job,
	int *reregister_error_ret)
{
	if(reregister_error_ret)
	{  *reregister_error_ret=0;  }
	
	if(!hdpath_rv || !ft || !iot)
	{  TaskFile rtf;  return(rtf);  }
	
	InternalTaskFile *itf=_FindTaskFile(&hdpath_rv);
	if(itf)
	{
		// Check type: 
		int rereg_error=0;
		do {
			if(itf->ftype==ft && itf->iotype==iot)  break;
			if(itf->ftype==ft && itf->iotype==IOTRenderOutput && 
				iot==IOTFilterInput)
			{  itf->iotype=IOTFilterInput;  break;  }
			rereg_error=1;
			if(reregister_error_ret)
			{  *reregister_error_ret=-2;  }
			else
			{  Error("TF: File[%d] \"%s\" (%s/%s) re-registered as %s/%s. "
				"Expect trouble.\n",itf->refcnt,hdpath_rv.str(),
				FTypeString(itf->ftype),IOTypeString(itf->iotype),
				FTypeString(ft),IOTypeString(iot));  }
		} while(0);
		// See if we have to update the job path: 
		_AssignJobPath(itf,hdpath_job,iot,rereg_error);
		return(TaskFile(itf));
	}
	
	// hdpath_job may be unset for additional files or if 
	// TaskFile already exists. 
	if((!hdpath_job || !*hdpath_job) && ft!=FTAdd)
	{  TaskFile rtf;  return(rtf);  }
	
	itf=NEW<InternalTaskFile>();
	if(!itf)
	{  TaskFile rtf;  return(rtf);  }
	itf->ftype=ft;
	itf->iotype=iot;
	itf->fcreator=fc;
	itf->hdpath_rv=hdpath_rv;
	_AssignJobPath(itf,hdpath_job,iot,/*reregister_error=*/0);
	_InsertTaskFile(itf);
	return(TaskFile(itf));
}


void TaskFileManager::destroy(InternalTaskFile *itf)
{
	if(!itf)  return;
	assert(itf->refcnt==0);
	
	// See what we have to do for it: 
	int do_remove=1;
	if(itf->delspec>=DelFrame)
	{
		// We have to delete it: 
		if(itf->was_downloaded || itf->downloading || 
		   itf->iotype==IOTRenderOutput || itf->iotype==IOTFilterInput)
		{  _DoUnlinkFile(itf);  }
	}
	// Got to keep files in the hash if they have to be deleted lateron. 
	else if(itf->delspec>=DelExit)
	{  do_remove=0;  }
	
	if(do_remove)
	{  delete _RemoveTaskFile(itf);  }
	else
	{
		//itf->was_downloaded=0;
		itf->downloading=0;
		itf->fixed_state=0;
		itf->skip_flag=0;
	}
}


const char *TaskFileManager::BaseNamePtr(const InternalTaskFile *itf) const
{
	if(!itf || !itf->hdpath_rv)  return(NULL);
	const char *c=itf->hdpath_rv.str();
	const char *ptr=strrchr(c,'/');
	return(ptr ? (ptr+1) : c);
}


int64_t TaskFileManager::FileLength(const InternalTaskFile *itf,HTime *mtime,
	bool aware_of_fixed_state)
{
	if(!itf)
	{  return(-2);  }
	// If we're downloading, we should probably return fix_size/fix_mtime. 
	assert(aware_of_fixed_state || !itf->downloading);
	assert(aware_of_fixed_state || !itf->fixed_state);
	return(GetFileLength(itf->hdpath_rv.str(),mtime));
}


int TaskFileManager::SetFixedState(InternalTaskFile *itf,
	int64_t size,const HTime *mtime,bool dld_ing)
{
	if(!itf)  return(-2);
	if(itf->downloading)  return(-3);
	if(size<0 || !mtime || mtime->IsInvalid())  return(-4);
	itf->fix_size=size;
	itf->fix_mtime=*mtime;
	itf->downloading=dld_ing;
	itf->fixed_state=1;
	return(0);
}

int64_t TaskFileManager::GetFixedState(InternalTaskFile *itf,HTime *mtime)
{
	if(!itf)  return(-2);
	if(!itf->fixed_state)  return(-3);
	if(mtime)
	{  *mtime=itf->fix_mtime;  }
	return(itf->fix_size);
}

int TaskFileManager::ClearFixedState(InternalTaskFile *itf)
{
	if(!itf)  return(-2);
	if(!itf->fixed_state)  return(-3);
	itf->fix_size=-1;
	itf->fix_mtime.SetInvalid();
	if(!itf->was_downloaded)
	{  itf->was_downloaded=itf->downloading;  }
	itf->downloading=0;
	itf->fixed_state=0;
	return(0);
}


int TaskFileManager::SetIncompleteRename(InternalTaskFile *itf,
	RefString *new_name_rv,bool may_not_exist,const char *error_prefix)
{
	if(!itf)  return(-2);
	
	itf->is_incomplete=1;
	int rv=RenameFile(&itf->hdpath_rv,new_name_rv,may_not_exist,error_prefix);
	if(rv<0)  return(-3);
	
	// No failure. Change hdpath. 
	_RemoveTaskFile(itf);
	
	itf->hdpath_rv=*new_name_rv;
	
	#if 0
	/* This is currently not needed, becuase at the time this    */
	/* function is called, the job hdpaths are no longer needed. */
	// Also change job-relative paths. This fails if the 
	// "rename" did something different than appending a string 
	// which should not happen in SetIncompleteRename(). 
	const char *oldn=itf->hdpath_rv.str();
	const char *newn=new_name_rv->str();
	size_t oldn_len=strlen(oldn);
	size_t newn_len=strlen(newn);
	// Find the appended string: 
	hack_assert(!strncmp(oldn,newn,oldn_len));
	hack_assert(newn_len>=oldn_len);
	const char *append=newn+oldn_len;
	for(int i=0; i<_DTLast; i++)
	{
		if(!itf->hdpath_job[i].str())  continue;
		if(!itf->hdpath_job[i].append(append))  continue;
		// Agrr... alloc failure. 
		Error("TF: %s\n",cstrings.allocfail);
		rv=-1;  <---- DOCUMENT RETVAL WHICH SHADOWS rv=+1!
	}
	#endif
	
	_InsertTaskFile(itf);
	
	return(rv);
}


int TaskFileManager::TidyUp(DeleteSpec ds)
{
	assert(ds>=DelExit);
	
	int ndel[_FCLast];
	for(int i=0; i<_FCLast; i++)
	{  ndel[i]=0;  }
	for(int i=0; i<tfhash_size; i++)
	{
		for(InternalTaskFile *_tf=tfhash[i].first(); _tf; )
		{
			InternalTaskFile *tf=_tf;
			_tf=_tf->next;
			
			if(tf->delspec<ds)  continue;
			// Okay, we have to delete it, if there are no more refs to it. 
			// Currently, if there are still refs than this is a bug. 
			if(tf->refcnt)
			{
				Error("TF: OOPS: Still %d refs to \"%s\". (ctsks left: %d)\n",
					tf->refcnt,tf->hdpath_rv.str(),
					CompleteTask::n_complete_tasks);
				assert(0);
			}
			
			if(tf->was_downloaded || tf->downloading || 
			   tf->iotype==IOTRenderOutput || tf->iotype==IOTFilterInput)
			{
				// This already writes an error. 
				_DoUnlinkFile(tf);
				assert(tf->fcreator>=0 && tf->fcreator<_FCLast);
				++ndel[tf->fcreator];
			}
			
			// Finally remove it: 
			delete _RemoveTaskFile(tf);
		}
	}
	if(ndel[FCLocal])
	{  Verbose(TSLR,"Local: Deleted %d (temporary) files.\n",ndel[FCLocal]);  }
	if(ndel[FCLDR])
	{  Verbose(TSLR,"LDR: Deleted %d (temporary) files.\n",ndel[FCLDR]);  }
	if(ndel[FCGeneral])
	{  Verbose(MiscInfo,"Deleted %d (temporary) files.\n",ndel[FCGeneral]);  }
	assert(_FCLast==3);   // otherwise: need more messages...
	
	Verbose(DBG,"TF: TidyUp(%d): Max number of files: %d (now: %d)\n",
		ds,max_file_no,curr_file_no);
	
	return(0);
}


TaskFileManager::TaskFileManager(int *failflag)
{
	int failed=0;
	
	max_file_no=0;
	curr_file_no=0;
	
	tfhash_size=31;
	tfhash=NEWarray< LinkedList<InternalTaskFile> >(tfhash_size);
	if(!tfhash)
	{  ++failed;  }
	
	
	if(failed)
	{
		if(failflag)
		{  (*failflag)-=failed;  return;  }
		ConstructorFailedExit("TFMan");
	}
	// Register global manager. 
	assert(!TaskFileNamespace::_tfmanager_ptr);
	TaskFileNamespace::_tfmanager_ptr=this;
}

TaskFileManager::~TaskFileManager()
{
	for(int i=0; i<tfhash_size; i++)
	{
		while(tfhash[i].first())
		{
			fprintf(stderr,"OOPS: TFMan: %s still in hash.\n",
				tfhash[i].first()->hdpath_rv.str());
			_DecFileNumber();
			delete tfhash[i].popfirst();
		}
	}
	
	tfhash=DELarray(tfhash);
	tfhash_size=0;
	assert(TaskFileNamespace::_tfmanager_ptr==this);
	TaskFileNamespace::_tfmanager_ptr=NULL;
	
	assert(curr_file_no==0);
}


/******************************************************************************/

InternalTaskFile::InternalTaskFile(int *failflag) : 
	LinkedListBase<InternalTaskFile>(),
	hdpath_rv(failflag),
	dl_srv_mtime(HTime::Invalid),
	fix_mtime(HTime::Invalid)
{
	assert(failflag);
	
	refcnt=0;
	
	ftype=FTNone;
	iotype=IOTNone;
	fcreator=FCGeneral;
	
	delspec=DontDel;
	
	is_incomplete=0;
	was_downloaded=0;
	downloading=0;
	fixed_state=0;
	skip_flag=0;
	
	fix_size=-1;
	
	hdpath_job=NEWarray<RefString>(_DTLast);
	if(!hdpath_job)
	{  --(*failflag);  }
}

InternalTaskFile::~InternalTaskFile()
{
	hdpath_job=DELarray(hdpath_job);
}


/******************************************************************************/

const char *TaskFileNamespace::FTypeString(FType ft)
{
	switch(ft)
	{
		case FTNone:   return("[none]");
		case FTImage:  return("image");
		case FTFrame:  return("frame");
		case FTAdd:    return("additional");
	}
	return("???");
}

const char *TaskFileNamespace::IOTypeString(IOType iot)
{
	switch(iot)
	{
		case IOTNone:          return("[none]");
		case IOTRenderInput:   return("render input");
		case IOTRenderOutput:  return("render output");
		case IOTFilterInput:   return("filter input");
		case IOTFilterOutput:  return("filter output");
	}
	return("???");
}

const char *TaskFileNamespace::FCreatorString(FCreator fc,bool with_colon)
{
	switch(fc)
	{
		case FCGeneral:  return("");
		case FCLocal:    return(with_colon ? "Local: " : "local");
		case FCLDR:      return(with_colon ? "LDR: " : "LDR");
	}
	return(with_colon ? "???: " : "???");
}

const char *TaskFileNamespace::DeleteSpecString(DeleteSpec ds)
{
	switch(ds)
	{
		case DontDel:   return("never");
		case DelExit:   return("on exit");
		case DelCycle:  return("at cycle end");
		case DelFrame:  return("when frame done");
	}
	return("???");
}


/******************************************************************************/

RefString TaskFile::HDPathJob(TaskDriverType dtype) const
{
	if(!itf || dtype<0 || dtype>=_DTLast)
	{  RefString empty; return(empty);  }
	return(itf->hdpath_job[dtype]);
}

int TaskFile::SetDownload(int s)
{
	if(!itf)
	{  return(-2);  }
	
	if(s==1)
	{  itf->downloading=1;  }
	if(s==2)
	{
		itf->downloading=0;
		itf->was_downloaded=1;
	}
	else
	{  return(-3);  }
	
	return(0);
}

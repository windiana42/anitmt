/*
 * tfmanager.cpp
 * 
 * Task file manager class; knowing any file used by rendview. 
 * 
 * Copyright (c) 2002 -- 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#include "taskfile.hpp"

#include <assert.h>


#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif



// Static data setup: 
TaskFileManager *TaskFileNamespace::_tfmanager_ptr=NULL;


inline int TaskFileManager::_DoHash(const RefString *hdpath) const
{
	int accu=0;
	for(const unsigned char *c=(const unsigned char*)hdpath->str(); *c; c++)
	{  accu+=int(*c);  }
	return(accu % tfhash_size);
}


InternalTaskFile *TaskFileManager::_FindTaskFile(const RefString *hdpath)
{
	if(!hdpath)  return(NULL);
	LinkedList<InternalTaskFile> *head=&(tfhash[_DoHash(hdpath)]);
	for(InternalTaskFile *i=head->first(); i; i=i->next)
	{  if(i->hdpath==*hdpath)  return(i);  }
	return(NULL);
}


void TaskFileManager::_InsertTaskFile(InternalTaskFile *itf)
{
	if(!itf)  return;
	LinkedList<InternalTaskFile> *head=&(tfhash[_DoHash(&itf->hdpath)]);
	assert(!itf->next && head->last()!=itf);  // assert !queued
	head->insert(itf);
}

InternalTaskFile *TaskFileManager::_RemoveTaskFile(InternalTaskFile *itf)
{
	if(!itf)  return(NULL);
	LinkedList<InternalTaskFile> *head=&(tfhash[_DoHash(&itf->hdpath)]);
	assert(itf->next || head->last()==itf);  // assert !queued
	head->dequeue(itf);
	return(itf);
}



TaskFile TaskFileManager::GetTaskFile(RefString hdpath,
	FType ft,IOType iot,FCreator fc)
{
	if(!hdpath || !ft || !iot)
	{  TaskFile rtf;  return(rtf);  }
	
	InternalTaskFile *itf=_FindTaskFile(&hdpath);
	if(itf)
	{
		// Check type: 
		do {
			if(itf->ftype==ft && itf->iotype==iot)  break;
			if(itf->ftype==ft && itf->iotype==IOTRenderOutput && iot==IOTFilterInput)
			{  itf->iotype=IOTFilterInput;  break;  }
			Error("TF: File[%d] \"%s\" (%s/%s) re-registered as %s/%s. "
				"Expect trouble.\n",itf->refcnt,hdpath.str(),
				FTypeString(itf->ftype),IOTypeString(itf->iotype),
				FTypeString(ft),IOTypeString(iot));
		} while(0);
		return(TaskFile(itf));
	}
	
	itf=NEW<InternalTaskFile>();
	itf->ftype=ft;
	itf->iotype=iot;
	itf->fcreator=fc;
	itf->hdpath=hdpath;
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
		DeleteFile(&itf->hdpath,/*may_not_exist=*/1,
			/*error_prefix=*/FCreatorString(itf->fcreator,/*with_colon=*/1));
		itf->delspec=DontDel;
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
	if(!itf || !itf->hdpath)  return(NULL);
	const char *c=itf->hdpath.str();
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
	return(GetFileLength(itf->hdpath.str(),mtime));
}


int TaskFileManager::SetFixedState(InternalTaskFile *itf,
	int64_t size,HTime *mtime,bool dld_ing)
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
	itf->was_downloaded=itf->downloading;
	itf->downloading=0;
	itf->fixed_state=0;
	return(0);
}


int TaskFileManager::SetIncompleteRename(InternalTaskFile *itf,
	RefString *new_name,bool may_not_exist,const char *error_prefix)
{
	if(!itf)  return(-2);
	itf->is_incomplete=1;
	int rv=RenameFile(&itf->hdpath,new_name,may_not_exist,error_prefix);
	if(rv<0)  return(-3);
	// No failure. Change hdpath. 
	_RemoveTaskFile(itf);
	itf->hdpath=*new_name;
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
			assert(tf->refcnt==0);
			
			// This already writes an error. 
			DeleteFile(&tf->hdpath,/*may_not_exist=*/1,
				/*error_prefix=*/FCreatorString(tf->fcreator,/*with_colon=*/1));
			tf->delspec=DontDel;
			assert(tf->fcreator>=0 && tf->fcreator<_FCLast);
			++ndel[tf->fcreator];
			
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
	
	return(0);
}


TaskFileManager::TaskFileManager(int *failflag)
{
	int failed=0;
	
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
				tfhash[i].first()->hdpath.str());
			delete tfhash[i].popfirst();
		}
	}
	
	tfhash=DELarray(tfhash);
	tfhash_size=0;
	assert(TaskFileNamespace::_tfmanager_ptr==this);
	TaskFileNamespace::_tfmanager_ptr=NULL;
}


/******************************************************************************/

InternalTaskFile::InternalTaskFile(int *failflag) : 
	LinkedListBase<InternalTaskFile>(),
	hdpath(failflag),
	fix_mtime(HTime::Invalid)
{
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
}

InternalTaskFile::~InternalTaskFile()
{
	// Nothing to do. 
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

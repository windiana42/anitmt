/*
 * file.cpp
 * Some routines dealing with file attribs. 
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


#include "prototypes.hpp"

#include <hlib/refstring.h>
#include <hlib/htime.h>

#include <limits.h>

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

/*#if HAVE_FCNTL_H*/
# include <fcntl.h>
/*#endif*/


// Get current working directory: 
// Return value: 
//   0 -> OK
//  -1 -> alloc failure
//  -2 -> getcwd() failed: see errno
//  -3 -> path too long
int GetCurrentWorkingDir(RefString *dest)
{
	dest->deref();
	for(size_t len=32; len<=PATH_MAX; len*=2)
	{
		char *buf=(char*)LMalloc(len);
		if(!buf)  return(-1);
		if(getcwd(buf,len))
		{
			int rv=dest->set(buf);
			LFree(buf);
			return(rv ? -1 : 0);
		}
		int errval=errno;
		LFree(buf);
		if(errval!=ERANGE)
		{
			if(dest->sprintf(0,"[getcwd: %s]",strerror(errno)))
			{  return(-1);  }
			errno=errval;
			return(-2);
		}
	}
	if(dest->sprintf(0,"[name too long (>%u)?!]",(unsigned)PATH_MAX))
	{  return(-1);  }
	return(-3);
}


// Check if a file exists and we have the passed permissions: 
int CheckExistFile(RefString *f,int want_read,int want_write)
{
	if(!f->str())
	{  return(0);  }
	int af=0;
	if(want_read)  af|=R_OK;
	if(want_write)  af|=W_OK;
	int rv;
	do
	{  errno=0;  rv=access(f->str(),af);  }
	while(rv<0 && errno==EINTR);
	Verbose(DBGV,"Accessing %s (%d): %s\n",f->str(),af,strerror(errno));
	return(!rv);
}


// Get file length and also modification time if non-NULL. 
// Returns -1 if stat fails. 
int64_t GetFileLength(const char *path,HTime *mtime)
{
	struct stat st;
	Verbose(DBGV,"Stating %s: ",path);
	retry:;
	errno=0;
	if(::stat(path,&st)<0)
	{
		if(errno==EINTR)  goto retry;
		Verbose(DBGV,"failed: %s\n",strerror(errno));
		if(mtime)
		{  mtime->SetInvalid();  }
		return(-1);
	}
	if(mtime)
	{  mtime->SetTimeT(st.st_mtime);  }
	Verbose(DBGV,"size=%lld%s%s\n",
		(long long)(st.st_size),
		mtime ? ", mtime=" : "",
		mtime ? mtime->PrintTime() : "");
	
	// This is for testing purposes only: 
	/*static int wait=0;
	if(++wait>5)
	{
		fprintf(stderr,"NOTE: RETURNING WRONG SIZE****\n");
		int rv=rand()%128;
		if(st.st_size>rv)  st.st_size+=rv;
	}*/
	
	return(st.st_size);
}


// Check if the first file is newer than the second one 
// (both should exist)
// Return value: 
//  1 -> first is newer
//  0 -> first is not newer
// -1 -> stat failed
// error_prefix is put before error message; use e.g. "Local: ". 
// save_mtime_a, if non-NULL stores a's mtime (or invalid if !existing). 
int FirstFileIsNewer(RefString *a,RefString *b,const char *error_prefix,
	HTime *save_mtime_a)
{
	if(save_mtime_a)
	{  save_mtime_a->SetInvalid();  }
	
	const char *sfile=a->str();
	do {
		struct stat a_st,b_st;
		int rv;
		do
		{  errno=0;  rv=stat(sfile,&a_st);  }
		while(rv<0 && errno==EINTR);
		if(rv<0)  break;;
		if(save_mtime_a)
		{  save_mtime_a->SetTimeT(a_st.st_mtime);  }
		
		sfile=b->str();
		do
		{  errno=0;  rv=stat(sfile,&b_st);  }
		while(rv<0 && errno==EINTR);
		if(rv<0)  break;
		
		rv=((a_st.st_mtime>b_st.st_mtime) ? 1 : 0);
		Verbose(DBGV,"Stating: %s is %s than %s.\n",
			a->str(),rv ? "newer" : "older",b->str());
		
		return(rv);
	} while(0);
	// ...or should this be a warning?
	Error("%sstat() failed on \"%s\": %s\n",error_prefix,
		sfile,strerror(errno));
	return(-1);
}


// Simply delete a file (but not verbosely). 
// error_prefix: is put before error message; use e.g. "Local: ". 
// If may_not_exist is set, no error occurs if the file does not exist. 
// Return value: 0 -> OK; -1 -> error; 1 -> ENOENT && may_not_exist
int DeleteFile(RefString *path,int may_not_exist,const char *error_prefix)
{
	if(!path->str())  return(-1);
	int rv;
	do
	{  errno=0;  rv=unlink(path->str());  }
	while(rv<0 && errno==EINTR);
	Verbose(DBGV,"Deleting file \"%s\": %s\n",path->str(),strerror(errno));
	if(!rv)  return(0);
	if(errno==ENOENT && may_not_exist)  return(1);
	Error("%sFailed to unlink \"%s\": %s\n",error_prefix,
		path->str(),strerror(errno));
	return(-1);
}


// Rename a file; error is written if that fails. 
// Set error_prefix to disable error. 
// Return val: <0 -> error; 0 -> okay; 1 -> ENOENT && may_not_exist
int _RenameFile(const char *old_name,const char *new_name,
	int may_not_exist,const char *error_prefix)
{
	if(!old_name || !new_name)  return(-1);
	int rv;
	do
	{  errno=0;  rv=rename(old_name,new_name);  }
	while(rv<0 && errno==EINTR);
	Verbose(DBGV,"Renaming file \"%s\" -> \"%s\": %s\n",
		old_name,new_name,strerror(errno));
	if(!rv)  return(0);
	if(errno==ENOENT && may_not_exist)  return(1);
	if(error_prefix)
	{  Error("%sFailed to rename \"%s\" -> \"%s\": %s\n",error_prefix,
		old_name,new_name,strerror(errno));  }
	return(-1);
}
int RenameFile(const RefString *old_name,const RefString *new_name,
	int may_not_exist,const char *error_prefix)
{
	return(_RenameFile(old_name->str(),new_name->str(),
		may_not_exist,error_prefix));
}


// Open input/output file. 
// dir: direction: -1 -> input; +1 -> output 
// Return value: 
//    >=0 -> valid FD
//     -1 -> file or file->str() NULL or dir==0
//     -2 -> open( failed (see errno)
int OpenIOFile(RefString *file,int dir)
{
	if(!file || !file->str())
	{  return(-1);  }
	
	int fd=-1;
	do {
		errno=0;
		if(dir<0)  // Open for input: 
		{  fd=open(file->str(),O_RDONLY);  }
		else if(dir>0)  // Open for output: 
		{  fd=open(file->str(),O_WRONLY | O_CREAT | O_TRUNC,0666);  }
		else
		{  return(-1);  }
	} while(fd<0 && errno==EINTR);
	
	Verbose(DBGV,"Opening %s for %s: %s\n",
		file->str(),dir<0 ? "reading" : "writing",strerror(errno));
	return(fd<0 ? (-2) : fd);
}



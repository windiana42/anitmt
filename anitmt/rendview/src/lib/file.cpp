/*
 * file.cpp
 * Some routines dealing with file attribs. 
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


#include "prototypes.hpp"

#include <hlib/refstring.h>
#include <hlib/htime.h>

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

/*#if HAVE_FCNTL_H*/
# include <fcntl.h>
/*#endif*/


// Check if a file exists and we have the passed permissions: 
int CheckExistFile(RefString *f,int want_read,int want_write)
{
	if(!f->str())
	{  return(0);  }
	int af=0;
	if(want_read)  af|=R_OK;
	if(want_write)  af|=W_OK;
	errno=0;
	int rv=!access(f->str(),af);
	Verbose(DBGV,"Accessing %s (%d): %s\n",f->str(),af,strerror(errno));
	return(rv);
}


// Get file length and also modification time if non-NULL. 
// Returns -1 if stat fails. 
int64_t GetFileLength(const char *path,HTime *mtime)
{
	struct stat st;
	Verbose(DBGV,"Stating %s: ",path);
	if(::stat(path,&st)<0)
	{
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
	return(st.st_size);
}


// Check if the first file is newer than the second one 
// (both should exist)
// Return value: 
//  1 -> first is newer
//  0 -> first is not newer
// -1 -> stat failed
// error_prefix is put before error message; use e.g. "Local: ". 
int FirstFileIsNewer(RefString *a,RefString *b,const char *error_prefix)
{
	struct stat a_st,b_st;
	const char *sfile=a->str();
	if(stat(sfile,&a_st))  goto error;
	sfile=b->str();
	if(stat(sfile,&b_st))  goto error;
	{ int rv=((a_st.st_mtime>b_st.st_mtime) ? 1 : 0);
	Verbose(DBGV,"Stating: %s is %s than %s.\n",
		a->str(),rv ? "newer" : "older",b->str());
	return(rv); }
error:;
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
	errno=0;
	int rv=unlink(path->str());
	Verbose(DBGV,"Deleting file \"%s\": %s\n",path->str(),strerror(errno));
	if(!rv)  return(0);
	if(errno==ENOENT && may_not_exist)  return(1);
	Error("%sFailed to unlink \"%s\": %s\n",error_prefix,
		path->str(),strerror(errno));
	return(-1);
}


// Rename a file; error is written if that fails. 
// Return val: <0 -> error; 0 -> okay; 1 -> ENOENT && may_not_exist
int RenameFile(RefString *old_name,RefString *new_name,
	int may_not_exist,const char *error_prefix)
{
	if(!old_name->str() || !new_name->str())  return(-1);
	errno=0;
	int rv=rename(old_name->str(),new_name->str());
	Verbose(DBGV,"Renaming file \"%s\" -> \"%s\": %s\n",
		old_name->str(),new_name->str(),strerror(errno));
	if(!rv)  return(0);
	if(errno==ENOENT && may_not_exist)  return(1);
	Error("%sFailed to rename \"%s\" -> \"%s\": %s\n",error_prefix,
		old_name->str(),new_name->str(),strerror(errno));
	return(-1);
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
	
	errno=0;
	int fd=-1;
	if(dir<0)  // Open for input: 
	{  fd=open(file->str(),O_RDONLY);  }
	else if(dir>0)  // Open for output: 
	{  fd=open(file->str(),O_WRONLY | O_CREAT | O_TRUNC,0666);  }
	else
	{  return(-1);  }
	
	Verbose(DBGV,"Opening %s for %s: %s\n",
		file->str(),dir<0 ? "reading" : "writing",strerror(errno));
	return(fd<0 ? (-2) : fd);
}



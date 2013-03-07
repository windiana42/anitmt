/*
 * alloctracer.cc
 * 
 * A very quick-and-dirty implementation of an allocation tracker which 
 * acts as a pipe for program output of HLIB's allocation routines 
 * (stdout/stderr). The alloc strings are filtered out, the rest is passed. 
 * The alloc strings are interpreted and checked for duplicate free, 
 * freeing not allocated memory, etc. 
 * (You need to use hlib_allocdebug_trace and AllocDebugging, see 
 * limitmalloc.c for this to work.) 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

// gcc -Os -fno-rtti -fno-exceptions alloctracer.cc -o alloctracer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>


struct MemChunk
{
	void *ptr;
	size_t size;
	size_t seq;
};


// Well -- this should be a hash. I'm using an array for now. 
MemChunk **hash=NULL;
size_t arr_size=0;
size_t arr_use=0;

size_t expect_seq=0;

void AddToHash(MemChunk *c)
{
	if(arr_use>=arr_size)
	{
		arr_size+=256;
		hash=(MemChunk**)realloc(hash,arr_size*sizeof(MemChunk*));
		assert(hash);
	}
	
	hash[arr_use++]=c;
}

MemChunk *FindInHash(void *ptr)
{
	for(size_t i=0; i<arr_use; i++)
	{
		if(hash[i]->ptr==ptr)
		{  return(hash[i]);  }
	}
	return(NULL);
}

MemChunk *FindRemoveHash(void *ptr)
{
	for(size_t i=0; i<arr_use; i++)
	{
		if(hash[i]->ptr==ptr)
		{
			MemChunk *c=hash[i];
			memmove(&hash[i],&hash[i+1],(arr_use-i-1)*sizeof(MemChunk*));
			--arr_use;
			return(c);
		}
	}
	return(NULL);
}

void CheckLeftInHash()
{
	for(size_t i=0; i<arr_use; i++)
	{
		fprintf(stderr,"OOPS: not freed: %p (%u bytes) [seq=%u]\n",
			hash[i]->ptr,hash[i]->size,hash[i]->seq);
	}
}


void _CheckExpectSeq(size_t seq)
{
	if(seq!=expect_seq)
	{
		fprintf(stderr,"*** Read seq=%u, expected=%u. OOPS, left out message.\n",
			seq,expect_seq);
		expect_seq=seq;
	}
	++expect_seq;
}


// Return value: 
//   1 -> no alloc message (ignored)
int ScanAllocMessage(char *str,char *end)
{
	*end='\0';
	if(strncmp(str,"@HLAT->",7))
	{  *end='\n';  return(1);  }
	//fprintf(stderr,"SCAN=<%s>\n",str);
	str+=7;
	if(!strncmp(str,"malloc[",7))
	{
		// Parse in -- no checks: 
		str+=7;
		MemChunk *chk=(MemChunk*)malloc(sizeof(MemChunk));
		assert(chk);
		chk->size=0;
		chk->ptr=NULL;
		sscanf(str,"%u%*c%*c%u%*c%*c%p",&chk->seq,&chk->size,&chk->ptr);
		_CheckExpectSeq(chk->seq);
		if(!chk->ptr)
		{  free(chk);  }
		else
		{
			//fprintf(stderr,"malloc(%u)=%p\n",chk->size,chk->ptr);
			
			MemChunk *oops=FindInHash(chk->ptr);
			if(oops)
			{
				assert(oops->ptr==chk->ptr);
				fprintf(stderr,"OOPS: malloc: chunk %p already allocated "
					"with size %u using seq=%u [seq=%u]\n",
					oops->ptr,oops->size,oops->seq,chk->seq);
				fprintf(stderr,"oops=%p, chk=%p, size=%u,%u, hash: %u/%u\n",
					oops,chk,oops->size,chk->size,arr_use,arr_size);
				free(chk);
				// Cannot go on very well in such situation. 
				exit(1);
			}
			
			AddToHash(chk);
		}
	}
	else if(!strncmp(str,"free[",5))
	{
		// Parse in -- no checks: 
		str+=5;
		void *ptr=NULL;
		size_t seq=0;
		sscanf(str,"%u%*c%*c%p",&seq,&ptr);
		_CheckExpectSeq(seq);
		if(ptr)
		{
			//fprintf(stderr,"free(%p)\n",ptr);
			MemChunk *chk=FindRemoveHash(ptr);
			if(!chk)
			{
				fprintf(stderr,"OOPS: free: chunk %p not allocated "
					"[seq=%u]\n",ptr,seq);
			}
			else
			{  free(chk);  }
		}
	}
	else if(!strncmp(str,"realloc[",8))
	{
		// Parse in -- no checks: 
		str+=8;
		void *optr=NULL;
		void *nptr=NULL;
		size_t nsize=0;
		size_t seq=0;
		sscanf(str,"%u%*c%*c%p%*c%u%*c%*c%p",&seq,&optr,&nsize,&nptr);
		_CheckExpectSeq(seq);
		if(optr && nptr)
		{
			//fprintf(stderr,"realloc(%p,%u)=%p\n",optr,nsize,nptr);
			MemChunk *chk=FindInHash(optr);
			if(!chk)
			{
				fprintf(stderr,"OOPS: realloc: chunk %p not allocated "
					"[seq=%u]\n",optr,seq);
			}
			else
			{
				// Update... for real hash, re-hashing would be needed. 
				chk->ptr=nptr;
				chk->size=nsize;
				//chk->seq=<unmodified>
			}
		}
	}
	
	return(0);
}

// Returns number of bytes to keep at the end of 
// current buffer. 
int ScanData(char *data,size_t size)
{
	//fprintf(stderr,"READ<%.100s>\n",data);
	char *pstart=data;
	char *cend=data+size;
	for(char *c=data; c<cend; c++)
	{
		if(*c!='@')  continue;
		if(c>pstart)
		{  write(1,pstart,c-pstart);  }
		// Possible alloc message. Find end: 
		char *start=c;
		for(; c<cend && *c!='\n'; c++);
		if(c>=cend)
		{
			//fprintf(stderr,"OVERLAP=%d <%.*s>\n",cend-start,cend-start,start);
			return(cend-start);
		}
		if(ScanAllocMessage(start,c))
		{  pstart=start;  }
		else
		{  pstart=c+1;  }
	}
	if(pstart<cend)
	{  write(1,pstart,cend-pstart);  }
	return(0);
}


// Read from stdin. 
size_t ReadData(char *buf,size_t len)
{
	size_t done=0;
	for(;;)
	{
		ssize_t rd;
		do
		{  rd=read(0,buf+done,len-done);  }
		while(rd<0 && errno==EINTR);
		if(rd<0)
		{  fprintf(stderr,"while reading: %s\n",strerror(errno));
			exit(1);  }
		if(!rd)  break;
		done+=rd;
	}
	return(done);
}


int main(int argc,char **arg)
{
	// Smaller buffer sizes give more instant screen update. 
	size_t bufsize=1024;
	char buf[bufsize];
	
	size_t bufuse=0;
	size_t buflen=0;
	for(;;)
	{
		// Read complete buffer: 
		buflen=ReadData(buf+bufuse,bufsize-bufuse);
		if(!buflen) break;
		buflen+=bufuse;
		int snip=ScanData(buf,buflen);
		bufuse=snip;
		assert(bufuse<buflen);
		memmove(buf,buf+buflen-bufuse,bufuse);
	}
	
	CheckLeftInHash();
	
	return(0);
}

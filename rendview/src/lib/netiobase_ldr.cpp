/*
 * netiobase_ldr.cpp
 * 
 * Basic network IO for LDR client and sever). 
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

#include "netiobase_ldr.hpp"

#include "../tdriver/local/tdriver.hpp"

#include <assert.h>


using namespace LDR;


#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif


static const size_t max_ldr_pack_len=65536;

void NetworkIOBase_LDR::LDRStoreTaskExecutionStatus(
	LDR_TaskExecutionStatus *dest,TaskExecutionStatus *src)
{
	if(src->rflags==TTR_Unset)
	{
		dest->ttr=0xffffU;   // "unset"
		dest->signal=htons(0+0x7fff);
		dest->outfile_status=htons(u_int16_t(OFS_Unset+0x7fff/*no U*/));
		dest->starttime=0;
		dest->endtime=0;
		dest->utime=0;
		dest->stime=0;
	}
	else
	{
		dest->ttr=htons(src->rflags);
		dest->signal=htons(u_int16_t(src->signal+0x7fff/*no U*/));
		dest->outfile_status=
			htons(u_int16_t(src->outfile_status+0x7fff/*no U*/));
		HTime2LDRTime(&src->starttime,&dest->starttime);
		HTime2LDRTime(&src->endtime,&dest->endtime);
		HTime2LDRTime(&src->utime,&dest->utime);
		HTime2LDRTime(&src->stime,&dest->stime);
	}
}

// Returns 1 on failure (illegal status)
int NetworkIOBase_LDR::LDRGetTaskExecutionStatus(
	TaskExecutionStatus *dest,LDR_TaskExecutionStatus *src)
{
	int error=0;
	if(src->ttr==0xffffU)
	{  dest->rflags=TTR_Unset;  }
	else
	{
		dest->rflags=(TaskTerminationReason)ntohs(src->ttr);
		// Must have exactly one set of these...
		switch(dest->rflags & TTR_TermMask)
		{
			case TTR_ExecFail:
			case TTR_Exit:
			case TTR_Killed:
			case TTR_Dumped:  break;
			default:  ++error;
		}
		// ...and one of these...
		switch(dest->rflags & TTR_JK_Mask)
		{
			case TTR_JK_NotKilled:
			case TTR_JK_UserIntr:
			case TTR_JK_Timeout:
			case TTR_JK_ServerErr:  break;
			default:  ++error;
		}
		// ...and nothing else: 
		if((dest->rflags & ~(TTR_JK_Mask | TTR_TermMask)))
		{  ++error;  }
	}
	
	dest->signal=int(ntohs(src->signal))-0x7fff;
	
	dest->outfile_status=(OutputFileStatus)
		(int(ntohs(src->outfile_status))-0x7fff);
	if(dest->outfile_status<OFS_Unset || 
	   dest->outfile_status>=_OFS_Last )
	{  ++error;  }
	
	// Probably, more sanity checks on the exec status could 
	// be made: ###FIXME###
	if(dest->rflags==TTR_Unset && 
	   (dest->signal!=0 || dest->outfile_status!=OFS_Unset) )
	{  ++error;  }
	
	LDRTime2HTime(&src->starttime,&dest->starttime);
	LDRTime2HTime(&src->endtime,&dest->endtime);
	LDRTime2HTime(&src->utime,&dest->utime);
	LDRTime2HTime(&src->stime,&dest->stime);
	
	return(error ? 1 : 0);
}


// dir: +1 -> output; -1 -> input; 0 -> both
// Returns NULL ref if not available. 
TaskFile NetworkIOBase_LDR::GetTaskFileByEntryDesc(int dir,
	CompleteTask *ctsk,u_int16_t file_type,u_int16_t file_idx)
{
	switch(file_type)
	{
		case FRFT_RenderIn:
			if(dir<=0 && ctsk->rt)  return(ctsk->rt->infile);
			break;
		case FRFT_RenderOut:
			if(dir>=0 && ctsk->rt)  return(ctsk->rt->outfile);
			if(dir<=0 && ctsk->ft)  return(ctsk->ft->infile);
			break;
		case FRFT_FilterOut:
			if(dir>=0 && ctsk->ft)  return(ctsk->ft->outfile);
			break;
		case FRFT_AddRender:
		case FRFT_AddFilter:
		{
			if(dir>0)  break;
			CompleteTask::AddFiles *af=
				(file_type==FRFT_AddRender) ? &ctsk->radd : &ctsk->fadd;
			int cnt=0;
			for(int i=0; i<af->nfiles; i++)
			{
				if(af->tfile[i].GetSkipFlag())  continue;
				if(cnt==int(file_idx))
				{  return(af->tfile[i]);  }
				++cnt;
			}
		}  break;
		default:  assert(0);
	}
	TaskFile nullref;
	return(nullref);
}


void NetworkIOBase_LDR::LDRStoreFileInfoEntry(LDR::LDRFileInfoEntry *dest,
	TaskFile tf)
{
	assert(!tf.GetSkipFlag());
	
	// Name length without trailing '\0': 
	size_t namelen=strlen(tf.BaseNamePtr());
	assert(namelen<0xffff && namelen>0);  // Name MUST be set (>=1 char). 
	// If this assert fails due to namelen being 0, add a check somewhere 
	// else (near the position where you read in the evil input). 
	
	HTime mtime;
	int64_t size=tf.GetFixedState(&mtime);
	// If this assert fails, either SetFixedState() was not 
	// called or it was called but invalid size was stored. 
	assert(size>=0);
	
	// Note that *dest may not be aligned, so I use a temporary. 
	LDRFileInfoEntry tmp;
	tmp.name_slen=htons(namelen);
	tmp.size=htonll(size);
	HTime2LDRTime(&mtime,&tmp.mtime);
	memcpy(dest,&tmp,sizeof(tmp));
	// Copy file name without trailing '\0'. 
	// tf.BaseNamePtr()!=NULL due to assert above. 
	memcpy(dest->name,tf.BaseNamePtr(),namelen);
}


// Returns updated dest buffer. 
char *NetworkIOBase_LDR::LDRStoreFileInfoEntries(char *destbuf,char *bufend,
	const CompleteTask::AddFiles *ctf)
{
	for(int i=0; i<ctf->nfiles; i++)
	{
		const TaskFile *tf=&(ctf->tfile[i]);
		if(tf->GetSkipFlag())  continue;
		char *nextbuf=destbuf+LDRFileInfoEntrySize(tf);
		assert(nextbuf<=bufend);  // buffer MUST be large enough (use LDRSumFileInfoSize())
		LDRStoreFileInfoEntry((LDRFileInfoEntry*)destbuf,*tf);
		destbuf=nextbuf;
	}
	return(destbuf);
}


// prep_path: path to prepend before file (base) name
// iotype: file IOType (IOTRenderInput or IOTFilterInput)
// Return value in retval: 
//   >0 -> size of this LDRFileInfoEntry
//   -1 -> alloc failure
//   -2 -> illegal file name length entry (0 or too long)
//   -3 -> illegal file name entry (containing forbidden chars 
//         like '\0' and '/')
//   -4 -> illegal file size entry
//   -5 -> buflen < sizeof(LDR::LDRFileInfoEntry)
//   -6 -> TaskFile::SetFixedState() failed
TaskFile NetworkIOBase_LDR::LDRGetFileInfoEntry(ssize_t *retval,
	LDR::LDRFileInfoEntry *_src,size_t buflen,
	RefString *prep_path,TaskFile::IOType iotype)
{
	TaskFile tf;
	if(buflen<sizeof(LDR::LDRFileInfoEntry))
	{  *retval=-5;  return(tf);  }
	
	// Note that *src may not be aligned, so I use a temporary. 
	LDR::LDRFileInfoEntry src;
	memcpy(&src,_src,sizeof(src));
	size_t namelen=ntohs(src.name_slen);
	
	if(!namelen || namelen+sizeof(LDR::LDRFileInfoEntry)>buflen)
	{  *retval=-2;  return(tf);  }
	
	int64_t size=ntohll(src.size);
	if(size<0)
	{  *retval=-4;  return(tf);  }
	
	for(const char *s=(char*)_src->name,*e=s+namelen; s<e; s++)
	{
		if(*s=='\0' || *s=='/')
		{  *retval=-3;  return(tf);  }
	}
	
	// Copy name: 
	RefString name;
	if(name.sprintf(0,"%s%s%.*s",
		prep_path->str() ? prep_path->str() : "",
		(!prep_path->str() || prep_path->str()[0]=='\0' || 
		  prep_path->str()[prep_path->len()-1]=='/') ? "" : "/",
		namelen,(char*)_src->name))
	{  *retval=-1;  return(tf);  }
	
	HTime mtime;
	LDRTime2HTime(&src.mtime,&mtime);
	
	// Store information of the passed packet. 
	// Must make sure that we temporarily use the passed size 
	// instead of the size obtained via stat() [because the file 
	// does not yet exist] etc. 
	
	// FLAT DIR STRUCTURE: hdpath_rv=hdpath_job. 
	tf=TaskFile::GetTaskFile(name,TaskFile::FTAdd,iotype,TaskFile::FCLDR,&name);
	if(!tf)
	{  *retval=-1;  return(tf);  }
	if(tf.SetFixedState(size,&mtime,/*downloading=*/0))
	{  *retval=-6;  return(tf);  }
	
	*retval=namelen+sizeof(LDR::LDRFileInfoEntry);
	return(tf);
}


size_t NetworkIOBase_LDR::LDRSumFileInfoSize(const CompleteTask::AddFiles *caf,
	int *counter)
{
	size_t sum=0;
	for(int i=0; i<caf->nfiles; i++)
	{
		TaskFile *tf=&(caf->tfile[i]);
		if(tf->GetSkipFlag())  continue;
		sum+=LDRFileInfoEntrySize(tf);
		++(*counter);
	}
	return(sum);
}


int NetworkIOBase_LDR::_StartReadingCommandBody(RespBuf *dest,LDRHeader *hdr)
{
	assert(in_active_cmd==Cmd_NoCommand);
	
	// NOTE: hdr in HOST order. 
	size_t len=hdr->length;
	if(len<sizeof(LDRHeader) || len>max_ldr_pack_len)
	{  return(-2);  }
	
	if(_ResizeRespBuf(dest,len))
	{  return(-1);  }
	
	memcpy(dest->data,hdr,sizeof(LDRHeader));
	
	int rv=_FDCopyStartRecvBuf(dest->data+sizeof(LDRHeader),
		len-sizeof(LDRHeader));
	if(!rv)
	{
		dest->content=LDRCommand(hdr->command);
		in_active_cmd=LDRCommand(hdr->command);
	}
	return(rv);
}


int NetworkIOBase_LDR::_FDCopyStartSendBuf(LDRHeader *hdr)
{
	assert(out_active_cmd==Cmd_NoCommand);
	
	// Translate into network order: 
	size_t len=hdr->length;
	LDRCommand cmd=LDRCommand(hdr->command);
	hdr->length=htonl(len);
	hdr->command=htons(cmd);
	
	int rv=NetworkIOBase::_FDCopyStartSendBuf((char*)hdr,len);
	if(!rv)
	{  out_active_cmd=cmd;  }
	return(rv);
}


int NetworkIOBase_LDR::_ResizeRespBuf(NetworkIOBase_LDR::RespBuf *buf,size_t newlen)
{
	// This assertion is for implementation security reason. 
	// If there is a case in which is fails although the code is okay, then 
	// remove it. 
	assert(buf->content==Cmd_NoCommand);
	
	if(buf->alloc_len<newlen || 
	  (newlen*2<buf->alloc_len && buf->alloc_len>2048))
	{
		char *oldval=buf->data;
		buf->data=(char*)LRealloc(buf->data,newlen);
		if(!buf->data)
		{
			LFree(oldval);
			buf->alloc_len=0;
			return(-1);
		}
		buf->alloc_len=newlen;
	}
	return(0);
}


NetworkIOBase_LDR::NetworkIOBase_LDR(int *failflag) : 
	NetworkIOBase(failflag)
{
	int failed=0;
	
	send_buf.alloc_len=0;
	send_buf.data=NULL;
	send_buf.content=Cmd_NoCommand;
	recv_buf.alloc_len=0;
	recv_buf.data=NULL;
	recv_buf.content=Cmd_NoCommand;
	
	out_active_cmd=Cmd_NoCommand;
	in_active_cmd=Cmd_NoCommand;
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("NetworkIOBase_LDR");  }
}

NetworkIOBase_LDR::~NetworkIOBase_LDR()
{
	send_buf.data=(char*)LFree(send_buf.data);
	recv_buf.data=(char*)LFree(recv_buf.data);
}

/*
 * netiobase_ldr.cpp
 * 
 * Basic network IO for LDR client and sever). 
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

#include "netiobase_ldr.hpp"

#include <assert.h>


using namespace LDR;


#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif


static const size_t max_ldr_pack_len=65536;


// dir: +1 -> output; -1 -> input; 0 -> both
// Returns NULL if not available. 
TaskFile *NetworkIOBase_LDR::GetTaskFileByEntryDesc(int dir,
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
			if(dir<=0 && int(file_idx)<ctsk->radd.nfiles)
				return(ctsk->radd.file[file_idx]);
			break;
		case FRFT_AddFilter:
			if(dir<=0 && int(file_idx)<ctsk->fadd.nfiles)
				return(ctsk->fadd.file[file_idx]);
			break;
		default:  assert(0);
	}
	return(NULL);
}


int NetworkIOBase_LDR::LDRStoreFileInfoEntry(LDR::LDRFileInfoEntry *dest,
	const TaskFile *tf)
{
	fprintf(stderr,"Implement me.\n");
	// BE CAREFUL! *dest is NOT aligned. 
	assert(0);
	return(-177);
}


int NetworkIOBase_LDR::LDRStoreFileInfoEntries(char *destbuf,char *bufend,
	const CompleteTask::AddFiles *ctf,int *err_elem)
{
	for(int i=0; i<ctf->nfiles; i++)
	{
		const TaskFile *tf=ctf->file[i];
		char *nextbuf=destbuf+LDRFileInfoEntrySize(tf);
		assert(nextbuf<=bufend);  // buffer MUST be large enough (use LDRSumFileInfoSize())
		int rv=LDRStoreFileInfoEntry((LDRFileInfoEntry*)destbuf,tf);
		if(rv)
		{  *err_elem=i; return(rv);  }
		destbuf=nextbuf;
	}
	return(0);
}


int NetworkIOBase_LDR::LDRGetFileInfoEntry(TaskFile *tf,
	LDR::LDRFileInfoEntry *src)
{
	fprintf(stderr,"Implement me.\n");
	// BE CAREFUL! *src is NOT aligned. 
	assert(0);
	return(-177);
}


size_t NetworkIOBase_LDR::LDRSumFileInfoSize(const CompleteTask::AddFiles *caf)
{
	size_t sum=0;
	for(int i=0; i<caf->nfiles; i++)
	{  sum+=LDRFileInfoEntrySize(caf->file[i]);  }
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
			return(1);
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

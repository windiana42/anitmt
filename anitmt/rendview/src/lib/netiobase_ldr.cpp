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
	
	if(buf->alloc_len<newlen || newlen*2<buf->alloc_len)
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

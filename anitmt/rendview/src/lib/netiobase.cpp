/*
 * netiobase.cpp
 * 
 * Network IO (e.g. for LDR client and sever) as a layer on top of
 * FDCopyBase. 
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


#include "netiobase.hpp"

#include <ctype.h>
#include <assert.h>


#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif


void NetworkIOBase::_DoChangeEvents_Error(int rv)
{
	fprintf(stderr,"OOPS: FDChangeEvents(sock=%d) returned %d\n",
		sock_fd,rv);
}


char *NetworkIOBase::CopyStrList2Data(char *dest,RefStrList *l)
{
	for(const RefStrList::Node *i=l->first(); i; i=i->next)
	{
		const char *s=i->str();
		while( (*(dest++)=*(s++)) );  // The routine that made C famous - ;)
	}
	return(dest);
}


char *NetworkIOBase::CopyData2StrList(RefStrList *dest,const char *data,
	int n,size_t data_len)
{
	const char *dataend=data+data_len;
	while(n>0)
	{
		for(const char *str=data; data<dataend; data++)
		{
			if(*data=='\0')
			{
				++data;
				if(dest->append(str))
				{  return(NULL);  }
				--n;
				goto done;
			}
		}
		// Reached data end. 
		return((char*)(dataend-data_len-1));  // -> original data[-1]
		done:;
	}
	return((char*)data);
}

int NetworkIOBase::CopyData2StrList(RefStrList *dest,const char *data,
	size_t data_len)
{
	if(data_len && data[data_len-1]!='\0')
	{  return(-2);  }
	
	const char *dataend=data+data_len;
	const char *str=data;
	int n=0;
	for(const char *c=data; c<dataend; c++)
	{
		if(*c=='\0')
		{
			if(dest->append(str))
			{  return(-1);  }
			str=c+1;
			++n;
		}
	}
	
	return(n);
}


// Start sending passed buffer using FDCopyBase etc. 
// Retval: 
//   0 -> OK
//  -1 -> (alloc) failure
int NetworkIOBase::_FDCopyStartSendBuf(char *buf,size_t len)
{
	assert(out.ioaction==IOA_None);
	
	out.io_buf->buf=buf;
	out.io_buf->buflen=len;
	out.io_buf->bufdone=0;
	
	assert(out.io_sock->pollid==pollid);
	
	int rv=out.pump_s->SetIO(out.io_buf,out.io_sock);
	if(rv!=0 && rv!=-1)
	{
		fprintf(stderr,"OOPS: out pump->SetIO() returned %d\n",rv);
		abort();
	}
	if(rv)  return(rv);
	
	rv=out.pump_s->Control(FDCopyPump::CC_Start);
	if(rv!=0)
	{
		fprintf(stderr,"OOPS: out pump->CC_Start returned %d\n",rv);
		abort();
	}
	
	out.ioaction=IOA_SendingBuf;
	
	return(0);
}

// See _FDCopyStartSendBuf()
int NetworkIOBase::_FDCopyStartRecvBuf(char *buf,size_t len)
{
	assert(in.ioaction==IOA_None);
	
	in.io_buf->buf=buf;
	in.io_buf->buflen=len;
	in.io_buf->bufdone=0;
	
	assert(in.io_sock->pollid==pollid);
	
	int rv=in.pump_s->SetIO(in.io_sock,in.io_buf);
	if(rv!=0 && rv!=-1)
	{
		fprintf(stderr,"OOPS: in pump->SetIO() returned %d\n",rv);
		abort();
	}
	if(rv)  return(rv);
	
	rv=in.pump_s->Control(FDCopyPump::CC_Start);
	if(rv!=0)
	{
		fprintf(stderr,"OOPS: in pump->CC_Start returned %d\n",rv);
		abort();
	}
	
	in.ioaction=IOA_SendingBuf;
	
	return(0);
}


// Return value: 0 -> OK; -1 -> alloc failure 
int NetworkIOBase::_Construct_FDCopy(NetworkIOBase::FDCopy *fdc)
{
	fdc->ioaction=IOA_Locked;
	fdc->io_sock=NULL;
	fdc->io_buf=NULL;
	fdc->io_fd=NULL;
	fdc->pump_s=NULL;
	fdc->pump_fd=NULL;
	do {
		fdc->io_sock=NEW<FDCopyIO_FD>();
		if(!fdc->io_sock)  break;
		fdc->io_buf=NEW<FDCopyIO_Buf>();
		if(!fdc->io_buf)  break;
		fdc->io_fd=NEW<FDCopyIO_FD>();
		if(!fdc->io_fd)  break;
		fdc->pump_s=NEW1<FDCopyPump_Simple>(this);
		if(!fdc->pump_s)  break;
		fdc->pump_fd=NEW1<FDCopyPump_FD2FD>(this);
		if(!fdc->pump_fd)  break;
		goto okay;
	} while(0);
	return(-1);
okay:;
	fdc->io_sock->persistent=1;
	fdc->io_buf->persistent=1;
	fdc->io_fd->persistent=1;
	fdc->pump_s->persistent=1;
	fdc->pump_fd->persistent=1;
	return(0);
}

void NetworkIOBase::_Destroy_FDCopy(NetworkIOBase::FDCopy *fdc)
{
	// This is smart becuase it can handle NULL correctly and sets 
	// the passed parameter to NULL (takes ref to ptr). 
	DELETE(fdc->io_sock);
	DELETE(fdc->io_buf);
	DELETE(fdc->io_fd);
	DELETE(fdc->pump_s);
	DELETE(fdc->pump_fd);
}


NetworkIOBase::NetworkIOBase(int *failflag) : 
	FDCopyBase(failflag)
{
	int failed=0;
	
	sock_fd=-1;
	pollid=NULL;
	
	if(_Construct_FDCopy(&in))   ++failed;
	if(_Construct_FDCopy(&out))  ++failed;
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("NetworkIOBase");  }
}

NetworkIOBase::~NetworkIOBase()
{
	ShutdownFD(pollid);  // be sure...
	
	_Destroy_FDCopy(&in);
	_Destroy_FDCopy(&out);
}
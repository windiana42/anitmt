/*
 * cmdqueue.hpp
 * 
 * Command queue class for network pipe with send & response 
 * command queues. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#include "cmdqueue.hpp"

#include <assert.h>

#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif


#if TESTING
#  define CHECK_QUEUE_ENTS \
	assert(queue_waitresp.count()==nentries_waitresp); \
	assert(queue_to_send.count()==nentries_to_send); 
#else
#  define CHECK_QUEUE_ENTS
#endif


void CommandQueue::EntryProcessed(CQEntry *ent,HTime *sendtime)
{
	if(!ent)  return;
	
	// Change queues: 
	if(queue_side>0)
	{
		queue_to_send.dequeue(ent);  --nentries_to_send;
		queue_waitresp.append(ent);  ++nentries_waitresp;
	}
	else // queue_side<0
	{
		queue_waitresp.dequeue(ent);  --nentries_waitresp;
		queue_to_send.append(ent);    ++nentries_to_send;
	}
	
	// Set time: 
	if(sendtime)
	{  ent->sendtime=*sendtime;  }
	//else  <-- do NOT set the time. 
	//{  ent->sendtime=HTime::Curr;  }
	
	CHECK_QUEUE_ENTS
}


int CommandQueue::RespRemoveEntry(u_int16_t cmd,u_int16_t seq)
{
	assert(queue_side>0);  // May not be called otherwise; internal bug. 
	
	// First entry is normally oldest. We normally match first one. 
	for(CQEntry *i=queue_waitresp.first(); i; i=i->next)
	{
		if(i->cmd!=cmd || i->seq!=seq)  continue;
		int retval = (i==queue_waitresp.first()) ? 0 : 1;
		delete queue_waitresp.dequeue(i);
		--nentries_waitresp;
		CHECK_QUEUE_ENTS
		return(retval);
	}
	return(-2);
}

void CommandQueue::DoneRemoveEntry(CQEntry *ent)
{
	assert(queue_side<0);  // May not be called otherwise; internal bug. 
	
	delete queue_to_send.dequeue(ent);
	--nentries_to_send;
	
	CHECK_QUEUE_ENTS
}


int CommandQueue::AddSendEntry(int pos,u_int16_t cmd,
	size_t datalen,uchar *data)
{
	CHECK_QUEUE_ENTS
	
	if((nentries_to_send+nentries_waitresp)>=max_queue_entries)
	{  return(-2);  }
	CQEntry *ent=CQEntry::Alloc(datalen);
	if(!ent)  return(-1);
	ent->seq=queue_curr_seq++;
	ent->cmd=cmd;
	if(datalen && data)
	{  memcpy(ent->data,data,datalen);  }
	if(pos>=0)
	{  queue_to_send.append(ent);  }
	else
	{  queue_to_send.insert(ent);  }
	++nentries_to_send;
	return(0);
}


int CommandQueue::RespAddEntry(u_int16_t cmd,u_int16_t seq,
	size_t datalen,uchar *data)
{
	CHECK_QUEUE_ENTS
	
	assert(queue_side<0);  // May not be called otherwise; internal bug. 
	
	if((nentries_to_send+nentries_waitresp)>=max_queue_entries)
	{  return(-2);  }
	CQEntry *ent=CQEntry::Alloc(datalen);
	if(!ent)  return(-1);
	ent->seq=seq;
	ent->cmd=cmd;
	if(datalen && data)
	{  memcpy(ent->data,data,datalen);  }
	queue_waitresp.append(ent);
	++nentries_waitresp;
	return(0);
}


int CommandQueue::CheckExpiredEntries(const HTime *mintime,CQEntry **ent)
{
	LinkedList<CQEntry> *queue=((queue_side<0) ? 
		&queue_to_send : &queue_waitresp);
	CQEntry *ent_with_time=NULL;
	// First entry is normally oldest. We normally match first one. 
	for(CQEntry *i=queue->first(); i; i=i->next)
	{
		if(i->sendtime.IsInvalid())  continue;
		if(i->sendtime<(*mintime))
		{
			if(ent)  *ent=i;
			return(2);
		}
		if(!ent_with_time)
		{  ent_with_time=i;  }
	}
	if(ent)  *ent=ent_with_time;
	return(ent_with_time ? 1 : 0);
}


CommandQueue::CommandQueue(int _queue_side,int *failflag) : 
	queue_to_send(failflag),
	queue_waitresp(failflag)
{
	queue_side=_queue_side;
	assert(queue_side!=0);
	
	queue_curr_seq=0;
	
	max_queue_entries=16;
	
	nentries_to_send=0;
	nentries_waitresp=0;
}

CommandQueue::~CommandQueue()
{
	// Make sure we delete all entries in the queues: 
	#if TESTING
	// This may probably happen in normal operation, 
	// but for testing purposes it is goot to know. 
	if(nentries_to_send || nentries_waitresp)
	{  fprintf(stderr,"OOPS: Left nodes in command queue: send: %d, resp: %d\n",
		nentries_to_send,nentries_waitresp);  }
	#endif
	while(!queue_to_send.is_empty())
	{  delete queue_to_send.popfirst();  --nentries_to_send;  }
	while(!queue_waitresp.is_empty())
	{  delete queue_waitresp.popfirst();  --nentries_waitresp;  }
	
	// Bug traps: Counters are corrupt if these fail. 
	assert(!nentries_to_send);
	assert(!nentries_waitresp);
}


/******************************************************************************/


CommandQueue::CQEntry *CommandQueue::CQEntry::Alloc(size_t datalen)
{
	int failflag=0;
	CQEntry *ent=NEWplus<CQEntry>(datalen,&failflag);
	if(failflag)
	{  DELETE(ent);  }  // <-- will set ent=NULL
	if(ent)
	{  ent->datalen=datalen;  }
	return(ent);
}


CommandQueue::CQEntry::CQEntry(int * /*failflag*/) : 
	LinkedListBase<CQEntry>(),
	sendtime(HTime::Invalid)
{
	seq=0;
	cmd=0;
	datalen=0;
}

CommandQueue::CQEntry::~CQEntry()
{
	/* empty */
}


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

#ifndef _RNDV_LIB_CMDQUEUE_HPP_
#define _RNDV_LIB_CMDQUEUE_HPP_ 1

#include <hlib/linkedlist.h>
#include <hlib/htime.h>

typedef unsigned char uchar;


class CommandQueue
{
	public:
		struct CQEntry : LinkedListBase<CQEntry>
		{
			u_int16_t seq;    // sequence number of command
			u_int16_t cmd;    // the commnd code
			HTime sendtime;   // when the command was sent
			size_t datalen;   // length of data
			uchar data[0];    // DATA BEGINS HERE
			
			// Use this to allocate a new data entry. 
			// Sets up retval->datalen. Use delete to delete it. 
			static CQEntry *Alloc(size_t datalen);
			
			_CPP_OPERATORS_FF
			CQEntry(int *failflag=NULL);
			~CQEntry();
		};
		
		// Max number of entries in both queues together. 
		// Must be set by derived class. Defaults to 16. 
		int max_queue_entries;
		
		// Queue of commands to be sent. first() gets sent first. 
		LinkedList<CQEntry> queue_to_send;
		// Queue of sent commands which did not yet get answered. 
		LinkedList<CQEntry> queue_waitresp;
		
	private:
		// Important switch which reverses send/waitresp queue. 
		// +1 -> sender side
		// -1 -> receiver side
		int queue_side;
		
		// Current command sequence number which is assigned 
		// to new commands: 
		u_int16_t queue_curr_seq;
		
		int nentries_to_send;  // number of entries in send queue
		int nentries_waitresp;  // number of entries in waitresp queue
		
	public:  _CPP_OPERATORS_FF
		// See public functions about queue_side. 
		CommandQueue(int queue_side,int *failflag=NULL);
		~CommandQueue();
		
		// Append (pos=+1) or insert (pos=-1) command to send 
		// queue INDEPENDENT of the value of queue_side. 
		// You MUST specify the data length in datalen but 
		// if the data is not yet available, you may pass 
		// data=NULL and copy it later into 
		//  queue_to_send.last()->data
		// Return value: 
		//   0 -> OK
		//  -1 -> alloc failed
		//  -2 -> already max_queue_entries entries in queues
		int AddSendEntry(int pos,u_int16_t cmd,
			size_t datalen=0,uchar *data=NULL);
		
		// MAY ONLY BE CALLED FOR queue_side<0. 
		// Add the specified entry (which was received from the server) 
		// to the waitresp queue. Works like AddSendEntry() just that 
		// you specify the seq here. Same return values: 
		int RespAddEntry(u_int16_t cmd,u_int16_t seq,
			size_t datalen=0,uchar *data=NULL);
		
		// Call this when the passed entry was sent 
		// (queue_side=+1) or processed (queue_side=-1). 
		// This will set the entry's sendtime (use NULL if you do not 
		// want to set the time), and: 
		// queue_side=+1: remove it from the send queue and append 
		//                it to the waitresp queue. 
		// queue_side=-1: remove it from the waitresp queue and 
		//                append it to the send queue. 
		void EntryProcessed(CQEntry *ent,HTime *sendtime=NULL);
		
		// MAY ONLY BE CALLED FOR queue_side>0. 
		// Call this when a response was recived. 
		// This will remove the corresponding entry in the 
		// waitresp queue. 
		// Return value: ("passed commans" means: cmd + seq)
		//   0 -> passed command was the first one in 
		//        waitresp queue; removed, okay
		//   1 -> passed command was at different location 
		//        in the command queue & removed
		//  -2 -> passed command unknown (not in queue)
		int RespRemoveEntry(u_int16_t cmd,u_int16_t seq);
		
		// MAY ONLY BE CALLED FOR queue_side<0. 
		// Call this when the response was sent (on the receiver 
		// side) to have the passed entry removed from the 
		// send (yes, send) queue. 
		void DoneRemoveEntry(CQEntry *ent);
		
		// This can be used to check if entries are older than 
		// the passed HTime. 
		// This only checks entries in queue_waitresp (queue_side=+1) 
		// or queue_to_send (queue_side=-1). 
		// Check: if entry.sendtime<mintime -> fond too old entry
		// Return value: 
		//   0 -> found no old entry and queue empty
		//   1 -> found no old entry but queue not empty, first 
		//        entry in queue returned in *ent if non-NULL
		//   2 -> found old entry in queue, returned in *ent 
		//        if not NULL
		int CheckExpiredEntries(const HTime *mintime,CQEntry **ent=NULL);
};

#endif  /* _RNDV_LIB_CMDQUEUE_HPP_ */

/*
 * cpmanager.h
 * 
 * Header containing class FDCopyManager, a class for 
 * copying from and to file descriptors which works in 
 * cooperation with classes derived from class FDCopyBase. 
 *
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_FDCopyManager_H_
#define _HLIB_FDCopyManager_H_ 1

#include <hlib/prototypes.h>
#include "htime.h"
#include <hlib/linkedlist.h>

class FDCopyBase;

class FDCopyManager : FDBase
{
	public:
		static FDCopyManager *manager;
		
		struct _CopyID {};
		typedef _CopyID (*CopyID);
		typedef off_t copylen_t;
		
		struct MCopyNode;
		
		enum ProgressAction
		{
			PAQuery=0,     // QueryProgress()
			PARead= 0x01,  // read was called
			PAWrite=0x02   // write was called
		};
		
		enum ControlCommand
		{
			CCTerm,
			CCKill,
			CCStop,
			CCCont
		};
		
		struct CopyRequest
		{
			//*** Source and destination: ***
			// Note: You must specify ONE data source and ONE data 
			//       destination. Thus, you may not specify srcfd AND 
			//       srcbuf / destfd AND destbuf. (fds are -1 for 
			//       unset, pointers NULL). 
			// Note also that you MUST supply at least one file 
			//       descriptor (thus, the FDCopyManager cannot be 
			//       used to copy large buffers in memory).
			// Note finally that you may not set srcfd=destfd. 
			int srcfd;       // fd to write data to
			int destfd;      // fd to read data from 
			char *srcbuf;    // where to read data from 
			char *destbuf;   // where to store read in data
			copylen_t len;   // How many bytes to copy max; 0 -> unlimited
			
			//*** Time restrictions: ***
			// Set timeout to -1 for `no timeout'. 
			long req_timeout;    // timeout for the complete request in msec
			long read_timeout;   // read timeout (time since last POLLIN) 
			long write_timeout;  // write timeout
			
			//*** Convenience: ***
			// costom data pointer to attach data to the CopyRequest: 
			void *dptr;
			// OR`ed flags when you would like to have cpnotify(ProgressInfo)
			// called. See ProgressAction above for details. 
			// (Default: PAQuery -> no progress info)
			ProgressAction progress_mask;
			
			//*** More tuning: ***
			size_t iobufsize;  // Size of IO buffer allocated for the copy job 
			// This needs some explanation: The filedescriptor is 
			// polled for reading, if less than low_read_thresh bytes are 
			// in the buffer and is no longer polled for reading if more 
			// than high_read_thresh is in the buffer. 
			// The dest fd is polled for writing if more than 
			// high_write_thresh bytes are in the buffer and not polled 
			// for writing if less than low_write_thresh in the buffer. 
			// You may count on reasonable defaults; 
			// values of -1 mean -> set defaults according to iobufsize. 
			ssize_t low_read_thresh;
			ssize_t high_read_thresh;
			ssize_t low_write_thresh;
			ssize_t high_write_thresh;
			// Max. number of bytes passed to a call to read / write. 
			// (One call is done per fdnotify()). 
			// Set to 0 for `unlimited'. 
			size_t max_read_len;
			size_t max_write_len;
			
			// Error code in case CopyFD() returns NULL. 
			// Values: 
			//   0 -> success (no error)
			//  -1 -> allocation failure
			//  -2 -> call to PollFD() failed. 
			//  -3 -> illegal src/dest spec 
			//   1 -> source or dest is buffer and len=0 
			//        (CopyFD() returns NULL) 
			int errcode;
			
			_CPP_OPERATORS_FF
			CopyRequest(int *failflag=NULL);
			~CopyRequest();
		};
		struct CopyStatistics
		{
			HTime starttime;
			// Number of bytes read/written so far (using read/write 
			// syscalls). 
			copylen_t read_bytes;
			copylen_t written_bytes;
			
			_CPP_OPERATORS_FF
			CopyStatistics(int *failflag=NULL);
			~CopyStatistics()  { }
		};
		
		enum StatusCode
		{
			// Note: that when reaching the copy limit, CopyManager cannot 
			// detect if EOF is reached simultaniously. If SCEOF is not 
			// set and SCLimit is set, you don't know if there is more 
			// data available. 
			// NOTE: BITWISE OR'ED: 
			//       One of the flags and optionally SCFinal is set. 
			//       If reading is done, one of the ECEOI-flags remains set. 
			
			//*** Status codes ***
			SCNone=   0x00,   // (mainly internal use; may never happen)
			SCFinal=  0x01,   // IMPORTANT: last time cpnotify() called 
			SCLimit=  0x02,   // copy limit reached 
			SCTimeout=0x04,   // some CopyRequest::*timeout passed (see errno)
			SCKilled= 0x08,   // CopyControl(CCKill) called
			SCTerm=   0x10,   // CopyControl(CCTerm) called
			// See below for combined flags of these: 
			SCInHup=  0x0100,   // hangup on input fd (POLLHUP)
			SCOutHup= 0x0200,   // hangup on output fd (POLLHUP)
			SCInPipe= 0x0400,   // EPIPE on input fd
			SCOutPipe=0x0800,   // EPIPE on output fd
			SCRead0=  0x1000,   // read 0 bytes although POLLIN (EOF)
			SCWrite0= 0x2000,   // wrote 0 bytes although POLLOUT
			// Combined flags: 
			// ( Use: if(scode & EDEOI) eof_occured(); )
			// EOI = ``end of input'' (could not read more data; EOF)
			SCEOI=(SCInHup|SCInPipe|SCRead0),     // ``end of input'' (EOF)
			// EOO = ``end of output'' (could not write more data)
			//       This can be an error for some applications. 
			SCEOO=(SCOutHup|SCOutPipe|SCWrite0),  // ``end of output''
			
			//*** Error codes ***
			// Note that SCFinal is normally set if an error occurs, 
			// i.e. the job is terminated on error. 
			SCErrPollI=0x10000,   // poll() returns POLLERR on input fd
			SCErrPollO=0x20000,   // poll() returns POLLERR on output fd
			SCErrRead= 0x40000,   // error during call to read()/readv()
			SCErrWrite=0x80000,   // error during call to write()/writev()
			// Combined: matches any error: 
			SCError=(SCErrPollI|SCErrPollO|SCErrRead|SCErrWrite)
		};
		struct CopyInfo
		{
			const CopyRequest *req;   // pointer to copy request 
			const CopyStatistics *stat;  // pointer to statistics
			HTime *fdtime;   // time passed to fdnotify() or NULL
			
			// See above for status code (bitwise OR'ed flags). 
			// IMPORTANT FLAG: Every time cpnotify(CopyInfo*) gets 
			// called, you should check if SCFinal is set. 
			// YES -> This is the last time a cpnotify() is called 
			//        for this copy job. Either it is done or 
			//        terminated by an error. 
			// NO ->  Not the last time cpnotify() gets called. 
			StatusCode scode;
			
			// Check this if(scode & SCError) for errors: 
			// In case of no error err_no is 0. 
			// Note: 
			// - In case of SCInPipe / SCOutPipe err_no=EPIPE. 
			// - In case of SCErrPollI/O, err_no is poll's revents 
			//   which may have POLLERR and POLLNVAL set. 
			// - In case of SCTimeout it is the timeout which 
			//   elapsed: 
			#warning <hack me>
			int err_no;          // errno value 
			
			_CPP_OPERATORS_FF
			CopyInfo(
				const MCopyNode *cpn,
				StatusCode scode,
				int *failflag=NULL);
			~CopyInfo()  {  req=NULL;  }
		};
		
		struct ProgressInfo
		{
			const CopyRequest *req;   // pointer to copy request 
			const CopyStatistics *stat;  // pointer to statistics
			HTime *fdtime;   // time passed to fdnotify() or NULL
			// See CopyInfo for the meaning. 
			// Note that SCFinal is never set in ProgressInfo 
			// (and that ProgressInfo is never called to report an 
			// error); all the rest is the same as in CopyInfo. 
			StatusCode scode;
			// What was done last; think about it as the `reason why 
			// you receive progress info': 
			// PAQuery -> QueryProgress() 
			// PARead -> data was read 
			// PAWrite -> data was written 
			ProgressAction action;
		};
		
		enum CPState
		{
			CPSStopped= 0x01,   // copying stopped (initially)
			CPSFlushing=0x02    // fd->fd copying and eof/limit on input fd
		};
		enum OperationMode
		{
			OM_None=0,
			OM_Buf2Fd,   // buf -> fd
			OM_Fd2Fd,    // fd -> fd 
			OM_Fd2Buf    // fd -> buf 
		};
		
		struct MCopyNode
		{
			FDCopyBase *client;  // client back pointer
			
			// The complete copy request:
			CopyRequest req;
			
			// If FDBase pointer was set: 
			FDBase *fdb;  // FDBase pointer
			enum { OFSrcSet=0x1,OFDestSet=0x2 };
			int orig_flags;  // OR of OFSrcSet, OFDestSet if following is set: 
			short orig_src_events;   // save original fd events 
			short orig_dest_events;  // used by FDBase
			
			// PollID of the source and dest fds. If src or dest is 
			// a buffer, the corresponding PollID is NULL. 
			PollID psrcid,pdestid;
			CPState cpstate;
			short srcfd_ev,destfd_ev;  // current poll events
			StatusCode persistent_sc;  // saves SCLimit/SCInHup/SInPipe/SCRead0/SCSCErrRead...
			OperationMode opmode;
			
			// Actual copy buffer: 
			// NULL for OM_Fd2Buf and OM_Buf2Fd
			size_t iobufsize;  // copy of req.iobufsize (against manipulation by client)
			char *buf;   // size: iobufsize
			// buf end: 
			// OM_Fd2Fd [-> buf!=NULL] -> end of buf
			// OM_Fd2Buf/OM_Buf2Fd -> end of req.srcbuf or req.destbuf
			char *bufend;
			// Head of buffer: 
			// OM_Fd2Fd  -> current head of cyclic io buffer
			// OM_Buf2Fd -> bufheadR read position in src buf 
			// OM_Fd2Buf -> bufheadW write position in dest buf 
			char *bufheadR;  // read data to write from here
			char *bufheadW;  // store read data here
			// Number of valid bytes in buffer starting at bufheadR 
			// (possibly wrapping around at bufend) and ending at
			// bufheadW-1; OR 0 if not OM_Fd2Fd. 
			size_t bufuse;
			
			// Statistics: 
			CopyStatistics stat;
			copylen_t read_bytes;   // mirrored here against manipulation by client
			
			_CPP_OPERATORS_FF
			MCopyNode(int *failflag=NULL);
			~MCopyNode();
		};
	private:
		// List of all FDCopyBase classes: 
		LinkedList<FDCopyBase> clients;
		
		void _ReadError(MCopyNode *cpn,HTime *fdtime);
		
		int _SavePollEvents(MCopyNode *cpn);
		int _RestorePollEvents(MCopyNode *cpn,int only_input_ev=0);
		
		void _KillRequest(MCopyNode *cpn);
		void _FinishInput(MCopyNode *cpn,CopyInfo *cpi);
		void _FinishRequest(MCopyNode *cpn,CopyInfo *cpi);
		
		void _FillProgressInfoStruct(ProgressInfo *pgi,MCopyNode *cpn,
			HTime *fdtime=NULL,ProgressAction act=PAQuery);
		
		// Overriding virtuals: 
		int fdnotify(FDInfo *);
		
	public:  _CPP_OPERATORS_FF
		FDCopyManager(int *failflag=NULL);
		~FDCopyManager();
		
		// Default values: 
		static size_t default_iobufsize;   // default for CopyRequest::iobufsize
		
		// Called by FDCopyBase when constructed:
		// Returns 0 on sucess; !=0 on error. 
		int Register(FDCopyBase *cb);
		
		// Called by FDCopyBase when it gets destroyed: 
		void Unregister(FDCopyBase *cb);
		
		// This is actally the routine for which all the 
		// FDCopyBase / FDCopyManager exist...
		// Tell FDCopyManager to copy something:
		// Returns CopyID of this copy request or NULL. 
		// Check req->errcode for errors.
		// (CopyRequest is copied; just pass pointer to object 
		// on stack.) 
		// Refer to FDCopyBase::CopyFD() for more information. 
		// (fdb may be NULL if not derived from FDBase.)
		CopyID CopyFD(FDCopyBase *client,CopyRequest *req,FDBase *fdb);
		
		// See FDCopyBase for details: 
		int CopyControl(CopyID cpid,ControlCommand cc);
		
		// See FDCopyBase for details: 
		inline int QueryProgress(CopyID cpid,ProgressInfo *save_here)
		{
			if(!cpid)  return(-2);
			if(save_here)  _FillProgressInfoStruct(save_here,(MCopyNode*)cpid);
			return(0);
		}
};

#endif  /* _HLIB_FDCopyManager_H_ */

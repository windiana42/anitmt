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
class FDCopyManager;

// This class is just there to hold some enums and structures. 
struct _FDCopyNamespace
{
	struct _CopyID {};
	typedef _CopyID (*CopyID);
	typedef int64_t copylen_t;
	
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
		CCKillI,CCKillO,
		CCStop,
		CCCont
	};
	
	struct CopyRequest
	{
		//*** Source and destination: ***
		// Note: There are four operation modes: 
		//    FD -> buf: set srcfd and destbuf
		//    buf -> FD: set srcbuf and destfd
		//    FD -> FD:  set srcfd and destfd
		//    FD <--> buf: set srcbuf, srcfd, destbuf, destfd 
		//        This last mode means that srcfd is copied to 
		//        destbuf while srcbuf is copied to destfd. 
		//        In this case srcfd MUST EQUAL destfd. 
		// Note that you MUST supply at least one file 
		//       descriptor (thus, the FDCopyManager cannot be 
		//       used to copy large buffers in memory).
		// Note finally that you may not set srcfd=destfd unless 
		// you use the FD <--> buf opmode (see above). 
		int srcfd;       // fd to write data to
		int destfd;      // fd to read data from 
		char *srcbuf;    // where to read data from 
		char *destbuf;   // where to store read in data
		union {
			copylen_t len;     // How many bytes to copy max; 0 -> unlimited
			copylen_t srclen;  // Size of src buf. 
		};
		copylen_t destlen;     // Size of dest buf. 
		
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
		
		//*** Advanced features: ***
		// In order for this to work, the FDBase pointer must be specified 
		// when calling CopyFD(). See also CopyFD(). Values: 
		//  0 -> Do not call fdnotify() for the passed FDBase. 
		//  1 -> Call fdnotify() for with all those flags set which 
		//       are not controlled by FDCOpyManager (i.e. for 
		//       FD -> buf copying, you will receive all but POLLIN). 
		//  2 -> Call fdnotify() with all flags set. Be careful with 
		//       that. 
		int recv_fdnotify : 3;  // default: 0
		// Padding...
		int : (sizeof(int)*8-3);
		
		//*** More tuning: ***
		// Size of IO buffer allocated for the copy job. 
		// This is set to 0 for fd->buf and buf->fd modes. 
		// This is automatically made smaller if larger than len. 
		size_t iobufsize;
		// This needs some explanation: 
		// The source file descriptor is 
		// * polled for reading, if less than low_read_thresh bytes 
		//   are in the buffer 
		//   (exactly: start reading if bufuse<=low_read_thresh) 
		// * and is no longer polled for reading if more than 
		//   high_read_thresh is in the buffer 
		//   (exactly: stop reading if bufuse>=high_read_thresh). 
		// The dest file descriptor is 
		// * polled for writing if more than high_write_thresh bytes 
		//   are in the buffer 
		//   (exactly: start writing if bufuse>=high_write_thresh) 
		// * and not polled for writing if less than low_write_thresh 
		//   bytes are in the buffer 
		//   (exactly: stop writing if bufuse<=low_write_thresh). 
		// THUS: hard max for thresh is iobufsize, 
		//       hard min for thresh is 0 and
		//       high thresh must be larger than low thresh (NOT equal)
		// You may count on reasonable defaults; 
		// Values of -1 mean -> set defaults according to iobufsize. 
		// No meaning for buf -> fd or fd -> buf modes. 
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
		//   1 -> source or dest is buffer and len=0 
		//        (CopyFD() returns NULL) 
		//   0 -> success (no error)
		//  -1 -> allocation failure
		//  -2 -> call to PollFD() failed. 
		//  -3 -> illegal src/dest spec 
		//  -4 -> no FDBase specified but needed (e.g. recv_fdnotify!=0)
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
		SCTerm=   0x10,   // CopyControl(CCTerm,CCTermI,CCTermO) called
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
		CopyID cpid;    // CopyID in case it is needed...
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
		#warning <hack me> (timeout number 0,1,2)
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
		CopyID cpid;    // CopyID in case it is needed...
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
		OM_Fd2Buf,   // fd -> buf 
		OM_FdBuf2FdBuf  // fd <--> buf
	};
	
	struct MCopyNode : LinkedListBase<MCopyNode>
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
		FDBase::PollID psrcid,pdestid;
		CPState cpstate;
		short srcfd_ev,destfd_ev;  // current poll events
		StatusCode persistent_sc;  // saves SCLimit/SCInHup/SInPipe/SCRead0/SCSCErrRead...
		OperationMode opmode;
		
		// Mask of FD events controlled by the FDCopyManager: 
		short controlled_src_ev,controlled_dest_ev;
		
		int recv_fdnotify : 3;  // see CopyRequest 
		int is_dead : 1;  // shall be deleted
		int : (sizeof(int)*8-4);
		
		// Actual copy buffer: 
		// NULL for OM_Fd2Buf and OM_Buf2Fd
		size_t iobufsize;  // copy of req.iobufsize (against manipulation by client)
		char *buf;   // size: iobufsize
		// buf end: 
		// OM_Fd2Fd [-> buf!=NULL] -> end of buf and NULL
		// OM_Fd2Buf/OM_Buf2Fd -> end of req.srcbuf and req.destbuf
		union {
			char *bufend;
			char *Rbufend;  };
		char *Wbufend;
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
		
		// This is needed to get the initial read/write balancing: 
		// This is (low_write_thresh+high_write_thresh)/2 if needed or 0. 
		size_t write_start;
		
		_CPP_OPERATORS_FF
		MCopyNode(int *failflag=NULL);
		~MCopyNode();
	};
};


/* Now, here comes the REAL class... */
class FDCopyManager : 
	public _FDCopyNamespace, 
	private FDBase
{
	public:
		static FDCopyManager *manager;
		
	private:
		// List of all FDCopyBase classes: 
		LinkedList<FDCopyBase> clients;
		LinkedList<MCopyNode> dead_cpn;
		
		// Which CopyID is just being processed by fdnotify(). 
		// Only set while FDBase::fdnotify() is called for one of the clients. 
		// MUST BE SET EVEN IF current_cpid->is_dead=1. 
		CopyID current_cpid;
		
		// Kill all requests in dead_cpn: 
		inline void FDCopyManager::_TidyUp()
		{
			while(dead_cpn.first())
			{  delete dead_cpn.popfirst();  }
		}
		
		int cpn_locked;
		inline void _LockCPN()  {  cpn_locked=1;  }
		inline void _UnlockCPN()  {  cpn_locked=0;  _TidyUp();  }
		
		inline void _ChangePollFDSrc(MCopyNode *cpn,short set_flags,short clear_flags);
		inline void _ChangePollFDDest(MCopyNode *cpn,short set_flags,short clear_flags);
		inline void _ChangePollFDSD(MCopyNode *cpn,short set_flags,short clear_flags);
		void _SetControlledEvents(MCopyNode *cpn);
		
		void _SendFDNotify(MCopyNode *cpn,FDInfo *_fdi);
		
		int _ReadInData_Buf(MCopyNode *cpn,HTime *fdtime);
		int _WriteOutData_Buf(MCopyNode *cpn,HTime *fdtime);
		int _ReadInData_FD(MCopyNode *cpn,HTime *fdtime);
		int _WriteOutData_FD(MCopyNode *cpn,HTime *fdtime);
		
		int _ReadError(MCopyNode *cpn,HTime *fdtime);
		int _WriteError(MCopyNode *cpn,HTime *fdtime);
		
		void _StartCopyRequest(MCopyNode *cpn);
		void _ContCopyRequest(MCopyNode *cpn);
		void _StopCopyRequest(MCopyNode *cpn);
		
		void _KillRequest(MCopyNode *cpn);
		int  _FinishRequest(MCopyNode *cpn,CopyInfo *cpi,int dir);
		void _MutateRequest(MCopyNode *cpn,CopyInfo *cpi,OperationMode new_opmode);
		
		void _ReDecidePollEvents(MCopyNode *cpn);
		void _SetWriteStart(MCopyNode *cpn);
		
		void _SavePollEvents(MCopyNode *cpn);
		void _RestorePollEvents(MCopyNode *cpn,int dir_only);
		
		void _SendProgressInfo(MCopyNode *cpn,HTime *fdtime,ProgressAction act);
		void _FillProgressInfoStruct(ProgressInfo *pgi,MCopyNode *cpn,
			HTime *fdtime=NULL,ProgressAction act=PAQuery);
		inline void _SendCpNotify(MCopyNode *cpn,ProgressInfo *pgi);
		inline void _SendCpNotify(MCopyNode *cpn,CopyInfo *cpi);
		
		int _SendFDNotify(MCopyNode *cpn,FDInfo *_fdi,int which);
		
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
		
		// See FDCopyBase. 
		inline CopyID CurrentCopyID(int *c_f_cpman)
		{
			if(c_f_cpman)  *c_f_cpman=(current_cpid ? 1 : 0);
			return((!current_cpid || ((MCopyNode*)current_cpid)->is_dead) ? 
				NULL : current_cpid);
		}
		
		// See FDCopyBase for details: 
		int CopyControl(CopyID cpid,ControlCommand cc);
		
		// See FDCopyBase for details: 
		inline int QueryProgress(CopyID cpid,ProgressInfo *save_here)
		{
			if(!cpid || ((MCopyNode*)cpid)->is_dead)  return(-2);
			if(save_here)  _FillProgressInfoStruct(save_here,(MCopyNode*)cpid);
			return(0);
		}
		
		// Advanced features: 
		// You can use this if you set recv_fdnotify at your request to 
		// control those events which are not controlled by FDCopyManager. 
		// set_events: these events (POLLIN,POLLOUT,...) are set...
		// clear_events: ...and these are cleared 
		// dir: <0 -> input FD; >0 -> output FD
		// Return value: 
		//   1 -> no such direction (e,g dir=-1 for Buf2Fd)
		//   0 -> success
		//  -1 -> specified flag controlled by FDCopyManager OR 
		//        recv_fdnotify not set at all
		//  -2 -> cpid NULL or dir=0
		int CPPollFD(CopyID cpid,int dir,
			short set_events,short clear_events);
};

#endif  /* _HLIB_FDCopyManager_H_ */

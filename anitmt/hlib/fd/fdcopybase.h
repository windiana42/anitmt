/*
 * fdcopybase.h
 * 
 * Header containing several classes which can be used to copy 
 * data from / to a file descriptor. 
 * Works in cooperation with class FDManager. 
 * 
 * Copyright (c) 2002--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_FDCopyBase_H_
#define _HLIB_FDCopyBase_H_ 1

// These are required because a class is derived from FDBase: 
#include <hlib/fdmanager.h>
#include <hlib/fdbase.h>
#include <hlib/linkedlist.h>
#include <hlib/fdfifobuf.h>

// Used for file sizes, etc. 
typedef int64_t copylen_t;

class FDCopyIO;
class FDCopyPump;
class FDCopyBase;


// NOTE: OBJECT LIFE CYCLE: 
// ------------------------
// 
// There are persistent and non-persistent FDCopyIO and FDCopyPump objects. 
// Whether an object is persistent or not is chosen by the creator by (you?)
// setting the persistent flag after creation. 
// 
// - The FDCopyBase has a list of all (active and non-active) FDCopyPump 
//   objects (associated with the particular FDCopyBase). 
// - Each FDCopyPump has one source and one dest FDCopyIO. 
//   (Or maybe, your custom one has more than 2 FDCopyIO classes.)
// 
// FDDataPump: 
//   non-persistent ones get destroyed: 
//     - when the associated FDCopyBase gets destroyed
//     - when the copy job it is doing is done (i.e. it exists as long 
//       as it is "active")
//   persistent ones get destroyed:
//     - never; the FDCopyBase destructor destroys them emitting an 
//       ugly warning
// 
// FDCopyIO: 
//   non-persistent ones get destroyed: 
//     - when the associated FDCopyPump gets destroyed
//     - when the FDCopyPump changes the FDCopyIO, then the old ones are 
//       destroyed (unless persistent) 
//   persistent ones get destroyed:
//     - never; the FDCopyPump destructor destroys the associated ones 
//       emitting an ugly warning

// Base class for FDCopyIO_Buf / FDCopyIO_FD used to copy data from 
// or to a buffer / fd. 
// Read more at the apropriate classes below. 
// NOTE: An FDCopyIO class may only be either source or destination 
//       and it may only be passed (active) for ONE FDCopyPump. 
//       If you have two things you want to do with the same fd, 
//       you need TWO FDCopyPumps and TWO FDCopyIO classes (which 
//       both operate on the same PollFD). 
class FDCopyIO
{
	public:
		// NOTE: This type identifies the FDCopyIO(derived) class. 
		//       If you write your own derivates, use a type 
		//       CPT_Custom0 or above. 
		enum CPType
		{
			CPT_None=0,
			CPT_Buf,    // FDCopyIO_Buf
			CPT_FD,     // FDCopyIO_FD
			// For costom use, take numbers >=16384. 
			CPT_Custom0=16384
		};
		
		// Used by dataptr(): 
		struct DataPtr
		{
			char *buf;       // where the data is
			size_t buflen;   // how long the buffer is
			int more;        // 0/1 more data to come?
		};
		struct DataDone
		{
			char *buf;       // the same as in DataPtr
			size_t donelen;  // number of bytes read/written 
			                 // at buf[0]...buf[donelen-1]
		};
	private:
		CPType type;
		
	public:
		// NOTE: These members are public for easy access. 
		// ****  Do not modify them while the copy job is active. 
		
		// Max time which may pass between two calls to read() or two 
		// calls to write(). 
	// NOTE: CURRENTLY NOT IMPLEMENTED. 
		long io_timeout;    // timeout between two read()/write() calls
		
		// When the job was started using FDCopyPump::Control(). 
		HTime starttime;
		
		// Custom data pointer attached to the FDCopyIO. Defaults 
		// to NULL and may be set to anything at any time. 
		void *dptr;
		
		// Ugh, this needs some explanation: 
		// Please see "Note: object lile cycle" above. 
		int persistent : 1;
		// active: currently working for an FDCopyPump. 
		int active : 1;
		
	public:
		// This function can be prvided for e.g. FDCopyPump_Simple 
		// to get the pointer where to store data at or where to 
		// read data from. 
		// NOTE: In case the passed DataPtr* is NULL, the function 
		//       MUST return 0 and do nothing more. A call to 
		//       dataptr(NULL) is used by the FDCopyPump to see 
		//       if it is supported (that is why the default 
		//       implementation returns -2 in any case). 
		// NOTE: DataPtr::more signalizes if buf[0]...buf[buflen-1] 
		//       is everything or if more data can be read/written 
		//       when calling dataptr() the next time. 
		// If you provide dataptr() you will also want to provide 
		// datadone(). 
		// Return value:   [used by FDCopyPump_Simple]
		//   0 -> OK
		//  -1 -> error
		//  -2 -> not supported
		virtual int dataptr(DataPtr * /*dp*/)
			{  return(-2);  }
		
		// After having queried the data location using dataptr(), 
		// the data is read stored. In order to tell the class the 
		// number of read/written bytes, datadone() is used. 
		// Return value:    [used by FDCopyPump_Simple]
		//   0 -> OK
		//        NOTHING DIFFERENT AT THE MOMENT. 
		virtual int datadone(DataDone * /*dd*/)
			{  return(0);  }
		
		// This is the reset function used by DoSuicide(): 
		// Only sensitive data is reset. 
		virtual void reset() HL_PureVirt(;)
		
	public:  _CPP_OPERATORS_FF
		// NOTE: You need not call this directly; use one of the 
		//       derived classes. 
		FDCopyIO(CPType t,int *failflag);
		virtual ~FDCopyIO();
		
		// Get Type ID of this FDCopyIO: 
		CPType Type() const
			{  return(type);  }
		
		// A non-persistent FDCopyIO gets deleted (delete this); 
		// a persistent one just gets reset by this call. 
		// Only sensitive data (like FDCopyIO_FD::transferred) is reset. 
		// Return value: 
		//   0 -> persistent; not deleted itself
		//   1 -> delete this; was done
		virtual int DoSuicide();
};


// This class can be used as data source or destination for copying 
// from or to a buffer. For this purpose, it must be passed when creating 
// an FDCopyPump. 
// Before creating the FDCopyPump using FDCopyBase::NEW_CopyPump() 
// set up the fields in FDCopyIO_Buf and FDCopyIO. 
class FDCopyIO_Buf : public FDCopyIO
{
	protected:
		// Ovrriding virtuals from FDCopyIO: 
		int dataptr(DataPtr *dp);
		int datadone(DataDone *dd);
		void reset();
		
	public:  /* _CPP_OPERATORS_FF from FDCopyIO */
		FDCopyIO_Buf(int *failflag=NULL);
		~FDCopyIO_Buf();
		
		// NOTE: These members are public for easy access. 
		// ****  Do not modify them while the copy job is active. 
		
		// Buffer to read data from or store data at. 
		char *buf;          // data buffer
		
		// Size of the buffer *buf above. DO NOT MODIFY. 
		size_t buflen;      // size of data buffer
		
		// This is the number of bytes aready stored in the 
		// buffer or the number of bytes already read from the 
		// buffer. DO NOT MODIFY. 
		size_t bufdone;     // number of bytes read/written so far
};


// This class can be used as data source or destination for copying 
// from or to a file descriptor. For this purpose, it must be passed 
// when creating an FDCopyPump. 
// Before creating the FDCopyPump using FDCopyBase::NEW_CopyPump() 
// set up the fields in FDCopyIO_FD and FDCopyIO. 
class FDCopyIO_FD : public FDCopyIO
{
	protected:
		// Ovrriding virtual from FDCopyIO: 
		void reset();
	
	public:  /* _CPP_OPERATORS_FF from FDCopyIO */
		FDCopyIO_FD(int *failflag=NULL);
		~FDCopyIO_FD();
		
		// NOTE: These members are public for easy access. 
		// ****  Do not modify them while the copy job is active. 
		
		// This is the PollID of the FD to read the data from or 
		// write the data to. Note that only those events needed 
		// are modified, the other events stay unchanged. 
		// (Uses FDBase::FDChangeEvents()). 
		// THE PollID MUST HAVE BEEN ALLOCATED BY THE CLASS CREATING 
		// THE FDCopyPUMP. THE CLASS MAY NOT UnpollFD THIS PollID 
		// UNTIL THE JOB IS DONE. 
		// (Note: FDCopyBase provides replacements for the poll 
		//        functions to handle such cases.)
		FDManager::PollID pollid;   // fd to read from / write to
		
		// This is the max numer of bytes fed into a call to write(2) 
		// or writev(2) or the max number of bytes requested by a 
		// call to read(2) or readv(2). 
		// Use 0 for unlimited (default). 
		size_t max_iolen;   // may read/write len (per call) (0 -> unlimited)
		
		// The number of already transferred bytes (i.e. the number 
		// of bytes aready written to or read from the fd). 
		// DO NOT MODIFY. 
		copylen_t transferred;      // number of bytes that went over the fd
};


// An FDCopyPump is a class which uses a source FDCopyIO and a dest FDCopyIO 
// and copies data from the source to the dest. #
// NOTE: - In order to use an FDCopyPump, your class must be derived from 
//         FDCopyBase. 
//       - As the FDCopyPump only modifies those FD events which are 
//         actually needed and does not touch the others (FDChangeEvents()), 
//         it is well possible to have two FDCopyPump operating on the same 
//         file descriptor (one reading from it and one writing to it; 
//         useful for network sockets). 
//       - There are several FDCopyPumps which are derived from FDCopyPump. 
//         Simply call FDCopyBase::NEW_CopyPump() and the correct one 
//         will be automatically chosen. (E.g. copying fd -> buf cannote 
//         be done with the same data pump as copying fd -> fd.)
//       - The FDCopyPump classes MUST all be allocated using operator new. 
class FDCopyPump : 
	#if __GNUG__ < 3
	public LinkedListBase<FDCopyPump>
	#else
	private LinkedListBase<FDCopyPump>
	#endif
{
	friend class LinkedList<FDCopyPump>;
	friend class FDCopyBase;
	public:
		// General control commands. 
		// Not all pumps must support all commands. 
		enum ControlCommand
		{
			CC_Start,   // Start job
			CC_Term,    // Term a job; normally stop input 
			            // but wait for output to finish
			CC_Kill,    // Kill job (job then no longer there)
			CC_Stop,    // Stop input & output
			CC_StopI,   // Stop input
			CC_StopO,   // Stop output
			CC_Cont,    // Continue with input & output
			CC_ContI,   // Continue stopped input
			CC_ContO    // Continue stopped output
		};
		
		enum StatusCode
		{
			// Note that when reaching the copy limit, the pump cannot 
			// detect if EOF is reached simultaniously. If SCEOF is not 
			// set and SCLimit is set, you don't know if there is more 
			// data available. 
			// NOTE: BITWISE OR'ED: 
			//       One of the flags and optionally SCFinal is set. 
			//       If reading is done, one of the ECEOI-flags remains set. 
			
			//*** Status codes ***
			SCNone=   0x00,   // (mainly internal use; may never happen)
			SCFinal=  0x01,   // IMPORTANT: last time cpnotify() called 
			SCLimit=  0x02,   // copy limit reached / buffer complete
			SCTimeout=0x04,   // some timeout passed (see err_no)
			SCKilled= 0x08,   // Control(CC_Kill) called (reasom can also 
			                  // be that needed PollID was UnpollFD()'d)
			SCTerm=   0x10,   // Control(CC_Term) called
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
			SCErrPollI= 0x010000,   // poll() returns POLLERR on input fd
			SCErrPollO= 0x020000,   // poll() returns POLLERR on output fd
			SCErrRead=  0x040000,   // error during call to read()/readv()
			SCErrWrite= 0x080000,   // error during call to write()/writev()
			SCErrCopyIO=0x100000,   // error in FDCopyIO::dataptr() and the like
			                        // error code stored in err_no 
			// Combined: matches any error: 
			SCError=(SCErrPollI|SCErrPollO|SCErrRead|SCErrWrite|SCErrCopyIO)
		};
		
		friend inline StatusCode &operator|=(StatusCode &s,int x)
			{  s=(StatusCode)(int(s)|x);  return(s);  }
		friend inline StatusCode &operator&=(StatusCode &s,int x)
			{  s=(StatusCode)(int(s)&x);  return(s);  }
		
		// CopyInfo structure used to inform FDCopyBase-derived 
		// class of done jobs, errors, etc (via cpnotify()). 
		struct CopyInfo
		{
			// Which pump we are talking about: 
			// pump's source and dest FDCopyIO are still set. 
			FDCopyPump *pump;
			
			// Time as passed down via fdnotify() or NULL. 
			HTime *fdtime;
			
			// Reason why we are here: 
			// IMPORTANT FLAG: Every time cpnotify(CopyInfo*) gets 
			// called, you should check if SCFinal is set. 
			// YES -> This is the last time a cpnotify() is called 
			//        for this copy job. Either it is done or 
			//        terminated by an error. 
			// NO ->  Not the last time cpnotify() gets called. 
			StatusCode scode;
			
			// Can hold more detail about what we're talking: 
			// Check this if(scode & SCError) for error code (errno): 
			// In case of no error err_no is 0. 
			// Note: 
			// - In case of SCInPipe / SCOutPipe err_no=EPIPE. 
			// - In case of SCErrPollI/O, err_no is poll's revents 
			//   which may have POLLERR and POLLNVAL set. 
			// - In case of SCTimeout it is the timeout which 
			//   elapsed: 
			//   NOTE: CURRENTLY TIMEOUT NOT IMPLEMENTED. 
			int err_no;
			
			CopyInfo(FDCopyPump *p,StatusCode sc,int err,HTime *t)
				{  pump=p;  fdtime=t;  scode=sc;  err_no=err;  }
			~CopyInfo() { }
		};
		
		// ProgressInfo structure used to inform FDCopyBase-derived 
		// class of job progress (via cpprogress()). 
		// Only for eye candy or timeout management; all termination 
		// and error-related information comes via cpnotify(). 
		struct ProgressInfo
		{
			// Which pump we are talking about: 
			FDCopyPump *pump;
			
			// Time as passed down via fdnotify(); pretty much current. 
			const HTime *fdtime;
			
			// Progress info: 
			// Number of bytes just read (moved_bytes<0) or written 
			// (moved_bytes>0) in this cycle. 
			ssize_t moved_bytes;
		};
		
	protected:
		FDCopyBase *fcb;  // needed for calls to PollFD/FDChangeEvents() AND 
	                	  // to deliver status info
		FDCopyIO *src;    // where data is read from 
		FDCopyIO *dest;   // where data is stored at
		
		enum PumpState
		{
			PS_Inactive=  0x0000,
			PS_Active=    0x0001,
			PS_StoppedIn= 0x0002,  // input stopped (do not read)
			PS_StoppedOut=0x0004,  // output stopped (do not write)
			PS_Stopped = PS_StoppedIn | PS_StoppedOut,
			PS_Flushing=  0x0008   // input is done, waiting for output to complete
		} state;
		
		friend inline PumpState &operator|=(PumpState &s,int x)
			{  s=(PumpState)(int(s)|x);  return(s);  }
		friend inline PumpState &operator&=(PumpState &s,int x)
			{  s=(PumpState)(int(s)&x);  return(s);  }
		
		// All the copy pups MUST use this function rather than 
		// the one of FDCopyBase or FDBase. 
		int IChangeEvents(FDManager::PollID pollid,short set_ev,short clear_ev);
		
		// Set the events which are controlled by this pump for the 
		// specified PollID. The PollID must be registered for the PollID 
		// using Want2Start(). 
		// Return value: 
		//   0 -> OK
		//  -2 -> not your PollID (see Want2Start())
		//  -3 -> PollID is NULL
		int ISetControlledEvents(FDBase::PollID pollid,short events);
		
		// Query the events controlled by this pump for specified PollID: 
		// Returns 0 if pollid==NULL. 
		short IGetControlledEvents(FDBase::PollID pollid);
		
		// This must be called by the pump before really trying 
		// to start the job. This tells the FDCopyBase that 
		// the pump now operates on the passed PollID. 
		// in_id ->  where data is read from or NULL if data 
		//           id not read from a file descriptor
		// out_id -> where data is written to or NULL
		// Return value: 
		//   0 -> OK (or: you are already operating on that PollID)
		//  -1 -> Somebody else operates on one of the PollIDs; 
		//        cannot go on. 
		//  -2 -> You are dead :)  [is_dead set]
		// NOTE: API might be changed to allow for more than 
		//       2 PollID's per FDCopyPump. 
		inline int Want2Start(FDManager::PollID in_id,FDManager::PollID out_id);
		
		// The opposite of Want2Start(): Tell FDCopyBase that we 
		// are no longer operating on the passed PollID's. 
		inline int DoneJob(FDManager::PollID in_id,FDManager::PollID out_id);
		
		// Used to call virtual cpnotify() and cpprogress() of the 
		// associated FDCopyBase: 
		inline int _CallCPNotify(CopyInfo *ci);
		inline int _CallCPProgress(ProgressInfo *pi);
		// 
		
		// This must be called by SetIO() of the derived class for 
		// basic checking. 
		int _BasicSetIOLogic(FDCopyIO *nsrc,FDCopyIO *ndest);
		
	private:
		// This is (normally) not meant to be called by derived classes. 
		void _DoSuicide();
		
	public:
		// NOTE: These members are public for easy access. 
		// ****  Do not modify them while the copy job is active. 
		
		// This is the timeout in msec for the complete copy job to 
		// finish. Use -1 to disable (default). 
	// NOTE: CURRENTLY NOT IMPLEMENTED. 
		long req_timeout;   // whole job
		
		// See "NOTE: object live cycle" above on meaning of 
		// the persistent flag. 
		int persistent : 1;
		
		// 1 -> waiting to get deleted. 
		int is_dead : 1;
	
	protected:
		// See fdcopybase.cc for an explanation on this flag: 
		int on_stack_of_fdnotify : 3;
		
	public:
	
		// Custom data pointer attached to the FDCopyIO. Defaults 
		// to NULL and may be set to anything at any time. 
		void *dptr;
		
		// Active: currently working, i.e. dest or src and dest are active. 
		int IsActive() const  {  return(state & PS_Active);  }
		// Stopped direction? (dir: 0 -> in OR out; -1 -> in; +1 -> out)
		int IsStopped(int dir) const
		{  return(state & (dir ? (dir<0 ? PS_StoppedIn : PS_StoppedOut) : PS_Stopped) );  }
		
	protected:
		// This is called by FDCopyBase. 
		// Receives fdnotify() calls just as generated by FDBase. 
		// Return value: 
		//   0 -> OK
		//  -1 -> job done (error)
		//   1 -> job done (regularly)
		virtual int HandleFDNotify(FDManager::FDInfo * /*fdi*/) HL_PureVirt(-1);
		
		// NOTE: NEVER CALL DIRECTLY. ALWAYS USE FDCopyPump::Control(). 
		// This is the virtual function implementing the control command 
		// in the derived classes. 
		virtual int VControl(ControlCommand /*cc*/)  {  return(-1);  }
		
	public:  _CPP_OPERATORS_FF
		// Use FDCopyBase::NEW_CopyPump(); you need not create the 
		// FDCopyPump yourself. 
		FDCopyPump(FDCopyBase *fcb,int *failflag);
		virtual ~FDCopyPump();
		
		// Get sourc and dest FDCopyIO: 
		FDCopyIO *Src()   {  return(src);   }
		FDCopyIO *Dest()  {  return(dest);  }
		
		// This has to be used to set the FDCopyIO classes to be used 
		// as data source and data destination. 
		// FDCopyBase::NEW_CopyPump() does that for you. 
		// Note the FDCopyIO *src and *dest are deleted by FDCopyPump 
		// when the FDCopyPump is _destroyed_ even if they are marked 
		// persistent (with the persistent flag). 
		// You may use SetIO(NULL,NULL) to unset the FDCopyIO's. 
		// The previous src and dest FDCopyIO's are always deleted 
		// unless persistent. 
		// Return value: 
		//   0 -> OK; success
		//  -1 -> allocation failure 
		//  -2 -> this combination of source and dest is not 
		//        supported by this FDCopyPump or one of them is NULL 
		//  -3 -> one of the passed FDCopyIO classes is maked active
		//  -4 -> FDCopyPump's current src / dest set and active 
		//  -5 -> FDCopyPump currently marked active or it is dead 
		//        AND dest or src are non-NULL. SEE ALSO PumpReuseNow(). 
		// UNLESS 0 is returned, the source and dest stored in *this 
		// were not modified. 
		// NOTE!! Unless 0 or -3 is returned and unless src / dest (as 
		//        passed to SetIO()) is persistent, src / dest was 
		//        deleted!
		// ** Additional return values are possible according to 
		// ** derived class. 
		virtual int SetIO(FDCopyIO * /*src*/,FDCopyIO * /*dest*/) HL_PureVirt(-2);
		
		// This function needs some explanation: 
		// If you want to re-use a pump during the call to (i.e. on the 
		// stack of) cpnotify(SCFinal), then you normally can not (solution 
		// below). That is because during the call to cpnotify() the data in 
		// the pump and the FDCopyIOs is still valid but the is_dead flag is 
		// set. In case the pump is non-persistent, it gets deleted when you 
		// return from cpnotify(). In case it is persistent, it simply does 
		// _DoSuicide(), i.e. gets rid of the FDCopyIOs and resets the 
		// is_dead flag. Now, if you want to re-use a persistent pump during 
		// a call to cpnotify(), then first cleanup the old state in your 
		// proggy, then call PumpReuseNow(), THEN (NOT BEFORE!) set the 
		// params for the FDCopyIOs and THEN call SetIO(). 
		// Return value: 
		//   0 -> OK
		//  -1 -> pump not persistent
		//  -2 -> not called on stack of fdnotify 
		int PumpReuseNow();
		
		// For available control commands see above. 
		// Note that not all FDCopyPumps must support all commands 
		// and that some commands may behave equally for some pumps. 
		// Return values are generally: 
		//   0 -> OK
		//   1 -> already the case (i.e. already stopped)
		//  -1 -> error
		//  -2 -> not supported / illegal command / not active
		// NOTE: This calls virtual VControl(). 
		// - If you use CC_Kill or CC_Term, the class (*this) will / may
		//   delete itself (using delete *this) [unless persistent]. 
		// - A failing CC_Start will NEVER cause a deletion of the class. 
		int Control(ControlCommand cc);
};


// This is a FDCopyPump which can be used as general-purpose copy 
// pump which supplies a (virtual) dataptr() function. 
// NOTE that in order to use FDCopyPump_Simple, either the source or 
// the dest must be an fd (i.e. FDCopyIO_FD). 
// This is the one you may use if you write your own FDCopyIO. 
// If your FDCopyIO cannot provide such a function, you will probably 
// have to write you own FDCopyPump as well. 
// NOTE: You can copy buffers of zero size to an fd without trouble 
//       but when reading into a buffer of size 0 you can run into 
//       trouble if POLLIN will not occur because the network socket fd 
//       has no more available data. Deadlock may occur. For this reason, 
//       avoid reading into buffers of size 0. 
class FDCopyPump_Simple : public FDCopyPump
{
	protected:
		int fd_dir;   // -1 -> fd is source; +1 -> fd is dest
		
		int _ReadData(int fd,HTime *fdtime);
		int _WriteData(int fd,HTime *fdtime);
		
		int _StartJob();
		int _EndJob(CopyInfo cpi);
		int _StopContJob(int stop);
		
		void _Cleanup(int go_dead);
		
	public:  // _CPP_OPERATORS_FF interited from FDCopyPump
		FDCopyPump_Simple(FDCopyBase *fcb,int *failflag=NULL);
		~FDCopyPump_Simple();
		
		// Overriding virtuals from FDCopyPump: 
		// Additional return values of SetIO(): 
		//   -20 -> passed FDCopyIO_Bud does not support dataptr() call
		//   -21 -> passed FDCopyIO_FD has pollid=NULL
		int SetIO(FDCopyIO *src,FDCopyIO *dest);
		int HandleFDNotify(FDManager::FDInfo *fdi);
		
		// Return value for CC_Start: 
		//   -3 -> somebody else is activeo on PollID; we may not start
		int VControl(ControlCommand cc);
};


// Special FDCopyPump to be used for fd -> fd copying. 
// This FDCopyPump can only work together with 2 FDCopyIO objects 
// of type FDCopyIO_FD. 
class FDCopyPump_FD2FD : public FDCopyPump
{
	public:
		// NOTE: These members are public for easy access. 
		// ****  Do not modify them while the copy job is active. 
		
		// Max umber of bytes to copy; if end of file / IO error / hangup 
		// etc occurs earlier, less data is copied. 
		// Set 0 for unlimited. 
		copylen_t limit;         
		
		// Size of fifo buffer used to store the data which is 
		// on the fly. Defaults to 16kb. 
		size_t io_bufsize;
		
		// Read/Write threshold values; this needs some explanation: 
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
		// THUS: hard max for thresh is io_bufsize, 
		//       hard min for thresh is 0 and
		//       high thresh must be larger than low thresh (NOT equal)
		// You may count on reasonable defaults; 
		// Values of -1 mean -> set defaults according to io_bufsize. 
		// NOTE: You may EITHER specify ALL values yourself OR use defaults 
		//       for ALL 4 values (by passing -1). 
		// 
		// Illustration on how it should be: 
		// 
		//         low_write_thresh         high_write_thresh
		//                |   low_read_thresh       |   high_read_thresh
		//                v       v                 v        v
		//        .-----------------------------------------------------.
		// read:  |               ############################          |
		// write: |       ###########################                   |
		//        `-----------------------------------------------------´
		//                        |<---operation--->|
		//                             in RW mode
		ssize_t low_read_thresh;
		ssize_t high_read_thresh;
		ssize_t low_write_thresh;
		ssize_t high_write_thresh;
		
	private:
		// Here, the on-the-fly data is stored: 
		FDFifoBuffer fifo;
		
		// When flushing is set, only the current fifo buffer is written 
		// to the output but no more data is read. 
		// Input PollID is no longer controlled. 
		int flushing : 1;
		// Are we currently reading/writing (i.e. polling for these events)?
		// TAKE CARE TO KEEP THESE UP TO DATE!
		int curr_reading : 1;
		int curr_writing : 1;
		// Status code for finished input saved here to be able to set it 
		// all following cpnotify() calls. 
		FDCopyPump::StatusCode persistent_sc;
		
		int _ReadInData(int fd,FDCopyIO_FD *cpio,HTime *fdtime);
		int _WriteOutData(int fd,FDCopyIO_FD *cpio,HTime *fdtime);
		void _ReDecidePollEvents();
		
		// Return value: See VControl()'s retvals for CC_Star. 
		int _StartSetup();
		int _StartJob();
		int _StopContJob(ControlCommand cc,int do_stop);
		int _FinishJob(int dir,CopyInfo cpi);
		
		void _Cleanup(int go_dead);
		
	public:  // _CPP_OPERATORS_FF interited from FDCopyPump
		FDCopyPump_FD2FD(FDCopyBase *fcb,int *failflag=NULL);
		~FDCopyPump_FD2FD();
		
		// Overriding virtuals from FDCopyPump: 
		// Additional return value of SetIO(): 
		//   -21 -> passed FDCopyIO_FD has pollid=NULL
		int SetIO(FDCopyIO *src,FDCopyIO *dest);
		int HandleFDNotify(FDManager::FDInfo *fdi);
		
		// Return value for CC_Start: 
		//   -3 -> somebody else is active on PollID; we may not start
		//   -4 -> fifo buffer (io_bufsize) allocation failure
		//   -5 -> illegal thresh values or limit<0
		int VControl(ControlCommand cc);
};


// All classes which want to use the FD copy facility must be derived from 
// FDCopyBase. 
// NOTE THAT FDCopyBase is itself derived from FDBase so that YOU MUST NOT 
//      DERIVE YOUR CLASS EXPLICITLY FROM FDBase. 
// **NOTE**
//   All the FD polling functions of FDBase are re-implemented here and 
//   forward your calls to the FDBase functions. 
// - For PollID's / FDs used by an active FDCopyIO, you may NOT call
//   PollFD() to set the poll events; you MUST use FDChangeEvents() 
//   (this can be used for all PollID's, of course). 
// - FDCopyBase uses FDManager's dptr (associated with each FD) to 
//   store internal info for the FD for efficient fdnotify() handling. 
//   You can still use dptr as FDCopyBase replaces the dptr with the 
//   one you expect when calling FDDPtr() or in fdnotify2(). 
// - fdnotify() is overridden by FDCopyBase which forwards the calls 
//   using the virtual function fdnotify2(). You MUST use this one. 
//   Only those calls interesting for you are forwarded: Those events 
//   used by one of the FDCopyPumps are removed and if no events rest, 
//   the call is not forwarded. 
class FDCopyBase : public FDBase
{
	friend class FDCopyPump;
	public:
		typedef FDCopyPump::ControlCommand ControlCommand;
		typedef FDCopyPump::StatusCode StatusCode;
		typedef FDCopyPump::CopyInfo CopyInfo;
		typedef FDCopyPump::ProgressInfo ProgressInfo;
		
		// This struct is the data hook at all dptr pointers 
		// for FDs. 
		struct FDDataHook
		{  _CPP_OPERATORS_FF
			struct HIO
			{
				FDCopyPump *pump;   // pump in this direction
				short ctrl_ev;      // controlled events by pump
			};
			
			HIO in;     // pump & controlled events in read direction
			HIO out;    // pump & controlled events in write direction
			
			const void *orig_dptr;   // dptr used by derived class
			int gets_deleted : 1;   // do not add pumps any more; FDDataHook 
			                        // is about to get deleted
			// lock_delete: do not delete, jut set gets_deleted. 
			// Use _DoUnlockDelete() to unlock...
			int lock_delete : 1;
			int : (sizeof(int)*8-2);
			
			FDDataHook(int *failflag=NULL);
			~FDDataHook();
		};
	private:
		// Use this to delete a data hook: 
		inline void _DeleteDataHook(FDDataHook *h,PollID pollid);
		// Use this to unlock the lock_delete flag in FDDataHook. 
		inline void _Hook_DoUnlockDelete(FDDataHook *h,PollID pollid);
		
		// Internally used by NEW_CopyPump(): 
		FDCopyPump *_NEW_CopyPump_DoRest(FDCopyPump *pump,FDCopyIO *src,FDCopyIO *dest);
		
		void _DoHandleFDNotify(FDManager::FDInfo *fdi,FDDataHook *h,FDCopyPump *pump);
		
		int _Can_AddPump2DataHook(FDCopyPump *pump,PollID pollid,int dir);
		void _Do_AddPump2DataHook(FDCopyPump *pump,PollID pollid,int dir);
		int _DelPumpFromDataHook(FDCopyPump *pump,PollID pollid,int dir);
		
		// Interface for FDCopyPump: 
		void RegisterPump(FDCopyPump *p);
		void UnregisterPump(FDCopyPump *p);
		// See FDCopyPump for more info. 
		int Want2Start(FDCopyPump *pump,PollID in_id,PollID out_id);
		int DoneJob(FDCopyPump *pump,PollID in_id,PollID out_id);
		// See FDCopyPump...
		int IChangeEvents(FDCopyPump *pump,PollID pollid,short set_ev,short clear_ev);
		int ISetControlledEvents(FDCopyPump *pump,PollID pollid,short events);
		short IGetControlledEvents(FDCopyPump *pump,PollID pollid);
		
		// Overridden from FDBase: 
		int fdnotify(FDManager::FDInfo *fdi);
	protected:
		// All existing FDCopyPumps which are associated with this 
		// FDCopyBase are queued here. 
		LinkedList<FDCopyPump> cplist;
		
		// Forwards calls from FDBase::fdnotify(): 
		// Return value: See FDBase::fdnotify(). 
		virtual int fdnotify2(FDManager::FDInfo * /*fdi*/)  {  return(0);  }
		// Status calls informing derived class abount copy job status: 
		// Return value: Currently unused; use 0. 
		virtual int cpnotify(CopyInfo * /*ci*/)  {  return(0);  }
		// Progress info from running copy requests. 
		// Only for eye candy or timeout management; all termination 
		// and error-related information comes via cpnotify(). 
		// Return value: Currently unused; use 0. 
		virtual int cpprogress(ProgressInfo * /*pi*/)  {  return(0);  }
		
	private:
		// Internally used: 
		void _DetachFromFD(PollID pollid);
		int _AttachToFD(PollID &pollid);
		
		// Made private because it may not be used!
		// You must use FDChangeEvents() and PollFDDPtr() instead. 
		int PollFD(PollID pollid,short events=0)  {  return(-1);  }
		int PollFD(PollID pollid,short events,const void *dptr)  {  return(-1);  }
	public:  // _CPP_OPERATORS_FF interited from FDBase
		FDCopyBase(int *failflag=NULL);
		~FDCopyBase();
		
		// Please check FDBase for these functions. 
		// They are re-implemented here for internal reasons; do 
		// not use the FDBase functions directly. NEVER. 
		// NOTE: There are functions made private which do nothing 
		//       (just return error) [see above]. DO NOT USE THEM. 
		// PollFD(fd,events,...) may only be used to allocate a new poll node. 
		// Returns -1 otherwise (use FDChangeEvents()). 
		int PollFD(int fd,short events=0)
			{  return(FDCopyBase::PollFD(fd,events,NULL,NULL));  }
		int PollFD(int fd,short events,const void *dptr,PollID *ret_id=NULL);
		int PollFDDPtr(PollID pollid,const void *dptr)
			{  if(!pollid)  return(-2);
				((FDDataHook*)FDBase::_FDDPtrNN(pollid))->orig_dptr=dptr;  return(1);  }
		// Additional return value: 
		//   2 -> flags controlled by copy pump were removed
		int FDChangeEvents(PollID pollid,short set_ev,short clear_ev);
		int UnpollFD(int fd)
			{  PollID tmp=FDBase::FDPollID(fd);  return(FDCopyBase::UnpollFD(tmp));  }
		int UnpollFD(PollID &pollid)
			{  _DetachFromFD(pollid);  return(FDBase::UnpollFD(pollid));  }
		int CloseFD(int fd);
		int CloseFD(PollID &pollid)
			{  _DetachFromFD(pollid);  return(FDBase::CloseFD(pollid));  }
		int ShutdownFD(int fd);
		int ShutdownFD(PollID &pollid)
			{  _DetachFromFD(pollid);  return(FDBase::ShutdownFD(pollid));  }
		
		const void *FDDPtr(int fd)
			{  return(FDCopyBase::FDDPtr(FDBase::FDPollID(fd)));  }
		const void *FDDPtr(PollID pollid)
			{  return(pollid ? ((FDDataHook*)FDBase::_FDDPtrNN(pollid))->orig_dptr : NULL);  }
		// Note: events controlled by a data pump are removed: 
		short FDEvents(int fd)
			{  return(FDCopyBase::FDEvents(FDBase::FDPollID(fd)));  }
		short FDEvents(PollID pollid);
		
		// These functins should be used to create FDCopyPump objects. 
		// Note that you can only use one of the specified modes. 
		// Especially copying Buf -> Buf is not (yet) supported 
		// because there is little need. 
		// NOTE: Make sure to set all necessary parameters in the
		//       FDCopyIO_* BEFORE calling NEW_CopyPump(). 
		// NOTE: The FDCopyIO_* classes must have been allocated 
		//       via operator new and get destroyed automatically 
		//       at the end (unless persistent). 
		// IN CASE you wrote your own FDCopyIO class, you will have 
		// to create the FDCopyPump yourself and call 
		// FDCopyPump::SetIO(). The FDCopyPump_Simple supports all 
		// FDCopyIO classes which supply (virtual) dataptr() 
		// function (operating on a buffer) as long as one of the 
		// FDCopyIO's is an FDCopyIO_FD. FD -> FD copying has a 
		// special FDCopyPump as it has to provide a temporary 
		// buffer. 
		// NOTE!! When copying from an fd and copying to the same 
		//        fd you have to use TWO FDCopyPumps and 
		//        _TWO_ FDCopyIO classes for that fd (one for each 
		//        pump) which both operate on the SAME _ONE_ PollID. 
		// Returns a pointer to the pump properly set up. 
		// In case something fails, it returns NULL. 
		//   In this case, src and dest were deleted (unless 
		//   persistent). 
		FDCopyPump_Simple *NEW_CopyPump(FDCopyIO_Buf *src,FDCopyIO_FD *dest)
		{  FDCopyPump_Simple *pump = NEW1<FDCopyPump_Simple>(this);
			return((FDCopyPump_Simple *)_NEW_CopyPump_DoRest(pump,src,dest));  }
		FDCopyPump_Simple *NEW_CopyPump(FDCopyIO_FD *src,FDCopyIO_Buf *dest)
		{  FDCopyPump_Simple *pump = NEW1<FDCopyPump_Simple>(this);
			return((FDCopyPump_Simple *)_NEW_CopyPump_DoRest(pump,src,dest));  }
		FDCopyPump_FD2FD  *NEW_CopyPump(FDCopyIO_FD *src,FDCopyIO_FD *dest)
		{  FDCopyPump_FD2FD *pump = NEW1<FDCopyPump_FD2FD>(this);
			return((FDCopyPump_FD2FD *)_NEW_CopyPump_DoRest(pump,src,dest));  }
		
		
};


// Some inline functions: 
inline int FDCopyPump::Want2Start(FDManager::PollID in_id,FDManager::PollID out_id)
	{  return(fcb->Want2Start(this,in_id,out_id));  }
inline int FDCopyPump::DoneJob(FDManager::PollID in_id,FDManager::PollID out_id)
	{  return(fcb->DoneJob(this,in_id,out_id));  }
inline int FDCopyPump::_CallCPNotify(CopyInfo *ci)
	{  return(fcb->cpnotify(ci));  }
inline int FDCopyPump::_CallCPProgress(ProgressInfo *pi)
	{  return(fcb->cpprogress(pi));  }
inline int FDCopyPump::IChangeEvents(FDManager::PollID pollid,short set_ev,short clear_ev)
	{  return(fcb->IChangeEvents(this,pollid,set_ev,clear_ev));  }
inline int FDCopyPump::ISetControlledEvents(FDBase::PollID pollid,short events)
	{  return(fcb->ISetControlledEvents(this,pollid,events));  }
inline short FDCopyPump::IGetControlledEvents(FDBase::PollID pollid)
	{  return(fcb->IGetControlledEvents(this,pollid));  }

// HOW IT WORKS, briefly: 
// ----------------------
// 
// Before starting the job: 
// - Create one FDCopyIO for the source and one for the destination. 
// - Set them up properly. 
// - Create FDCopyPump and pass FDCopyIO pointers (SetIO())
// - Set other parameters in FDCopyPump. 
// Starting the job: 
// - Call FDCopyPump::Control(CC_Start); which should return 0. 
//     This does the following: 
//     - call FDCopyBase::Want2Start() (via FDCopyPump) 
//     - Want2Start() puts the pump into FDDataHook's in_pump / 
//       out_pump associated with the apropriate PollID 
//       (and checks if that is possible at all). 
//     - If an error occurs,  FDCopyBase::DoneJob() must be 
//       called (removal from FDDataHook). 
// The job is now running. You may stop / cont / kill it using 
// - FDCopyPump::Control(...)
// When the job is done, 
//     - it calls FDCopyBase::DoneJob()


#endif  /* _HLIB_FDCopyBase_H_ */

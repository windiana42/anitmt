/*
 * logger.hpp
 * 
 * Message logger class header. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _RNDV_LOGGER_HPP_
#define _RNDV_LOGGER_HPP_ 1

#include <lib/prototypes.hpp>

#include <hlib/cdrecorder.h>
#include <hlib/refstring.h>
#include <hlib/valuehandler.h>
#include <hlib/parconsumerovl.h>
#include <hlib/htime.h>


class MessageLogger : 
	public par::ParameterConsumer_Overloaded
{
	public:
		// Global message logger: 
		static MessageLogger *logger;
	
	public:
		enum MType
		{
			MTNone=    0x00,
			MTDVerbose=0x01,   // debug
			MTVerbose= 0x02,
			MTSVerbose=0x04,   // special verbose
			MTWarning= 0x08,
			MTError=   0x10,
			MT_Prefix= 0x80,   // <-- not a message type but the prefix flag
		};
		
		struct MEntry
		{
			HTime time;
			RefString msg;
			MType mtype;
			
			_CPP_OPERATORS_FF
			inline MEntry(int *failflag) : time(HTime::Invalid),msg(failflag),
				mtype(MTNone)  {}
			inline ~MEntry() {}
			
			inline void zap()
				{  time.SetInvalid();  msg.deref();  mtype=MTNone;  }
			inline void set(const MEntry &e)
				{  time=e.time;  msg=e.msg;  mtype=e.mtype;  }
		};
		
	private:
		struct MyRecorder
		{
			CyclicDataRecorder<MEntry> *rec;  // Must be NULL if size==0. 
			
			// Some storage usage statistics: 
			size_t tot_bytes;
			size_t max_bytes;
			
			// Max number of messages: 
			size_t max_msgs;
			
			// Get size: 
			size_t Size() const
				{  return(rec ? rec->Size() : 0);  }
			
			// Allocate recorder; returns 0 -> OK; -1 -> alloc failure. 
			int Alloc(size_t size);
			
			_CPP_OPERATORS_FF
			MyRecorder(int *failflag);
			~MyRecorder();
		};
		
		// Message recorder: Records any message. 
		MyRecorder mrec;
		MType hist_mode;  // <-- what to store
		
		// Recorder for last errors and warnings: 
		MyRecorder erec;
		
		// What to write on tty: 
		MType tty_mode;
		
		// Params: 
		struct Params
		{
			RefString tty_str;
			int ehist_size;
			int hist_size;
			RefString hist_msg;
			
			_CPP_OPERATORS_FF
			Params(int *failflag);
			~Params();
		} *p;
		
		// "[allocation failure]" RefString: 
		RefString allocfail_str;
		
		// Internally used message handler: 
		static int _vaMessage(MType mtype,const char *fmt,va_list ap);
		int _LogMessage(MType mtype,const char *fmt,va_list ap);
		
		void _DoStore(MyRecorder *rec,
			const HTime &time,const RefString &msg,MType mtype);
		
		// Parse message spec (+/-aewsvd): 
		// Returns error count and writes error message. 
		int _ParseMsgSpec(const char *str,MType *mtype,
			const char *arg_for_error,bool is_tty);
		
		// [overriding virtual:]
		int CheckParams();
		
		// Install parameters. Returns error count. 
		int _SetUpParams();
		
	public: _CPP_OPERATORS_FF
		MessageLogger(par::ParameterManager *parman,int *failflag=NULL);
		~MessageLogger();
		
		// Log messages. 
		// Will also perform the TTY output (unless disabled). 
		static int _Error(const char *fmt,...)  __attribute__ ((__format__ (__printf__, 1, 2)));
		static int _Warning(const char *fmt,...)  __attribute__ ((__format__ (__printf__, 1, 2)));
		static int _Verbose(int vspec,const char *fmt,...)  __attribute__ ((__format__ (__printf__, 2, 3)));
		static int _VerboseSpecial(const char *fmt,...)  __attribute__ ((__format__ (__printf__, 1, 2)));
		
		// Needed as plug-ins for the parameter system: 
		static int _vaWarning(const char *fmt,va_list ap);
		static int _vaError(const char *fmt,va_list ap);
};

#endif  /* _RNDV_LOGGER_HPP_ */

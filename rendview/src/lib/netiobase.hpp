/*
 * netiobase.hpp
 * 
 * Network IO (e.g. for LDR client and sever) as a layer on top of
 * FDCopyBase. 
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

#ifndef _RNDV_LIB_NETWORKIOBASE_HPP_
#define _RNDV_LIB_NETWORKIOBASE_HPP_

#include <hlib/refstrlist.h>
#include <hlib/fdcopybase.h>

#include <lib/myaddrinfo.hpp>


class NetworkIOBase : 
	public FDCopyBase
{
	public:
		struct NetIOStatistics
		{
			// Total amount of transferred data through network socket. 
			// Keep that in sync if you read/write directly from/to 
			// the socket. 
			u_int64_t tot_transferred;
			unsigned int count_headers;   // transferred LDRHeaders
			unsigned int count_files;     // completely transferred files
			
			// Add passed statistics: 
			void Add(const NetIOStatistics *s);
			// Re-set to zero. 
			void Reset();
			
			NetIOStatistics()  {  Reset();  }
			~NetIOStatistics() {}
		};
	protected:
		enum IOAction
		{
			IOA_None=0,   // doing nothing, waiting
			IOA_Locked,   // locked (not yet authenticated)
			IOA_SendingBuf,   // sending something from buffer
			IOA_SendingFD,    // sending something from FD
			IOA_ReceivingBuf,   // receiving and storing in buffer
			IOA_ReceivingFD     // copying from sock to FD
		};
		
		// Socket for client/server connection: 
		int sock_fd;
		FDBase::PollID pollid;
		
		// Calls FDChangeEvents() and checks for errors: 
		inline void _DoPollFD(short set_events,short clear_events)
		{
			int rv=FDChangeEvents(pollid,set_events,clear_events);
			if(rv<0)
			{  _DoChangeEvents_Error(rv);  }
		}
		
		// These are persistent: 
		struct FDCopy
		{
			IOAction ioaction;  // What we're currently doing. 
			FDCopyIO_FD *io_sock;
			FDCopyIO_Buf *io_buf;
			FDCopyIO_FD *io_fd;
			FDCopyPump_Simple *pump_s;  // io_sock <-> io_buf
			FDCopyPump_FD2FD *pump_fd;  // io_sock <-> io_fd
			// Some statistics (number of bytes transferred, 
			// headers, files): 
			NetIOStatistics stat;
			
			FDCopy();   // NOTE: ONLY NULL-INITIALISATION
			~FDCopy() { /*empty*/ }
		};
		// FDCopy in is for READING FROM sock_fd: 
		// Read from io_sock and write to io_buf or io_fd. 
		FDCopy in;    // reading from sock_fd
		// FDCopy out is for WRITING TO sock_fd: 
		// Write data from io_buf or io_fd to io_sock. 
		FDCopy out;   // writing to sock_fd
		
		// Start sending passed buffer using FDCopyBase etc. 
		// Retval: 
		//   0 -> OK
		//  -1 -> (alloc) failure
		int _FDCopyStartSendBuf(char *buf,size_t len);
		// Analogon to read buffer: 
		int _FDCopyStartRecvBuf(char *buf,size_t len);
		
		// Start sending passed file using FDCopyBase etc. 
		// Retval: 
		//   0 -> OK
		//  -1 -> (alloc) failure
		//  -2 -> failure to open file
		//  -3 -> filelen<0
		int _FDCopyStartSendFile(const char *path,int64_t filelen);
		// Start receiving passed file using FDCopyBase etc. 
		// Retval: See _FDCopyStartSendFile(). 
		int _FDCopyStartRecvFile(const char *path,int64_t filelen);
		
		// Call this to shut down the connection (cancelling all IO jobs): 
		void _ShutdownConnection();
	private:
		void _DoChangeEvents_Error(int rv);
		
		// Like constructor and destructor...
		int _Construct_FDCopy(FDCopy *fdc);
		void _Destroy_FDCopy(FDCopy *fdc);
	public:
		NetworkIOBase(int *faiulflag=NULL);
		~NetworkIOBase();
		
		// Status code string using cpi->scode and cpi->err_no: 
		// Returns static data. 
		static char *CPNotifyStatusString(const FDCopyBase::CopyInfo *cpi);
		
		// Dump some nice statistics: transferred bytes and by/sec: 
		// side: +1 -> server; -1 -> client
		void DumpNetIOStatistics(int vlevel,const HTime *starttime,int side,
			const char *peer_name);
		// Used by the above function: actually print the statistics; 
		// title is written as "headline", NO newline required. 
		// indent: some spaces to be printed before each line. 
		static void DumpNetIOStatistics(int vlevel,
			const NetIOStatistics *downstream,const NetIOStatistics *upstream,
			const HTime *starttime,const char *title,const char *indent="");
		
		// Sum up the size of all strings in l including terminating '\0' for 
		// each string: 
		static inline size_t SumUpSize(const RefStrList *l)
		{
			size_t len=0;
			for(const RefStrList::Node *i=l->first(); i; i=i->next)
			{  len+=(i->len()+1);  }
			return(len);
		}
		
		// Copy string list into buffer *dest; make sure it is large 
		// enough (use SumUpSize()). 
		// Returns updated dest pointer; copies '\0' at the end.
		static char *CopyStrList2Data(char *dest,const RefStrList *l);
		
		// These 2 do the opposite of the function above. 
		//---
		// This one expects n entries (separated by '\0') in *data and 
		// stores these entries in dest. data_len is the length of the 
		// buffer *data and is used to avoid access beyond end of 
		// buffer in case there are less than n entries in there. 
		// dest is not cleared; data is appended. 
		// Return value: 
		//  - updated src pointer, i.e. the position after the last '\0' 
		//  - dest-1 if data_len was reached before n strings were read 
		//  - NULL if an allocation failure happend
		static char *CopyData2StrList(RefStrList *dest,const char *data,
			int n,size_t data_len);
		// This function works like the one above but does not need 
		// to know how many entries to expect but it reads in the 
		// complete buffer from *data to data+data_len-1. 
		// Return value: 
		//   number of read in entries
		//   -1 -> allocation failure
		//   -2 -> format error: last string not terminated by '\0' 
		static int CopyData2StrList(RefStrList *dest,const char *data,
			size_t data_len);
		
		// In bitarray arr, set bit x: 
		static void BitarraySBI(unsigned char *arr,int x)
			{  arr[x/8]|=(((unsigned char)1)<<(x%8));  }
		// In bitarray arr, return bit x: 
		static bool BitarrayTST(unsigned char *arr,int x)
			{  return(arr[x/8] & (((unsigned char)1)<<(x%8)));  }
		
		// Against aligment problems: Copy a 2-byte integer from src to dest. 
		static inline void _memcpy16(u_int16_t *dest,const char *src)
			{  char *d=(char*)dest;  *(d++)=*(src++);  *d=*src;  }
		static inline void _memcpy16(u_int16_t *dest,const u_int16_t *src)
			{  char *d=(char*)dest,*s=(char*)src;  *(d++)=*(s++);  *d=*s;  }
		
		// htons/htonl for 64bit integers: 
		static inline u_int64_t htonll(u_int64_t x)
		{
			u_int64_t r;
			unsigned char *d=((unsigned char*)&r)+7;
			*(d--)=(x & 0xffU);  x>>=8;
			*(d--)=(x & 0xffU);  x>>=8;
			*(d--)=(x & 0xffU);  x>>=8;
			*(d--)=(x & 0xffU);  x>>=8;
			*(d--)=(x & 0xffU);  x>>=8;
			*(d--)=(x & 0xffU);  x>>=8;
			*(d--)=(x & 0xffU);  x>>=8;
			*(d  )= x;
			return(r);
		}
		// ntohs/ntohl for 64bit integers: 
		static inline u_int64_t ntohll(u_int64_t x)
		{
			u_int64_t r;
			unsigned char *s=(unsigned char*)&x;
			r =u_int64_t(*(s++));  r<<=8;
			r|=u_int64_t(*(s++));  r<<=8;
			r|=u_int64_t(*(s++));  r<<=8;
			r|=u_int64_t(*(s++));  r<<=8;
			r|=u_int64_t(*(s++));  r<<=8;
			r|=u_int64_t(*(s++));  r<<=8;
			r|=u_int64_t(*(s++));  r<<=8;
			r|=u_int64_t(*(s  ));
			return(r);
		}
};

#endif  /* _RNDV_LIB_NETWORKIOBASE_HPP_ */

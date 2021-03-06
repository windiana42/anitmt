/*
 * ldrproto.hpp
 * LDR protocol header. 
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

#ifndef _RNDV_LIB_LDRPROTO_HPP_
#define _RNDV_LIB_LDRPROTO_HPP_ 1

#include "prototypes.hpp"
#include <netinet/in.h>

class HTime;

typedef unsigned char uchar;


namespace LDR
{

// Default LDR port (client side): 
extern const int DefaultLDRPort;

// LDR protocol version used by this version of RendView: 
extern const u_int16_t LDRProtocolVersion;


// PROTOCOL:
// SERVER                        CLIENT
//    ----------<connect>----------->
//   <----[LDRChallengeRequest]-----
//    ----[LDRChallengeResponse]---->
//   <------[LDRNowConnected]-------

//    -------[LDRTaskRequest]------->
//   <-------[LDRFileRequest]-------
//    ------[LDRFileDownload]------->
//    ---------[...data...]--------->
//   <------[LDRTaskResponse]-------

//   <--------[LDRTaskDone]---------
//   <-------[LDRFileUpload]--------
//   <---------[...data...]---------
//   <------[LDRDoneComplete]-------

enum LDRCommand
{
	// MAKE SURE THAT THE COMMAND NUMBERS ARE 0..._Cmd_LAST. 
	// WHEN MODIFYING; ALSO CHECK ARRAY IN ldrproto.cpp. 
	Cmd_NoCommand=0x0000,
	
	Cmd_ChallengeRequest,
	Cmd_ChallengeResponse,
	Cmd_NowConnected,     // challenge resp okay, now server -OR-
	                      // illegal challenge resp or already connected
	Cmd_TaskRequest,      // please do that task: blahblah...
	Cmd_TaskResponse,     // task accepted or refused (e.g. unknown 
	                      // renderer, filter, image format...)
	
	Cmd_FileRequest,      // need files x,y,z
	Cmd_FileDownload,     // here is file x... (server -> client)
	
	Cmd_FileUpload,       // client uploads results
	
	Cmd_TaskDone,         // task done
	Cmd_DoneComplete,     // task done; all complete
	
	Cmd_ControlRequest,   // interrupt, stop, cont, etc...
	Cmd_ControlResponse,  // notify server of processed control request
	
	_Cmd_LAST   // MUST BE LAST
};

extern const char *LDRCommandString(LDRCommand c);


struct LDRHeader
{
	u_int32_t length;   // length of header and body until next header
	u_int16_t command;  // Type of arriving packet
}__attribute__((__packed__));


// LDRTime: 
// Modified standard unix time format: msec since 1970. 
typedef u_int64_t LDRTime;

// Time conversion. (Can deal with HTime::Invalid.) 
// Note that LDRTime is always in network order. 
extern void LDRTime2HTime(const LDRTime *t,HTime *h);
extern void HTime2LDRTime(const HTime *h,LDRTime *t);

// To convert msec (interval) timers which are -1 if disabled. 
static inline long LDR_timeout_ntoh(u_int32_t x)
	{  return((x==0xffffffffU) ? (-1) : long(ntohl(x)));  }
static inline u_int32_t LDR_timeout_hton(long x)
	{  return(x<0 ? 0xffffffffU : htonl(u_int32_t(x)));  }

// FrameClockVal: UNUSED
#if 0
// Double conversion: 
// Range: +/- 9.8e19, precision: bettern than 1ppm (2.4e-7): 
// Return value: 
//   0 -> OK 
//   +/-1 -> out of range   (only DoubleToInt32())
//   +/-2 -> val is +/-Inf, 
//   3 -> val is NaN
// (Note that Inf anf NaN can be converted to u_int32_t and back 
// again; overflow is converted to Inf.)
// NOTE: u_int32_t value is converted to/from network order. 
extern int DoubleToInt32(double val,u_int32_t *x);
extern int Int32ToDouble(u_int32_t x,double *val);
// This can be used to test / set NaN as u_int32_t - double val: 
#define Tnt32Double_NAN      (htonl(0x7ffffffeU))
#endif

#define LDRIDStringLength 18
#define LDRChallengeLength 22   /* originally 16, but 22 is better for packet alignment... */
#define LDRChallengeRespLength 20   /* Yes, SHA hash size */

struct LDRChallengeRequest : LDRHeader
{
	uchar id_string[LDRIDStringLength];  // padded with zeros
	u_int16_t protocol_vers;   // protocol version spoken by the client
	uchar challenge[LDRChallengeLength];  // the actual challenge
}__attribute__((__packed__));

struct LDRChallengeResponse : LDRHeader
{
	uchar id_string[LDRIDStringLength];  // padded with zeros
	uchar response[LDRChallengeRespLength];
	u_int32_t keepalive_msec;   // server keepalive time or 0xffffffffU to disable
	LDRTime current_time;       // current time on server side
}__attribute__((__packed__));


// Store RendView ID string in id_dest of size len: 
extern void LDRSetIDString(char *id_dest,size_t len);

// Simply fill random challenge data into passed buffer of 
// length LDRChallengeLength. 
extern void LDRGetChallengeData(uchar *buf);

// resp_buf: buffer of size LDRChallengeRespLength. 
// passwd may be NULL for -none-. 
extern void LDRComputeCallengeResponse(uchar *challenge,char *resp_buf,
	const char *passwd);

enum  // ConnectionAuthCode
{
	CAC_Success=0,         // MUST BE 0. 
	CAC_AuthFailed,        // illegal challenge resp. 
	CAC_AlreadyConnected
};

// NOTE: In case of error (auth_code != 0), 
//       the pack is truncated after auth_code. 
struct LDRNowConnected : LDRHeader
{
	u_int16_t auth_code;  // CAC_*
	//----------------------<only if successful>----------------------
	LDRTime current_time;  // current time on client side
	LDRTime starttime;  // when client was started
	LDRTime time_diff_min;  // server-client time diff (min) 
	LDRTime time_diff_max;  // server-client time diff (max) 
	u_int16_t njobs;   // njobs param as passed on startup of client
	u_int16_t task_thresh_high;  // no more than this many tasks will be assigned to client
	u_int16_t loadval;  // loadavg*100 (0xffff for unknown)
	u_int16_t work_cycle_count;  // this work cycle number for client
	LDRTime idle_elapsed;  // idle time since start / last work cycle end
	// Could pass a bit more like unsuccessful connection attempts, etc. 
	u_int16_t r_descs_size;
	u_int16_t f_descs_size;
	// Following: \0-separated string lists of size r_descs_size, f_descs_size. 
	u_int32_t _padding64;   // padding for 64bit if !packed. 
	uchar data[0];
}__attribute__((__packed__));


struct LDRFileInfoEntry
{
	LDRTime mtime;   // in msec since 1970
	u_int64_t size;    // actally 32 bit would also do it, right?
	u_int16_t name_slen;  // length of file name (without '\0')
	uchar name[0];   // Actual file name entry without '\0'. 
}__attribute__((__packed__));

enum //TaskRequestFlags
{
	TRRF_Unfinished=0x0001,    // output is unfinished (-> request output)
	TRRF_ResumeFlagActive=0x0002,  // render resume flag set, i.e. it makes 
			// sense to upload unfinished frame
	TRRF_UseFrameClock=0x0004  // use POVRay frame clock...
};

// ALL PASSED STRINGS ARE NOT '\0' - TERMINATED. 

struct LDRTaskRequest : LDRHeader
{
	u_int16_t _padding;
	
	u_int32_t frame_no;
	u_int32_t task_id;   // unique task ID
	
	// Render task: -64bit-boundary-
	u_int16_t r_width;     // width of image
	u_int16_t r_height;    // height of image
	u_int32_t r_timeout;   // render timeout (seconds; 0xffffffff to disable)
	u_int32_t r_add_args_size;  // size of additional args (each \0-terminated)
	u_int16_t r_desc_slen;    // length of rdesc string (0 -> no render task)
	u_int16_t r_oformat_slen;   // length of oformat string
	// FrameClockVal: UNUSED  -> need layout change for word border issues
	//u_int32_t r_frame_clock;  // frame clock value or NaN, see DoubleToInt32()
	u_int64_t r_in_file_size;   // render input file size or -1
	LDRTime   r_in_file_mtime;  // render input file mtime or HTime::Invalid
	u_int16_t r_iofile_slen;   // length of input/outout file name
	u_int16_t r_flags;       // flags, see above, TaskRequestFlags
	
	// Filter task: 
	u_int32_t f_timeout;   // render timeout (seconds; 0xffffffff to disable)
	u_int64_t f_in_file_size;   // filter input file size or -1
	LDRTime   f_in_file_mtime;  // filter input file mtime or HTime::Invalid
	u_int32_t f_add_args_size;  // size of additional args (each \0-terminated)
	u_int16_t f_desc_slen;    // length of fdesc string (0 -> no filter task)
	u_int16_t f_iofile_slen;   // length of input/output file name
	
	u_int16_t r_n_files;    // number of LDRFileInfoEntries (only required 
			// additional files in rdir; NOT render/filter input/output) 
	u_int16_t f_n_files;    // dito for fdir 
	
	u_int32_t _padding64;
	// <---- be sure we're on a 64bit boundary here. ----->
	
	uchar data[0];  // More data following: [ORDER: easiest parsable for client]
	// uchar r_in_and_out_file[r_iofile_slen]   '\0'-separated, but not terminated
	// uchar f_in_and_out_file[f_iofile_slen]   '\0'-separated, but not terminated
	// uchar rdesc[r_desc_slen]
	// uchar fdesc[f_desc_slen]
	// uchar oformat[oformat_slen]
	// uchar radd_args[r_add_args_size]
	// uchar fadd_args[f_add_args_size]
	// LDRFileInfoEntry[0]
	//    mtime,size,name_slen,
	//    name[name_slen]
	// LDRFileInfoEntry[1]
	//    ...
	// LDRFileInfoEntry[r_n_files+f_n_files-1]
}__attribute__((__packed__));


enum // TaskResponseCode
{
	TRC_Accepted=0,       // task accepted; will be processed later; 
	                      // (can download next task)
	TRC_UnknownRender,    // render desc unknown
	TRC_UnknownFilter,    // filter desc unknown
	TRC_UnknownROFormat,  // unknown render output image format
	TRC_TooManyTasks      // client thinks he already has enough tasks 
	                      // (should never happen)
};
extern char *LDRTaskResponseString(int resp_code);

struct LDRTaskResponse : LDRHeader
{
	u_int16_t resp_code;  // one of the TRC_* above
	u_int32_t task_id;    // unique task ID
	u_int32_t _padding54;   // for 64 bit if !packed
}__attribute__((__packed__));

enum // FileRequestFileType
{
	FRFT_None=0,  // illegal
	FRFT_RenderIn,
	FRFT_RenderOut,
	//FRFT_FilterIn,
	FRFT_FilterOut,
	FRFT_AddRender,
	FRFT_AddFilter
};
extern const char *FileRequestFileTypeString(int frf_type);

struct LDRFileRequest : LDRHeader
{
	u_int16_t file_type;  // FRFT_*
	u_int32_t task_id;    // unique task ID
	u_int16_t file_idx;   // for FRFT_Add{Render,Filter}
	u_int16_t _padding64;  // for 64 bit if !packed
}__attribute__((__packed__));

struct LDRFileDownload : LDRHeader
{
	// NOTE: This is just redundancy...
	// Actually, no pipelining is supported, so the LDRFileDownload is 
	// just the response to the previous LDRFileRequest. 
	u_int16_t file_type;  // FRFT_*
	u_int64_t size;       // file size
	u_int32_t task_id;    // unique task ID
	u_int16_t file_idx;   // for FRFT_Add{Render,Filter}
	u_int16_t _padding64;  // for 64 bit if !packed
}__attribute__((__packed__));


// This is the LDR analogon of struct TaskExecutionStatus. 
struct LDR_TaskExecutionStatus
{
	u_int16_t ttr;  // task termination reason  0xffff for "unset"
	u_int16_t signal;  // signal/exit code
	u_int16_t outfile_status;  // OFS_*
	u_int16_t _padding64;  // <- be sure on 64 bit boundary 
	// Make sure this stays on a 4byte boundary: 
	LDRTime starttime;
	LDRTime endtime;
	LDRTime utime;
	LDRTime stime;
}__attribute__((__packed__));

struct LDRTaskDone : LDRHeader
{
	u_int16_t _padding;
	
	// Identification: 
	u_int32_t frame_no;
	u_int32_t task_id;   // unique task ID
	
	// Be sure we are on an 64bit boundary for these: 
	LDR_TaskExecutionStatus rtes;
	LDR_TaskExecutionStatus ftes;
}__attribute__((__packed__));


struct LDRFileUpload : LDRHeader
{
	// NOTE: task_id is just redundancy...
	// Actually, no pipelining is supported, so the LDRFileUpload is 
	// always referring to the last LDRTaskDone. 
	u_int16_t file_type;  // FRFT_*
	u_int64_t size;       // file size
	u_int32_t task_id;    // unique task ID
	u_int32_t _padding64;  // for 64 bit if !packed
}__attribute__((__packed__));


struct LDRDoneComplete : LDRHeader
{
	u_int16_t _padding;
	
	// Identification: 
	u_int32_t task_id;   // unique task ID
	u_int32_t _padding64;  // for 64 bit if !packed
}__attribute__((__packed__));


// NOTE: DO NOT MIX UP WITH TaskSource::ClientControlCommand. 
enum LDRClientControlCommand
{
	LCCC_None=0,
	
	LCCC_ClientQuit,
	
	LCCC_Kill_UserInterrupt,
	LCCC_Kill_ServerError,
	LCCC_StopJobs,
	LCCC_ContJobs,
	
	LCCC_PingPong,     // Like a IP ping which has to be sent back. 
	
	// give back tasks: 
	// request has data: LDRCCC_GBT_Req_Data
	// response has data: LDRCCC_GBT_Resp_Data
	LCCC_GiveBackTasks,
	
	// Client notifications: 
	LCCC_CN_NoMoreTasks    // client wants no more tasks
};

extern const char *LDRClientControlCommandString(LDRClientControlCommand cccmd);


struct LDRClientControlRequest : LDRHeader
{
	u_int16_t ctrl_cmd;  // one of the LCCC_* above
	u_int16_t seq;       // sequence number
	u_int16_t _padding;
	u_int32_t _padding64;  // for 64 bit if !packed
	uchar data[0];       // more data depending on ctrl_cmd
}__attribute__((__packed__));

struct LDRClientControlResponse : LDRHeader
{
	u_int16_t ctrl_cmd;  // one of the LCCC_* above
	u_int16_t seq;       // sequence number
	u_int16_t _padding;
	u_int32_t _padding64;  // for 64 bit if !packed
	uchar data[0];       // more data depending on ctrl_cmd
}__attribute__((__packed__));

// Data part for LCCC_GiveBackTasks request: 
struct LDRCCC_GBT_Req_Data
{
	// Number of not-yet-processed tasks the client may keep:
	u_int32_t may_keep;
}__attribute__((__packed__));

// Data part for LCCC_GiveBackTasks response: 
struct LDRCCC_GBT_Resp_Data
{
	// Number of tasks in queues of the client at the time 
	// directly after processing the give back request (i.e. 
	// the tasks to be given back are all in done list). 
	u_int32_t n_todo;
	u_int32_t n_proc;
	u_int32_t n_done;
}__attribute__((__packed__));


// If passwd.str() is NULL or has zero length, default password if 
// specified, otherwise none. (Calls passwd.deref() if no passwd.) 
// Get pass using getpass(prompt) if passwd is "prompt". 
// If passwd is "none", use no password (-> passwd.deref()). 
// If passwd is "file:path", read max 128 bytes from path as passwd. 
extern void LDRGetPassIfNeeded(RefString *passwd,const char *prompt,
	RefString *defpass=NULL);

// Call this to do structure size checks for the LDR "packets" 
// (tests against alignment issues). 
// Returns on failure only if return_fail is set. 
extern int LDRCheckCorrectLDRPaketSizes(int return_fail=0);

}  // end of namespace LDR

#endif  /* _RNDV_LIB_LDRPROTO_HPP_ */

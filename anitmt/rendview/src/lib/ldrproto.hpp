/*
 * ldrproto.hpp
 * LDR protocol header. 
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

#ifndef _RNDV_LIB_LDRPROTO_HPP_
#define _RNDV_LIB_LDRPROTO_HPP_ 1

#include <hlib/prototypes.h>

class HTime;

namespace LDR
{

extern const u_int16_t LDREndMagic;

// Default LDR port (client side): 
extern const int DefaultLDRPort;

// LDR protocol version used by this version of RendView: 
extern const u_int16_t LDRProtocolVersion;


typedef unsigned char uchar;


// PROTOCOL:
// SERVER                        CLIENT
//    ----------<connect>----------->
//   <----[LDRChallengeRequest]-----
//    ----[LDRChallengeResponse]---->
//   <------[LDRNowConnected]-------

//    -------[LDRTaskRequest]------->
//   <-------[LDRFileRequest]-------
//    ------[LDRFileDownload]------->
//   <------[LDRTaskResponse]-------

enum LDRCommand
{
	// MAKE SURE THAT THE COMMAND NUMBERS ARE 0..._Cmd_LAST. 
	Cmd_NoCommand=0x0000,
	Cmd_QuitNow,          // ONLY server -> client
	
	Cmd_ChallengeRequest,
	Cmd_ChallengeResponse,
	Cmd_NowConnected,     // challenge resp okay, now server -OR-
	                      // illegal challenge resp or already connected
	Cmd_TaskRequest,      // please do that task: blahblah...
	Cmd_TaskResponse,     // task accepted or refused (e.g. unknown 
	                      // renderer, filter, image format...)
	
	Cmd_FileRequest,      // need files x,y,z
	Cmd_FileDownload,     // here is file x... (server -> client)
	
	Cmd_TaskState,        // working, killed (?)
	Cmd_TaskDone,         // task done
	Cmd_SpecialTaskRequest,  // e.g. interrupt, stop, cont
	
	_Cmd_LAST   // MUST BE LAST
};

const char *LDRCommandString(LDRCommand c);


struct LDRHeader
{
	u_int32_t length;   // length of header and body until next header
	u_int16_t command;  // Type of arriving packet
}__attribute__((__packed__));


// LDRTime: 
// Modified standard unix time format: msec since 1970. 
typedef u_int64_t LDRTime;

// Time conversion. 
// Note that LDRTime is always in network order. 
void LDRTime2HTime(const LDRTime *t,HTime *h);
void HTime2LDRTime(const HTime *h,LDRTime *t);

#define LDRIDStringLength 18
#define LDRChallengeLength 16
#define LDRChallengeRespLength 20   /* Yes, SHA hash size */

struct LDRChallengeRequest : LDRHeader
{
	uchar id_string[LDRIDStringLength];  // padded with zeros
	u_int16_t protocol_vers;   // protocol version spoken by the client
	uchar challenge[LDRChallengeLength];  // the actual challenge
};

struct LDRChallengeResponse : LDRHeader
{
	uchar id_string[LDRIDStringLength];  // padded with zeros
	uchar response[LDRChallengeRespLength];
};


// Store RendView ID string in id_dest of size len: 
extern void LDRSetIDString(char *id_dest,size_t len);

// resp_buf: buffer of size LDRChallengeRespLength. 
// passwd may be NULL for -none-. 
extern void LDRComputeCallengeResponse(LDRChallengeRequest *d,char *resp_buf,
	const char *passwd);


enum  // ConnectionAuthCode
{
	CAC_Success=0,
	CAC_AuthFailed,        // illegal challenge resp. 
	CAC_AlreadyConnected
};

// NOTE: In case of error (auth_code != 0), 
//       the pack is truncated after auth_code. 
struct LDRNowConnected : LDRHeader
{
	u_int16_t auth_code;  // CAC_*
	LDRTime starttime;  // when client was started
	u_int16_t njobs;   // njobs param as passed on startup of client
	u_int16_t loadval;  // loadavg*100 (0xffff for unknown)
	// Could pass a bit more like unsuccessful connection attempts, etc. 
};


struct LDRQuitNow : LDRHeader
{
	// NOTE!! MAY NOT BE LONGER THAN ANY OTHER LDR "PACKET". 
	//        --EVEN WORSE: it may not contain data. 
};


struct LDRFileInfoEntry
{
	u_int16_t name_slen;  // length of file name (without '\0')
	LDRTime mtime;   // in msec since 1970
	u_int64_t size;    // actally 32 bit would also do it, right?
	uchar name[0];   // Actual file name entry without '\0'. 
};

// ALL PASSED STRINGS ARE NOT '\0' - TERMINATED. 

struct LDRTaskRequest : LDRHeader
{
	u_int16_t _padding;
	
	u_int32_t frame_no;
	u_int32_t task_id;   // unique task ID
	
	// Render task: 
	u_int16_t r_width,r_height;   // of image
	u_int16_t r_desc_slen;    // length of rdesc string (0 -> no render task)
	u_int32_t r_timeout;   // render timeout (seconds; 0xffffffff to disable)
	u_int32_t r_add_args_size;  // size of additional args (each \0-terminated)
	u_int16_t r_oformat_slen;   // length of oformat string
	
	// Filter task: 
	u_int16_t f_desc_slen;    // length of fdesc string (0 -> no filter task)
	u_int32_t f_timeout;   // render timeout (seconds; 0xffffffff to disable)
	u_int32_t f_add_args_size;  // size of additional args (each \0-terminated)
	
	u_int16_t r_n_files;    // number of LDRFileInfoEntries (only required 
			// additional files in rdir; NOT render/filter input/output) 
	u_int16_t f_n_files;    // dito for fdir 
	
	uchar data[0];  // More data following: [ORDER: easiest parsable for client]
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
};


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

struct LDRTaskResponse : LDRHeader
{
	u_int16_t resp_code;  // one of the TRC_* above
	u_int32_t task_id;   // unique task ID
};


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

struct LDRFileRequest : LDRHeader
{
	u_int16_t file_type;  // FRFT_*
	u_int32_t task_id;    // unique task ID
	u_int16_t file_idx;   // for FRFT_Add{Render,Filter}
};

struct LDRFileDownload : LDRHeader
{
	// NOTE: This is just redundancy...
	// Actually, no pipelining is supported, so the LDRFileDownload is 
	// just the response to the previous LDRFileRequest. 
	u_int16_t file_type;  // FRFT_*
	u_int64_t size;       // file size
	u_int32_t task_id;    // unique task ID
	u_int16_t file_idx;   // for FRFT_Add{Render,Filter}
};


// If passwd.str() is NULL or has zero length, default password if 
// specified, otherwise none. (Calls passwd.deref() if no passwd.) 
// Get pass using getpass(prompt) if passwd is "prompt". 
// If passwd is "none", use no password (-> passwd.deref()). 
extern void LDRGetPassIfNeeded(RefString *passwd,const char *prompt,
	RefString *defpass=NULL);

}  // end of namespace LDR

#endif  /* _RNDV_LIB_LDRPROTO_HPP_ */

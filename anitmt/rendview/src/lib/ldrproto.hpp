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
// 

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
	Cmd_TaskRefused,      // e.g. unknown renderer
	Cmd_FileRequest,      // need files x,y,z
	Cmd_TaskState,        // working, killed (?)
	Cmd_TaskDone,         // task done
	Cmd_FileTransmission, // here is file x...
	Cmd_SpecialTaskRequest,  // e.g. interrupt, stop, cont
	
	_Cmd_LAST,   // MUST BE LAST
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
//   the packet is truncated after 
struct LDRNowConnected : LDRHeader
{
	u_int16_t auth_code;  // CAC_*
	u_int16_t njobs;   // njobs param as passed on startup of client
	LDRTime starttime;  // when client was started
	// Could pass a lot more like unsuccessful connection attempts, etc. 
};


struct LDRQuitNow : LDRHeader
{
	// NOTE!! MAY NOT BE LONGER THAN ANY OTHER LDR "PACKET". 
	//        --EVEN WORSE: it may not comtain data. 
};


struct LDRFileInfoEntry
{
	u_int64_t mtime;   // in msec since 1970
	u_int64_t size;    // actally 32 bit would also do it, right?
	u_int16_t name_slen;  // length of file name (without '\0')
	uchar name[0];
};

// ALL PASSED STRINGS ARE NOT '\0' - TERMINATED. 

struct LDRDoTask : LDRHeader
{
	u_int32_t frame_no;
	u_int16_t width,height;   // of image
	u_int16_t oformat_slen;   // length of oformat string
	u_int16_t n_files;    // number of LDRFileInfoEntries
	u_int16_t n_add_args;
	
	
	uchar data[0];  // More data following: 
	// unsigned char oformat[oformat_slen]
	// LDRFileInfoEntry[0]
	//    mtime,size,name_slen,
	//    name[name_slen]
	// LDRFileInfoEntry[1]
	//    ...
	// LDRFileInfoEntry[n_files-1]
	//    ...
	// additional args
};

}  // end of namespace LDR

#endif  /* _RNDV_LIB_LDRPROTO_HPP_ */

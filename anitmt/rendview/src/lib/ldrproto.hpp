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

namespace LDR
{

extern const u_int16_t LDREndMagic;

typedef unsigned char uchar;


// PROTOCOL:
// SERVER                        CLIENT
//    ----------<connect>----------->
//   <----[LDRChallengeRequest]-----
//    ----[LDRChallengeResponse]---->
// 

enum LDRCommand
{
	LDR_NoCommand=0x0000,
	LDR_ChallengeRequest,
	LDR_ChallengeResponse,
	LDR_NowConnected,     // challenge resp okay, now server
	LDR_ConnectionRefused,  // illegal challenge resp or already connected
	LDR_TaskRequest,      // please do that task: blahblah...
	LDR_TaskRefused,      // e.g. unknown renderer
	LDR_FileRequest,      // need files x,y,z
	LDR_TaskState,        // working, killed (?)
	LDR_TaskDone,         // task done
	LDR_FileTransmission, // here is file x...
	LDR_SpecialTaskRequest,  // e.g. interrupt, stop, cont
};

struct LDRHeader
{
	u_int32_t length;   // length of header and body until next header
	u_int16_t command;  // Type of arriving packet
	// These can be used to implement pipelining. Note that they 
	// roll back to 0 after 65535. 
	u_int16_t seq_no;   // sequence number of packet
	u_int16_t ack_no;   // acknowlege number 
};


// LDRTime: 
// Modified standard unix time format: msec since 1970. 
typedef u_int64_t LDRTime;


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


struct LDRNowConnected : LDRHeader
{
	u_int16_t njobs;   // njobs param as passed on startup of client
	LDRTime starttime;  // when client was started
	// Could pass a lot more like unsuccessful connection attempts, etc. 
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

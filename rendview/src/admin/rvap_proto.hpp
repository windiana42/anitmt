/*
 * rvap_proto.hpp
 * 
 * Header for for the RendView Admin (network) Protocol structures. 
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

#ifndef _RNDV_ADMIN_RVADMINPROTO_HPP_
#define _RNDV_ADMIN_RVADMINPROTO_HPP_ 1

// Including LDR prototypes here because we're using some of the LDR 
// functions and constants. 
#include <lib/ldrproto.hpp>


namespace RVAP
{

// RVAP protocol version used by this version of RendView: 
extern const u_int16_t RVAPProtocolVersion;

// Packet types for RVAP: Do not use 0. 
enum
{
	RVAP_ChallengeRequest=1,
	RVAP_ChallengeResponse,
	RVAP_NowConnected,     // ...which is simply a header and nothing more. 
	RVAP_NoOp,             // Just a no-op. Simply an RVAPHeader for the idle timer. 
	RVAP_CommandString,    // command string to execute
	RVAP_CommandResp,      // command response
	RVAP_Message,          // RendView message (verbose,...)
};

struct RVAPHeader
{
	u_int32_t length;   // length of header and body until next header
	u_int16_t ptype;    // Type of arriving packet
}__attribute__((__packed__));


#define RVAPIDStringLength      20
#define RVAPChallengeLength     LDRChallengeLength
#define RVAPChallengeRespLength LDRChallengeRespLength

struct RVAPChallengeRequest : RVAPHeader
{
	uchar id_string[RVAPIDStringLength];  // padded with zeros
	u_int16_t protocol_vers;   // protocol version spoken by the "server"
	u_int32_t idle_msec;   // "server" idle disconnect time or 0xffffffffU to disable
	uchar challenge[RVAPChallengeLength];  // the actual challenge
}__attribute__((__packed__));

struct RVAPChallengeResponse : RVAPHeader
{
	uchar id_string[RVAPIDStringLength];  // padded with zeros
	uchar response[RVAPChallengeRespLength];
}__attribute__((__packed__));

// We use these from LDR: 
#define RVAPSetIDString(s,l)                LDR::LDRSetIDString(s,l)
#define RVAPGetChallengeData(x)             LDR::LDRGetChallengeData(x)
#define RVAPComputeCallengeResponse(c,r,p)  LDR::LDRComputeCallengeResponse(c,r,p)

struct RVAPNowConnected : RVAPHeader
{
	// empty, currently
}__attribute__((__packed__));


struct RVAPCommandString : RVAPHeader
{
	// Beginning of command string; length is known from packet length. 
	// No NUL-termination. 
	uchar command_str[0];
}__attribute__((__packed__));

struct RVAPCommandResp : RVAPHeader
{
	// Beginning of response; length is known from packet length. 
	// No NUL-termination. 
	uchar response[0];
}__attribute__((__packed__));

struct RVAPMessage : RVAPHeader
{
	//u_int16_t mtype;
	//RVAPTime time;
	//uchar message[0];
}__attribute__((__packed__));

}  // end of namespace RVAP

#endif  /* _RNDV_ADMIN_RVADMINPROTO_HPP_ */

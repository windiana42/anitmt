/*
 * ldrproto.cpp
 * LDR protocol constants and functions. 
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

#include "ldrproto.hpp"

#include <hlib/htime.h>

#include <assert.h>
#include <netinet/in.h>


namespace LDR
{

const int DefaultLDRPort=3104;

const u_int16_t LDREndMagic=0xdeadU;

const u_int16_t LDRProtocolVersion=0x001;


static const char *_command_str[_Cmd_LAST]=
{
	"[no command]",
	"quit",
	"challenge request",
	"challenge response",
	"now connected",
	"task request",
	"task refused",
	"file request",
	"task state",
	"task done",
	"file transmission",
	"special task request"
};

const char *LDRCommandString(LDRCommand c)
{
	return((c<0 || c>=_Cmd_LAST) ? "[unknown]" : _command_str[c]);
}


// Time conversion. 
// Note that LDRTime is always in network order. 
void LDRTime2HTime(const LDRTime *t,HTime *h)
{
	const u_int32_t *d=(u_int32_t*)t;
	u_int64_t tmp=ntohl(*(d++));
	tmp|=u_int64_t(ntohl(*d))<<32;
	h->SetL(tmp,HTime::msec);
}

void HTime2LDRTime(const HTime *h,LDRTime *t)
{
	int64_t tmp=h->GetL(HTime::msec);
	assert(tmp>=0);
	u_int32_t *d=(u_int32_t*)t;
	*(d++)=htonl(tmp & 0xffffffffU);
	* d   =htonl(tmp >> 32);
}


// Store RendView ID string in id_dest of size len: 
void LDRSetIDString(char *id_dest,size_t len)
{
	const char *id_string="RendView-" VERSION;
	strncpy(id_dest,id_string,len);  // does padding with nulls which is what we want. 
}


// resp_buf: buffer of size LDRChallengeRespLength. 
// passwd may be NULL for -none-. 
void LDRComputeCallengeResponse(LDRChallengeRequest *d,char *resp_buf,
	const char *passwd)
{
	// For empty passwords, the challenge is simply copied and 
	// the remainder is padded with zeros. 
	if(!passwd)
	{
		memcpy(resp_buf,d->challenge,
			LDRChallengeLength<LDRChallengeRespLength ? 
				LDRChallengeLength : LDRChallengeRespLength);
		if(LDRChallengeRespLength>LDRChallengeLength)
		{  memset(resp_buf+LDRChallengeLength,0,
			LDRChallengeRespLength-LDRChallengeLength);  }
		return;
	}
	
	// For non-empty passwords, a hash is computed: 
	// Feed challenge and password several times into hash...
	size_t pwl=strlen(passwd);
	for(int i=0; i<17; i++)
	{
		#warning Challenge response not yet implemented. 
		// h.Feed(d->challenge,LDRChallengeLength);
		// h.Feed(passwd,pwl);
	}
	// h.Final();
	// assert(h.hash_len==LDRChallengeRespLength);
	// h.Get(resp_buf);
}

}  // namespace end

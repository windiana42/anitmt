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

#include "prototypes.hpp"  /* NEEDED for config.h / VERSION */
#include "ldrproto.hpp"

#include <hlib/htime.h>
#include <hlib/refstring.h>

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
	"task response",
	
	"file request",
	"file download",
	
	"task state",
	"task done",
	"special task request"
};

const char *LDRCommandString(LDRCommand c)
{
	if(c<0 || c>=_Cmd_LAST)  return("[unknown]");
	return(_command_str[c]);
}


char *LDRTaskResponseString(int resp_code)
{
	switch(resp_code)
	{
		case TRC_Accepted:         return("task accepted (working)");
		case TRC_UnknownRender:    return("unknown render desc");
		case TRC_UnknownFilter:    return("unknown filter desc");
		case TRC_UnknownROFormat:  return("unknown render output format");
		case TRC_TooManyTasks:     return("too many tasks");
	}
	return("???");
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
		
		// THIS IS FOR TESTING ONLY: COPY PASSWORD: 
		strncpy(resp_buf,passwd,LDRChallengeRespLength);
		
		// h.Feed(d->challenge,LDRChallengeLength);
		// h.Feed(passwd,pwl);
	}
	// h.Final();
	// assert(h.hash_len==LDRChallengeRespLength);
	// h.Get(resp_buf);
}


// If passwd.str() is NULL or has zero length, default password if 
// specified, otherwise none. (Calls passwd.deref() if no passwd.) 
// Get pass using getpass(prompt) if passwd is "prompt". 
// If passwd is "none", use no password (-> passwd.deref()). 
void LDRGetPassIfNeeded(RefString *passwd,const char *prompt,RefString *defpass)
{
	if(defpass && (!passwd->str() || *(passwd->str())=='\0') )
	{  *passwd=*defpass;  }
	if(!passwd->str() || *(passwd->str())=='\0')
	{
		passwd->deref();
		return;
	}
	
	// pssword->str() NOT NULL here. 
	if(!strcmp(passwd->str(),"prompt"))
	{
		char *pass=getpass(prompt);
		if(*pass)
		{
			if(passwd->set(pass))
			{  Error("Allocation failure.\n");  abort();  }
		}
		else
		{  passwd->deref();  }
	}
	else if(!strcmp(passwd->str(),"none"))
	{
		passwd->deref();
	}
}

}  // namespace end

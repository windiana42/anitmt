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

#include <math.h>

#include "sha1hash.hpp"


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
	
	"file upload",
	
	"task done",
	"done complete",
	
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


const char *FileRequestFileTypeString(int frf_type)
{
	switch(frf_type)
	{
		case FRFT_None:       return("[none]");
		case FRFT_RenderIn:   return("render input");
		case FRFT_RenderOut:  return("render output");
		case FRFT_FilterOut:  return("filter output");
		case FRFT_AddRender:  return("additional render");
		case FRFT_AddFilter:  return("additional filter");
		// default: fall through
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


// Range: +/- 9.8e19, precision: bettern than 1ppm (2.4e-7):
// Return value: 
//   0 -> OK 
//   +/-1 -> out of range   (only DoubleToInt32())
//   +/-2 -> val is +/-Inf, 
//   3 -> val is NaN
// (Note that Inf anf NaN can be converted to u_int32_t and back 
// again; overflow is converted to Inf.)
int DoubleToInt32(double val,u_int32_t *x)
{
	u_int32_t sign;
	if(val<0.0)
	{  sign=1U<<31;  val=-val;  }
	else
	{  sign=0U;  }
	
	if(isinf(val))
	{  *x=htonl(0x7fffffffU | sign);  return(sign ? (-2) : (+2));  }
	if(isnan(val))
	{  *x=Tnt32Double_NAN;  return(3);  }
	
	if(val<=1e-15)  // This is treated as 0.0.
	{  *x=htonl(2000000U);  return(0);  }
	
	static const double log15=log(15.0);
	int ev=int(floor(log(val)/log15)+0.1);
	if(ev<-15)  // This is treated as 0.0.
	{  *x=htonl(2000000U);  return(0);  }
	if(ev>16)   // overflow
	{  *x=htonl(0x7fffffffU | sign);  return(sign ? (-1) : (+1));  }
	
	val*=pow(15.0,double(-ev));
	int32_t man=int(val*4.19e6 + 0.5)+2000000;
	assert(man>=0 && man<0x3fffffe);  // Note: ...ffe and above special
	
	*x=htonl(sign | (u_int32_t(ev+15)<<26) | u_int32_t(man));
	return(0);
}

int Int32ToDouble(u_int32_t x,double *val)
{
	x=ntohl(x);
	int man=int(x & 0x3ffffffU);
	int sign=(x>>31);
	if(man==0x3ffffff)
	{
		if(sign)
		{  *val=-HUGE_VAL;  return(-2);  }
		*val=HUGE_VAL;  return(+2);
	}
	if(man==0x3fffffe)
	{  *val=NAN;  return(3);  }
	man-=2000000;
	int ev=((x>>26) & 31)-15;
	double tmp=(man/4.19e6);
	if(ev)
	{  tmp*=pow(15.0,double(ev));  }
	*val = sign ? (-tmp) : tmp;
	return(0);
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
	SHA1Hash hash;
	for(int i=0; i<17; i++)
	{
		hash.Feed((char*)d->challenge,LDRChallengeLength);
		hash.Feed(passwd,pwl);
	}
	hash.Final();
	assert(hash.HashSize()==LDRChallengeRespLength);
	hash.GetHash(resp_buf);
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
			{  Error("%s.\n",cstrings.allocfail);  abort();  }
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

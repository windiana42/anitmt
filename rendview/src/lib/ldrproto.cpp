/*
 * ldrproto.cpp
 * LDR protocol constants and functions. 
 *
 * Copyright (c) 2002--2004 by Wolfgang Wieser (wwieser@gmx.de) 
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

#include <ctype.h>

#include <assert.h>
#include <netinet/in.h>

#include "mymath.hpp"

/*#if HAVE_FCNTL_H*/
# include <fcntl.h>
/*#endif*/


namespace LDR
{

const int DefaultLDRPort=3104;

const u_int16_t LDRProtocolVersion=0x0007;


const char *LDRCommandString(LDRCommand c)
{
	switch(c)
	{
		case Cmd_NoCommand:          return("[no command]");
		case Cmd_ChallengeRequest:   return("challenge request");
		case Cmd_ChallengeResponse:  return("challenge response");
		case Cmd_NowConnected:       return("now connected");
		case Cmd_TaskRequest:        return("task request");
		case Cmd_TaskResponse:       return("task response");
		case Cmd_FileRequest:        return("file request");
		case Cmd_FileDownload:       return("file download");
		case Cmd_FileUpload:         return("file upload");
		case Cmd_TaskDone:           return("task done");
		case Cmd_DoneComplete:       return("done complete");
		case Cmd_ControlRequest:     return("control request");
		case Cmd_ControlResponse:    return("control response");
		//default:  fall through
	}
	return("[unknown]");
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


const char *LDRClientControlCommandString(LDRClientControlCommand cccmd)
{
	switch(cccmd)
	{
		case LCCC_None:                return("[none]");
		case LCCC_ClientQuit:          return("client quit");
		case LCCC_Kill_UserInterrupt:  return("kill jobs (intr)");
		case LCCC_Kill_ServerError:    return("kill jobs (error)");
		case LCCC_StopJobs:            return("stop jobs");
		case LCCC_ContJobs:            return("cont jobs");
		case LCCC_PingPong:            return("ping");
		case LCCC_GiveBackTasks:       return("give back tasks");
		case LCCC_CN_NoMoreTasks:      return("CN no more tasks");
		// default: fall through
	}
	return("???");
}



// Time conversion. 
// Note that LDRTime is always in network order. 
void LDRTime2HTime(const LDRTime *t,HTime *h)
{
	if(*t==0xffffffffffffffffLLU)
	{  h->SetInvalid();  }
	else
	{
		const u_int32_t *d=(u_int32_t*)t;
		u_int64_t tmp=ntohl(*(d++));
		tmp|=u_int64_t(ntohl(*d))<<32;
		int64_t ctmp;
		if(tmp&0x8000000000000000LLU)
		{  ctmp=-int64_t(tmp&~0x8000000000000000LLU);  }
		else
		{  ctmp=int64_t(tmp);  }
		h->SetL(ctmp,HTime::msec);
	}
}

void HTime2LDRTime(const HTime *h,LDRTime *t)
{
	if(!h || h->IsInvalid())
	{  *t=0xffffffffffffffffLLU;  }
	else
	{
		int64_t tmp=h->GetLR(HTime::msec);
		u_int64_t ctmp;
		if(tmp<0LL)
		{
			ctmp=u_int64_t(-tmp);
			assert(!(ctmp&0x8000000000000000LLU));
			ctmp|=0x8000000000000000LLU;
		}
		else
		{  ctmp=u_int64_t(tmp);  }
		u_int32_t *d=(u_int32_t*)t;
		*(d++)=htonl(ctmp & 0xffffffffLLU);
		* d   =htonl(ctmp >> 32);
	}
}


// FrameClockVal: UNUSED
#if 0
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
#endif


// Store RendView ID string in id_dest of size len: 
void LDRSetIDString(char *id_dest,size_t len)
{
	const char *id_string="RendView-" VERSION;
	strncpy(id_dest,id_string,len);  // does padding with nulls which is what we want. 
}


// Simply fill random challenge data into passed buffer of 
// length LDRChallengeLength. 
void LDRGetChallengeData(uchar *buf)
{
	// NOTE: We could speed up this if we read e.g. 8 times the 
	//       number of requested bytes and cache it. 
	static const char *dev_urandom_path="/dev/urandom";
	static int have_dev_urandom=1;
	while(have_dev_urandom)
	{
		// First, try to read /dev/urandom. 
		int fd=open(dev_urandom_path,O_RDONLY);
		ssize_t rd=-100;
		if(fd<0)
		{
			Warning("Warning: Failed to open \"%s\": %s\n",
				dev_urandom_path,strerror(errno));
			goto fail;
		}
		
		if(SetNonblocking(fd)<0)
		{  goto fail;  }
		
		// We expect this read to be atomically. 
		rd=::read(fd,(char*)buf,LDRChallengeLength);
		if(rd<ssize_t(LDRChallengeLength))
		{  goto fail;  }
		
		// Okay, random challenge stored. 
		errno=0; int rv;
		do
		{  rv=close(fd);  }
		while(rv<0 && errno==EINTR);
		return;
		
		fail:;
		have_dev_urandom=0;
		if(fd>=0)
		{
			Warning("Warning: Failed to use %s (rd=%d).\n",
				dev_urandom_path,int(rd));
			errno=0; int rv;
			do
			{  rv=close(fd);  }
			while(rv<0 && errno==EINTR);
		}
		break;
	}
	
	// Compute semi-random challenge by other means: 
	HTime curr(HTime::Curr);
	unsigned long usec=curr.Get(HTime::usec);
	srandom(usec*(getpid()+5LU));
	long rv;  // NOT uninitialized, even if compiler warns about that. 
	for(int i=0; i<LDRChallengeLength; i++)
	{
		rv = (i%3) ? (rv>>8) : random();
		buf[i]=(unsigned char)(rv & 0xff);
	}
}


// If passwd.str() is NULL or has zero length, default password if 
// specified, otherwise none. (Calls passwd.deref() if no passwd.) 
// Get pass using getpass(prompt) if passwd is "prompt". 
// If passwd is "none", use no password (-> passwd.deref()). 
// If passwd is "file:path", read max 128 bytes from path as passwd. 
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
	bool do_prompt=0;
	if(!strncmp(passwd->str(),"file:",5))
	{
		bool okay=0;
		const char *file=passwd->str()+5;
		while(isspace(*file))  ++file;
		int fd=-1;
		do {
			if(!*file)
			{
				Error("Password spec \"file:\": file name omitted.\n");
				break;
			}
			
			fd=open(file,O_RDONLY);
			if(fd<0)
			{
				Error("Failed to open password file \"%s\": %s\n",
					file,strerror(errno));
				break;
			}
			
			const size_t maxlen=128;
			char buf[maxlen];
			ssize_t rd;
			errno=0;
			do
			{  rd=read(fd,buf,maxlen);  }
			while(rd<0 && errno==EINTR);
			if(rd<0)
			{
				Error("Failed to read password file \"%s\": %s\n",
					file,strerror(errno));
				break;
			}
			if(rd==0)
			{
				Error("Password file \"%s\" is empty.\n",file);
				break;
			}
			
			if(passwd->set0(buf,rd))
			{  Error("%s.\n",cstrings.allocfail);  abort();  }
			
			Verbose(DBG,"Okay, read %d password bytes from file \"%s\".\n",
				int(rd),file);
			
			memset(buf,0,rd);
			
			okay=1;
		} while(0);
		
		if(fd>=0)
		{  close(fd);  }
		
		if(!okay)
		{  do_prompt=1;  }
	}
	// NO ELSE. 
	if(do_prompt || !strcmp(passwd->str(),"prompt"))
	{
		char *pass=getpass(prompt);
		assert(pass);
		if(*pass)
		{
			if(passwd->set(pass))
			{  Error("%s.\n",cstrings.allocfail);  abort();  }
		}
		else
		{  passwd->deref();  }
		memset(pass,0,strlen(pass));
	}
	else if(!strcmp(passwd->str(),"none"))
	{
		passwd->deref();
	}
	// else -> passwd stays as it is. 
}


struct _SzEntry
{
	size_t size;
	size_t expect;
	LDRCommand cmd;  // Or use name below if Cmd_NoCommand
	const char *name;
	bool mfm8;   // may fail test if size is multiple of 8
};

const _SzEntry packsz[]=
{
	{ sizeof(LDRHeader), 6, Cmd_NoCommand, "LDR header", 1 },
	{ sizeof(LDRChallengeRequest), 6+LDRIDStringLength+2+LDRChallengeLength,
		Cmd_ChallengeRequest, NULL, 0 },
	{ sizeof(LDRChallengeResponse), 6+LDRIDStringLength+12+LDRChallengeRespLength,
		Cmd_ChallengeResponse, NULL, 0 },
	{ sizeof(LDRNowConnected), 6+58, Cmd_NowConnected, NULL, 0 },
	{ sizeof(LDRFileInfoEntry), 18, Cmd_NoCommand, "file info entry", 1 },
	{ sizeof(LDRTaskRequest), 6+82, Cmd_TaskRequest, NULL, 0 },
	{ sizeof(LDRTaskResponse), 6+10, Cmd_TaskResponse, NULL, 0 },
	{ sizeof(LDRFileRequest), 6+10, Cmd_FileRequest, NULL, 0 },
	{ sizeof(LDRFileDownload), 6+18, Cmd_FileDownload, NULL, 0 },
	{ sizeof(LDR_TaskExecutionStatus), 40, Cmd_NoCommand, "task exec status", 
		0 /*really!*/ },
	{ sizeof(LDRTaskDone), 6+10+2*sizeof(LDR_TaskExecutionStatus), 
		Cmd_TaskDone, NULL, 0 },
	{ sizeof(LDRFileUpload), 6+18, Cmd_FileUpload, NULL, 0 },
	{ sizeof(LDRDoneComplete), 6+10, Cmd_DoneComplete, NULL, 0 },
	{ sizeof(LDRClientControlRequest), 6+10, Cmd_ControlRequest, NULL, 0 },
	{ sizeof(LDRClientControlResponse), 6+10, Cmd_ControlResponse, NULL, 0 },
	{ sizeof(LDRCCC_GBT_Req_Data), 4, Cmd_NoCommand, "GBT req data", 1 },
	{ sizeof(LDRCCC_GBT_Resp_Data), 12, Cmd_NoCommand, "GBT resp data", 1 },
	{ 0,0, Cmd_NoCommand, NULL }
};

// Call this to do structure size checks for the LDR "packets" 
// (tests against alignment issues). 
// Returns on failure only if return_fail is set. 
int LDRCheckCorrectLDRPaketSizes(int return_fail)
{
	static int did_check=0;
	if(did_check)  return(0);
	did_check=1;
	
	int failed=0;
	for(int i=0; packsz[i].size; i++)
	{
		if(packsz[i].size!=packsz[i].expect)
		{
			Error("OOPS: Compilation/padding problem: size mismatch: "
				"actual=%u, expect=%u (%s)\n",
				packsz[i].size,packsz[i].expect,
				packsz[i].cmd==Cmd_NoCommand ? 
					packsz[i].name : LDRCommandString(packsz[i].cmd));
			++failed;
		}
		if(!packsz[i].mfm8 && (packsz[i].expect % 8)!=0)
		{
			Error("OOPS: Compilation/padding problem: expected size %u not "
				"multiple of 8 (%s)\n",
				packsz[i].expect,
				packsz[i].cmd==Cmd_NoCommand ? 
					packsz[i].name : LDRCommandString(packsz[i].cmd));
			++failed;
		}
	}
	if(failed && !return_fail)
	{
		Error("Aborting due to these padding problems. "
			"Please submit bug report.\n");
		abort();
	}
	return(failed);
}

}  // namespace end

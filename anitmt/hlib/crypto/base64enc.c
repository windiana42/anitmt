/*
 * base64enc.c
 *
 * Implementation of base64 encoding algorithm. 
 *
 * Copyright (c) 2001--2002 by Wolfgang Wieser (wwieser@gmx.de)
 *
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include "base64.h"

/* Maps the values in range 0...63 to base64 chars. */
static const unsigned char *_base64_encode=(unsigned char *)
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

/*
 * Reads in binary (8 bit) input data in _in of size inlen and encodes 
 * it as base64 storing the result in _out of size outlen. 
 * You may set lwidth to something non-zero to make Base64Encode() 
 * insert newlines every lwidth*4 chars. (lwidth=1 -> every 4 chars; 
 * lwidth=0 -> disable this feature)
 * If state is non-NULL state info is updated. 
 * State info (carry) is needed only if you have a large amount of 
 * 8bit data which cannot be encoded in one single call. 
 * To do this, set up the u_int32_t state value to zero and pass a 
 * pointer to it whenever calling Base64Encode(). The function will 
 * store non-encoded bytes (max. 2 bytes + 2 bit counter) in state 
 * to be able to process them the next time unless you call 
 * Base64Encode() with inlen=0 to tell it that this is the end of the 
 * input data. (state will be 0 after that but better relay on the 
 * return values for error diagnosis.) 
 * When reaching input data end (either state!=NULL and inlen=0 or 
 * state=NULL), the base64 termination char `=' is appended to the 
 * output. 
 * Note: the current byte quartett count is also stored in state as 
 *   14 bit value so that the base64 text stays formatted even if you 
 *   have to call Base64Encode() repeatedly. So, you don't try to 
 *   set lwidth to values >2^14 (that is 2^14*4=65536 chars per line). 
 * 
 * Return value: 
 *  >=0 -> number of bytes in output
 *   -4 -> output buffer too small 
 */
ssize_t Base64Encode(
	const char *_in,size_t inlen,
	char *_out,size_t outlen,
	u_int32_t *state/*=NULL*/,int lwidth/*=0*/)
{
	unsigned char *in=(unsigned char *)_in;
	unsigned char *out=(unsigned char *)_out;
	unsigned char *inend=in+inlen;
	unsigned char *outend=out+outlen;
	u_int32_t acc;
	int ai,ci;
	if(state)
	{
		acc=(*state)&0xffffU;
		ai=((*state)>>16)&0x3U;
		ci=((*state)>>18);
	}
	else
	{  acc=0U;  ai=0;  ci=0;  }
	
	for(;in<inend;in++)
	{
		acc<<=8;
		acc|=(u_int32_t)(*in);
		++ai;
		if(ai==3)
		{
			if(out+4>outend)
			{  return(-4);  }
			//for(int i=18; i>=0; i-=6)
			//{  *(out++)=_base64_encode[(acc>>i) & 0x3fU];  }
			*(out++)=_base64_encode[(acc>>18) & 0x3fU];
			*(out++)=_base64_encode[(acc>>12) & 0x3fU];
			*(out++)=_base64_encode[(acc>> 6) & 0x3fU];
			*(out++)=_base64_encode[ acc      & 0x3fU];
			ai=0;
			acc=0U;
			if(lwidth)  if((++ci)==lwidth)
			{
				if(out>=outend)  return(-4);
				*((char*)(out++))='\n';
				ci=0;
			}
		}
	}
	
	if(state && inlen)
	{
		/* save carry in state (ai=0,1,2) */
		*state = 
			(((u_int32_t)ci)<<18) | 
			(((u_int32_t)ai)<<16) | 
			acc;
	}
	else
	{
		if(state)  *state=0;
		
		/* store rest of input in output */
		if(outend-out<(ai+2))   /* +2 for expand & termination char */
		{  return(-4);  }
		
		/* ai==3 never possible here. */
		switch(ai)
		{
			case 1:
				*(out++)=_base64_encode[(acc>>2) & 0x3fU];
				*(out++)=_base64_encode[(acc<<4) & 0x3fU];
				break;
			case 2:
				*(out++)=_base64_encode[(acc>>10) & 0x3fU];
				*(out++)=_base64_encode[(acc>> 4) & 0x3fU];
				*(out++)=_base64_encode[(acc<< 2) & 0x3fU];
				break;
		}
		
		/* and append termination char */
		*((char*)(out++))='=';
	}
	
	return(out-(unsigned char*)_out);
}


#if 0   /* compile little test proggy */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#define OBLEN 256000
int main(int argc,char **arg)
{
	char ob[OBLEN];
	ssize_t rv;
	/*rv=Base64Encode(arg[1],strlen(arg[1]),ob,OBLEN,NULL,8);
	fprintf(stderr,"Return=%d\n",rv);
	if(rv>0)
	{  write(1,ob,rv);  printf("<<\n");  fflush(stdout);  }
	*/
	
	srandom(time(NULL)*getpid());
	
	{
		u_int32_t state=0;
		char *ibuf=arg[1];
		char *ibufend=ibuf+strlen(ibuf);
		char *obuf=ob;
		for(;;)
		{
			int inlen=(random()%32)+1;
			if(ibuf+inlen>ibufend)
			{  inlen=ibufend-ibuf;  }
			rv=Base64Encode(ibuf,inlen,obuf,ob+OBLEN-obuf,&state,8);
			if(rv<0)
			{  fprintf(stderr,"Dec(%d)=%d\n",inlen,rv);  }
			if(rv<0)  break;
			ibuf+=inlen;
			obuf+=rv;
			if(!inlen)  break;
		}
		
		write(1,ob,obuf-ob);  printf("<<\n");  fflush(stdout);
	}
	
	return(0);
}

#endif

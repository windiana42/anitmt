/*
 * base64dec.c
 *
 * Implementation of base64 decoding algorithm. 
 *
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de)
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <hlib/prototypes.h>


/* Maps the first 128 ASCII chars to their base64 code (in range 0...63) 
 * Special meaning: 
 * 0xff -> illegal
 * 0xfe -> ignored (space, CR, LF, tab, backslash, \f, \v, \b, DEL)
 * 0xfd -> end of message (`=') */
static unsigned char *_base64_decode=
	"\xff\xff\xff\xff\xff\xff\xff\xff\xfe\xfe\xfe\xfe\xfe\xfe\xff\xff"
	"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
	"\xfe\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x3e\xff\xff\xff\x3f"
	"\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\xff\xff\xff\xfd\xff\xff"
	"\xff\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e"
	"\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\xff\xfe\xff\xff\xff"
	"\xff\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28"
	"\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\xff\xff\xff\xff\xfe";

/*
 * Reads in the base64 encoded string passed in _in of size inlen and 
 * stores the decoded result in buffer _out of size outlen. 
 * If state is non-NULL state info is updated. 
 * State info (carry) is needed only if you have a large amount of 
 * base64 data which cannot be decoded in one single call. 
 * To do this, set up the u_int32_t state value to zero and pass a 
 * pointer to it whenever calling Base64Decode(). The function will 
 * store non-decoded chars (max. 18 bits + 2 bit counter) in state 
 * to be able to process them the next time unless 
 *  - the base64 termination char (`=') is encountered or
 *  - you call Base64Decode() with inlen=0 which does just the same 
 *    as if you merely pass "=" as input. 
 * (state will be 0 after reading `=' / when passig inlen=0 but 
 *  better relay on the return values for error diagnosis.)
 * Return value: 
 *  >=0 -> number of bytes in output
 *   -2 -> illegal char in input
 *   -3 -> premature end (only if state is NULL or base64 termination 
 *         char `=' is encountered too early)
 *   -4 -> output buffer too small 
 */
ssize_t Base64Decode(
	const char *_in,size_t inlen,
	char *_out,size_t outlen,
	u_int32_t *state /*=NULL*/)
{
	unsigned char *ib=(unsigned char*)_in;
	unsigned char *ib_end=ib+inlen;
	unsigned char *ob=(unsigned char*)_out;
	unsigned char *ob_end=ob+outlen;
	u_int32_t dec=0xfdU;   /* simulate end char in case inlen=0 */
	u_int32_t acc = state ? ((*state)&0xffffffU) : 0U;
	int       ai  = state ? ((*state)>>24) : 0;
	for(; ib<ib_end; ib++)
	{
		if((*ib)>=128U)  return(-2);
		dec=_base64_decode[(int)(*ib)];
		if(dec<64)
		{
			acc=(acc<<6)|dec;
			++ai;
			if(ai==4)  /* got 4 chars */
			{
				if(ob+2>=ob_end)  return(-4);
				*(ob++)=(unsigned char)((acc>>16) & 0xffU);
				*(ob++)=(unsigned char)((acc>> 8) & 0xffU);
				*(ob++)=(unsigned char)( acc      & 0xffU);
				acc=0U;  ai=0;
			}
		}
		else switch(dec)
		{
			case 0xffU:  return(-2);
			/*case 0xfeU:  goto forcont;*/
			case 0xfdU:  goto breakfor;
		}
		/*forcont:;*/
	}
	breakfor:;
	/* ai==4 not possible here. ai<4 here. */
	
	if(state && dec!=0xfdU)  /* state and no end char */
	{
		/* Save carry in state. */
		*state = (((u_int32_t)ai)<<24) | acc;
	}
	else
	{
		if(state)  *state=0U;
		/* Save the last bytes in the output buffer. */
		if(ob+ai-2>=ob_end)
		{  return(-4);  }
		switch(ai)
		{
			case 3:  /* got 3 chars */
				*(ob++)=(unsigned char)((acc>>10) & 0xffU);
				*(ob++)=(unsigned char)((acc>> 2) & 0xffU);
				break;
			case 2:  /* got 2 chars */
				*(ob++)=(unsigned char)((acc>> 4) & 0xffU);
				break;
			case 1: return(-3);
		}
	}
	return((char*)ob-_out);
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
	/*rv=Base64Decode(arg[1],strlen(arg[1]),ob,OBLEN,NULL);
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
			int inlen=(rand()%33)+1;
			if(ibuf+inlen>ibufend)
			{  inlen=ibufend-ibuf;  }
			rv=Base64Decode(ibuf,inlen,obuf,ob+OBLEN-obuf,&state);
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

/*
 * sha1hash.cpp 
 * 
 * Originally sha1hash.cc in HLib but copied for RendView. 
 * 
 * Implementation of class SHA1Hash, a class for computing the 
 * SHA1 mesage digest / hash algorithm as specified by NIST/NSA 
 * (not the original weaker SHA0 algorithm, but the one extended 
 * with a rotation in the expanding function as added lateron by 
 * the NSA).
 * 
 * Copyright (c) 2000--2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#include "sha1hash.hpp"


static inline u_int32_t sha_func0(u_int32_t x,u_int32_t y,u_int32_t z)
//{  return((x & y) | ((~x) & z));  }  // slower 
{  return(z ^ (x & (y ^ z) & x));  }  // faster

static inline u_int32_t sha_func1(u_int32_t x,u_int32_t y,u_int32_t z)
{  return(x ^ y ^ z);  }

static inline u_int32_t sha_func2(u_int32_t x,u_int32_t y,u_int32_t z)
//{  return((x & y) | (x & z) | (y & z));  }  // slower
{  return((x & y) | (z & (x | y)));  }  // faster

static inline u_int32_t sha_func3(u_int32_t x,u_int32_t y,u_int32_t z)
{  return(x ^ y ^ z);  }

static inline u_int32_t rotate(u_int32_t x,int n)
{  return((x<<n) | (x>>(32-n)));  }


// All this magic stuff...
static const u_int32_t sha_const0 = 0x5a827999U;
static const u_int32_t sha_const1 = 0x6ed9eba1U;
static const u_int32_t sha_const2 = 0x8f1bbcdcU;
static const u_int32_t sha_const3 = 0xca62c1d6U;

static const u_int32_t sha_init_state[5]=
{ 0x67452301U, 0xefcdab89U, 0x98badcfeU, 0x10325476U, 0xc3d2e1f0U };


// NOTE: the rotation was added later by the NSA... 
#define EXPAND(pa,pb,pc,pd) \
	rotate(*(pa++) ^ *(pb++) ^ *(pc++) ^ *(pd++),1)


// msg: size 64 bytes
// w: temporary buffer where AtomicHash stores (expanded) input data (msg). 
void SHA1Hash::AtomicHash(const unsigned char *msg,u_int32_t *w)
{
	register u_int32_t *p;
	u_int32_t *we=&w[16];
	
	// Fill message into w-buffer. 
	for(p=w; p<we; p++)
	{
		// We want MSB first 
		// (htonl() converts LSBfirst -> MSBfirst in i386; but not used 
		// here as it would need word alignment of msg data.) 
		*p= u_int32_t(*(msg++));   *p<<=8;
		*p|=u_int32_t(*(msg++));   *p<<=8;
		*p|=u_int32_t(*(msg++));   *p<<=8;
		*p|=u_int32_t(*(msg++));
	}
	
	u_int32_t *p_03=p-3;
	u_int32_t *p_08=p-8;
	u_int32_t *p_14=p-14;
	u_int32_t *p_16=p-16;
	we=&w[20];
	while(p<we)   // 20-16=4 times
	{  *(p++) = EXPAND(p_03,p_08,p_14,p_16);  }
	
	// Now compute the loops: 
	u_int32_t a=state[0], b=state[1], c=state[2], d=state[3], e=state[4];
	u_int32_t tmp,sha_const=sha_const0;
	for(p=w; p<we; p++)
	{
		tmp = rotate(a,5) + sha_func0(b,c,d) + e + (*p) + sha_const;
		e=d;  d=c;
		c=rotate(b,30);
		b=a;  a=tmp;
	}
	sha_const=sha_const1;
	for(we+=20; p<we; p++)
	{
		*p = EXPAND(p_03,p_08,p_14,p_16);
		tmp = rotate(a,5) + sha_func1(b,c,d) + e + (*p) + sha_const;
		e=d;  d=c;
		c=rotate(b,30);
		b=a;  a=tmp;
	}
	sha_const=sha_const2;
	for(we+=20; p<we; p++)
	{
		*p = EXPAND(p_03,p_08,p_14,p_16);
		tmp = rotate(a,5) + sha_func2(b,c,d) + e + (*p) + sha_const;
		e=d;  d=c;
		c=rotate(b,30);
		b=a;  a=tmp;
	}
	sha_const=sha_const3;
	for(we+=20; p<we; p++)
	{
		*p = EXPAND(p_03,p_08,p_14,p_16);
		tmp = rotate(a,5) + sha_func3(b,c,d) + e + (*p) + sha_const;
		e=d;  d=c;
		c=rotate(b,30);
		b=a;  a=tmp;
	}
	
	// Add the result: 
	state[0]+=a;
	state[1]+=b;
	state[2]+=c;
	state[3]+=d;
	state[4]+=e;
}


void SHA1Hash::GetHash(char *buf)
{
	register unsigned char *hsh=(unsigned char*)buf+3;
	for(register u_int32_t *i=state,*ie=&state[5]; i<ie; i++)
	{
		*(hsh--)=(unsigned char)((*i) & 0xffU);  *i>>=8;
		*(hsh--)=(unsigned char)((*i) & 0xffU);  *i>>=8;
		*(hsh--)=(unsigned char)((*i) & 0xffU);  *i>>=8;
		*(hsh  )=(unsigned char)((*i) & 0xffU);
		hsh+=7;  // 4+3; would be 4+4 if the last *(hsh  )=... was *(hsh--)=...
	}
}


void SHA1Hash::Reset()
{
	length=0LLU;
	state[0]=sha_init_state[0];
	state[1]=sha_init_state[1];
	state[2]=sha_init_state[2];
	state[3]=sha_init_state[3];
	state[4]=sha_init_state[4];
	tmp_size=0;
	for(unsigned char *d=tmpbuf,*de=d+64; d<de; d++)
	{  *d=(unsigned char)0;  }
}


void SHA1Hash::Feed(const char *_buf,size_t len)
{
	if(!len)
	{  return;  }
	
	const unsigned char *buf=(const unsigned char*)_buf;
	u_int32_t w[80];   // Yes, this is 4*80 bytes, NOT 4*20 bytes. 
	
	if(tmp_size)  // There is some data left from last time. 
	{
		size_t needed=64-tmp_size;
		if(len>=needed)
		{
			for(unsigned char *d=&tmpbuf[tmp_size],*de=&tmpbuf[64]; d<de; d++)
			{  *d=*(buf++);  }
			AtomicHash(tmpbuf,w);
			len-=needed;
			tmp_size=0;
			length+=64LLU;
			if(!len)
			{  return;  }
		}
		else
		{
			unsigned char *d=&tmpbuf[tmp_size];
			tmp_size+=int(len);
			for(unsigned char *de=&tmpbuf[tmp_size]; d<de; d++)
			{  *d=*(buf++);  }
			return;
		}
	}
	
	size_t l=len;
	for(;;)
	{
		if(l<64)
		{  break;  }
		AtomicHash((const unsigned char*)buf,w);
		buf+=64;
		l-=64;
	}
	length+=u_int64_t(len-l);
	if(l)  // Some bytes left; store them for later. 
	{
		for(unsigned char *d=tmpbuf,*de=d+l; d<de; d++)
		{  *d=*(buf++);  }
		tmp_size=int(l);
	}
}


void SHA1Hash::Final()
{
	u_int32_t w[80];   // Yes, this is 4*80 bytes, NOT 4*20 bytes. 
	int pad80=0;
	
	// Now, let's see if we need padding. 
	// We must check for >=56 as we always have to add one 0x80 padding. 
	// Padding does not count for the length, of course.
	length+=u_int64_t(tmp_size);
	if(tmp_size>=56)  // We need to pad the tmpbuf with zeros and hash it. 
	{
		unsigned char *d=&tmpbuf[tmp_size];
		*(d++)=(unsigned char)0x80;   // append binary 10000000. 
		for(unsigned char *de=&tmpbuf[64]; d<de; d++)
		{  *d=(unsigned char)0;  }
		AtomicHash(tmpbuf,w);
		tmp_size=0;
		++pad80;
	}
	
	// The length was counted in bytes, but we need the number of bits here: 
	u_int64_t len=length*8LLU;
	
	// Now, we have to fill up tmpbuf with zeros and add the length in bits 
	// as the last 8 bytes. 
	unsigned char *d=&tmpbuf[tmp_size];
	if(!pad80)
	{  *(d++)=(unsigned char)0x80;  }  // append binary 10000000. 
	for(unsigned char *de=&tmpbuf[56]; d<de; d++)
	{  *d=(unsigned char)0;  }
	
	// Add more significant word: 
	u_int32_t word = u_int32_t(len >> 32);
	d+=3;
	*(d--)=(unsigned char)(word & 0xffU);  word>>=8;
	*(d--)=(unsigned char)(word & 0xffU);  word>>=8;
	*(d--)=(unsigned char)(word & 0xffU);  word>>=8;
	*(d  )=(unsigned char)(word & 0xffU);
	
	// Add less significant word: 
	word = u_int32_t(len & 0xffffffffLLU);
	d+=7;
	*(d--)=(unsigned char)(word & 0xffU);  word>>=8;
	*(d--)=(unsigned char)(word & 0xffU);  word>>=8;
	*(d--)=(unsigned char)(word & 0xffU);  word>>=8;
	*(d  )=(unsigned char)(word & 0xffU);
	
	AtomicHash(tmpbuf,w);
	tmp_size=0;
}

/*
 * check_crypto.cc
 * 
 * This is not a part of libhlib.a but a part of hlib distribution. 
 * This code is used to test some crypto routines of hlib. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#define HLIB_IN_HLIB 1
#include <hlib/htime.h>

#include <hlib/base64.h>

#include <string.h>
#include <assert.h>


static void FillRandom(char *_dest,size_t len)
{
	if(!len)  return;
	
	unsigned char *dest=(unsigned char *)_dest;
	unsigned char *destend=dest+len;
	for(;;)
	{
		unsigned int r=random();
		for(int _j=0; _j<4; _j++)
		{
			*(dest++)=(unsigned char)(r & 0xffU);
			if(dest>=destend)  return;
			r>>=8;
		}
	}
}



// BASE64: 
static int base64_ctest(const char *str)
{
	size_t inlen=strlen(str);
	size_t outlen=inlen*4/3+4 + inlen*4/3/32+1;  // second half for inserted '\n'
	char enc[outlen];
	
	// Encode it: 
	u_int32_t state=0;
	const char *ibuf=str;
	const char *ibufend=ibuf+inlen;
	char *obuf=enc;
	char *obufend=obuf+outlen;
	for(;;)
	{
		int inlen=(random()%22)+1;
		if(ibuf+inlen>ibufend)
		{  inlen=ibufend-ibuf;  }
		ssize_t rv=Base64Encode(ibuf,inlen,obuf,obufend-obuf,&state,8);
		if(rv<0)
		{
			fprintf(stderr,"Base64Encode(%d)=%d\n",inlen,rv);
			return(1);
		}
		ibuf+=inlen;
		obuf+=rv;
		if(!inlen)  break;
	}
	assert(ibuf==ibufend);
	assert(obuf<=obufend);
	
	outlen=obuf-enc;
	
	char dec[inlen];
	
	// Decode it: 
	state=0;
	ibuf=enc;
	ibufend=enc+outlen;
	obuf=dec;
	obufend=dec+inlen;
	for(;;)
	{
		int inlen=(random()%23)+1;
		if(ibuf+inlen>ibufend)
		{  inlen=ibufend-ibuf;  }
		ssize_t rv=Base64Decode(ibuf,inlen,obuf,obufend-obuf,&state);
		if(rv<0)
		{
			fprintf(stderr,"Base64Encode(%d)=%d\n",inlen,rv);
			return(1);
		}
		ibuf+=inlen;
		obuf+=rv;
		if(!inlen)  break;
	}
	assert(ibuf==ibufend);
	assert(obuf<=obufend);
	
	// Compare...
	if(inlen!=size_t(obuf-dec))
	{
		fprintf(stderr,"Length mismatch %u != %u (%u)\n",
			inlen,size_t(obuf-dec),outlen);
		return(1);
	}
	if(memcmp(str,dec,inlen))
	{
		fprintf(stderr,"Compare failed.\n");
		return(1);
	}
	return(0);
}

static struct Base64_TEST
{
	const char *txt;
	const char *res;
} base64_test[]=
{
	{ "T", "VA=" },
	{ "hi", "aGk=" },
	{ "s i", "cyBp=" },
	{ "s th", "cyB0aA=" },
	{ "e tex", "ZSB0ZXg=" },
	{ "t whic", "dCB3aGlj=" },
	{ "h shall", "aCBzaGFsbA=" },
	{ " serve a", "IHNlcnZlIGE=" },
	{ "s base64 ", "cyBiYXNlNjQg=" },
	{ "test vecto", "dGVzdCB2ZWN0bw=" },
	{ "r.  Well, t", "ci4gIFdlbGwsIHQ=" },
	{ "he sentence ", "aGUgc2VudGVuY2Ug=" },
	{ "here was a bi", "aGVyZSB3YXMgYSBiaQ=" },
	{ "t short so som", "dCBzaG9ydCBzbyBzb20=" },
	{ "e blah blah nee", "ZSBibGFoIGJsYWggbmVl=" },
	{ "ds to be added. ", "ZHMgdG8gYmUgYWRkZWQuIA=" },
	{ " We want good tes", "IFdlIHdhbnQgZ29vZCB0ZXM=" },
	{ "ting, don't we?  T", "dGluZywgZG9uJ3Qgd2U/ICBU=" },
	{ "he longer the text ", "aGUgbG9uZ2VyIHRoZSB0ZXh0IA=" },
	{ "the lesser the chanc", "dGhlIGxlc3NlciB0aGUgY2hhbmM=" },
	{ "e that things are wro", "ZSB0aGF0IHRoaW5ncyBhcmUgd3Jv=" },
	{ "ng.  Okay, that should", "bmcuICBPa2F5LCB0aGF0IHNob3VsZA=" },
	{ " be enough now, blah bl", "IGJlIGVub3VnaCBub3csIGJsYWggYmw=" },
	{ "ah blah blah blah blah..", "YWggYmxhaCBibGFoIGJsYWggYmxhaC4u=" },
	{ NULL, NULL }
};


static int do_test_base64()
{
	int fail=0;
	int ntests=0;
	
	fprintf(stderr,"Testing base64");
	
	// First, check if some data when encoded and decoded is the same again: 
	// Run a test for different data lengths: 
	for(size_t len=1; len<2048; len++)
	{
		char data[len+1];
		FillRandom(data,len);
		data[len]='\0';
		
		fail+=base64_ctest(data);
		++ntests;
		if(!fail && !(len%128))
		{  fprintf(stderr,".");  }
	}
	if(fail)
	{
		fprintf(stderr," *** FAILED!! (%d errors)\n",fail);
		return(1);
	}
	
	// Next, check if the output is correct: 
	for(Base64_TEST *ts=base64_test; ts->txt; ts++,ntests++)
	{
		size_t inlen=strlen(ts->txt);
		size_t outlen=inlen*4/3+4;
		char enc[outlen+1];
		
		ssize_t rv=Base64Encode(ts->txt,inlen,enc,outlen,NULL,0);
		if(rv<0)
		{
			fprintf(stderr,"Base64Decode(%u)=%d\n",inlen,rv);
			++fail;
			continue;
		}
		assert(rv<ssize_t(outlen));
		enc[rv]='\0';
		
		int thisfail=0;
		if(ssize_t(strlen(ts->res))!=rv)
		{
			fprintf(stderr,"Length mismatch %d != %d (%u)\n",
				rv,strlen(ts->res),inlen);
			++thisfail;
		}
		else if(strcmp(ts->res,enc))
		{
			fprintf(stderr,"Wrong result (inlen=%u)\n",inlen);
			++thisfail;
		}
		if(thisfail)
		{
			fprintf(stderr,
				"Test string: \"%s\"\n"
				"Result:   \"%s\"\n"
				"Expecred: \"%s\"\n",
				ts->txt,enc,ts->res);
			++fail;
		}
	}
	if(fail)
	{
		fprintf(stderr," ***FAILED!! (%d errors)\n",fail);
		return(1);
	}
	
	fprintf(stderr,". passed (%d)\n",ntests);
	return(0);
}

char *prg_name="test_crypto";

int main()
{
	fprintf(stderr,"TESTING CRYPTO:\n");
	int failed=0;
	
	HTime t(HTime::Curr);
	srandom(t.Get(HTime::seconds));
	
	failed+=do_test_base64();
	
	if(failed)
	{  fprintf(stderr,"*** ERRORS. TESTS FAILED.\n");  }
	else
	{  fprintf(stderr,"ALL TESTS PASSED SUCCESSFULLY.\n");  }
	
	return(failed ? 1 :0);
}

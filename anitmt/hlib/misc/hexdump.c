/*
 * hexdump.c 
 * Hexadecimal dump of arbitrary data to stream. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include <hlib/prototypes.h>

#include <ctype.h>


#define BYTES_PER_LINE 16

void WriteHexDump(FILE *out,const char *data,size_t len)
{
	const char *cend=data+len;
	const char *line=data;
	const char *i,*iend;
	char tmp[BYTES_PER_LINE+1];
	char *d;
	int nfill,j;
	while(line<cend)
	{
		iend=line+BYTES_PER_LINE;
		if(iend>cend)
		{  nfill=iend-cend;  iend=cend;  }
		else nfill=0;
		fprintf(out,"0x%04x   ",line-data);
		
		for(i=line; i<iend; i++)
		{  fprintf(out," %02x",(unsigned int)*(unsigned char*)i);  }
		for(j=0; j<nfill; j++)
		{  fprintf(out,"   ");  }
		
		d=tmp;
		for(i=line; i<iend; i++,d++)
		{
			*d = isprint(*i) ? *i : '.';
			/* unsigned short int v=*(unsigned char*)i   */
			/* *d=(v<32 || v==127 || v==255) ? '.' : *i; */
		}
		*d='\0';
		fprintf(out,"    %s\n",tmp);
		
		line=iend;
	}
}

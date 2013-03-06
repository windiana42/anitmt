/*
 *
 * Copyright (C) 1996 by Josh Osborne.
 * All rights reserved.
 *
 * This software may be freely copied, modified and redistributed without
 * fee for non-commerical purposes provided that this copyright notice is
 * preserved intact on all copies and modified copies.
 * 
 * There is no warranty or other guarantee of fitness of this software.
 * It is provided solely "as is". The author(s) disclaim(s) all
 * responsibility and liability with respect to this software's usage
 * or its effect upon hardware, computer systems, other software, or
 * anything else.
 *
 */

// changed from ppm.cc by: Martin Trautmann

#include "tga.h"
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

// How many types of IO can we use in one program??
extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
}

typedef unsigned char  BYTE;
typedef unsigned short WORD;

typedef struct Headertype{
  BYTE bIDFeldLen;
  BYTE fColormapTyp;
  BYTE fImageTyp;
  BYTE wColorMapOriginLO; // muﬂ getrennt werden! (wegen ungeradem Offset)
  BYTE wColorMapOriginHI;
  BYTE wColorMapLengthLO;
  BYTE wColorMapLengthHI;
  BYTE bColorMapEntrySize;
  WORD wXOrigin; // X-Ursprung
  WORD wYOrigin; // Y-Ursprung
  WORD wWidth;
  WORD wHeight;
  BYTE bpp;
  BYTE fImageDescriptor;
};

const int BUFSZ = 1024;

tga::tga(const char *filename)
{
  cerr << "tga for " << filename << " at " << (void *)this << endl;
  FILE *pf = fopen(filename, "rb");

  if (!pf) {
    cerr << "can't open " << filename << " (" << strerror(errno) 
	 << ") bailing" << endl;
    exit(17);
  }

  Headertype Header;
  fread(&Header,sizeof(Header),1,pf);

  if ( Header.bpp != 24 ) {
    cerr << "File " << filename 
	 << " is not a RAWBITS tga file with 24bpp, bailing"
	 << endl;
    exit(19);
  }

  fseek( pf, Header.bIDFeldLen, SEEK_CUR );

  this->width  = Header.wWidth;
  this->height = Header.wHeight;

  assert(Header.wWidth != 0 && Header.wHeight != 0);

  this->image = new unsigned char[width*height*3];
  cerr << "tga image at " << (void *)this->image << endl;
  int rc = fread(image, 1, width*height*3, pf);
  assert(rc == width*height*3);

  // reverse red and blue to bgr colors
  for( long i=0; i<width*height; i++ )
    {
      char buf = image[i*3];
      image[i*3]     = image[i*3 + 2];
      image[i*3 + 2] = buf;
    }

  rc = fclose(pf);
  if (rc != 0) {
    cerr << "Close on " << filename << "failed " << strerror(errno) << endl;
    exit(20);
  }
}

tga::~tga()
{
  delete image;
}

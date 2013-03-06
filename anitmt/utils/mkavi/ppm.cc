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


#include "ppm.h"

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

const int BUFSZ = 1024;

ppm::ppm(const char *filename)
{
	cerr << "ppm for " << filename << " at " << (void *)this << endl;
	FILE *pf = fopen(filename, "rb");

	if (!pf) {
		cerr << "can't open " << filename << " (" << strerror(errno) 
		  << ") bailing" << endl;
		exit(17);
	}

	char buf[BUFSZ];
	char *cp = fgets(buf, BUFSZ, pf);
	if (!cp) {
		cerr << "Read on " << filename << " failed (" << strerror(errno)
		  << ") bailing" << endl;
		exit(18);
	}

	if (!strcmp("P6", buf)) {
		cerr << "File " << filename << " is not a RAWBITS ppm file, bailing"
		  << endl;
		exit(19);
	}

	cp = fgets(buf, BUFSZ, pf);
	if (!cp) {
		cerr << "Read on " << filename << " failed (" << strerror(errno)
		  << ") bailing" << endl;
		exit(18);
	}

	int rc = sscanf(buf, "%d %d", &this->width, &this->height);
	assert(rc == 2);
	assert(this->width != 0 && this->height != 0);

	cp = fgets(buf, BUFSZ, pf);
	if (!cp) {
		cerr << "Read on " << filename << " failed (" << strerror(errno)
		  << ") bailing" << endl;
		exit(18);
	}
	
	int maxval;
	rc = sscanf(buf, "%d", &maxval);
	assert(maxval != 0 && maxval <= 255);

	this->image = new unsigned char[width*height*3];
	cerr << "ppm image at " << (void *)this->image << endl;
	rc = fread(image, 1, width*height*3, pf);
	assert(rc == width*height*3);

	rc = fclose(pf);
	if (rc != 0) {
		cerr << "Close on " << filename << "failed " << strerror(errno) << endl;
		exit(20);
	}
}

ppm::~ppm()
{
	delete image;
}

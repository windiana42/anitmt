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


#ifndef PPM_INCLUDED
#define PPM_INCLUDED

#include <iostream.h>
#include <assert.h>
#include "image.h"

class ppm : public image {
  public:
	ppm(const char *filename);
	int w() { return width; }
	int h() { return height; }
	const unsigned char *pixel_row(int row) {
		//cerr << "pixel_row " << row << endl;
		assert(row >= 0 && row <= this->height);
		return image + row * 3 * this->width;
	}
	~ppm();
  private:
	int width, height;
	unsigned char *image;
};

#endif

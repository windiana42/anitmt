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


#ifndef IMAGE_INCLUDED
#define IMAGE_INCLUDED

class image {
 public:
  virtual int w() = 0;
  virtual int h() = 0;
  virtual const unsigned char *pixel_row(int row) = 0;
  virtual ~image();
};

#endif

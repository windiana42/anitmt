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

// changed by Martin Trautmann:
//   - TGA ability 
//   - new parameter system accorting to tga2avi
//   - working AVIs for the Windows 98/NT Player

#include <strings.h>
#include <assert.h>
#include <iostream>
#include <stdlib.h>

// iostream has pissed me off, we will use open(2)
extern "C" {
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
}

#include "codec.h"
#include "avi.h"
#include "image.h"
#include "ppm.h"
#include "tga.h"
#include "chunk.h"
#include "patchlevel.h"

#include <string>

using std::cerr;
using std::cout;
using std::endl;

// search %0nd and replace with the number
std::string num_in_string( std::string str, int num ){
  std::string::size_type p1 = str.find("%0");
  std::string::size_type p2 = str.find("d",p1);
  
  int digits = atoi( str.substr( p1+2, p2 ).c_str() );
  
  char buf[20];
  sprintf(buf,"%0*d", digits, num );
  
  return str.substr(0,p1) + buf + str.substr(p2+1);
}

#define NEED_ARG if (++z >= argc) { \
	cerr << argv[0] << ": option " << argv[z-1] << " needs an argument" \
	  << endl; \
	exit(3); \
}

image *get_image( int num, std::string basename, int type )
{
  switch( type ){
    case 0:
      return new tga( num_in_string( basename, num ).c_str() );
    case 1:
      return new ppm( num_in_string( basename, num ).c_str() );
  }

  return 0;
}

int main(int argc, char *argv[])
{
  if( argc > 1 )
    {
      if( !strcmp(argv[1],"--help") )
	argc = 1;
      if( !strcmp(argv[1],"/h") )
	argc = 1;
      if( !strcmp(argv[1],"-h") )
	argc = 1;
    }

  if(argc < 2)
    {
      printf(" Syntax:\n");
      printf("   mkavi options \n\n");
      printf("   [-i filenames ] name of the Image-files that includes number of digits:\n");
      printf("                   %c0nd (f%c04d.tga for f0000.tga,...)\n\n",'%','%');
      printf("   [-f fps]        frames per second (24)\n");
      printf("   [-o outputname] filename of the AVI-file (film.avi)\n");
      printf("   [-s startframe] first frame number of the image set to use (0)\n");
      printf("   -e endframe     last  frame number of the image set to use\n");
      printf("   [-j step]       use every step frames (1)\n");
      printf("   [-t imagetype ] file format of images {tga, ppm} (tga)\n");
      printf("   [-c codec     ] avi codec to use {cram16,rgb24} (cram16)\n");
      printf("\n");
      printf(" example:\n");
      printf("   mkavi -i %c04d.tga -o dest.avi -e29 -c rgb24\n",'%');

      return 0;
    }

  
  char *outputname = (char *) malloc( 300 );
  char *fpstext = (char *) malloc( 100 );
  char *mintext = (char *) malloc( 100 );
  char *maxtext = (char *) malloc( 100 );
  char *steptext = (char *) malloc( 100 );

  char *format = (char *) malloc( 100 );
  char *codecname = (char *) malloc( 100 );

  std::string image_names = "f%07d";
  strcpy(outputname, "film.avi");
  strcpy(fpstext, "24");
  strcpy(mintext, "0");
  strcpy(steptext, "1");
  maxtext[0] = 0;
  strcpy(format, "");
  strcpy(codecname, "cram16");

  for(int z=1;z<argc;z++)
    {
      if((argv[z])[0]=='-')
	{
	  if((argv[z])[1]==0 )continue;

	  if((argv[z])[1]=='i')
	    {
	      if((argv[z])[2]==0)
		{
		  NEED_ARG;
		  image_names = argv[z];
		  continue;
		}
	      else
		{
		  image_names = argv[z]+2;
		  continue;
		}
	    }

	  if((argv[z])[1]=='o')
	    {
	      if((argv[z])[2]==0)
		{
		  NEED_ARG;
		  strcpy(outputname,argv[z]);
		  continue;
		}
	      else
		{
		  strcpy(outputname,argv[z]+2);
		  continue;
		}
	    }

	  if((argv[z])[1]=='f')
	    {
	      if((argv[z])[2]==0)
		{
		  NEED_ARG;
		  strcpy(fpstext,argv[z]);
		  continue;
		}
	      else
		{
		  strcpy(fpstext,argv[z]+2);
		  continue;
		}
	    }
	  
	  if((argv[z])[1]=='s')
	    {
	      if((argv[z])[2]==0)
		{
		  NEED_ARG;
		  strcpy(mintext,argv[z]);
		  continue;
		}
	      else
		{
		  strcpy(mintext,argv[z]+2);
		  continue;
		}
	    }

	  if((argv[z])[1]=='e')
	    {
	      if((argv[z])[2]==0)
		{
		  NEED_ARG;
		  strcpy(maxtext,argv[z]);
		  continue;
		}
	      else
		{
		  strcpy(maxtext,argv[z]+2);
		  continue;
		}
	    }

	  if((argv[z])[1]=='t')
	    {
	      if((argv[z])[2]==0)
		{
		  NEED_ARG;
		  strcpy(format,argv[z]);
		  continue;
		}
	      else
		{
		  strcpy(format,argv[z]+2);
		  continue;
		}
	    }

	  if((argv[z])[1]=='c')
	    {
	      if((argv[z])[2]==0)
		{
		  NEED_ARG;
		  strcpy(codecname, argv[z]);
		  continue;
		}
	      else
		{
		  strcpy(codecname, argv[z]+2);
		  continue;
		}
	    }

	  if((argv[z])[1]=='j')
	    {
	      if((argv[z])[2]==0)
		{
		  NEED_ARG;
		  strcpy(steptext, argv[z]);
		  continue;
		}
	      else
		{
		  strcpy(steptext, argv[z]+2);
		  continue;
		}
	    }
	}
      printf("warning: unknown option %s\n",argv[z]);
    }

  if(mintext[0] == 0)
    {
      printf("Error: first frame number not defined\n");
      return -1;
    }

  if(maxtext[0] == 0)
    {
      printf("Error: last frame number not specified\n");
      return -1;
    }

  int startframe = strtol(mintext,(char **)NULL,0);
  int endframe = strtol(maxtext,(char **)NULL,0);
  int step = strtol(steptext,(char **)NULL,0);
  if( step == 0 ) step = 1;
  int nframes = abs((endframe - startframe) / step) + 1;
  int fps = abs(strtol(fpstext,(char **)NULL,0) / step);
  if( fps == 0 ) fps = 1;

  // check image format
  int image_type = -1;		// image file format 
  if( !strcmp(format,"tga") || !strcmp(format,"TGA") )
    image_type = 0;
  if( !strcmp(format,"ppm") || !strcmp(format,"PPM") )
    image_type = 1;

  if( image_type < 0 )		// if imagetype is undefined
    {
      int len = image_names.size();
      const char *suffix = image_names.c_str() + len-3;
      if( !strcmp(suffix, "tga") || !strcmp(suffix, "TGA") )
	image_type = 0;
      else
	{ 
	  if( !strcmp(suffix, "ppm") || !strcmp(suffix, "PPM") )
	    image_type = 1;
	  else
	    image_type = 0;	// tga is default type
	}
	
    }

  if( image_type < 0 ){
    cerr << argv[0] << ": invalid image format " << format << endl;
    exit(3);
  }

  // test if input filname as file type extension
  if( image_names.find('.') == std::string::npos )
    {
      switch( image_type )
	{
	default:
	case 0: image_names += ".tga"; break;
	case 1: image_names += ".ppm"; break;
	}
    }


  // check codec
  codec *cd = NULL;		// avi codec
  if( !strcmp(codecname,"cram16") || !strcmp(codecname,"CRAM16") )
    cd = new cram16;
  if( !strcmp(codecname,"rgb24") || !strcmp(codecname,"RGB24") )
    cd = new rgb24;

  if( !cd ){
    cerr << argv[0] << ": invalid avi codec " << codecname << endl;
    exit(3);
  }

  FILE *avif = fopen( outputname, "w+b");
  if (!avif) {
    cerr << "Couldn't open for write " << outputname << endl;
    exit(4);
  }

  chunkstream avistr(avif);

  // it is wasteful to read the first ppm file twice, but that's life.
  image *p = get_image( startframe, image_names, image_type );

  cd->start(&avistr, p->w(), p->h(), nframes );

  riffchunk *riff = new riffchunk(&avistr, "AVI RIFF", 0, "AVI ");
  riff->write();
  avi_header *avih = new avi_header(&avistr, cd, p->w(), p->h(), nframes, fps);
  avih->write();
  delete(p);

  //int headsize = avih->get_size(); // do now to get size of avi_header
  int headsize = ftell( avif );

  listchunk *movi = new listchunk(&avistr, "movi chunk", 1, "movi");
  movi->write();

  cd->set_head_size( headsize + 8 );

  int i;

  if (cd->need_prescan()) {
    int num = 0;
    for(i = startframe; i <= endframe; i+=step) {
      image *p = get_image( i, image_names, image_type );

      cd->prescan(p, num++);
      delete p;
    }
  }

  int num = 0;
  for(i = startframe; i <= endframe; i+=step) {
    image *p = get_image( i, image_names, image_type );
    cd->frame(p, num++);
    delete p;
  }

  indexchunk *idx = new indexchunk(&avistr, "index chunk", -1 );
  idx->write();
  cd->write_index();

  cd->done();
  riff->done(1);
 
  delete outputname;
  delete fpstext;
  delete mintext;
  delete maxtext;

  delete format;
  delete codecname;

  delete cd;
  delete riff;
  delete avih;
  delete movi;

  return 0;
}

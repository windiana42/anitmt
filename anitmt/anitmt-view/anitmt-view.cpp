/*****************************************************************************/
/** AniTMT -- A program for creating foto-realistic animations		    **/
/**   This file belongs to the component anitmt-view			    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** Website: http://www.anitmt.org/      				    **/
/** EMail:   anitmt@theofel.de						    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#define maxexeclen 1000

#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include "vals.hpp"
#include "parser.hpp"
#include "error.hpp"
#include "mystream.hpp"

#include "osdep.hpp"

namespace anitmt_view{
  
  enum filetypes{ tga, ppm };
  filetypes outfiletype;
  int image_width, image_height;

  bool redo = false;

#include "mytga.h"
#include "myavi.h"
#include "myfile.h"

  char *image_name(int z){
    char *suffix;
    switch( outfiletype )
      {
      case ppm: suffix = "ppm"; break;
      case tga: 
      default:  suffix = "tga"; break;
      }
    

    char *str=(char*)malloc(maxexeclen);
    sprintf(str, "f%04d.%s",z,suffix);
    return str;
  }

  char *image_param(int z){
    char ft = 'T';
    switch( outfiletype )
      {
      case tga: ft = 'T';break;
      case ppm: ft = 'P';break;
      }

    char *str=(char*)malloc(maxexeclen);
    sprintf(str,"+F%c -O %s",ft,image_name(z));
    return str;
  }

  // return:
  //  0: image file is ok
  //  negative: image file is corrupt
  //  1: image file is only filled partially (rendering may be continued)
  //  2: image has wrong width
  //  3: image has wrong height
  char image_check( char *filename ){
    std::ifstream test( filename );
    
    if( !test ) return -1;

    TGA tga_bild;
    switch( outfiletype )
      {
      case tga: 
	{
	  // check header
	  tga_bild.open( filename );
	  tga_bild.getHeader();
	  if( tga_bild.Header.bpp != 24 )
	    {
	      return -2;
	    }

	  if( image_width != tga_bild.Header.wWidth ) return 2;
	  if( image_height != tga_bild.Header.wHeight ) return 3;
	  
	  // check file size
	  long len = (long)tga_bild.Header.wWidth * 
	    (long)tga_bild.Header.wHeight * 3L;
	  fseek(tga_bild.dh, len-1, SEEK_CUR );
	  // check if exactly one character may be read until eof
	  char buf[3];
	  int bytes_remaining = fread( buf, 1, 3, tga_bild.dh );
	  if( bytes_remaining != 1 )
	    {
	      return (bytes_remaining == 0) ? 1:-4;
	    }
	}
	break;
      case ppm: 
	{
	  // check header
	  std::string id;
	  int width, height, maxval;
	  test >> id >> width >> height >> maxval;
	  if( id != "P6" ) return -5;
	  if( maxval == 0 ) return -6;
	  if( maxval > 255 ) return -7;
	  if( image_width != width ) return 2;
	  if( image_height != height ) return 3;
	
	  // check file size
	  long pos = test.tellg();
	  FILE *dh = fopen(filename, "rb");
	  if(!dh) return -7;
	  long len = (long)width * (long)height * 3L + pos + 1;
	  fseek(dh, len-1, SEEK_SET );

	  // check if exactly one character may be read until eof
	  char buf[3];
	  int bytes_remaining = fread( buf, 1, 3, dh );
	  if( bytes_remaining != 1 )
	    {
	      return (bytes_remaining == 0) ? 1:-8;
	    }
	  fclose(dh);
	}
	break;
      }
    return 0;
  }

  AVI Film;

  void dowithtgaanf(int fps,int anz,int startframe){
    aniinf INF;
    Film.getSize(image_name(startframe),INF);
    INF.fps=fps;
    INF.anz=anz;
    if(Film.NewAVI("film.avi",INF))
      {
	char error[100];
	strcpy(error,"Error while writing AVI: ");
	perror(error);
	//printf("Error: %s!\n", error);
	exit(-1);
      }
  }

  void dowithtganow(int z){
    Film.AddTGAFrame(image_name(z));
  }

  void dowithtgaend(){
    Film.closeAVI();
  }

  // call povray and return the number of pictures rendered
  int callpov( char* path,char *params,char* LOGname, 
	       int startframe, int endframe, int step, int create_avi, 
	       int thread){
    int numpics = 0;

    typedef std::map< std::string, values::Valtype > settype;
    settype setting;

    std::ifstream rif( LOGname );

    if( !rif ){
      std::cerr << "Unable to read rif file " << LOGname << std::endl;
      return -1;
    }

    std::string line;
    while( getline( rif, line ) ){
      std::string::size_type i = line.find( '=' );

      if( i != std::string::npos ){
	stream::String_Stream stream( line.substr(i+1) );
	stream::mk_C_Stream( stream );
	parser::Parser p( &stream );

	values::Valtype res;
	try{
	  res = p.getTerm();
	}
	catch(err::Basic_Error e){
	  std::cout << "Error: " << e.get_message() << std::endl;  
	  continue;
	}
      
	setting[ line.substr(0,i) ] = res;
      }
    }
    
    image_width = (int)setting["width"].get_scalar();
    image_height = (int)setting["height"].get_scalar();
    std::string dir = setting["ani_dir"].get_string();

    osdep::chdir( dir );

    // if in threaded mode as thread <thread>
    if( thread != -1 ){
      char num[20];
      sprintf( num, "%02d", thread );

      std::string basic_frame_dest = setting["frame_dest"].get_string(); 
      std::string frame_dest       = num + basic_frame_dest;
      setting["frame_dest"]        = frame_dest;
      std::string basic_main_file = setting["main_file"].get_string();
      std::string main_file       = num + basic_main_file;
      setting["main_file"]  = main_file;

      std::ifstream in(basic_main_file.c_str());
      std::ofstream out(main_file.c_str());

      bool found = false;
      while( in ){
	std::string buf;
	getline(in,buf);

	// replace frame include file
	if( !found )
	  {
	    std::string::size_type i = buf.find( basic_frame_dest );
	    if( i != std::string::npos ) 
	      {
		buf.insert( i, num );
		found = true;
	      }
	  }

	out << buf << std::endl;
      }
      if( !found ){
	std::cout << "Error: Couldn't find frame include file " << basic_frame_dest << " in main pov file " << basic_main_file << std::endl;
      }
    }

    if( startframe < 0 ) startframe = 0;
    if( endframe   < 0 ) endframe   = int(setting["frames"].get_scalar() +
					  0.5) - 1 ;

    char *curINCname=(char *)malloc(maxexeclen);
    OutFile *INC;

    int fps = abs( int(setting["fps"].get_scalar()+0.5) / step );
    if( fps < 1 ) fps = 1; 
    int nframes = abs((endframe - startframe) / step) + 1;

    char first=1;
    for(int z=startframe;z<=endframe;z+=step)
      {
	// INCfile erzeugen/kopieren
	sprintf(curINCname, setting["frame_src"].get_string().c_str(), z);
	INC=new OutFile(setting["frame_dest"].get_string().c_str());
	INC->putS("#include \"");
	INC->putS(curINCname);
	INC->putS("\"");
	delete INC;

	char *name = image_name(z);
	int ret;
	if( redo ) ret = image_check( name );
	if( !redo || ret ) // is redo deactivated or 
	  {			           // is picture not rendered yet
	    // render with povray
	    char *execstr=(char *)malloc(maxexeclen);
	    sprintf(execstr,"%s -P +i %s +w%d +h%d %s %s %s",path,
		    setting["main_file"].get_string().c_str(), 
		    image_width, image_height, 
		    (setting["params"].get_string() + params).c_str(), 
		    image_param(z),
		    redo && (ret==1) ? "+C":""); // continue aborted trace?
	    printf("%s\n",execstr);
	    
	    if( system(execstr) )
	      {
		free( execstr );

		std::cout << "Error: POVRay returned with an error" << std::endl;
		endframe = z-1;

		if( first ) exit(-1);
		break;
	      }
	    
	    free( execstr );

	    if( image_check( name ) )
	      {
		std::cout << "Error: POVRay didn't create correct picture" 
			  << std::endl;
		endframe = z-1;
		break;
	      }

	    numpics++;
	  }
	free( name );
      
	if( create_avi == 1 )
	  {
	    if(first)
	      {
		dowithtgaanf( fps, nframes, startframe);
		first=0;
	      }
	    
	    dowithtganow(z);
	  }
      }

    if( create_avi == 1 )
      dowithtgaend();

    if( create_avi == 2 )
      {
	char *execstr=(char *)malloc(maxexeclen);
	sprintf( execstr, "mkavi -s %d -e %d -f %d -j %d -t %s", 
		 startframe, endframe, fps, step, 
		 outfiletype == ppm ? "ppm":"tga" );

	printf("%s\n",execstr);
	system( execstr );
	free( execstr );
      }

    return numpics;
  }
}


using namespace anitmt_view;

int main(int argn,char **argv)
{
  outfiletype = tga;
  bool help = false;

  if( argn > 1 )
    {
      if( scmp(argv[1],"--help") )
	help = true;
      if( scmp(argv[1],"/h") )
	help = true;
      if( scmp(argv[1],"-h") )
	help = true;
    }

  if( help ){
      printf(" Syntax:\n");
      printf("   anitmt-view [options] \n\n");
      printf("   [-p executable]  povray call (povray)\n");
      printf("   [-r riffile]     filename of render information file (ani.rif)\n");
      printf("   [-s startframe]  first frame number (0)\n");
      printf("   [-e endframe]    last  frame number (max)\n");
      printf("   [-j step]        use every step frames (1)\n");
      printf("   [-a]             create avi file\n");
      printf("   [-A]             call \"mkavi\" to create avi file\n");
      printf("   [-t n]           multithreaded mode as thread n\n");
      printf("   [--ppm]          create ppm files\n");
      printf("   [--redo]         redo animation; skip files already rendered\n");
      printf("\n");
      
      printf(" example:\n");
      printf("   anitmt-view -e 20\n");

    return 0;
  }

  char *path=(char *)malloc(maxexeclen);
  strcpy(path,"povray");

  char *logf=(char *)malloc(maxexeclen);
  strcpy(logf,"ani.rif");

  char *params=(char *)malloc(maxexeclen);
  strcpy(params," ");

  int startframe = -1;
  int endframe = -1;
  int step = 1;
  int create_avi = 0;
  int thread=-1;

  for(int z=1;z<argn;z++)
    {
      if((argv[z])[0]=='-')
	{
	  if((argv[z])[1]==0 )continue;

	  if((argv[z])[1]=='-' )
	    {
	      if( !strcmp(argv[z], "--ppm") )
		{
		  outfiletype = ppm;
		  continue;
		}
	      if( !strcmp(argv[z], "--redo") )
		{
		  redo = true;
		  continue;
		}
	    }

	  if((argv[z])[1]=='a')
	    {
	      create_avi = 1;
	      continue;
	    }

	  if((argv[z])[1]=='A')
	    {
	      create_avi = 2;
	      continue;
	    }

	  if((argv[z])[1]=='p')
	    {
	      if((argv[z])[2]==0)
		{
		  z++;
		  strcpy(path,argv[z]);
		  continue;
		}
	      else
		{
		  strcpy(path,argv[z]+2);
		  continue;
		}
	    }

	  if((argv[z])[1]=='r')
	    {
	      if((argv[z])[2]==0)
		{
		  z++;
		  strcpy(logf,argv[z]);
		  continue;
		}
	      else
		{
		  strcpy(logf,argv[z]+2);
		  continue;
		}
	    }

	  if((argv[z])[1]=='s')
	    {
	      if((argv[z])[2]==0)
		{
		  z++;
		  startframe = atoi(argv[z]);
		  continue;
		}
	      else
		{
		  startframe = atoi(argv[z]+2);
		  continue;
		}
	    }

	  if((argv[z])[1]=='e')
	    {
	      if((argv[z])[2]==0)
		{
		  z++;
		  endframe = atoi(argv[z]);
		  continue;
		}
	      else
		{
		  endframe = atoi(argv[z]+2);
		  continue;
		}
	    }
	  if((argv[z])[1]=='j')
	    {
	      if((argv[z])[2]==0)
		{
		  z++;
		  step = atoi(argv[z]);
		  continue;
		}
	      else
		{
		  step = atoi(argv[z]+2);
		  continue;
		}
	    }
	  if((argv[z])[1]=='t')
	    {
	      if((argv[z])[2]==0)
		{
		  z++;
		  thread = atoi(argv[z]);
		  continue;
		}
	      else
		{
		  thread = atoi(argv[z]+2);
		  continue;
		}
	    }
	}
      strcat(params,argv[z]);
      strcat(params," ");
    }
  
  return callpov(path,params,logf,startframe,endframe, step, create_avi, thread);
}





/*****************************************************************************/
/** AniTMT -- A program for creating foto-realistic animations		    **/
/**   This file belongs to the component tga2avi    			    **/
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

#include "mytga.h"
#include "myavi.h"
#include "myfile.h"

#include <string>

#define maxexeclen 1000

readAVI Film;

std::string tga_names;

// search %0nd and replace with the number
std::string num_in_string( std::string str, int num ){
  std::string::size_type p1 = str.find("%0");
  std::string::size_type p2 = str.find("d",p1);
  
  int digits = atoi( str.substr( p1+2, p2 ).c_str() );
  
  char buf[20];
  sprintf(buf,"%0*d", digits, num );
  
  return str.substr(0,p1) + buf + str.substr(p2+1);
}

const char *TGAname(int nummer)
{
  return num_in_string( tga_names, nummer ).c_str();
}


void avi2tga( std::string TGAfilename, 
	      int min, int max, int step )
{
  tga_names = TGAfilename;

  for(int z=min;z<=max;z+= step)
    {
      printf(" Decoding frame %d... \n", z );
      Film.NewTGAFrame( TGAname(z) );
    }
}

int main(int argn,char **argv)
{
  if( argn > 1 )
    {
      if( scmp(argv[1],"--help") )
	argn = 1;
      if( scmp(argv[1],"/h") )
	argn = 1;
      if( scmp(argv[1],"-h") )
	argn = 1;
    }

  if(argn < 2)
    {
      printf(" Syntax:\n");
      printf("   avi2tga options \n\n");
      printf("   [-o tganames ]  name of the TGA-files that includes number of digits:\n");
      printf("                   %c0nd (f%c04d.tga for f0000.tga,...)\n\n",'%','%');
      printf("   [-i inputname]  filename of the AVI-file (film.avi)\n");
      printf("   [-s startframe] first frame number of the TGA set to use (0)\n");
      printf("   [-e endframe]   last  frame number of the TGA set to use\n");
      printf("   [-j step]       use every step frames (1)\n");
      printf("\n");
      printf(" example:\n");
      printf("   avi2tga -o %c04d.tga -i src.avi -e29\n",'%');

      return 0;
    }
  
  char *inputname = (char *) malloc( 100 );
  char *fpstext = (char *) malloc( 100 );
  char *mintext = (char *) malloc( 100 );
  char *maxtext = (char *) malloc( 100 );
  char *steptext = (char *) malloc( 100 );

  std::string tga_names = "f%04d.tga";
  strcpy(inputname, "film.avi");
  strcpy(fpstext, "24");
  strcpy(mintext, "0");
  maxtext[0] = 0;
  strcpy(steptext, "1");

  for(int z=1;z<argn;z++)
    {
      if((argv[z])[0]=='-')
	{
	  if((argv[z])[1]==0 )continue;

	  if((argv[z])[1]=='o')
	    {
	      if((argv[z])[2]==0)
		{
		  z++;
		  tga_names = argv[z];
		  continue;
		}
	      else
		{
		  tga_names = argv[z]+2;
		  continue;
		}
	    }

	  if((argv[z])[1]=='i')
	    {
	      if((argv[z])[2]==0)
		{
		  z++;
		  strcpy(inputname,argv[z]);
		  continue;
		}
	      else
		{
		  strcpy(inputname,argv[z]+2);
		  continue;
		}
	    }

	  if((argv[z])[1]=='f')
	    {
	      if((argv[z])[2]==0)
		{
		  z++;
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
		  z++;
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
		  z++;
		  strcpy(maxtext,argv[z]);
		  continue;
		}
	      else
		{
		  strcpy(maxtext,argv[z]+2);
		  continue;
		}
	    }
	  if((argv[z])[1]=='j')
	    {
	      if((argv[z])[2]==0)
		{
		  z++;
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
      else
	{
	  if( z==1 ){
	    strcpy(inputname,argv[z]);
	    continue;
	  }
	}

      printf("Warning: parameter %s unknown (see avi2tga --help)\n",argv[z]);
    }

  int anz = Film.LoadAVI( inputname );
  if( anz < 0 )
    {
      printf("Error: Could not open input file: %s\n", inputname);
      return -2;
    }
  int max;

  if(mintext[0] == 0)
    {
      printf("Error: minFramenumber not defined\n");
      return -1;
    }

  if(maxtext[0] == 0)
    {
      max = anz - 1;
      //printf("Error: maxFramenumber not defined\n");
      //return -1;
    }
  else
    {
      char *end;
      max = strtol(maxtext,&end,0);
      if( *end != 0 ){
	printf("Warning: parameter -e should be followed by an integer (ignoring -e)\n");
	max = anz-1;
      }
      if( max >= anz ) max = anz-1;
    }

  avi2tga(tga_names, 
	  strtol(mintext,(char **)NULL,0),
	  max,
	  strtol(steptext,(char **)NULL,0) );

  Film.closeAVI();

  return 0;
}













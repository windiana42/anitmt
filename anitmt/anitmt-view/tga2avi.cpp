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
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "mytga.h"
#include "myavi.h"
#include "myfile.h"

#include <string>

#define maxexeclen 1000

AVI Film;

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

std::string TGAname(int nummer)
{
  return num_in_string( tga_names, nummer );
}

void dowithtgaanf(char *AVIname,int fps,int anz,int startframe)
{
  aniinf INF;
  if( Film.getSize(TGAname(startframe).c_str(),INF) )
    {
      printf("%s:Error: Could not open first TGA!\n", TGAname(startframe).c_str());
      exit(-1);
    }

  INF.fps=fps;
  INF.anz=anz;
  if( Film.NewAVI(AVIname,INF) )
    {
      printf("Error: Could not write AVI!\n");
      exit(-1);
    }
}

void dowithtganow(int z)
{
  if( Film.AddTGAFrame(TGAname(z).c_str()) )
    printf("%s:Error: Could not open TGA!\n", TGAname(z).c_str());
}

void dowithtgaend()
{
  Film.closeAVI();
}

void tga2avi( std::string TGAname, char *AVIname, int fps, 
	      int min, int max, int step, int stellen)
{
  tga_names = TGAname;

  int film_fps = abs( int(fps / float(step) + 0.5) );
  if( film_fps == 0 ) film_fps = 1;
  dowithtgaanf(AVIname, film_fps, abs((max-min) / step)+1 ,min);

  for(int z=min;z<=max;z+= step)
    {
      printf(" Adding frame %d... \n", z );
      dowithtganow(z);
    }

  dowithtgaend();
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
      printf("   tga2avi options \n\n");
      printf("   [-i tganames ]  name of the TGA-files that includes number of digits:\n");
      printf("                   %c0nd (f%c04d.tga for f0000.tga,...)\n\n",'%','%');
      printf("   [-f fps]        frames per second (24)\n");
      printf("   [-o outputname] filename of the AVI-file (film.avi)\n");
      printf("   [-s startframe] first frame number of the TGA set to use (0)\n");
      printf("   -e endframe     last  frame number of the TGA set to use\n");
      printf("   [-j step]       use every step frames (1)\n");
      printf("\n");
      printf(" example:\n");
      printf("   tga2avi -i %c04d.tga -o dest.avi -e29\n",'%');

      return 0;
    }
  
  char *outputname = (char *) malloc( 100 );
  char *fpstext = (char *) malloc( 100 );
  char *mintext = (char *) malloc( 100 );
  char *maxtext = (char *) malloc( 100 );
  char *steptext = (char *) malloc( 100 );
  char *sanztext = (char *) malloc( 100 );

  std::string tga_names = "f%04d.tga";
  strcpy(outputname, "film.avi");
  strcpy(fpstext, "24");
  strcpy(mintext, "0");
  strcpy(steptext, "1");
  maxtext[0] = 0;
  sanztext[0] = 0;

  for(int z=1;z<argn;z++)
    {
      if((argv[z])[0]=='-')
	{
	  if((argv[z])[1]==0 )continue;

	  if((argv[z])[1]=='i')
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

	  if((argv[z])[1]=='o')
	    {
	      if((argv[z])[2]==0)
		{
		  z++;
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
      printf("Warning: parameter %s unknown (see avi2tga --help)\n",argv[z]);
    }

  if(mintext[0] == 0)
    {
      printf("Error: minFramenumber not defined\n");
      return -1;
    }

  if(maxtext[0] == 0)
    {
      printf("Error: maxFramenumber not defined\n");
      return -1;
    }

  if(sanztext[0] == 0)
    {
      int max = strtol(maxtext,(char **)NULL,0);
      if(max<10) strcpy(sanztext,"1");
      else
	{
	  if(max<100) strcpy(sanztext,"2");
	  else
	    {
	      if(max<1000) strcpy(sanztext,"3");
	      else
		strcpy(sanztext,"4");
	    }
	}
    }

  tga2avi(tga_names, outputname,
	  strtol(fpstext,(char **)NULL,0),
	  strtol(mintext,(char **)NULL,0),
	  strtol(maxtext,(char **)NULL,0),
	  strtol(steptext,(char **)NULL,0),
	  strtol(sanztext,(char **)NULL,0) );
  return 0;
}













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

#ifndef __mytga__
#define __mytga__

#include <stdio.h> 
#include <stdlib.h> 
#include "common.h"

typedef struct Headertyp{
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

Headertyp default_header( WORD width, WORD height ){
  Headertyp h;
  h.bIDFeldLen = 0;
  h.fColormapTyp = 0;
  h.fImageTyp = 2;
  h.wColorMapOriginLO = 0; // muﬂ getrennt werden! (wegen ungeradem Offset)
  h.wColorMapOriginHI = 0;
  h.wColorMapLengthLO = 0;
  h.wColorMapLengthHI = 0;
  h.bColorMapEntrySize = 0;
  h.wXOrigin = 0; // X-Ursprung
  h.wYOrigin = 0; // Y-Ursprung
  h.wWidth = width;
  h.wHeight = height;
  h.bpp = 24;
  h.fImageDescriptor = 0x20;
  return h;
}

class TGA{
public:
  Headertyp Header;

  FILE *dh;
  char open(char const *name){
    dh = fopen(name,"rb");
    if(dh==NULL)
      return -1;
    return 0;
  }
  void getHeader(){
    fread(&Header,sizeof(Header),1,dh);
  }
  int getWidth(){
    return Header.wWidth;
  }
  int getHeight(){
    return Header.wHeight;
  }
  
  static char writeTGA( char const *name, unsigned width, unsigned height, 
			char *data ){
    FILE *out;
    out = fopen(name,"wb");
    if(out==NULL) return -1;
    
    Headertyp head = default_header( width, height );
    fwrite( &head, sizeof(Headertyp),1,out );
    fwrite( data, 1, width*height*3, out );

    fclose(out);

    return 0;
  }

  void close() {
    if( dh != NULL ) fclose( dh );
  }

  TGA(){
    dh = NULL;
  }
  ~TGA(){
    close();
  }
};

#endif










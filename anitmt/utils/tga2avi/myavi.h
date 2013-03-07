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

#ifndef __myavi__
#define __myavi__

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "mytga.h"

#define bytes_per_pixel 3
// JUNKlänge muß durch 4 teilbar sein!
#define JUNK "AVI created by ANITMT, the best tool for POVray animations;)"

#define Bufferlen 1024

typedef struct AVIHtyp{
  DWORD dwMicroSecPerFrame;
  DWORD dwMaxBytesPerSec;
  DWORD dwReserved1;
  DWORD dwFlags;
  DWORD dwTotalFrames;
  DWORD dwInitialFrames;
  DWORD dwStreams;
  DWORD dwSuggestedBufferSize;
  DWORD dwWidth;
  DWORD dwHeight;
  DWORD dwScale;
  DWORD dwRate;
  DWORD dwStart;
  DWORD dwLength;
};

typedef struct aniinf{
  DWORD b;
  DWORD h;
  DWORD anz;
  DWORD fps;
};

typedef struct STRHtyp {
  DWORD fccType;
  DWORD fccHandler;
  DWORD dwFlags;
  DWORD dwReserved1;
  DWORD dwInitialFrames;
  DWORD dwScale;
  DWORD dwRate;
  DWORD dwStart;
  DWORD dwLength;
  DWORD dwSuggestedBufferSize;
  DWORD dwQuality;
  DWORD dwSampleSize;
  DWORD Reserved1;	
  DWORD Reserved2;	
};

typedef struct STRFtyp {
  DWORD dwImmer;
  DWORD dwWidth;
  DWORD dwHeight;
  DWORD dwFlag1;
  DWORD dwFlag2;
  DWORD dwFramelen;
  DWORD Reserved1;	
  DWORD Reserved2;	
  DWORD Reserved3;	
  DWORD Reserved4;	
};


#include <memory.h>

class AVI{
protected:
  AVIHtyp AVIH;
  STRHtyp STRH;
  STRFtyp STRF;
  
  aniinf INF;
  
  // Längen, die in den AVI-File geschrieben werden müssen
  DWORD Frameanz;
  DWORD Filelen;	
  DWORD Headerlen;   
  DWORD AVIHlen; 
  DWORD STRHlen;     
  DWORD STRFlen;
  DWORD STRLlen; 		
  DWORD JUNKlen;
  DWORD MOVIlen;     
  DWORD Framelen; 
  DWORD IDEXlen;	
  DWORD Indexlen;  	
  
  void fillHeader(){
    AVIH.dwMicroSecPerFrame = 1000000 / INF.fps;
    AVIH.dwMaxBytesPerSec=INF.b * INF.h * bytes_per_pixel * INF.fps; 
    //                              Movie Actor nimmt 4 statt INF.fps!
    AVIH.dwReserved1=0;
    AVIH.dwFlags=0x10;
    AVIH.dwTotalFrames = INF.anz;
    AVIH.dwInitialFrames=0;
    AVIH.dwStreams=1;
    AVIH.dwSuggestedBufferSize=0;
    AVIH.dwWidth = INF.b;
    AVIH.dwHeight = INF.h;
    AVIH.dwScale=AVIH.dwMicroSecPerFrame;
    AVIH.dwRate=1000000;
    AVIH.dwStart=0;
    AVIH.dwLength=AVIH.dwTotalFrames;
	
    STRH.fccType = 'v' + ((DWORD)'i' << 8) + ((DWORD)'d' << 16) +
                         ((DWORD)'s' << 24);
    STRH.fccHandler = 'D' + ((DWORD)'I' << 8) + ((DWORD)'B' << 16) + 
                            ((DWORD)' ' << 24);
    STRH.dwFlags = 0;
    STRH.dwReserved1 = 0;
    STRH.dwInitialFrames = 0;
    STRH.dwScale = AVIH.dwScale;
    STRH.dwRate = AVIH.dwRate;
    STRH.dwStart = 0;
    STRH.dwLength = AVIH.dwLength;
    STRH.dwSuggestedBufferSize = AVIH.dwSuggestedBufferSize;
    STRH.dwQuality = 0xFFFFFFFF;
    STRH.dwSampleSize = 0;
    STRH.Reserved1 = 0;	
    STRH.Reserved2 = 0;	
	
    STRF.dwImmer = 0x28;
    STRF.dwWidth = AVIH.dwWidth;
    STRF.dwHeight = AVIH.dwHeight;
    STRF.dwFlag1 = 0x00180001;
    STRF.dwFlag2 = 0x00000000;
    STRF.dwFramelen = AVIH.dwWidth * AVIH.dwHeight * bytes_per_pixel;
    STRF.Reserved1 = 0;	
    STRF.Reserved2 = 0;	
    STRF.Reserved3 = 0;	
    STRF.Reserved4 = 0;	
    
  }
  
  void Makelen(){
    Framelen = STRF.dwFramelen;
    Frameanz = AVIH.dwTotalFrames;
    JUNKlen = strlen(JUNK);
    
    AVIHlen = sizeof(AVIHtyp);
    
    STRHlen = sizeof(STRHtyp);
    STRFlen = sizeof(STRFtyp);
    STRLlen = 4 + STRHlen + 8 + STRFlen + 8;
    
    Indexlen = 16;
    
    Headerlen = 4 + AVIHlen + 8 + STRLlen + 8 + JUNKlen + 8;
    MOVIlen = (Framelen+8)*Frameanz + 4;
    IDEXlen = Indexlen*Frameanz;
    
    Filelen = Headerlen + 12 + MOVIlen + 8 + IDEXlen + 8;
  }
  
  FILE *dh;
  char openAVI(char const *name){
    dh = fopen(name,"wb");

    if(dh==NULL)
      return -1;
    return 0;
  }

  void putfccName(char const *name){
    fwrite(name,4,1,dh);
  }
  void putdwNum(DWORD dw){
    char num[4];
    num[0] =  dw        & 255;
    num[1] = (dw >>  8) & 255;
    num[2] = (dw >> 16) & 255;
    num[3] = (dw >> 24);
    fwrite(num,4,1,dh);
  }

  void putAVIH(){
    putfccName("avih");
    putdwNum(AVIHlen);
    fwrite(&AVIH,AVIHlen,1,dh);
  }  
  void putHDRL(){
    putfccName("LIST");
    putdwNum(Headerlen);
    putfccName("hdrl");
    putAVIH();
  }


  void putSTRH(){
    putfccName("strh");
    putdwNum(STRHlen);
    fwrite(&STRH,STRHlen,1,dh);
  }
  void putSTRF(){
    putfccName("strf");
    putdwNum(STRFlen);
    fwrite(&STRF,STRFlen,1,dh);
  }
  void putJUNK(){
    putfccName("JUNK");
    putdwNum(JUNKlen);
    fwrite(JUNK,JUNKlen,1,dh);
  }
  void putSTRL(){
    putfccName("LIST");
    putdwNum(STRLlen);
    putfccName("strl");
    putSTRH();
    putSTRF();
    putJUNK();
  }

  int aktFanz;
  DWORD *IPOS;
  DWORD *ILEN;
  
  void putFrame(){
    if(aktFanz)
      ILEN[aktFanz-1] = ftell(dh) - IPOS[aktFanz-1] - 8;
    IPOS[aktFanz] = ftell(dh);
    aktFanz++;
    char str[10];
    sprintf(str,"00db");
    putfccName(str);
    putdwNum(Framelen);
  }
  void putIndex(){
    if(aktFanz)
      ILEN[aktFanz-1] = ftell(dh) - IPOS[aktFanz-1] - 8;
    putfccName("idx1");
    putdwNum(IDEXlen);
    char str[10];
    for(int i=0;i<aktFanz;i++){
      sprintf(str,"00db");
      putfccName(str);
      putdwNum(Indexlen);
      putdwNum(IPOS[i]);
      putdwNum(ILEN[i]);
    }
  }
  void putMOVI(){
    putfccName("LIST");
    putdwNum(MOVIlen);
    putfccName("movi");
    IPOS = (DWORD*) malloc(Frameanz*4);
    ILEN = (DWORD*) malloc(Frameanz*4);
    aktFanz=0;
  }
  
  void putRIFF(){
    putfccName("RIFF");
    putdwNum(Filelen);
    putfccName("AVI ");
    putHDRL();
    putSTRL();
    putMOVI();
  }

public:

  char getSize(char const *TGAname,aniinf &ainf){
    TGA TGABild;
    if(TGABild.open(TGAname))return -1;
    TGABild.getHeader();
    
    ainf.b = TGABild.getWidth();
    ainf.h = TGABild.getHeight();

    return 0;
  }

  char NewAVI(char const *name,aniinf ainf){
    memmove(&INF,&ainf,sizeof(aniinf));
    fillHeader();
    Makelen();
    if(openAVI(name)) return -1;
    putRIFF();

    return 0;
  }
  
  char AddTGAFrame(char const *TGAname){
    putFrame();

    TGA TGABild;
    if(TGABild.open(TGAname))return -1;
    TGABild.getHeader();

    int linelen = TGABild.getWidth()*3;

    char *BUF = (char*) malloc(linelen);
    for( int i = 1; i<= TGABild.getHeight(); ++i ){
      // !!! check this condition for correctness !!!
      if( TGABild.Header.fImageDescriptor & 0x20 ) // are lines reverted
        fseek( TGABild.dh, -i*linelen, SEEK_END );
      fread(BUF,1,linelen,TGABild.dh);
      fwrite(BUF,1,linelen,dh);
    }

    return 0;
  }

  char closeAVI(){
    putIndex();
    if( dh!= NULL) fclose(dh);
    return 0;
  }
  
  AVI(){
    dh = NULL;
  }
  ~AVI(){
  }
};

class readAVI{
protected:
  AVIHtyp AVIH;
  STRHtyp STRH;
  STRFtyp STRF;
  
  aniinf INF;
  
  // Längen, die in den AVI-File geschrieben werden müssen
  DWORD Frameanz;
  DWORD Filelen;	
  DWORD Headerlen;   
  DWORD AVIHlen; 
  DWORD STRHlen;     
  DWORD STRFlen;
  DWORD STRLlen; 		
  DWORD JUNKlen;
  DWORD MOVIlen;     
  DWORD Framelen; 
  DWORD IDEXlen;	
  DWORD Indexlen;  	
  
  FILE *dh;
  char openAVI(char const *name){
    dh = fopen(name,"rb");

    if(dh==NULL)
      return -1;
    return 0;
  }
  void getfccName(){
    char name[4];
    fread(name,4,1,dh);
  }
  DWORD getdwNum(){
    DWORD dw;
    char num[4];
    fread(num,4,1,dh);
    dw =  num[0];
    dw += DWORD(num[1]) <<  8;
    dw += DWORD(num[2]) << 16;
    dw += DWORD(num[3]) << 24;
    return dw;
  }
  void getAVIH(){
    getfccName();
    AVIHlen=getdwNum();
    fread(&AVIH,AVIHlen,1,dh);
  }  
  void getHDRL(){
    getfccName();
    Headerlen = getdwNum();
    getfccName();
    getAVIH();
  }
  void getSTRH(){
    getfccName();
    STRHlen = getdwNum();
    fread(&STRH,STRHlen,1,dh);
  }
  void getSTRF(){
    getfccName();
    STRFlen = getdwNum();
    fread(&STRF,STRFlen,1,dh);
  }
  void getJUNK(){
    getfccName();
    JUNKlen = getdwNum();
    char *J = new char[JUNKlen];
    fread(J,JUNKlen,1,dh);
    delete[] J;			/* dump Junk immediately */
  }
  void getSTRL(){
    getfccName();
    STRLlen = getdwNum();
    getfccName();
    getSTRH();
    getSTRF();
    getJUNK();
  }
  void getFrame(){
    getfccName();
    Framelen = getdwNum();
  }
  void getMOVI(){
    getfccName();
    MOVIlen = getdwNum();
    getfccName();
  }
  void getRIFF(){
    getfccName();
    Filelen = getdwNum();
    getfccName();
    getHDRL();
    getSTRL();
    getMOVI();
  }
public:
  long LoadAVI(char const *name){
    if(openAVI(name)) return -1;
    getRIFF();
    
    return AVIH.dwTotalFrames;
  }
  
  char NewTGAFrame(char const *TGAname){
    getFrame();

    char *data = new char[AVIH.dwWidth*AVIH.dwHeight*3];
    long linelen = AVIH.dwWidth*3;
    long anf = (AVIH.dwHeight-1) * linelen;

    for( int i = 0; i< AVIH.dwHeight; ++i ){
      fread(data + anf - i*linelen, 1, linelen,dh);
    }
    TGA::writeTGA( TGAname, AVIH.dwWidth, AVIH.dwHeight, data );
    
    delete data;

    return 0;
  }

  char closeAVI(){
    if( dh!= NULL) fclose(dh);
    return 0;
  }
  
  readAVI(){
    dh = NULL;
  }
  ~readAVI(){
  }
};
#endif





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

#ifndef	__MYFILE__
#define __MYFILE__

#include<stdio.h> 
#include<stdlib.h> 
#include<string.h>

#include"mystr.h"
#include"myvektor.h"

/*****************************************************************************/
/*****************************************************************************/
/** Definition der Klassen + Implementation des Konstruktors u. Destruktors **/
/*****************************************************************************/
/*****************************************************************************/

/****************************************************************************/
/* Datei                                                                    */
/****************************************************************************/

class Datei{
 public:
  FILE *dh;
  char *DName;
  char open;
  long line;
  
  int eof(){
    return(feof(dh));
  }
};

/****************************************************************************/
/* InFile                                                                   */
/****************************************************************************/

#define t_braceleft     1
#define t_braceright    2
#define t_point         4
#define t_equal         8
#define t_semicolon    16
#define t_comma        32
#define t_all          63

#define t_CR          128

#define t_kleiner     256
#define t_groesser    512


#define maxwordlen 80
#define maxbufferlen 10000

#define CopyBufferMode 1
#define CopyDestMode   2


class OutFile;

class InFile:public Datei{
 public:
  char *trenn;
  int  trennanz;
  char lasttrenn;
  
  char ignorecomment;

  char    *CopyBuffer;
  int      cbuflen;

  OutFile *CopyDest;
  char    *CopyDestBuffer;
  int      CDBpos;

  int      copymode;
  
  char  setCopyDest(OutFile *Dest);
  char  setCopyBuffer();
  char  leaveCopyMode();
  char  clearCopyBuffer();
  char  clearCopyDestBuffer();

  int    getTrenn(long);
  char  *getWord(long);
  char  *getItem();
  int    getVal();
  int    getIntVal();
  double getFloatVal();
  vektor *getVektor();

  int   finishcomment(char typ);
  int   findStrings(my_string **,int);
  int   scanlineTrenn(long trennflag, char* &line);

  InFile(char *Name){
    line=1;

    trennanz=10;
    trenn=(char *)malloc(trennanz);
    trenn[0]='{';
    trenn[1]='}';
    trenn[2]='.';
    trenn[3]='=';
    trenn[4]=';';
    trenn[5]=',';
    trenn[6]=' ';
    trenn[7]='\n';
    trenn[8]='<';
    trenn[9]='>';
    lasttrenn=-1;

    ignorecomment=0;

    CopyBuffer=NULL; cbuflen=0;
    CopyDest=NULL; CopyDestBuffer=NULL; CDBpos=0;
    copymode=0;
    
    DName=SCopy(Name);

    dh=fopen(Name,"rt");
    if(dh==NULL)
      open=0;
    else
      open=1;
    
  }
  virtual ~InFile(){
    free(trenn);
    fclose(dh);
    free(DName);
  }
};

/****************************************************************************/
/* OutFile                                                                   */
/****************************************************************************/

class OutFile:public Datei{
 public:
  
  char putCh(char);
  char putS(char *my_string);
  char putInt(int);
  char putFloat(double);
  char putBuffer(char* &Buf,int &Buflen);

  OutFile(char const * Name){
    DName=SCopy(Name);
    dh=fopen(Name,"w");
    if(!dh)
      open=0;
    else
      open=1;
  }
  virtual ~OutFile(){
    fclose(dh);
    free(DName);
  }
};

/*****************************************************************************/
/*****************************************************************************/
/** Implementation der Klassen                                              **/
/*****************************************************************************/
/*****************************************************************************/

/****************************************************************************/
/* InFile                                                                   */
/****************************************************************************/


char InFile::setCopyBuffer()
{
  if( (CopyBuffer!=NULL) || (copymode & CopyBufferMode) ) return -1;
  CopyBuffer=(char *) malloc(maxbufferlen);
  copymode|=CopyBufferMode;
  return 0;
}

char InFile::setCopyDest(OutFile *Dest)
{
  if( (CopyDest!=NULL) || (copymode & CopyDestMode) ) return -1;
  CopyDest=Dest;
  copymode|=CopyDestMode;
  return 0;
}

char InFile::leaveCopyMode()
{
  if(!copymode)return -1;
  if(CopyDest!=NULL){
    CopyDest=NULL;
    if(CopyDestBuffer!=NULL){
      free(CopyDestBuffer);
      CopyDestBuffer=NULL;
    }
  }
  if(CopyBuffer!=NULL){
    free(CopyBuffer);
    CopyBuffer=NULL;
  }
  copymode=0;
  return 0;
} 

char InFile::clearCopyBuffer()
{
  if(CopyBuffer==NULL) return -1;
  free(CopyBuffer);
  CopyBuffer=NULL;
  cbuflen=0;
  return 0;
}

char InFile::clearCopyDestBuffer()
{
  if(CopyDestBuffer==NULL) return -1;
  free(CopyDestBuffer);
  CopyDestBuffer=NULL;
  CDBpos=0;
  return 0;
}

char *InFile::getWord(long trennflag)
{
  if(!open) return NULL;

  char *word=(char *)malloc(maxwordlen+1);
  int wlen=0;
  char fertig=0;

  my_string *commentstr[2];
  commentstr[0]=new my_string("//");
  commentstr[1]=new my_string("/*");
  
  if(ignorecomment){
    for(int t=0;t<2;t++)
      commentstr[t]->initisearch();
  }
  
  int schreib=1;
  
  char *sbuffer=NULL;
  int sPos;
  
  do{
    // einlesen eines Zeichens
    int c=fgetc(dh);
    if(c==EOF){
      if(wlen)
	break;
      else{
	free(word);
	return NULL;
      }
    }
    
    // fgetc liefert eigentlich einen int: int -> unsigned char
    unsigned char ch=(unsigned char) c;
    
    // ist ch ein besonderes "Trennzeichen" ?    
    for(int z=0;z<trennanz;z++)
      if( (trenn[z]==ch) && (trennflag & (1<<z) )) {
	if(wlen){
	  ungetc(ch,dh);
	  fertig=2;
	  break;
	}
	else{
	  // Wenn ganz normale Zeichen eingegeben werden
	  addstr(word,wlen,maxwordlen,ch);
	  fertig=1;
	  break;
	}
      }
    if(fertig==2)break; // wenn Trennzeichen am Ende -> sofort schluß   
 
    // soll das eingelesene unverändert gespeichert werden? (Kopie)
    if(copymode&CopyBufferMode){
      addstr(CopyBuffer,cbuflen,maxbufferlen,ch);
    }
    if(copymode&CopyDestMode){
      if(CopyDestBuffer==NULL)
	CopyDest->putCh(ch);
      else{
	addstr(CopyDestBuffer,CDBpos,maxbufferlen,ch);
      }
    }
    if(fertig) break; // wenn Trennzeichen alleine noch "copy"-en lassen

    // Sollen C-Kommentare ignoriert werden?    
    if(ignorecomment){
      schreib=1; // mal sehen ob es ein "comment" auf 0 setzt :-)

      for(int t=0;t<2;t++){
	int ret=commentstr[t]->isearch(ch);
	if(ret<0){
          finishcomment(t);
	  clearCopyDestBuffer();

	  free(sbuffer);
	  sbuffer=NULL;

	  schreib=2;
	}       
	if(ret>0){
	  schreib=0;
	  if(sbuffer==NULL){	      
	    sbuffer=(char *)malloc(maxwordlen);
            sPos=0;         
          }
	}    
      }       
      if(schreib==2) // wurde ein Kommentar gefunden
	ch=' '; // Kommentar zählt wie leerzeichen
      
      if((schreib!=0) && (sbuffer!=NULL)){
	for(int z=0;z<sPos;z++){
	  addstr(word,wlen,maxwordlen,sbuffer[z]);
	}         
	free(sbuffer);
	sbuffer=NULL;
	schreib=1;	        
      }
    }

    // Zeichen auswerten
    switch( ch ){
    case '\n':
      if(wlen)fertig=1;
      line++;
      break;
    case '\r':
      printf("nächste DOS-Zeile\n");
      break;
    case ' ':
    case '\t':
      if(wlen)fertig=1;
      break;
    default: // Wenn ganz normale Zeichen eingegeben werden
      if(schreib)
        addstr(word,wlen,maxwordlen,ch); 
      else
        addstr(sbuffer,sPos,maxwordlen,ch);      
    }
  }while(!fertig);

  if(sbuffer!=NULL){
    free(sbuffer);
    printf("warning: Unclosed comment!\n");
  }
  word[wlen]='\0';
  return word;
}

int InFile::getTrenn(long trennflag)
{
  char *str;
  str=getWord(trennflag);  
  for(int z=0;z<trennanz;z++){
    if( (str[0]==trenn[z]) && (trennflag & (1<<z)) && (strlen(str)==1))
      return 0;
  }
  return -1;
}  

// Fehlt noch einiges... !!!

int InFile::getVal()
{
  int w;
  fscanf(dh,"%d",&w);
  return w;
}

int InFile::getIntVal()
{
  int w=(int)strtol(getWord(0),(char **)NULL,0);
  return w;
}

double InFile::getFloatVal()
{
  double w;
  fscanf(dh,"%lf",&w);
  return w;
}

vektor *InFile::getVektor()
{
  vektor *v=new vektor();
  char *Item=getWord(t_kleiner|t_groesser);
  if( strcmp(Item,"<") ){
    printf("< expected\n");
    return NULL;
  }

  v->x = getFloatVal();
  Item=getWord(t_all);
  if( strcmp(Item,",") ){
    printf(", expected\n");
    return NULL;
  }

  v->y = getFloatVal();
  Item=getWord(t_all);
  if( strcmp(Item,",") ){
    printf(", expected\n");
    return NULL;
  }

  v->z = getFloatVal();
  Item=getWord(t_all);
  if( strcmp(Item,">") ){
    printf("> expected\n");
    return NULL;
  }
  return v;
}

// bei get-funktionen !!!

/**************************************************************************/
/*  findStrings                          sucht <anz> Strings in der Datei */
/**************************************************************************/
/*  Rückgabe (int):                                                       */
/*    -1 : String ist gefunden                                            */
/*    >=0: Position im String bis zu dem der String gefunden wurde        */
/**************************************************************************/
 
int InFile::findStrings(my_string* *strings,int anz)
{
  for(int i=0;i<anz;i++){
    strings[i] -> initisearch();
  }
	  
  int found=-1;
  do{
    int c=fgetc(dh);
    if(c==EOF)break;
    unsigned char ch = (unsigned char)c;
    char schreib=1;
    for(int i=0;i<anz;i++){
      int zust=strings[i] -> isearch(ch);
      if(zust ==-1) found=i;
      if(zust != 0) schreib=0;
    }


    // Hier soll eine Kopie der Datei nebenher angelegt werden
    if(copymode&CopyDestMode){
      if(schreib){
	if(CopyDestBuffer!=NULL){
	  CopyDest->putBuffer(CopyDestBuffer,CDBpos);
	}	    
	CopyDest->putCh(ch);
      }
      else{
	if(CopyDestBuffer==NULL){
	  CopyDestBuffer=(char *)malloc(maxbufferlen);
	  CDBpos=0;
	}
	addstr(CopyDestBuffer,CDBpos,maxbufferlen,ch);
      }
    }	
    if(found !=-1)return found;
  }while(1);
  return -1;
}

int InFile::finishcomment(char typ)
{
  char nl[2]={'\n',0}; // new line !!!! muß für DOS vielleicht geändert werden!
  my_string *commentend[2];
  commentend[0]=new my_string(nl);
  commentend[1]=new my_string("*/");
  unsigned char ch;

  commentend[typ]->initisearch();
  do{
    int c=fgetc(dh);
    if(c==EOF){
      return -1;
    }
    
    ch=(unsigned char) c;
    
    if(copymode&CopyBufferMode){
      addstr(CopyBuffer,cbuflen,maxbufferlen,ch);
    }
    if(copymode&CopyDestMode){
      if(CopyDestBuffer==NULL)
	CopyDest->putCh(ch);
      else{
	addstr(CopyDestBuffer,CDBpos,maxbufferlen,ch);
      }
    }
  }while(commentend[typ]->isearch(ch)>=0);  // solange die Kombination nicht kommt
  delete commentend[0];
  delete commentend[1];
  return 0;
}

/*
int InFile::scanlineTrenn(long trennflag, char* &line)
{
  line=malloc(maxwordlen+1);
  fgets(line,maxwordlen,dh);
  int len=strlen(line);
  for(int z=0;z<len;z++){

    for(int y=0;y<trennanz;y++){
      if( (line[z]==trenn[y]) && (trennflag & (1<<y)) )
	return 0;
    }

  }
  return -1;
}  
*/


/****************************************************************************/
/* OutFile                                                                  */
/****************************************************************************/

char OutFile::putCh(char ch)
{
  if(!open) return -1;
  fputc(ch,dh);
  return 0;
}

char OutFile::putS(char *my_string)
{
  if(!open) return -1;
  fputs(my_string,dh);
  return 0;
}

char OutFile::putInt(int val)
{
  if(!open) return -1;
  fprintf(dh,"%d",val);
  return 0;
}

char OutFile::putFloat(double val)
{
  if(!open) return -1;
  fprintf(dh,"%f",val);
  return 0;
}

char OutFile::putBuffer(char* &Buf,int &Buflen)
{
  if(!open) return -1;
  if(Buf==NULL) return -2;
  fwrite(Buf,Buflen,1,dh);
  free(Buf);
  Buf=NULL;
  Buflen=0;
  return 0;
}

#endif








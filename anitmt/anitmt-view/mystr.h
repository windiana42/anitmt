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

/*****************************************************************************/
/* mystr.cpp : basic string functions                                        */
/*****************************************************************************/
/*                                                                           */
/* Date:        6.11.99                                                      */
/* Author:      Martin Trautmann                                             */
/*                                                                           */
/* Remarks:                                                                  */
/*                                                                           */
/*****************************************************************************/

#ifndef __MYSTR__
#define __MYSTR__

#define maxlen 1000
#define maxlen2 32000

#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<ctype.h>

char *SCopy(char const *str)
{
  char *d=(char *)malloc(strlen(str)+1);
  strcpy( d , str );

  return d;
}

// vergleicht zwei Strings ohne Beachtung von Gross-Kleinschreibung
char scmp(char const *s1,char const *s2)
{
  int len=strlen(s1);
  for(int i=0;i<=len;i++)
  {
    if(toupper(s1[i]) != toupper(s2[i]))return 0;
  }
  return 1;
}

void scpy(char *s1,int i1,char *s2,int i2)
{
  int i,j;
  for(i=i2,j=i1;i<maxlen2;i++,j++)
  {
    if(s2[i]==0){s1[j]='\0';break;}
    s1[j]=s2[i];
  }
 
}

int addstr(char *str,int &pos,int len,char zeichen)
{
  if(pos < len){
    str[pos]=zeichen;
    pos++;
  }
  else{
    printf("warning: Stringsize too small! Try to shorten sensless parts of the file\n");
    return -1;
  }
  return 0;
}

// liefert ein Wort bis zum Auftreten eines bestimmten Zeichens zurueck
char *getTill( char *text, char ende )
{
  int len = strlen(text);
  int last=len;

  for( int i=0; i<len; i++ )
    if( text[ i ] == ende )
      {
	last = i;
	break;
      }

  char *d=(char *)malloc(last+1);
  memcpy( d , text, last );
  text[last] = 0;

  return d;
}

class my_string{
 public:
  char *str;
  int aktpos;

  int initisearch(){
    aktpos=0;    
    return 0;
  }

  /**************************************************************************/
  /*  isearch       sucht den String in einer Reihe von übergebenen Zeichen */ 
  /**************************************************************************/
  /*  Rückgabe (int):                                                       */ 
  /*    -1 : String ist gefunden                                            */ 
  /*    >=0: Position im String bis zu dem der String gefunden wurde        */ 
  /**************************************************************************/

  int isearch(char ch){ // incrementelle Suche
    if(str[aktpos]==ch){
      aktpos++;
      if(str[aktpos] == 0){ // string ganz durchsucht
	aktpos=0;
	return -1;
      }
    }
    else
      aktpos = 0;
    return aktpos;
  }

  int len(){
    return(strlen(str));
  }

  my_string(char *setstr){
    str = SCopy( setstr );
  }

  ~my_string(){
    free(str);
  }

};

#define AnfBufLen 100

class fifobuf{
 public:
  char *buf;
  int buflen;

  int start;
  int len;

  void put( char ch )
    {
      buf[ (start+len) % buflen ] = ch;
      len++;
      
      if( len == buflen )
	{
	  char *oldbuf = buf;
	  int oldbuflen = buflen;

	  buflen *= 10;
	  buf = (char *)malloc( buflen );
	  memcpy( buf                    , oldbuf + start, oldbuflen - start );
	  memcpy( buf + oldbuflen - start, oldbuf        , start );
	  free( oldbuf );
	  start = 0;
	}
    }
  char get()
    {
      if( !len ) return 0;

      char ret = buf[ start ];
      start ++;
      start %= buflen;
      len --;

      return ret;
    }

  fifobuf()
    {
      buflen = AnfBufLen;
      buf = (char *) malloc( buflen );
      start = 0;
      len = 0;
    }

  ~fifobuf()
    {
      free( buf );
    }
};

class lifobuf{
 public:
  char *buf;
  int buflen;

  int len;

  void put( char ch )
    {
      buf[ len ] = ch;
      len++;
      
      if( len == buflen )
	{
	  char *oldbuf = buf;
	  int oldbuflen = buflen;

	  buflen *= 10;
	  buf = (char *)malloc( buflen );
	  memcpy( buf, oldbuf, oldbuflen );
	  free( oldbuf );
	}
    }

  char get()
    {
      if( !len ) return 0;

      len --;
      char ret = buf[ len ];

      return ret;
    }

  lifobuf()
    {
      buflen = AnfBufLen;
      buf = (char *) malloc( buflen );
      len = 0;
    }

  ~lifobuf()
    {
      free( buf );
    }
};

#endif





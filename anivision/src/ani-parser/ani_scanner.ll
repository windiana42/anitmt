%{
/*
 * ani-parser/scanner.ll.
 * 
 * Lexical analyzer for animation language parser and for 
 * POV command comment parser. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2002--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * Early roots of the flex code were inspired by Martin Trautmann's 
 * scanner.ll for AniTMT's funcgen. 
 * Uses reentrant C scanner and thus needs flex-2.5.11 or higher. 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

/* TODO: 
 * String parsing: recognize \n\t\", etc. (but do not allow \0?!)
 * Must count errors (e.g. unterminated string, comment)
 */

#include <hlib/prototypes.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

/* This also includes the tokens (grammar.hh). */
#include "ani_scanner.h"

#include <assert.h>


#define MyMagic 0xcafebabeU


/* Private part of ani_scanner_context: */
struct ani_scanner_private
{
	/* For simple detection of memory corruption or evil */
	/* pointer mesh... */
	unsigned int magic;
	
	/* The scanner used for scanning. Only one. */
	/* This is NULL, if scanner inactive, i.e. no input. */
	yyscan_t scanner;
	
	/* Which input the scanner is currently reading from: */
	AniLexerScanner::ALSInput *top_inp;
	/* Which input has the token buffer we're currently processing. */
	/* Is different from top_inp around an #include. */
	AniLexerScanner::ALSInput *read_inp;
	
	/* Current include hierarchy (->SetInput(,ALS_PF_Push)) depth. */
	int file_depth;
	
	/* Warned that this comment is nested? */
	int nested_comment_warned;
	/* Number of newlines in the C-style comment: */
	int newlines_in_ccomment;
	
	/* Does not set magic value: */
	ani_scanner_private();
	~ani_scanner_private();
	_CPP_OPERATORS
};


#define YY_EXTRA_TYPE  class AniLexerScanner*
#define YY_NO_UNPUT

	/* Note: yylloc of type YYLTYPE */
#define INC_COL() \
	do { yyextra->lpos1+=yyleng; } while(0)
	/* Call for one TAB (\t): */
#define INC_COL_TAB() \
	do { yyextra->lpos1=(yyextra->lpos1/yyextra->tab_width+1)*yyextra->tab_width; } while(0);
#define INC_LINE() \
	do { ++yyextra->line1;  yyextra->lpos1=0; } while(0)
	/* Update position after having read the passed character. */
#define INC_POS_CHAR(c) \
	do { \
		switch(c) \
		{ \
			case '\n':  INC_LINE();  break; \
			case '\r':  break; \
			case '\t':  INC_COL_TAB();  break; \
			default:  ++yyextra->lpos1;  break; \
		} \
	} while(0)
	/* Update position after having read the passed string which */
	/* possibly contains special chars (newline, tab, return).   */
#define INC_POS_TEXT(str,len) \
	do { \
		for(char *c=str,*cend=str+len; c<cend; c++) \
		{  INC_POS_CHAR(*c);  } \
	} while(0)
	/* Make current pos the beginning of a token. Call after 
	 * whitespace and comments. */
#define POS_RE_SET() \
	do { yyextra->line0=yyextra->line1; \
	     yyextra->lpos0=yyextra->lpos1; } while(0);

/* This macro is needed to set the yyscanner pointer used by the 
 * access macros like yyextra, yyin, etc. */
#define YY_SCANNER_MAGIC \
	yyscan_t yyscanner=p->scanner; \
	/* Special hack because the macros yyextra, etc. need yyg... (flex-2.5.31) */ \
	struct yyguts_t *yyg = (struct yyguts_t*)yyscanner;

#define YY_INPUT(buf,result,max_size) \
	{ \
		AniLexerScanner::ALSInput *inp=(AniLexerScanner::ALSInput*)yyin;  \
		ssize_t res=inp->read(buf,max_size); \
		if(res<0)  /* fatal error */  \
		{  abort();  }  \
		result=res;  \
	}

%}

/* Generate scanner file by using:                   */
/* flex -s scanner.ll    [works with flex >= 2.5.10] */

%pointer
%option prefix="AniS_"
%option outfile="ani_scanner.cc"
%option noyywrap
/*%option yylineno*/
%option reentrant
%option bison-bridge 
/* option bison-locations does NOT calculate locations; just passes 
 * the structure to store the pos. Then, I don't need it. */
 /*%option bison-locations*/
%option stack

	/*IN_CMT    -> C-style comment   */
	/*IN_CPPCMT -> C++-style comment */
	/*IN_PP     -> preprocessor      */
%x IN_CMT
%x IN_CPPCMT
%x IN_PP
	/* Main parsing states. Parsing must be done in one of these states: */
	/* IN_ANI -> parse animation file */
	/* IN_POV -> parse POV file _command_comment_, NOT the rest. */
%s IN_ANI
%s IN_POV

identifier  \$?[[:alpha:]_][[:alnum:]_]*
ninteger    [[:digit:]]+
xinteger    (0?[[:digit:]]+)|(0x[[:xdigit:]]+)
/* Integer with beginning range spec: */
integer_r   {xinteger}+\.\.
_floatexp   [eE][+-]?{ninteger}
float       ((({ninteger}\.)|([[:digit:]]*\.{ninteger})){_floatexp}?)|({ninteger}{_floatexp})
string      \"([^\"\n]|\\\")*\"
ut_string   \"([^\"\n]|\\\")*

%%

"//"  {
		INC_COL();
		yy_push_state(IN_CPPCMT,yyscanner);
	}
<IN_CPPCMT>.*\n  {
		INC_LINE();
		POS_RE_SET();
		yy_pop_state(yyscanner);
	}
<IN_CPPCMT><<EOF>>  {
		INC_COL();
		POS_RE_SET();
		yy_pop_state(yyscanner);
		yyextra->p->top_inp->read_eof=1;
		return(0);
	}
	/* This rule will normally not match (only if terminated by EOF). */
<IN_CPPCMT>.  {
		INC_POS_CHAR(yytext[0]);
	}

"/*"  {
		INC_COL();
		yy_push_state(IN_CMT,yyscanner);
		yyextra->p->nested_comment_warned=0;
		yyextra->p->newlines_in_ccomment=0;
	}
<IN_CMT>"*/"  {
		INC_COL();
		POS_RE_SET();
		yy_pop_state(yyscanner);
		/* If called from preprocessor, leave PP state when comment */
		/* was more than 1 line. */
		if(YY_START==IN_PP && yyextra->p->newlines_in_ccomment)
		{  yy_pop_state(yyscanner);  }
	}
<IN_CMT>"/*"  {
		POS_RE_SET();  /* BEFORE inc_col here. */
		INC_COL();
		if(yyextra->allow_nested_comments)
		{
			if(yyextra->allow_nested_comments>=2 && 
			   !yyextra->p->nested_comment_warned)
			{
				Warning(yyextra->_MakeCurrLocation(),
					"nested C-style comment\n");
				++yyextra->p->nested_comment_warned;
			}
			yy_push_state(IN_CMT,yyscanner);
		}
		POS_RE_SET();
	}
<IN_CMT>[^\n/\*]*\n    {  INC_LINE();  ++yyextra->p->newlines_in_ccomment;  }
<IN_CMT>[^\n\r\t/\*]*  {  INC_COL();  }
	/* The following rule cannot be matched. */
	/* <IN_CMT>\n             {  INC_LINE();  ++yyextra->p->newlines_in_ccomment;  } */
<IN_CMT>\r             {  ; /* DOS/MAC crap */  }
<IN_CMT>\t             {  INC_COL_TAB();  }
<IN_CMT><<EOF>>  {
		INC_COL();
		/* FIXME: we do not yet report where the unterminated comment starts!!! */
		Error(yyextra->_MakeCurrLocation(/*start=*/yyextra->line1,yyextra->lpos1),
			"unterminated C-style comment\n");
		++yyextra->n_errors;
		yyextra->p->top_inp->read_eof=1;
		while(YY_START==IN_CMT || YY_START==IN_PP)
		{  yy_pop_state(yyscanner);  }
		return(0);
	}
<IN_CMT>.              {  INC_COL();  /* single '*' would fail otherwise */  }

	/* Check for preprocessor commands: */
<IN_ANI>[ \t]*"#"  {
		int _lpos=yyextra->lpos0;
		INC_POS_TEXT(yytext,yyleng-1);   /* All but the "#". */
		POS_RE_SET();
		INC_POS_CHAR(yytext[yyleng-1]);  /* The "#" alone. */
		/* Check if this is a preprocessor command: */
		if(_lpos==0)  /* At the beginning of the line */
		{  yy_push_state(IN_PP,yyscanner);  }
		else
		{  return(yytext[yyleng-1]);  }
	}
<IN_PP>"include"[ \t]+{string}  {
		/* Parse include statement. */
		/* Set correct position for beginning of string: */
		int start_line=yyextra->line1,start_lpos=yyextra->lpos1;
		char *fname=NULL,*fnameend=NULL;
		int fname_line=-1,fname_lpos=-1;
		int fail=0;
		for(char *c=yytext,*cend=yytext+yyleng; c<cend; c++)
		{
			if(*c=='\"' && !fail)
			{
				++yyextra->lpos1;
				if(!fname)
				{
					fname=c+1;
					fname_line=yyextra->line1;
					fname_lpos=yyextra->lpos1-1;
				}
				else if(!fnameend)
				{  fnameend=c;  }
				else goto write_error;
			}
			else if(*c=='\t')
			{  INC_COL_TAB();  }
			else
			{  ++yyextra->lpos1;  }  /* Do NOT use INC_COL() here. */
		}
		if(!fail && !fnameend)
		{
			write_error:;
			Error(yyextra->_MakeCurrLocation(),
				"invalid include statement: parse error at %d:%d\n",
				yyextra->line1,yyextra->lpos1);
			++yyextra->n_errors;
			fail=1;
		}
		if(!fail)
		{
			RefString tmp;
			tmp.set0(fname,fnameend-fname);
			int rv=yyextra->SetInput(tmp,ALS_PF_File|ALS_PF_Push,
				/*first extra token=*/-1);
			assert(rv!=-3);
			if(rv)
			{
				Error(yyextra->_MakeCurrLocation(
					/*start=fname_line,fname_lpos*/),
					"failed to #include \"%s\": %s\n",
					tmp.str(),
					rv==-4 ? "includes nested too deeply" : strerror(errno));
				++yyextra->n_errors;
			}
			else
			{
				/* Must push initial state here in order not to read */
				/* the new file in preprocessor state. */
				yy_push_state(yyextra->main_start_cond,yyscanner);
				yyextra->p->top_inp->must_pop_state=1;
				yyextra->p->top_inp->must_pop_pos_arch=1;
				
				assert(yyextra->pos_arch);
				if(!yyextra->pos_arch->IncludeFile(tmp,start_line,start_lpos))
				{  CheckMalloc(NULL);  }
			}
		}
	}
<IN_PP>[ \t\r]+  {  INC_POS_TEXT(yytext,yyleng);  POS_RE_SET();  }
<IN_PP>"//"  {
		INC_COL();
		yy_pop_state(yyscanner);
		yy_push_state(IN_CPPCMT,yyscanner);
	}
<IN_PP>"/*"  {
		INC_COL();
		yy_push_state(IN_CMT,yyscanner);
		yyextra->p->nested_comment_warned=0;
		yyextra->p->newlines_in_ccomment=0;
	}
<IN_PP><<EOF>>  {
		/* End of preprocessor command. */
		INC_LINE();  POS_RE_SET();
		yy_pop_state(yyscanner);
		yyextra->p->top_inp->read_eof=1;
		return(0);
	}
<IN_PP>\n  {
		/* End of preprocessor command. */
		INC_LINE();  POS_RE_SET();
		yy_pop_state(yyscanner);
	}
<IN_PP>{string}|{identifier}|{xinteger}|.  {
		POS_RE_SET();
		INC_POS_TEXT(yytext,yyleng);
		Error(yyextra->_MakeCurrLocation(),
			"garbage in preprocessor line - \"%.*s\"\n",
			yyleng,yytext);
		#warning "set preproc_error_reported=1..."
		++yyextra->n_errors;
		POS_RE_SET();
	}

	/* Match whitespace: */
[ \t]+  {
		INC_POS_TEXT(yytext,yyleng);
		#if 0
		const char *lstart=yytext;
		const char *cend=yytext+yyleng;
		/* Count lines and find last newline: */
		for(const char *c=yytext; c<cend; c++)
		{	if(*c=='\n')
			{  INC_LINE();  lstart=c+1;  }  }
		for(const char *c=lstart; c<cend; c++)
		{  INC_POS_CHAR(*c);  }
		#endif
		/* As this is not a token, re-set token start pos: */
		POS_RE_SET();
	}
\r  {  ; /* DOS/MAC crap */  }
\n  {  INC_LINE();  POS_RE_SET();  }


"=="	{  INC_COL();  return(TS_EQ);  }
"!="	{  INC_COL();  return(TS_NEQ);  }
"<="	{  INC_COL();  return(TS_LE);  }
">="	{  INC_COL();  return(TS_GE);  }
"&&"	{  INC_COL();  return(TS_ANDAND);  }
"||"	{  INC_COL();  return(TS_OROR);  }
".."	{  INC_COL();  return(TS_DOTDOT);  }
"+="    {  INC_COL();  return(TS_ASS_PLUS);  }
"-="    {  INC_COL();  return(TS_ASS_MINUS);  }
"*="    {  INC_COL();  return(TS_ASS_MUL);  }
"/="    {  INC_COL();  return(TS_ASS_DIV);  }
"::"    {  INC_COL();  return(TS_COLONCOLON);  }
"++"    {  INC_COL();  return(TS_PLUSPLUS);  }
"--"    {  INC_COL();  return(TS_MINUSMINUS);  }
"->"    {  INC_COL();  return(TS_MAPPING);  }

<IN_ANI>"int"     {  INC_COL();  return(TSL_INT);  }
<IN_ANI>"scalar"  {  INC_COL();  return(TSL_SCALAR);  }
<IN_ANI>"range"   {  INC_COL();  return(TSL_RANGE);  }
<IN_ANI>"vector"  {  INC_COL();  return(TSL_VECTOR);  }
<IN_ANI>"matrix"  {  INC_COL();  return(TSL_MATRIX);  }
<IN_ANI>"string"  {  INC_COL();  return(TSL_STRING);  }
<IN_ANI>"anyobj"  {  INC_COL();  return(TSL_ANYOBJ);  }

<IN_ANI>"new"     {  INC_COL();  return(TS_NEW);  }
<IN_ANI>"delete"  {  INC_COL();  return(TS_DELETE);  }
<IN_ANI>"if"      {  INC_COL();  return(TS_IF);  }
<IN_ANI>"else"    {  INC_COL();  return(TS_ELSE);  }
<IN_ANI>"for"     {  INC_COL();  return(TS_FOR);  }
<IN_ANI>"do"      {  INC_COL();  return(TS_DO);  }
<IN_ANI>"while"   {  INC_COL();  return(TS_WHILE);  }
<IN_ANI>"break"   {  INC_COL();  return(TS_BREAK);  }
<IN_ANI>"return"  {  INC_COL();  return(TS_RETURN);  }

<IN_ANI>"const"   {  INC_COL();  return(TS_CONST);  }
<IN_ANI>"static"  {  INC_COL();  return(TS_STATIC);  }

<IN_ANI>"object"    {  INC_COL();  return(TS_OBJECT);  }
<IN_ANI>"method"    {  INC_COL();  return(TS_METHOD);  }
<IN_ANI>"setting"   {  INC_COL();  return(TS_SETTING);  }
<IN_ANI>"animation" {  INC_COL();  return(TS_ANIMATION);  }

<IN_ANI>"this"    {  INC_COL();  return(TS_THIS);  }
<IN_ANI>"NULL"    {  INC_COL();  return(TS_NULL);  }


<IN_POV>"object"  {  INC_COL();  return(TSP_OBJECT);  }
<IN_POV>"params"  {  INC_COL();  return(TSP_PARAMS);  }
<IN_POV>"defs"    {  INC_COL();  return(TSP_DEFS);  }
<IN_POV>"macro"   {  INC_COL();  return(TSP_MACRO);  }
<IN_POV>"declare" {  INC_COL();  return(TSP_DECLARE);  }
<IN_POV>"include" {  INC_COL();  return(TSP_INCLUDE);  }
<IN_POV>"type"    {  INC_COL();  return(TSP_TYPE);  }
<IN_POV>"append_raw" {INC_COL(); return(TSP_APPEND_RAW);  }
<IN_POV>"fileID"  {  INC_COL();  return(TSP_FILEID);  }


{float}  {
		int tok;
		INC_COL();
		tok=yyextra->_ParseFloat(yytext,yyleng,yylval);
		return(tok ? tok : TS_FLOAT);  /* Correct. */
	}
{xinteger}  {
		int tok;
		INC_COL();
		tok=yyextra->_ParseInt(yytext,yyleng,yylval);
		return(tok ? tok : TS_FLOAT);  /* Correct. */
	}
{integer_r}  {
		int tok;
		INC_COL();
		tok=yyextra->_ParseInt(yytext,yyleng-2,yylval);
		
		/* Make sure next token is ".."; we already ate it. */
		/* This does no correct location reporting. FIXME */
		/* (TSINTEGER end contains "..", the ".." itself has start=end=<end of TSINTEGER>) */
		assert(yyextra->p->top_inp->special_next_tok==-1);  /* No token may be currently set. */
		yyextra->p->top_inp->special_next_tok=TS_DOTDOT;
		
		return(tok ? tok : TS_FLOAT);  /* Correct. */
	}
{ut_string}  {
		INC_COL();
		Error(yyextra->_MakeCurrLocation(),
			"unterminated string\n");
		++yyextra->n_errors;
		return(0);
	}
{string}  {
		INC_COL();
		yyextra->_ParseString(yytext,yyleng,yylval);
		return(TS_STRING);
	}
{identifier}  {
		INC_COL();
		yylval->str_val=LStrNDup(yytext,yyleng);
		return(TS_IDENTIFIER);
	}

<<EOF>>	 {
		/* The normal action would be to YYWarp() here but I cannot */
		/* because of the token buffer. */
		/*if(yyextra->_YYWrap())*/
		/*{  return(0);  }*/
		yyextra->p->top_inp->read_eof=1;
		return(0);
	}

.	{  INC_COL();  return(yytext[0]);  }

%%

/* -----------<INTERNAL SUPPORT ROUTINES>----------- */

/* Return value: IS_INTEGER, TS_FLOAT or 0 on error. */
int AniLexerScanner::_ParseInt(char *yy_text,int yy_leng,YYSTYPE *yy_lval)
{
	int rv=TS_INTEGER;
	char *end;
	char tmp=yy_text[yy_leng];
	yy_text[yy_leng]='\0';
	
	errno=0;
	yy_lval->int_val=strtol(yy_text,&end,0);
	if(errno==ERANGE)
	{
		if(yy_text[0]=='0' && (yy_text[1]=='x' || isdigit(yy_text[1])) )
		{
			Error(_MakeCurrLocation(),
				"non-decimal argument `%s' is out of integer range\n",
				yy_text);
			++n_errors;
			return(0);
		}
		else
		{
			Error(_MakeCurrLocation(),
				"argument `%s' is out of integer range, using scalar\n",
				yy_text);
			rv=_ParseFloat(yy_text,yy_leng,yy_lval);
		}
	}
	else
	{
		/* Check for trouble in octal int spec: */
		if(*end && (*end=='8' || *end=='9') && 
			yy_text[0]=='0' && isdigit(yy_text[1]))
		{
			Error(_MakeCurrLocation(),
				"digit `%c' out of range for octal base in `%s'\n",
				*end,yy_text);
			++n_errors;
			return(0);
		}
		else assert(!(*end));   /* make sure whole string could be read */
	}
	
	yy_text[yy_leng]=tmp;
	return(rv);
}

/* Return value: TS_FLOAT or 0 on error. */
int AniLexerScanner::_ParseFloat(char *yy_text,int yy_leng,YYSTYPE *yy_lval)
{
	int rv=TS_FLOAT;
	char *end;
	char tmp=yy_text[yy_leng];
	yy_text[yy_leng]='\0';
	
	errno=0;
	yy_lval->float_val=strtod(yy_text,&end);
	if(errno==ERANGE)
	{
		Error(_MakeCurrLocation(),
			"argument `%s' is out of scalar range\n",yy_text);
		++n_errors;
		rv=0;
	}
	else assert(!(*end));   /* make sure whole string could be read */
	
	yy_text[yy_leng]=tmp;
	return(rv);
}

int AniLexerScanner::_ParseString(char *yy_text,int yy_leng,YYSTYPE *yy_lval)
{
	char *start=yy_text;
	char *end=start+yy_leng;
	assert(*start=='\"' && *(end-1)=='\"');
	
	char *cpy=LStrNDup(yy_text+1,yy_leng-2);
	yy_lval->str_val=cpy;
	
	/* Replace escapes in the string:                   */
	/* NOTE: This works because we know that the string */
	/*       may be getting smaller but NEVER larger.   */
	char *dest=cpy;
	for(char *src=cpy; *src; src++)
	{
		if(*src=='\\')
		{
			++src;
			switch(*src)
			{
				case 'a':  *(dest++)='\a';  break;
				case 'b':  *(dest++)='\b';  break;
				case 'f':  *(dest++)='\f';  break;
				case 'n':  *(dest++)='\n';  break;
				case 'r':  *(dest++)='\r';  break;
				case 't':  *(dest++)='\t';  break;
				case 'v':  *(dest++)='\v';  break;
				case '\\': *(dest++)='\\';  break;
				case '\'': *(dest++)='\'';  break;
				case '\"': *(dest++)='\"';  break;
				default:
					Warning(_MakeCurrLocation(),
						"ignoring unknown string excape sequence \"%.5s\"\n",
						src-1);
					*(dest++)=*(src-1);
					*(dest++)=*src;
					break;
			}
		}
		else
		{  *(dest++)=*src;  }
	}
	*dest='\0';
	
	return(0);
}


ANI::TNLocation AniLexerScanner::_MakeCurrLocation() const
{
	return(ANI::TNLocation(
			pos_arch->GetPos(line0,loc_use_lpos ? lpos0 : (-1)),
			pos_arch->GetPos(line1,loc_use_lpos ? lpos1 : (-1)) ));
}

ANI::TNLocation AniLexerScanner::_MakeCurrLocation(
	int start_line,int start_lpos) const
{
	return(ANI::TNLocation(
			pos_arch->GetPos(start_line,loc_use_lpos ? start_lpos : (-1)),
			pos_arch->GetPos(line1,loc_use_lpos ? lpos1 : (-1)) ));
}


/* -----------<INTERFACE>----------- */

int AniLexerScanner::YYLex(YYSTYPE *lvalp,YYLTYPE *llocp)
{
	if(!p || !p->scanner)
	{  return(-1);  }
	
	for(;;)
	{
		cont2:;
		
		/* WARNING: DO NOT TOUCH THAT CODE UNLESS YOU **REALLY** KNOW */
		/*          WHAT YOU ARE DOING.  SOME CALLS HAVE UNEXPECTED   */
		/*          SIDE EFFECTS (esp. when processing #includes).    */
		/*                                         -- Wolfgang Jul/03 */
		
		assert(p->magic==MyMagic);
		assert(p->top_inp);
		
		if(!p->read_inp)
		{  p->read_inp=p->top_inp;  }
		
		/* Okay, I maintain a token buffer for some additional */
		/* lookahead and look-back if needed. */
		ALSInput *from_inp=NULL;
		
		/* In case p->read_inp!=p->top_inp. */
		/* The lexer is in a different input than we are. */
		/* This is due to some #include. */
		/* Must first return all tokens from the current read_inp. */
		if(p->read_inp!=p->top_inp)
		{
			/* See if we have tokens: */
			if(p->read_inp->next_toks.is_empty())
			{
				/* No toks left; go on: */
				p->read_inp=p->top_inp;
			}
			else
			{  from_inp=p->read_inp;  }
		} /* NO ELSE. */
		if(p->read_inp==p->top_inp)
		{
			/* Make sure token buffer is full... */
			ALSInput *inp=p->top_inp;
			/* I call count() here because for 2 or 3 tokens this costs "no" time. */
			int n_next=inp->next_toks.count();
			int n_prev=inp->prev_toks.count();
			/* Want to have ahead_toks_wanted in the next_toks list. */
			/* Get new ones unless we read EOF. */
			while(n_next<ahead_toks_wanted && !inp->read_eof)
			{
				/* Get TokenEntry to use: */
				TokenEntry *te=NULL;
				if(n_prev>old_toks_wanted)
				{
					te=inp->prev_toks.popfirst();
					--n_prev;
					/* Read comment about zombie_list in ani_scanner.h. */
					if(te->may_be_cleared())
					{  te->clear();  } /* Get rid of old content; free str_val. */
					else
					{
						zombie_list.append(te);
						++zombie_list_nents;
						te=NULL;
					}
				}
				if(!te)
				{  te=new TokenEntry();  }
				
				/* Do the lexical analysis & queue token: */
				int state_change=0;
				_DoYYLex(te);
				if(inp!=p->top_inp)   /* <-- May have changed... */
				{  inp=p->top_inp;  state_change=1;  }
				inp->next_toks.append(te);
				++n_next;
				
				/* If scanner state changed, go back to beginning: */
				if(state_change)
				{  goto cont2;  }
			}
			while(n_prev>old_toks_wanted)
			{
				TokenEntry *tmp=inp->prev_toks.popfirst();
				--n_prev;
				/* Read comment about zombie_list in ani_scanner.h. */
				if(tmp->may_be_cleared())
				{  delete tmp;  }
				else
				{  zombie_list.append(tmp);  ++zombie_list_nents;  }
				/* Should not happen unless we just #included something. */
				if(!inp->read_eof)
				{ fprintf(stderr,"HMMM: Del Old Tok\n"); }
			}
			from_inp=inp;
		}
		
		_TidyUpZombieList();  /* inline check */
		
		/* Get token to be returned: */
		TokenEntry *te=from_inp->next_toks.popfirst();
		assert(te);
		/* If we have ahead_toks_wanted=1 (minimum value) it is important */
		/* that we really do not read more tokens (we may need to switch  */
		/* context thus differently tokens may be recognized.             */
		assert(ahead_toks_wanted>1 || from_inp->next_toks.is_empty());
		/* "Eat" token: */
		from_inp->prev_toks.append(te);
		
		/* Copy the data: */
		*lvalp=te->lval;
		*llocp=te->lloc;
		cloc=te->lloc;  /* another copy */
		
		if(te->token)   /* Not EOF token. */
		{
			/* Special processing: Alter token according to context: */
			/* May need to change '>' into TS_G: */
			/* I cannot completely de-ambiguate but better than nothing. */
			while(te->token=='>' && smart_lookahead)
			{
				TokenEntry *ahead=from_inp->next_toks.first();
				if(!ahead) break;
				int check_token=ahead->token;
				
				/* Address this problem first: if( "a>-1" ) */
				/* Will help againt "a>-1" but not against "a>- -1": */
				TokenEntry *ahead2=ahead ? ahead->next : NULL;
				if(ahead && (ahead->token=='+' || ahead->token=='-' || 
					ahead->token=='!') && ahead2)
				{  check_token=ahead2->token;  }
				
				/* NOTE: Since the new matrix/vector decl, things get more */
				/* complicated: "vector<3> v;" disallows diagnostics by */
				/* following TS_IDENTIFIER; solutions are not easy becuase */
				/* vector<3> or matrix<3,3> can also be used in cast exprs. */
				
				switch(check_token)
				{
					case TS_INTEGER:  case TS_FLOAT:
					case TS_STRING:   /*case TS_IDENTIFIER:*/
					case '(': case '[': case '<':
					/* Problem: TS_PLUSPLUS, TS_MINUSMINUS */
					/*  <1,2,3>++  <--> 3>++i */
					/* The former does not make sense, so... */
					case TS_PLUSPLUS: case TS_MINUSMINUS:
					case TS_COLONCOLON:   /* semi-correct, too. */
						te->token=TS_G;  break;
				}
				
				break;
			}
			
			/*if(te->token<256)
			{  fprintf(stderr,"\tTOK=\'%c\'\t\t[%s]\n",te->token,
				from_inp->fp_path.str());  }
			else if(te->token==TS_IDENTIFIER)
			{  fprintf(stderr,"\tTOK=\"%s\"\t\t[%s]\n",te->lval.str_val,
				from_inp->fp_path.str());  }
			else
			{  fprintf(stderr,"\tTOK=%d\t\t[%s]\n",te->token,
				from_inp->fp_path.str());  }*/
			return(te->token);
		}
		
		/* EOF token here. */
		/* The EOF token is only returned if we cannot YYWrap any more. */
		/* First, copy the data, becuase _YYWrap() will delete */
		/* the current ALSInput and thus the token buffer and *te. */
		/* --> Already done above. */
		
		p->read_inp=NULL;
		if(_YYWrap())
		{
			/* Real EOF reached. */
			break;
		}
	}
	
	return(0);  /* EOF token */
}

/* This is internally called by YYLex() because YYLex() also respects */
/* the token buffer while this function just implements the pure      */
/* lexter... */
void AniLexerScanner::_DoYYLex(TokenEntry *dest)
{
	YY_SCANNER_MAGIC
	
	assert(yyextra==this);
	assert((void*)yyin==p->top_inp);
	
	/* "Shift" up position... */
	line0=line1;
	lpos0=lpos1;
	
	/* Do the actual lex'ing: */
	int rv;
	if(p->top_inp->special_next_tok>=0)
	{
		rv=p->top_inp->special_next_tok;
		p->top_inp->special_next_tok=-1;
	}
	else
	{  rv=yylex(&dest->lval,/*llocp,*/p->scanner);  }
	
	/* Store the token: */
	dest->token=rv;
	if(!rv)  assert(p->top_inp->read_eof || n_errors);
	
	/* Store the position: */
	/*dest->lloc.first_line=  line0;
	dest->lloc.first_column=lpos0;
	dest->lloc.last_line=   line1;
	dest->lloc.last_column= lpos1;*/
	dest->lloc=_MakeCurrLocation();
	
	/*if(rv<256)
	{  fprintf(stderr,"   LEX=\'%c\'\n",rv);  }
	else
	{  fprintf(stderr,"   LEX=%d\n",rv);  }*/
}


void AniLexerScanner::SetMainStartCond(StartCond _main_start_cond)
{
	assert(p->magic==MyMagic);
	
	/* NOTE: INITIAL is illegal for main_start_cond... well we */
	/*       CAN do it but most of the interesting tokens will */
	/*       not be recognized. Must be IN_ANI or IN_POV.      */
	switch(_main_start_cond)
	{
		case SC_INITIAL:  main_start_cond=INITIAL;   break;
		case SC_ANI:      main_start_cond=IN_ANI;    break;
		case SC_POV:      main_start_cond=IN_POV;    break;
		default:  assert(0);
	}
	
	if(p->scanner)
	{
		/* If this function is used while parsing, the lookahead cache */
		/* must have been disabled using the runtime_change=1 when     */
		/* calling the constructor. */
		assert(ahead_toks_wanted<=1);
		
		YY_SCANNER_MAGIC
		BEGIN(main_start_cond);
	}
}


int AniLexerScanner::SetInput(RefString str_or_file,int flags,int first_token,
	const RefString *pos_file,int pos_line,int pos_lpos,
	SourcePositionArchive *alt_pos_arch)
{
	assert(p->magic==MyMagic);
	
	/* Input validation: */
	if(((flags & ALS_PF_File) && (flags & ALS_PF_String)) || 
	   !(flags & (ALS_PF_File | ALS_PF_String)) || 
	   (pos_file || alt_pos_arch) && (flags & ALS_PF_Push) )
	{  return(-3);  }
	
	if(p->scanner && !(flags & ALS_PF_Push))
	{  return(1);  }
	
	if((flags & ALS_PF_Push) && p->file_depth>=max_file_depth)
	{  return(-4);  }
	
	/* Set up ALSInput: (set "down" pointer further down) */
	ALSInput *inp = NEW1<ALSInput>(/*down=*/(ALSInput*)NULL);   /* _CPP_OPERATORS_FF... */
	int set_rv;
	if(flags & ALS_PF_File)  set_rv=inp->SetFile(str_or_file);
	else                     set_rv=inp->SetBuffer(str_or_file);
	if(set_rv)
	{  DELETE(inp);  return(-2);  }
	
	/* Set special next token if specified: */
	if(first_token>=0)
	{  inp->special_next_tok=first_token;  }
	
	/* Do scanner setup (if needed): */
	if(!p->scanner)
	{
		yylex_init(&p->scanner);
		yyset_extra(this,p->scanner);
		YY_SCANNER_MAGIC
		BEGIN(main_start_cond);
	}
	
	/* See if we really need to push: */
	YY_SCANNER_MAGIC
	if(YY_CURRENT_BUFFER==NULL)
	{  flags&=~ALS_PF_Push;  }
	
	/* Set the input: */
	size_t needlen=YY_BUF_SIZE;
	if(!!inp->strbuf)  /* Note the two "!". */
	{
		/* Reading from string. */
		needlen=inp->strbuf.len()+16;  /* Add 16 to be sure (internal flex stuff). */
		if(needlen>YY_BUF_SIZE)
		{  needlen=YY_BUF_SIZE;  }
	}
	/* Notice that I pass the ALSInput instead of the FILE. */
	YY_BUFFER_STATE buf=yy_create_buffer((FILE*)inp,needlen,p->scanner);
	CheckMalloc(buf);
	
	/* Save the current position (in old top_inp): */
	if(p->top_inp)
	{
		p->top_inp->saved_line=line1;
		p->top_inp->saved_lpos=lpos1;
	}
	
	/* Read on in passed ALSInput: */
	line0=pos_line;  line1=pos_line;
	lpos0=pos_lpos;  lpos1=pos_lpos;
	if(flags & ALS_PF_Push)
	{
		assert(pos_arch);
		assert(!alt_pos_arch);  /* Checks above should return error in that case. */
		yypush_buffer_state(buf,p->scanner);
		++p->file_depth;
	}
	else
	{
		assert(!pos_arch);
		
		RefString file_name;
		if(pos_file)
		{  file_name=*pos_file;  }
		else if((flags & ALS_PF_File))
		{  file_name=str_or_file;  }
		else
		{  file_name.set("[buffer]");  }
		if(alt_pos_arch)
		{
			pos_arch=alt_pos_arch;
			pos_arch_allocated=0;
			/* Check that the passed source pos archive contains */
			/* the correct file.                                 */
			if(pos_arch->CurrentFile().GetPath()!=file_name)
			{  assert(0);  }
		}
		else
		{
			pos_arch=NEW1<SourcePositionArchive>(file_name);
			pos_arch_allocated=1;
		}
		
		yy_switch_to_buffer(buf,p->scanner);
		//yypush_buffer_state(buf,p->scanner);
		assert(p->file_depth==0);
		p->file_depth=1;
	}
	
	/* Make sure we know which input is on top: */
	inp->down=p->top_inp;
	p->top_inp=inp;

	/*fprintf(stderr,"SetInput(%s,%s)\n",p->top_inp->fp_path.str(),
		(flags & ALS_PF_Push) ? "push" : "switch");*/
	
	return(0);
}


int AniLexerScanner::_YYWrap()
{
	assert(p->magic==MyMagic);
	assert(p->scanner);  /* Otherwise we may not be here. */
	
	YY_SCANNER_MAGIC
	
	/* See if we are currently reading a buffer */
	/* (i.e. there is one on the stack):        */
	if(YY_CURRENT_BUFFER==NULL)
	{  assert(!p->top_inp);  return(1);  }
	
	/*fprintf(stderr,"YYWRAP %s --> ",
		yyin ? ((ALSInput*)yyin)->fp_path.str() : "NULL");*/
	
	/* Close input file if needed: */
	int must_pop_state=0;
	if(yyin)
	{
		ALSInput *inp=(ALSInput*)yyin;
		assert(p->top_inp==inp);
		assert(p->read_inp!=inp);
		
		must_pop_state=inp->must_pop_state;
		if(inp->must_pop_pos_arch)
		{
			assert(pos_arch);
			pos_arch->EndFile();
		}
		
		/* Must save the previous tokens which may not (yet) be deleted: */
		for(TokenEntry *_i=inp->prev_toks.first(); _i; )
		{
			TokenEntry *i=_i;
			_i=_i->next;
			if(!i->may_be_cleared())
			{
				zombie_list.append(inp->prev_toks.dequeue(i));
				++zombie_list_nents;
			}
		}
		_TidyUpZombieList();  /* inline check */
		
		delete inp;
		yyin=NULL;
		p->top_inp=NULL;
	}
	else assert(p->top_inp==NULL);
	
	/* Delete the current buffer. yypop_buffer_state() */
	/* internally calls yy_delete_buffer(). */
	yypop_buffer_state(p->scanner);
	--p->file_depth;
	/*yy_delete_buffer(buf,p->scanner);*/
	
	int retval;
	if(YY_CURRENT_BUFFER==NULL)
	{
		/* Cannot read on; we're at EOF. */
		assert(yyin==NULL);
		assert(!p->file_depth);
		retval=1;
		/*fprintf(stderr,"EOF\n");*/
	}
	else
	{
		/* We'll read on... */
		assert(yyin!=NULL);
		assert(p->file_depth>0);
		p->top_inp=(ALSInput*)yyin;
		
		/* Restore position: */
		line1=p->top_inp->saved_line;
		lpos1=p->top_inp->saved_lpos;
		
		/* Pop state if needed: */
		if(must_pop_state)
		{  yy_pop_state(p->scanner);  }
		
		retval=0;
		/*fprintf(stderr,"%s  [pos=%d:%d]\n",p->top_inp->fp_path.str(),line1,lpos1);*/
	}
	
	return(retval);
}


/* NOTE: Always use _TidyUpZombieList) and not the "__" version. */
void AniLexerScanner::__TidyUpZombieList(bool force)
{
	int _cnt=0,_nforce=0;
	for(TokenEntry *_i=zombie_list.first(); _i; )
	{
		TokenEntry *i=_i;
		_i=_i->next;
		
		bool we_may=i->may_be_cleared();
		if(force && !we_may)
		{
			i->clear(/*force=*/1);
			++_nforce;
			we_may=1;
		} /* NO else. */
		if(we_may)
		{
			delete zombie_list.dequeue(i);
			--zombie_list_nents;
			++_cnt;
		}
	}
	
	/* Getting this message is perfectly normal. */
	if(_cnt || zombie_list_nents)
	{  fprintf(stderr,"Hmmm: Deleted %d (%d forced) zombie tokens, %d left "
		"(force=%d).\n",
		_cnt,_nforce,zombie_list_nents,int(force));  }
	if(force)
	{  assert(zombie_list_nents==0);  }
	
	assert(zombie_list_nents==zombie_list.count());
	last_zombie_list_nents=zombie_list_nents;
}


void AniLexerScanner::Reset()
{
	assert(p->magic==MyMagic);
	
	if(p->scanner)
	{
		/* Tidy up the stack: */
		p->read_inp=NULL;
		while(!_YYWrap());
		
		/* With the code above, this is pretty useless. */
		/* I leave it here as a reminder. */
		YY_SCANNER_MAGIC
		if(YY_CURRENT_BUFFER!=NULL)
		{
			fprintf(stderr,"OOps, still current buffer set.\n");
			/* Need to abort. Otherwise, yylex_destroy() will free */
			/* current buffer again yielding to crash.             */
			abort();
		}
		
		assert(!p->file_depth);
		assert(!p->top_inp);
		
		yylex_destroy(p->scanner);
		p->scanner=NULL;
	}
	
	line0=1;  line1=1;
	lpos0=0;  lpos1=0;
	
	if(pos_arch)
	{
		if(pos_arch_allocated)
		{  delete pos_arch;  }
		pos_arch=NULL;
		pos_arch_allocated=0;
	}
	
	_TidyUpZombieList(/*force=*/1);
	
	n_errors=0;
	
	p->top_inp=NULL;
	p->read_inp=NULL;
	p->file_depth=0;
	p->nested_comment_warned=0;
	p->newlines_in_ccomment=0;
	
	/*cloc.first_line=  -1;
	cloc.first_column=-1;
	cloc.last_line=   -1;
	cloc.last_column= -1;*/
	cloc.pos0=SourcePosition::NullPos;
	cloc.pos1=SourcePosition::NullPos;
}


AniLexerScanner::AniLexerScanner(StartCond _main_start_cond,
	bool runtime_change) : 
	LexerScannerBase(),
	zombie_list(),
	ahead_toks_wanted(runtime_change ? 1 : 3),
	old_toks_wanted(16),
	cloc(SourcePosition::NullPos,SourcePosition::NullPos)
{
	p=new ani_scanner_private;
	p->magic=MyMagic;
	
	zombie_list_nents=0;
	last_zombie_list_nents=0;
	
	SetMainStartCond(_main_start_cond);
	
	/* Defaults for values which are not changed by ani_scanner_Reset(): */
	tab_width=8;
	allow_nested_comments=2;
	max_file_depth=32;
	loc_use_lpos=1;
	smart_lookahead=0;
	
	pos_arch=NULL;
	pos_arch_allocated=0;
	
	/* All the other values get their initial values by the reset function: */
	Reset();
}

AniLexerScanner::~AniLexerScanner()
{
	assert(p->magic==MyMagic);
	
	if(p)
	{
		Reset();
		delete p;
	}
}


/******************************************************************************/

ssize_t AniLexerScanner::ALSInput::read(char *buf,size_t len)
{
	if(!!strbuf)
	{
		if(strbuf.stype())
		{
			/* NOT '\0'-terminated. */
			const char *src=strbuf.str()+strbuf_pos;
			size_t copylen=strbuf.len()-strbuf_pos;
			if(copylen>len)  copylen=len;
			memcpy(buf,src,copylen);
			strbuf_pos+=copylen;
			return(copylen);
		}
		else
		{
			/* '\0'-terminated string. */
			const char *src=strbuf.str()+strbuf_pos;
			char *dest=buf,*destend=dest+len;
			while(*src)
			{
				if(dest>=destend)  break;
				*(dest++)=*(src++);
			}
			size_t readbytes=dest-buf;
			strbuf_pos+=readbytes;
			return(readbytes);
		}
	}
	else if(fp)
	{
		errno=0;
		size_t result;
		while ( (result = fread(buf,1,len,fp))==0 && ferror(fp))
		{
			if(errno!=EINTR)
			{
				fprintf(stderr,"reading %s failed: %s\n",
					fp_path.str(),strerror(errno));
				return(-1);
				break;
			}
			errno=0;
			clearerr(fp);
		}
		return(result);
	}
	
	return(0);
}


int AniLexerScanner::ALSInput::SetFile(RefString path)
{
	if(fp || !!strbuf)
	{  return(1);  }
	fp=fopen(path.str(),"r");
	if(!fp)
	{  return(-2);  }
	fp_path=path;
	return(0);
}

int AniLexerScanner::ALSInput::SetBuffer(RefString buf)
{
	if(fp || !!strbuf)
	{  return(1);  }
	strbuf=buf;
	strbuf_pos=0;
	return(0);
}


AniLexerScanner::ALSInput::ALSInput(AniLexerScanner::ALSInput *_down,
	int *failflag) : 
	strbuf(failflag),
	fp_path(failflag),
	next_toks(failflag),
	prev_toks(failflag)
{
	down=_down;
	
	fp=NULL;
	strbuf_pos=0;
	
	saved_line=-1;
	saved_lpos=-1;
	must_pop_state=0;
	must_pop_pos_arch=0;
	
	special_next_tok=-1;
	
	read_eof=0;
}

AniLexerScanner::ALSInput::~ALSInput()
{
	if(fp)
	{  fclose(fp);  fp=NULL;  }
	
	while(!next_toks.is_empty())
	{
		fprintf(stderr,"OOPS: ~ALSInput: Token %d left in next_toks.\n",
			next_toks.first()->token);
		next_toks.first()->clear(/*force=*/1);
		delete next_toks.popfirst();
	}
	if(prev_toks.count()>16+1)  /* We want 16 in prev_toks, allow 17 here. */
	{  fprintf(stderr,"OOPS: ~ALSInput: %d tokens left in prev_toks.\n",
		prev_toks.count());  }
	/* Tokens with may_be_cleared()==0 from this list must have been */
	/* requeued in zombie_list before. */
	while(!prev_toks.is_empty())
	{  delete prev_toks.popfirst();  }
}


/******************************************************************************/

ani_scanner_private::ani_scanner_private()
{
	scanner=NULL;
	top_inp=NULL;
	read_inp=NULL;
	file_depth=0;
	nested_comment_warned=0;
	newlines_in_ccomment=0;
}

ani_scanner_private::~ani_scanner_private()
{
	assert(!scanner);
}


/******************************************************************************/

bool AniLexerScanner::TokenEntry::may_be_cleared() const
{
	if(token!=TS_IDENTIFIER && token!=TS_STRING)  return(1);
	if(!lval.str_val || *lval.str_val=='\0')  return(1);
	return(0);
}


AniLexerScanner::TokenEntry::TokenEntry() : 
	LinkedListBase<TokenEntry>(),
	lloc(SourcePosition::NullPos,SourcePosition::NullPos)
{
	token=-1;
	/*lloc.first_line=  -1;
	lloc.first_column=-1;
	lloc.last_line=   -1;
	lloc.last_column= -1;*/
	lval.str_val=NULL;   /* ugly union... */
}


void AniLexerScanner::TokenEntry::clear(bool force)
{
	/* We need to deallocate the str_val in the YYSTYPE    */
	/* if necessary; this is only the case for two tokens. */
	if(force==0)
	{  assert(may_be_cleared());  }
	if(token==TS_IDENTIFIER || token==TS_STRING)
	{  LFree(lval.str_val);  }
	token=-1;
}

%{
/* 
 * pov-scan/pov_scanner.ll
 * 
 * Lexical analyzer for POV special comment parser. 
 * Uses reentrant C scanner and thus needs flex-2.5.11 or higher. 
 * 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2002--2003 by Wolfgang Wieser (wwieser@gmx.de)
 * 
 * Flex code based on AniVision's language parser ani-parser/scanner.ll. 
 *
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <hlib/prototypes.h>
#include <hlib/linkedlist.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "pov_scanner.h"

#include <assert.h>

/* Non-critical assert: */
#define nc_assert(x)  assert(x)


#define MyMagic 0xbeefbabeU

/* Internally used for include hierarchy:          */
/* The linked list is essentially used as a stack. */
struct file_hierarch_entry : LinkedListBase<file_hierarch_entry>
{
	RefString path;
	FILE *fp;
	YY_BUFFER_STATE buf;
	int saved_line,saved_lpos;
	
	file_hierarch_entry() : path()
		{  fp=NULL;  buf=NULL;  }
	~file_hierarch_entry()
		{  assert(!fp && !buf);  }
	_CPP_OPERATORS
};


/* Private part of pov_scanner_context: */
struct pov_scanner_private
{
	/* For simple detection of memory corruption or evil */
	/* pointer mesh... */
	unsigned long magic;
	
	/* The "stack" of the include hierarchy: parsed tile is last(). */
	LinkedList<file_hierarch_entry> fhstack;
	int file_depth;  /* Current include hierarchy depth. */
	yyscan_t scanner;
	
	/* Warned that this comment is nested? */
	int nested_comment_warned;
	
	
	/* Does not set magic value: */
	pov_scanner_private();
	~pov_scanner_private();
	_CPP_OPERATORS
};


/* Return values for yylex(): */
#define LEX_EOF         0
#define LEX_ABORT     999
#define LEX_COMMAND  1000


#define YY_EXTRA_TYPE  class POVLexerScanner*
#define YY_NO_UNPUT

	/* Note: yylloc of type YYLTYPE */
#define INC_COL() \
	do { yyextra->lpos+=yyleng; } while(0)
	/* Call for one TAB (\t): */
#define INC_COL_TAB() \
	do { yyextra->lpos=(yyextra->lpos/yyextra->tab_width+1)*yyextra->tab_width; } while(0);
#define INC_LINE() \
	do { ++yyextra->line;  yyextra->lpos=0; } while(0)
	/* Update position after having read the passed character. */
#define INC_POS_CHAR(c) \
	do { \
		switch(c) \
		{ \
			case '\n':  INC_LINE();  break; \
			case '\r':  break; \
			case '\t':  INC_COL_TAB();  break; \
			default:  ++yyextra->lpos;  break; \
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
	do { yyextra->line0=yyextra->line; \
	     yyextra->lpos0=yyextra->lpos; } while(0);

%}

/* Generate scanner file by using:                */
/* flex -s scanner.ll    [works with flex-2.5.31] */

%pointer
%option prefix="POVS_"
%option outfile="pov_scanner.cc"
%option noyywrap
/*%option yylineno*/
%option reentrant
%option stack

%x IN_CCMT
%x IN_CPPCMT
%x IN_CPPCMT2
%x IN_COMM

identifier  [[:alpha:]_][[:alnum:]_]*
integer     [[:digit:]]+
/* Integer with beginning range spec: */
integer_r   [[:digit:]]+\.\.
float       (({integer}\.)|([[:digit:]]*\.{integer}))([eE][+-]?{integer})?
string      \"([^\"\n]|\\\")*\"
ut_string   \"([^\"\n]|\\\")*
incl_stmt   "#include"[ \t]+{string}
command     (("/*"[ \n\r\t]*)|("//"[ \t]*))"@"{identifier}

%%

	/*----------------- Special command comment: -----------------*/

<IN_COMM>{ut_string}  {
		POS_RE_SET();
		INC_COL();
		Error(yyextra->_MakeCurrLocation(),
			"unterminated string in command comment\n");
		++yyextra->n_errors;
		
		return(LEX_ABORT);
	}
<IN_COMM>{string}  {
		INC_COL();
		yyextra->cc.AppendText(yytext,yyleng,0);
		POS_RE_SET();
	}
<IN_COMM>([ \t\r]*)\n(\r*)([ \t]*)"//"([ \t]*)  {
		INC_POS_TEXT(yytext,yyleng);
		/* C++ style command commant, maybe continued. */
		/* Nothing special for C style. */
		if(yyextra->cc.style==1)  /* C style */
		{  yyextra->cc.AppendText(yytext,yyleng,0);  }
		else  /* C++ style */
		{
			/* See if we can continue. */
			char next_char=yyinput(yyscanner);
			INC_POS_CHAR(next_char);
			if(next_char=='@')
			{  yyextra->cc.AppendText("\n@",2,0);  }
			else if(!yyextra->cc.BalancedBrackets())
			{
				char tmp[2]={'\n',next_char};
				yyextra->cc.AppendText(tmp,2,1);
			}
			else
			{
				/* No, this is a "normal" C++ comment. */
				int rv=yyextra->_EndCCommand();
				yy_push_state(IN_CPPCMT,yyscanner);
				return(rv);
			}
		}
		POS_RE_SET();
	}
<IN_COMM>([ \t\r]*)\n  {   /* We catch C++-style comment continuation above. */
		INC_LINE();
		if(yyextra->cc.style==2)  /* C++ style command comment */
		{  return(yyextra->_EndCCommand());  }
		else
		{  yyextra->cc.AppendText("\n",1,0);  }
		POS_RE_SET();
	}
<IN_COMM>([ \t\r]*)"/*"  {
		INC_POS_TEXT(yytext,yyleng);
		if(yyextra->cc.style==1)  /* C style command comment */
		{
			#warning "Could warn about comment in command comment."
			yy_push_state(IN_CCMT,yyscanner);
		}
		else
		{  yyextra->cc.AppendText(yytext,yyleng,0);  }
		POS_RE_SET();
	}
<IN_COMM>([ \t\r]*)"//"  {
		INC_POS_TEXT(yytext,yyleng);
		#warning "Could warn about comment in command comment."
		yy_push_state(IN_CPPCMT2,yyscanner);
		POS_RE_SET();
	}
<IN_COMM>([ \t\r]*)"*/"  {
		INC_POS_TEXT(yytext,yyleng);
		if(yyextra->cc.style==1)  /* C style command comment */
		{  return(yyextra->_EndCCommand());  }
		else
		{  yyextra->cc.AppendText(yytext,yyleng,0);  }
		POS_RE_SET();
	}
<IN_COMM>([ \t\r]*)"@-"  {
		/* This means there will not be any command in this comment any more. */
		INC_POS_TEXT(yytext,yyleng);
		int rv=yyextra->_EndCCommand();
		/* Change into the context of an ordinary comment: */
		if(yyextra->cc.style==1)  /* C style command comment */
		{  yy_push_state(IN_CCMT,yyscanner);  }
		else
		{  yy_push_state(IN_CPPCMT,yyscanner);  }
		return(rv);
	}
<IN_COMM>\r  {  ; /* DOS/MAC crap */  }
<IN_COMM>\t  {
		INC_COL_TAB();
		yyextra->cc.AppendText(yytext,yyleng,0);
		POS_RE_SET();
	}
<IN_COMM>" "+  {
		INC_COL();
		yyextra->cc.AppendText(yytext,yyleng,0);
		POS_RE_SET();
	}
<IN_COMM><<EOF>>  {
		if(yyextra->cc.style==1)  /* C style command comment */
		{
			INC_COL();
			/* FIXME: we do not yet report where the unterminated comment starts??? */
			Error(yyextra->_MakeCurrLocation(/*start=*/yyextra->line,yyextra->lpos),
				"unterminated C-style comment\n");
			++yyextra->n_errors;
			
			yy_pop_state(yyscanner);
			if(yyextra->_YYWrap())
			{  return(LEX_EOF);  }
		}
		else
		{  return(yyextra->_EndCCommand());  }
		POS_RE_SET();
	}
<IN_COMM>{identifier}|{integer}|{float}  {
		INC_COL();
		yyextra->cc.AppendText(yytext,yyleng,1);
		POS_RE_SET();
	}
<IN_COMM>.  {
		/*fprintf(stderr,">%c<",*yytext);*/
		INC_COL();
		yyextra->cc.AppendText(yytext,yyleng,1);
		POS_RE_SET();
	}

	/*----------------- Normal C-style comments: -----------------*/
<IN_CCMT>"*/"  {
		INC_COL();
		yy_pop_state(yyscanner);
		POS_RE_SET();
	}
<IN_CCMT>"/*"  {
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
			yy_push_state(IN_CCMT,yyscanner);
		}
		POS_RE_SET();
	}
<IN_CCMT>[^\n/\*]*\n    {  INC_LINE();  POS_RE_SET();  }
<IN_CCMT>[^\n\r\t/\*]*  {  INC_COL();   POS_RE_SET();  }
<IN_CCMT>\n             {  INC_LINE();  POS_RE_SET();  }
<IN_CCMT>\r             {  ; /* DOS/MAC crap */  }
<IN_CCMT>\t             {  INC_COL_TAB();  POS_RE_SET();  }
<IN_CCMT><<EOF>>  {
		INC_COL();
		/* FIXME: we do not yet report where the unterminated comment starts!!! */
		Error(yyextra->_MakeCurrLocation(/*start=*/yyextra->line,yyextra->lpos),
			"unterminated C-style comment\n");
		++yyextra->n_errors;
		while(YY_START==IN_CCMT || YY_START==IN_COMM)
		{  yy_pop_state(yyscanner);  }
		if(yyextra->_YYWrap())
		{  return(LEX_EOF);  }
		POS_RE_SET();
	}
<IN_CCMT>.           {  INC_COL();  POS_RE_SET();  /* single '*' would fail otherwise */  }

	/*----------------- Normal C++ - style comments: -----------------*/
<IN_CPPCMT>.*\n   {  INC_LINE();  yy_pop_state(yyscanner);  POS_RE_SET();  }
<IN_CPPCMT><EOF>  {
		INC_COL();
		while(YY_START==IN_CPPCMT || YY_START==IN_COMM)
		{  yy_pop_state(yyscanner);  }
		if(yyextra->_YYWrap())
		{  return(LEX_EOF);  }
		POS_RE_SET();
	}
<IN_CPPCMT>.      {  INC_COL();  POS_RE_SET();  }

	/*----------------- Special version for commands: -----------------*/
<IN_CPPCMT2>[^\n]*  {
		INC_POS_TEXT(yytext,yyleng);
		yy_pop_state(yyscanner);
		POS_RE_SET();
	}

	/* POV code parsing: */

	/* Match all whitespace we can get... */
[ \t\n\r]+	{   /* \r -> DOS/MAC crap */
		INC_POS_TEXT(yytext,yyleng);
		POS_RE_SET();
	}

	/* These things are interpreted in POV code: */
{command}  {
		/* Parse in beginning of special command - comment: */
		POVLexerScanner::CCommand *cc=&yyextra->cc;
		assert(cc->style==0);
		for(char *c=yytext,*cend=yytext+yyleng; c<cend; c++)
		{
			/* Search the beginning of the command: */
			if(*c=='@')
			{
				assert(cc->style==0);
				
				/* Command style: 1 -> C; 2 -> C++ */
				switch(yytext[1])
				{
					case '*':  cc->style=1;  break;
					case '/':  cc->style=2;  break;
					default:  nc_assert(0);
				}
				
				/* Position: yyextra->line,lpos */
				cc->file=yyextra->p->fhstack.last()->path;
				cc->start_line=yyextra->line;
				cc->start_lpos=yyextra->lpos;
				
				/* Copy what we have:*/
				cc->SetText(c,cend-c,1);
			}
			INC_POS_CHAR(*c);
		}
		nc_assert(cc->style);
		
		/* Switch into command mode: */
		yy_push_state(IN_COMM,yyscanner);
		POS_RE_SET();
	}
"/*"  {
		INC_COL();
		yyextra->p->nested_comment_warned=0;
		yy_push_state(IN_CCMT,yyscanner);
		POS_RE_SET();
	}
"//"  {  INC_COL();  yy_push_state(IN_CPPCMT,yyscanner);  POS_RE_SET();  }
{incl_stmt}  {
		/* Parse include statement. */
		if(yyextra->ignore_includes)
		{  INC_POS_TEXT(yytext,yyleng);  }
		else
		{
			/* Set correct position for beginning of string: */
			int start_line=yyextra->line,start_lpos=yyextra->lpos;
			char *fname=NULL,*fnameend=NULL;
			int fname_line=-1,fname_lpos=-1;
			int fail=0;
			for(char *c=yytext,*cend=yytext+yyleng; c<cend; c++)
			{
				if(*c=='\"' && !fail)
				{
					++yyextra->lpos;
					if(!fname)
					{
						fname=c+1;
						fname_line=yyextra->line;
						fname_lpos=yyextra->lpos-1;
					}
					else if(!fnameend)
					{  fnameend=c;  }
					else goto write_error;
				}
				else if(*c=='\t')
				{  INC_COL_TAB();  }
				else
				{  ++yyextra->lpos;  }  /* Do NOT use INC_COL() here. */
			}
			if(!fail && !fnameend)
			{
				write_error:;
				Error(yyextra->_MakeCurrLocation(),
					"invalid include statement: parse error at %d:%d\n",
					yyextra->line,yyextra->lpos);
				++yyextra->n_errors;
				fail=1;
			}
			if(!fail)
			{
				RefString tmp;
				tmp.set0(fname,fnameend-fname);
				int rv=yyextra->_OpenFile(tmp);
				if(rv)
				{
					Warning(yyextra->_MakeCurrLocation(
						/*start=fname_line,fname_lpos*/),
						"failed to #include \"%s\": %s\n",
						tmp.str(),
						rv==-3 ? "includes nested too deeply" : strerror(errno));
				}
				else  /* Virtual downcall: */
				{  yyextra->IncludingFile(tmp,start_line,start_lpos);  }
			}
		}
		POS_RE_SET();
	}

	/* Make sure we do not interprete things in strings: */
{ut_string}  {
		INC_COL();
		Error(yyextra->_MakeCurrLocation(),
			"unterminated string\n");
		++yyextra->n_errors;
		
		return(LEX_ABORT);
	}
{string}      {  INC_COL();  POS_RE_SET();  }

{identifier}  {  INC_COL();  POS_RE_SET();  }
{integer}     {  INC_COL();  POS_RE_SET();  }
{float}       {  INC_COL();  POS_RE_SET();  }
"{"  {  INC_COL();  POS_RE_SET();  }
"}"  {  INC_COL();  POS_RE_SET();  }

<<EOF>>	 {
		if(yyextra->_YYWrap())
		{  return(LEX_EOF);  }
	}

	/* All the rest of POV code. This could probably be optimized...?!!!! */
.    {  INC_COL();  POS_RE_SET();  }

%%

/* -----------<INTERNAL SUPPORT ROUTINES>----------- */

int POVLexerScanner::_EndCCommand()
{
	nc_assert(cc.style!=0);
	
	/* This position may not be 100% accurate but that does not matter... */
	cc.end_line=line;
	cc.end_lpos=lpos;
	
	yy_pop_state(p->scanner);
	return(LEX_COMMAND);
}


int POVLexerScanner::_OpenFile(const RefString &file)
{
	if(p->file_depth>=max_file_depth)
	{  return(-3);  }
	
	FILE *fp=fopen(file.str(),"r");
	if(!fp)
	{  return(-2);  }
	
	YY_BUFFER_STATE buf=yy_create_buffer(fp,YY_BUF_SIZE,p->scanner);
	CheckMalloc(buf);
	
	/* Save current position: */
	file_hierarch_entry *curr=p->fhstack.last();
	if(curr)
	{
		curr->saved_line=line;
		curr->saved_lpos=lpos;
	}
	
	/* Push on top of the stack: */
	file_hierarch_entry *fhe=new file_hierarch_entry;
	fhe->path=file;
	fhe->fp=fp;
	fhe->buf=buf;
	p->fhstack.append(fhe);
	++p->file_depth;
	
	/* Read on in passed file. */
	line=1;  line0=1;
	lpos=0;  lpos0=0;
	yy_switch_to_buffer(p->fhstack.last()->buf,p->scanner);
	
	return(0);
}


int POVLexerScanner::_YYWrap()
{
	file_hierarch_entry *fhe=p->fhstack.poplast();
	if(!fhe)
	{  return(1);  }
	--p->file_depth;
	
	if(fhe->fp)
	{
		fclose(fhe->fp);
		fhe->fp=NULL;
	}
	yy_delete_buffer(fhe->buf,p->scanner);
	fhe->buf=NULL;
	delete fhe;
	
	/* Get the file to read on with: */
	fhe=p->fhstack.last();
	if(!fhe)
	{  return(1);  }
	
	yy_switch_to_buffer(fhe->buf,p->scanner);
	line=fhe->saved_line;
	lpos=fhe->saved_lpos;
	/* Virtual downcall: */
	RefString nullref;
	IncludingFile(nullref,-1,-1);
	
	return(0);
}


ANI::TNLocation POVLexerScanner::_MakeCurrLocation() const
{
	return(ANI::TNLocation(
			pos_arch->GetPos(line0,error_loc_use_lpos ? lpos0 : (-1)),
			pos_arch->GetPos(line,error_loc_use_lpos ? lpos : (-1)) ));
}

ANI::TNLocation POVLexerScanner::_MakeCurrLocation(
	int start_line,int start_lpos) const
{
	return(ANI::TNLocation(
			pos_arch->GetPos(start_line,error_loc_use_lpos ? start_lpos : (-1)),
			pos_arch->GetPos(line,error_loc_use_lpos ? lpos : (-1)) ));
}


/* -----------<INTERFACE>----------- */

int POVLexerScanner::Parse()
{
	if(!p || !p->scanner)
	{  return(-1);  }
	nc_assert(p->magic==MyMagic);
	
	{
		/* This is a bit hacky... */
		yyscan_t yyscanner=p->scanner;
		/* Special hack because the macros yyextra, etc. need yyg... (flex-2.5.31) */ \
		struct yyguts_t *yyg = (struct yyguts_t*)yyscanner;
		nc_assert(yyextra==this);
	}
	
	/* Make sure there is no command left: */
	cc.Clear();
	
	/* Make sure we're not at EOF... -> check fhstack */
	while(!p->fhstack.is_empty())
	{
		/* "Shift" up position... */
		line0=line;
		lpos0=lpos;
		
		int rv=yylex(p->scanner);
		if(rv==LEX_ABORT)
		{
			cc.Clear();
			return(-2);
		}
		if(rv==LEX_EOF) break;
		if(rv==LEX_COMMAND) return(1);
		/* If this assert fails, yylex() returned illegal value. */
		nc_assert(0);
	}
	
	cc.Clear();
	return(0);
}


int POVLexerScanner::SetInput(const RefString &file,
	SourcePositionArchive *pos_arch_for_errors)
{
	//yyscan_t yyscanner=NULL;
	
	nc_assert(p->magic==MyMagic);
	if(p->scanner)  return(1);
	
	pos_arch=pos_arch_for_errors;
	assert(pos_arch);
	
	yylex_init(&p->scanner);
	yyset_extra(this,p->scanner);
	if(_OpenFile(file))
	{
		Reset();
		return(-2);
	}
	
	return(0);
}


void POVLexerScanner::SkipFile()
{
	nc_assert(p->magic==MyMagic);
	if(!p->scanner)  return;
	
	if(p->file_depth>0)
	{  _YYWrap();  }
}


void POVLexerScanner::Reset()
{
	nc_assert(p->magic==MyMagic);
	
	/* Tidy up the stack: */
	while(!_YYWrap());
	nc_assert(p->file_depth==0);
	
	if(p->scanner)
	{
		yylex_destroy(p->scanner);
		p->scanner=NULL;
	}
	
	n_errors=0;
	
	p->file_depth=0;
	p->nested_comment_warned=0;
	
	line=1;  line0=1;
	lpos=0;  lpos0=0;
	
	pos_arch=NULL;  /* Not allocated by us. */
	
	cc.Clear();
}


void POVLexerScanner::IncludingFile(const RefString & /*file*/,
	int /*line*/,int /*lpos*/)
{
	/* Do nothing; this is just a virtual function for derived classes. */
}


POVLexerScanner::POVLexerScanner()
{
	p=new pov_scanner_private;
	p->magic=MyMagic;
	
	/* Defaults for values which are not changed by pov_scanner_Reset(): */
	allow_nested_comments=1;
	tab_width=8;
	max_file_depth=32;
	ignore_includes=0;
	error_loc_use_lpos=1;
	
	/* All the other values get their initial values by the reset function: */
	Reset();
}

POVLexerScanner::~POVLexerScanner()
{
	nc_assert(p->magic==MyMagic);
	
	if(p)
	{
		Reset();
		delete p;
	}
}


/******************************************************************************/

void POVLexerScanner::CCommand::SetText(const char *src,size_t len,
	bool check_brackets)
{
	text_len=0;
	if(text_asize>8192)
	{
		text=(char*)LFree(text);
		text_asize=0;
	}
	AppendText(src,len,check_brackets);
}

void POVLexerScanner::CCommand::AppendText(const char *src,size_t len,
	bool check_brackets)
{
	assert(style!=0);
	size_t need_len=text_len+len;
	if(text_asize<need_len)
	{
		/* Must re-allocate. */
		text_asize=((need_len/512)+2)*512;
		text=(char*)LRealloc(text,text_asize);
	}
	if(len==1)
	{  text[text_len++]=*src;  }
	else
	{
		strncpy(text+text_len,src,len);
		text_len+=len;
	}
	if(check_brackets)
	{
		for(const char *c=src,*cend=src+len; c<cend; c++)
		{
			switch(*c)
			{
				case '(':  ++bracket_depth;  break;
				case ')':  --bracket_depth;  break;
			}
		}
		if(bracket_depth<0)
		{  bracket_depth=0;  }
	}
}

void POVLexerScanner::CCommand::Clear()
{
	text_len=0;
	style=0;
	bracket_depth=0;
	file.set(NULL);
	if(text_asize>8192)
	{
		text=(char*)LFree(text);
		text_asize=0;
	}
}


POVLexerScanner::CCommand::CCommand() : file()
{
	start_line=-1;
	start_lpos=-1;
	text=NULL;
	text_len=0;
	text_asize=0;
	style=0;
	bracket_depth=0;
}

POVLexerScanner::CCommand::~CCommand()
{
	text=(char*)LFree(text);
	text_len=0;
	text_asize=0;
}


/******************************************************************************/

pov_scanner_private::pov_scanner_private() : 
	fhstack()
{
	scanner=NULL;
	nested_comment_warned=0;
	file_depth=0;
	
}

pov_scanner_private::~pov_scanner_private()
{
	assert(!scanner);
}

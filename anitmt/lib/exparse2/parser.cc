
/*  A Bison parser, made from /home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	TOK_INVALID_ID	257
#define	TOK_IS_EQUAL	258
#define	TOK_NOT_EQUAL	259
#define	TOK_MORE_EQUAL	260
#define	TOK_LESS_EQUAL	261
#define	TOK_FUNC_SIN	262
#define	TOK_OP_FLAG	263
#define	TOK_OP_SCALAR	264
#define	TOK_OP_VECTOR	265
#define	TOK_OP_MATRIX	266
#define	TOK_OP_STRING	267
#define	TOK_FLAG	268
#define	TOK_SCALAR	269
#define	TOK_VECTOR	270
#define	TOK_MATRIX	271
#define	TOK_STRING	272
#define	UMINUS	273
#define	OP_CONVERTION	274

#line 3 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"

#include <iostream>

#include <val/val.hpp>
#include <solve/operand.hpp>
#include <solve/operator.hpp>
#include <message/message.hpp>

#include "exparser.hpp"

#define YYPARSE_PARAM info
#define YYLEX_PARAM info
#define YYLEX_PARAM_TYPE (parser_info&)
// interface to lexer
#define yyFlexLexer exparser_FlexLexer
#include <FlexLexer.h>
  
  namespace exparser
  {
    ::exparser_FlexLexer lexer;

    #define YYSTYPE Token
    int yylex( Token *lvalp, void *info )
    {
      lexer.info = static_cast<parser_info*> (info);
      lexer.yylval = lvalp;	// lvalue variable to return token value
      return lexer.yylex();
    }

    // redefine error output
#define yyerror( s ) ( ((parser_info*)info)-> \
  msg.error(new message::File_Position("unknown", lexer.lineno() )) << s, 1 )

    /*		       
    int yyerror( char *s )
    {
      cerr << "error: " << s;
      return 0;
    }
    */

/*
  int yylex()
  {
    return ::yylex();
  }
  int yylex()
  {
    return lexer.yylex();
  }
*/
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		34
#define	YYFLAG		-32768
#define	YYNTBASE	31

#define YYTRANSLATE(x) ((unsigned)(x) <= 274 ? yytranslate[x] : 36)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    26,
    27,    21,    19,    29,    20,     2,    22,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    25,    28,
     2,    30,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    23,    24
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     7,    10,    12,    16,    20,    24,    28,
    31,    36,    38,    42,    50,    52,    54,    56,    58,    62,
    66,    70,    74,    77,    79,    83,    91,    93,    95
};

static const short yyrhs[] = {    32,
     0,    33,     0,    34,    25,     0,    35,    25,     0,    14,
     0,    34,    19,    34,     0,    34,    20,    34,     0,    34,
    21,    34,     0,    34,    22,    34,     0,    20,    34,     0,
     8,    26,    34,    27,     0,    15,     0,    35,    19,    35,
     0,    28,    34,    29,    34,    29,    34,    30,     0,    16,
     0,    17,     0,    18,     0,     9,     0,     0,    19,     0,
     0,     0,    20,     0,     0,     0,    21,     0,     0,     0,
    22,     0,     0,    20,     0,     0,    10,     0,     0,    19,
     0,     0,    28,     0,    29,     0,    29,     0,    30,     0,
    11,     0,    12,     0,    13,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    92,    93,    96,   103,   111,   113,   114,   115,   116,   117,
   118,   119,   122,   123,   124,   127,   129,   133,   136,   137,
   138,   139,   140,   141,   144,   145,   149,   152,   154
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","TOK_INVALID_ID",
"TOK_IS_EQUAL","TOK_NOT_EQUAL","TOK_MORE_EQUAL","TOK_LESS_EQUAL","TOK_FUNC_SIN",
"TOK_OP_FLAG","TOK_OP_SCALAR","TOK_OP_VECTOR","TOK_OP_MATRIX","TOK_OP_STRING",
"TOK_FLAG","TOK_SCALAR","TOK_VECTOR","TOK_MATRIX","TOK_STRING","'+'","'-'","'*'",
"'/'","UMINUS","OP_CONVERTION","';'","'('","')'","'<'","','","'>'","any_statement",
"scalar_statement","vector_statement","scalar_exp","vector_exp", NULL
};
#endif

static const short yyr1[] = {     0,
    31,    31,    32,    33,    -1,    34,    34,    34,    34,    34,
    34,    34,    35,    35,    35,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1
};

static const short yyr2[] = {     0,
     1,     1,     2,     2,     1,     3,     3,     3,     3,     2,
     4,     1,     3,     7,     1,     1,     1,     1,     3,     3,
     3,     3,     2,     1,     3,     7,     1,     1,     1
};

static const short yydefact[] = {     0,
     0,    12,    15,     0,     0,     1,     2,     0,     0,     0,
    10,     0,     0,     0,     0,     0,     3,     0,     4,     0,
     0,     6,     7,     8,     9,    13,    11,     0,     0,     0,
    14,     0,     0,     0
};

static const short yydefgoto[] = {    32,
     6,     7,     8,     9
};

static const short yypact[] = {    -1,
    -2,-32768,-32768,     8,     8,-32768,-32768,    29,    34,     8,
-32768,    10,     8,     8,     8,     8,-32768,    -8,-32768,    25,
     8,     0,     0,-32768,-32768,-32768,-32768,    14,     8,   -17,
-32768,    18,    26,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,    -4,    19
};


#define	YYLAST		59


static const short yytable[] = {    11,
    12,    13,    14,    15,    16,    20,     1,     3,    22,    23,
    24,    25,    31,     2,     3,     1,    28,    33,     4,     5,
    15,    16,     2,    10,    30,    34,     5,     4,    13,    14,
    15,    16,    13,    14,    15,    16,    26,     0,    21,     0,
     0,     0,    29,    13,    14,    15,    16,    13,    14,    15,
    16,    27,    18,    17,     0,     0,     0,     0,    19
};

static const short yycheck[] = {     4,
     5,    19,    20,    21,    22,    10,     8,    16,    13,    14,
    15,    16,    30,    15,    16,     8,    21,     0,    20,    28,
    21,    22,    15,    26,    29,     0,    28,    20,    19,    20,
    21,    22,    19,    20,    21,    22,    18,    -1,    29,    -1,
    -1,    -1,    29,    19,    20,    21,    22,    19,    20,    21,
    22,    27,    19,    25,    -1,    -1,    -1,    -1,    25
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYPARSE_RETURN_TYPE
#define YYPARSE_RETURN_TYPE int
#endif


#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 222 "/usr/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
YYPARSE_RETURN_TYPE
yyparse (void *);
#else
YYPARSE_RETURN_TYPE
yyparse (void);
#endif
#endif

YYPARSE_RETURN_TYPE
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
#ifndef YYSTACK_USE_ALLOCA
  int yyfree_stacks = 0;
#endif

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
#ifndef YYSTACK_USE_ALLOCA
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
#endif	    
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 3:
#line 97 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{ 
   Token result; result.scalar() = yyvsp[-1].scalar();
   static_cast<parser_info*>(info)->result = result; 
   return 0;
 ;
    break;}
case 4:
#line 104 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{ 
   Token result; result.vector() = yyvsp[-1].vector();
   static_cast<parser_info*>(info)->result = result; 
   return 0;
 ;
    break;}
case 5:
#line 111 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.flag() = yyvsp[0].flag();;
    break;}
case 6:
#line 113 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.scalar() = yyvsp[-2].scalar() + yyvsp[0].scalar();;
    break;}
case 7:
#line 114 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.scalar() = yyvsp[-2].scalar() - yyvsp[0].scalar();;
    break;}
case 8:
#line 115 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.scalar() = yyvsp[-2].scalar() * yyvsp[0].scalar();;
    break;}
case 9:
#line 116 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.scalar() = yyvsp[-2].scalar() / yyvsp[0].scalar();;
    break;}
case 10:
#line 117 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.scalar() = -yyvsp[0].scalar();;
    break;}
case 11:
#line 118 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.scalar() = sin(yyvsp[-1].scalar());;
    break;}
case 12:
#line 119 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.scalar() = yyvsp[0].scalar();;
    break;}
case 13:
#line 122 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.vector() = yyvsp[-2].vector() + yyvsp[0].vector();;
    break;}
case 14:
#line 123 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{ yyval.vector() = values::Vector( yyvsp[-5].scalar(), yyvsp[-3].scalar(), yyvsp[-1].scalar() ); ;
    break;}
case 15:
#line 124 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.vector() = yyvsp[0].vector();;
    break;}
case 16:
#line 127 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.matrix() = yyvsp[0].matrix();;
    break;}
case 17:
#line 129 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.string() = yyvsp[0].string();;
    break;}
case 18:
#line 133 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.op_flag() = yyvsp[0].op_flag();;
    break;}
case 19:
#line 136 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.op_scalar() = yyvsp[-2].op_scalar() + yyvsp[0].op_scalar();;
    break;}
case 20:
#line 137 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.op_scalar() = yyvsp[-2].op_scalar() - yyvsp[0].op_scalar();;
    break;}
case 21:
#line 138 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.op_scalar() = yyvsp[-2].op_scalar() * yyvsp[0].op_scalar();;
    break;}
case 22:
#line 139 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.op_scalar() = yyvsp[-2].op_scalar() / yyvsp[0].op_scalar();;
    break;}
case 23:
#line 140 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.op_scalar() = -yyvsp[0].op_scalar();;
    break;}
case 24:
#line 141 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.op_scalar() = yyvsp[0].op_scalar();;
    break;}
case 25:
#line 144 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.op_vector() = yyvsp[-2].op_vector() + yyvsp[0].op_vector();;
    break;}
case 26:
#line 146 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{ /*$$ = values::Vector( $2, $4, $6 ); not supported yet*/ 
	  yyerror("vector creation from operands not supported yet!");
	  assert(0); ;
    break;}
case 27:
#line 149 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.op_vector() = yyvsp[0].op_vector();;
    break;}
case 28:
#line 152 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.op_matrix() = yyvsp[0].op_matrix();;
    break;}
case 29:
#line 154 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"
{yyval.op_string() = yyvsp[0].op_string();;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 554 "/usr/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
#ifndef YYSTACK_USE_ALLOCA
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
#endif
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
#ifndef YYSTACK_USE_ALLOCA
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
#endif    
  return 1;
}
#line 157 "/home/martin/Programmieren/sourceforge/anitmt/lib/exparse2/./parser.yy"


  Token get_expression( parser_info &info )
  {
    yyparse( static_cast<void*>(&info) );
    return info.result;
  }
 
  solve::Operand<values::Scalar> &get_scalar_expression( parser_info &info )
  {
    yyparse( static_cast<void*>(&info) );
    switch(info.result.get_type())
    {
    case TOK_SCALAR:
      std::cout << info.result.scalar() << std::endl;
      return solve::const_op( info.result.scalar() );	
      break;
    case TOK_OP_SCALAR:
      std::cout << info.result.op_scalar()() << std::endl;
      return info.result.op_scalar();	
      break;
    case TOK_VECTOR:
      std::cout << info.result.vector() << std::endl;
      std::cerr << "wrong type!" << std::endl; // should use info.msg.error()
      break;
    default:
      std::cerr << "fatal error: unknown expression type" << std::endl;
      assert(0);
    }
  }

  solve::Operand<values::Vector> &get_vector_expression( parser_info &info )
  {
    yyparse( static_cast<void*>(&info) );
    switch(info.result.get_type())
    {
    case TOK_SCALAR:
      std::cout << info.result.scalar() << std::endl;
      break;
    case TOK_VECTOR:
      std::cout << info.result.vector() << std::endl;
      return solve::const_op( info.result.vector() );	
      break;
    default:
      std::cerr << "fatal error: unknown expression type" << std::endl;
      assert(0);
    }
  }
}

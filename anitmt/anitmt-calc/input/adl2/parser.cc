
/*  A Bison parser, made from /home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	TOK_INVALID_ID	257
#define	TOK_ERROR	258
#define	TOK_DUMMY_OPERAND	259
#define	TOK_DUMMY_OPERATOR	260
#define	TOK_IS_EQUAL	261
#define	TOK_NOT_EQUAL	262
#define	TOK_MORE_EQUAL	263
#define	TOK_LESS_EQUAL	264
#define	TOK_FUNC_SIN	265
#define	TOK_FUNC_SQRT	266
#define	TOK_IDENTIFIER	267
#define	TOK_FLAG	268
#define	TOK_SCALAR	269
#define	TOK_VECTOR	270
#define	TOK_MATRIX	271
#define	TOK_STRING	272
#define	TOK_OP_FLAG	273
#define	TOK_OP_SCALAR	274
#define	TOK_OP_VECTOR	275
#define	TOK_OP_MATRIX	276
#define	TOK_OP_STRING	277
#define	TOK_PROP_FLAG	278
#define	TOK_PROP_SCALAR	279
#define	TOK_PROP_VECTOR	280
#define	TOK_PROP_MATRIX	281
#define	TOK_PROP_STRING	282
#define	UMINUS	283
#define	OP_CONVERTION	284
#define	left_associated	285
#define	right_associated	286
#define	lower_precedence	287
#define	higher_precedence	288

#line 19 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"

#include <iostream>
#include <string>

#include <val/val.hpp>
#include <solve/operand.hpp>
#include <solve/operator.hpp>
#include <solve/reference.hpp>
#include <message/message.hpp>

#include "token.hpp"
#include "parsinfo.hpp"
#include "adlparser.hpp"

#include "parser_functions.hpp"	// help functions for the parser
				// including nessessary defines 

  // open namespaces
  namespace anitmt
  {
    namespace adlparser
    {

#define MAX_OLD_POSITIONS 	10

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		176
#define	YYFLAG		-32768
#define	YYNTBASE	47

#define YYTRANSLATE(x) ((unsigned)(x) <= 288 ? yytranslate[x] : 79)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    42,
    43,    31,    29,    45,    30,     2,    32,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    39,    44,
     2,    46,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    40,     2,    41,     2,     2,     2,     2,     2,
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
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    33,    34,    35,    36,    37,    38
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     6,     8,    11,    14,    17,    20,    23,
    26,    28,    30,    31,    37,    38,    45,    46,    50,    52,
    54,    56,    57,    61,    63,    65,    67,    68,    72,    74,
    76,    78,    79,    83,    85,    87,    89,    90,    94,    96,
    98,   100,   104,   106,   110,   114,   118,   122,   125,   128,
   132,   137,   142,   144,   148,   156,   160,   162,   166,   168,
   172,   174,   178,   180,   182,   186,   190,   194,   198,   201,
   204,   208,   213,   215,   217,   219,   223,   231,   235,   237,
   239,   243,   245,   247,   251,   253,   255,   259,   264,   268,
   270
};

static const short yyrhs[] = {    -1,
    47,    48,     0,     1,     0,    50,     0,    53,    49,     0,
    56,    49,     0,    59,    49,     0,    62,    49,     0,    65,
    49,     0,     1,    39,     0,    39,     0,     1,     0,     0,
    13,    40,    51,    47,    41,     0,     0,    13,    13,    40,
    52,    47,    41,     0,     0,    24,    54,    55,     0,    68,
     0,    73,     0,    78,     0,     0,    25,    57,    58,     0,
    69,     0,    74,     0,    78,     0,     0,    26,    60,    61,
     0,    70,     0,    75,     0,    78,     0,     0,    27,    63,
    64,     0,    71,     0,    76,     0,    78,     0,     0,    28,
    66,    67,     0,    72,     0,    77,     0,    78,     0,    42,
    14,    43,     0,    14,     0,    69,    29,    69,     0,    69,
    30,    69,     0,    69,    31,    69,     0,    69,    32,    69,
     0,    30,    69,     0,    29,    69,     0,    42,    69,    43,
     0,    11,    42,    69,    43,     0,    12,    42,    69,    43,
     0,    15,     0,    70,    29,    70,     0,    44,    69,    45,
    69,    45,    69,    46,     0,    42,    70,    43,     0,    16,
     0,    42,    17,    43,     0,    17,     0,    42,    18,    43,
     0,    18,     0,    42,    19,    43,     0,    24,     0,    19,
     0,    74,    29,    74,     0,    74,    30,    74,     0,    74,
    31,    74,     0,    74,    32,    74,     0,    30,    74,     0,
    29,    74,     0,    42,    74,    43,     0,    12,    42,    74,
    43,     0,    69,     0,    25,     0,    20,     0,    75,    29,
    75,     0,    44,    74,    45,    74,    45,    74,    46,     0,
    42,    21,    43,     0,    26,     0,    21,     0,    42,    22,
    43,     0,    27,     0,    22,     0,    42,    23,    43,     0,
    28,     0,    23,     0,    78,     6,    78,     0,     5,    42,
    78,    43,     0,    42,    78,    43,     0,     5,     0,     1,
     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   124,   125,   126,   129,   131,   132,   133,   134,   135,   136,
   139,   141,   143,   145,   146,   147,   150,   152,   154,   156,
   157,   160,   162,   164,   166,   167,   170,   172,   174,   176,
   177,   180,   182,   184,   186,   187,   190,   192,   194,   196,
   197,   201,   203,   206,   208,   209,   210,   211,   212,   213,
   214,   215,   216,   219,   221,   223,   224,   227,   229,   232,
   234,   237,   239,   240,   244,   246,   247,   248,   249,   250,
   251,   252,   253,   255,   256,   259,   261,   265,   266,   267,
   270,   272,   273,   276,   278,   279,   282,   284,   285,   286,
   287
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","TOK_INVALID_ID",
"TOK_ERROR","TOK_DUMMY_OPERAND","TOK_DUMMY_OPERATOR","TOK_IS_EQUAL","TOK_NOT_EQUAL",
"TOK_MORE_EQUAL","TOK_LESS_EQUAL","TOK_FUNC_SIN","TOK_FUNC_SQRT","TOK_IDENTIFIER",
"TOK_FLAG","TOK_SCALAR","TOK_VECTOR","TOK_MATRIX","TOK_STRING","TOK_OP_FLAG",
"TOK_OP_SCALAR","TOK_OP_VECTOR","TOK_OP_MATRIX","TOK_OP_STRING","TOK_PROP_FLAG",
"TOK_PROP_SCALAR","TOK_PROP_VECTOR","TOK_PROP_MATRIX","TOK_PROP_STRING","'+'",
"'-'","'*'","'/'","UMINUS","OP_CONVERTION","left_associated","right_associated",
"lower_precedence","higher_precedence","';'","'{'","'}'","'('","')'","'<'","','",
"'>'","tree_node_block","statement","expect_semicolon","child_declaration","@1",
"@2","flag_statement","@3","any_flag_exp","scalar_statement","@4","any_scalar_exp",
"vector_statement","@5","any_vector_exp","matrix_statement","@6","any_matrix_exp",
"string_statement","@7","any_string_exp","flag_exp","scalar_exp","vector_exp",
"matrix_exp","string_exp","op_flag_exp","op_scalar_exp","op_vector_exp","op_matrix_exp",
"op_string_exp","dummy_exp", NULL
};
#endif

static const short yyr1[] = {     0,
    47,    47,    47,    48,    48,    48,    48,    48,    48,    48,
    49,    49,    51,    50,    52,    50,    54,    53,    55,    55,
    55,    57,    56,    58,    58,    58,    60,    59,    61,    61,
    61,    63,    62,    64,    64,    64,    66,    65,    67,    67,
    67,    68,    68,    69,    69,    69,    69,    69,    69,    69,
    69,    69,    69,    70,    70,    70,    70,    71,    71,    72,
    72,    73,    73,    73,    74,    74,    74,    74,    74,    74,
    74,    74,    74,    74,    74,    75,    75,    75,    75,    75,
    76,    76,    76,    77,    77,    77,    78,    78,    78,    78,
    78
};

static const short yyr2[] = {     0,
     0,     2,     1,     1,     2,     2,     2,     2,     2,     2,
     1,     1,     0,     5,     0,     6,     0,     3,     1,     1,
     1,     0,     3,     1,     1,     1,     0,     3,     1,     1,
     1,     0,     3,     1,     1,     1,     0,     3,     1,     1,
     1,     3,     1,     3,     3,     3,     3,     2,     2,     3,
     4,     4,     1,     3,     7,     3,     1,     3,     1,     3,
     1,     3,     1,     1,     3,     3,     3,     3,     2,     2,
     3,     4,     1,     1,     1,     3,     7,     3,     1,     1,
     3,     1,     1,     3,     1,     1,     3,     4,     3,     1,
     1
};

static const short yydefact[] = {     0,
     3,     0,     0,     0,    17,    22,    27,    32,    37,     2,
     4,     0,     0,     0,     0,     0,    10,     0,    13,     0,
     0,     0,     0,     0,    12,    11,     5,     6,     7,     8,
     9,    15,     0,    91,    90,    43,    64,    63,     0,    18,
    19,    20,    21,     0,     0,    53,    75,    74,     0,     0,
     0,    23,    73,    25,    26,    57,    80,    79,     0,     0,
    28,    29,    30,    31,    59,    83,    82,     0,    33,    34,
    35,    36,    61,    86,    85,     0,    38,    39,    40,    41,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    49,    70,    48,    69,    73,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    73,
     0,     0,     0,     0,     0,     0,     0,     0,    14,     0,
    42,    62,    89,    87,     0,     0,     0,     0,     0,    73,
     0,    50,    71,    44,    45,    46,    47,    73,    65,    66,
    67,    68,    78,     0,    56,     0,     0,     0,    54,     0,
     0,    76,    58,    81,    60,    84,    16,    88,     0,    49,
    48,     0,    51,    52,    72,     0,     0,     0,     0,     0,
     0,     0,    55,    77,     0,     0
};

static const short yydefgoto[] = {     2,
    10,    27,    11,    33,    81,    12,    20,    40,    13,    21,
    52,    14,    22,    61,    15,    23,    69,    16,    24,    77,
    41,   138,   109,    70,    78,    42,    97,    63,    71,    79,
    87
};

static const short yypact[] = {   193,
-32768,   201,   -23,    59,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,    17,    17,    17,    17,    17,-32768,   -18,-32768,    56,
    92,     9,    89,    22,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   126,-32768,   -22,-32768,-32768,-32768,    54,-32768,
-32768,-32768,    28,    -1,     6,-32768,-32768,-32768,   180,   180,
    92,-32768,    23,   147,    28,-32768,-32768,-32768,    10,   180,
-32768,    38,    45,    28,-32768,-32768,-32768,   118,-32768,-32768,
-32768,    28,-32768,-32768,-32768,   119,-32768,-32768,-32768,    28,
   126,   144,   124,    57,    58,   124,     3,   124,   169,   180,
   180,-32768,-32768,-32768,-32768,    62,   241,   169,   169,   169,
   169,   180,   180,   180,   180,    65,    16,   169,   -10,    20,
   212,    47,   209,    66,    67,    75,    85,   162,-32768,    36,
-32768,-32768,-32768,    28,    60,   169,   169,   169,   256,    93,
   260,-32768,-32768,   -19,   -19,-32768,-32768,-32768,    12,    12,
-32768,-32768,-32768,   229,-32768,   169,   180,    47,-32768,   111,
   180,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   169,-32768,
-32768,   264,-32768,-32768,-32768,   233,   237,   279,   169,   180,
   202,   208,-32768,-32768,   138,-32768
};

static const short yypgoto[] = {   -32,
-32768,   299,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   -13,   -20,-32768,-32768,-32768,   -21,    30,-32768,-32768,
   -17
};


#define	YYLAST		322


static const short yytable[] = {    54,
    82,    62,    43,    55,    64,    72,    80,    53,    88,    34,
    34,   100,   101,    35,    35,    17,    34,    25,   112,    83,
    35,    32,    34,   -24,    56,    56,    35,    93,    95,    57,
   106,    56,   145,    88,    58,    92,    94,    96,   111,    73,
    89,    88,   104,   105,    74,   123,   110,    90,   118,    75,
    59,   107,    60,   108,    34,    26,    34,   107,    35,   108,
    35,   -24,    56,    76,   146,   120,   112,    84,   131,    36,
   124,    18,    85,   113,    37,   129,   130,    96,   158,    38,
   139,   140,   141,   142,   134,   135,   136,   137,   148,    34,
   108,   149,    34,    35,   144,    86,    35,    39,    19,   121,
   122,   159,    44,    45,   132,    65,    46,   143,   153,   154,
    66,    47,   160,   161,   162,    67,    48,   155,    34,    34,
    49,    50,    35,    35,    34,   167,     1,   156,    35,   111,
    68,   106,   166,    51,   114,   164,   116,   176,    -1,   115,
     0,   117,   152,     0,     3,   168,     0,     0,   172,    -1,
    -1,    -1,    -1,    -1,     0,   171,     4,     0,     0,    86,
    86,     0,     3,     0,     0,    86,    -1,     5,     6,     7,
     8,     9,     0,     0,     4,   102,   103,   104,   105,    44,
   125,     0,     0,    46,   119,     5,     6,     7,     8,     9,
    44,    45,    -1,     1,    46,     0,     0,   126,   127,    47,
   175,     3,   157,     0,    48,    -1,     0,     0,    49,    50,
   128,     0,     0,     4,     0,     0,    -1,    -1,    -1,    -1,
    -1,    91,     0,     0,     5,     6,     7,     8,     9,    57,
    98,    99,   100,   101,    58,     0,   102,   103,   104,   105,
   102,   103,   104,   105,     0,     0,     0,   173,     0,     0,
   150,     0,   151,   174,     0,     0,   147,    98,    99,   100,
   101,    98,    99,   100,   101,   102,   103,   104,   105,   102,
   103,   104,   105,   146,     0,     0,     0,   169,     0,     0,
     0,   170,     0,   133,    98,    99,   100,   101,   102,   103,
   104,   105,    98,    99,   100,   101,     0,     0,   163,     0,
     0,     0,   165,     0,     0,     0,   132,    98,    99,   100,
   101,    28,    29,    30,    31,     0,     0,     0,     0,     0,
     0,   164
};

static const short yycheck[] = {    21,
    33,    22,    20,    21,    22,    23,    24,    21,     6,     1,
     1,    31,    32,     5,     5,    39,     1,     1,    29,    42,
     5,    40,     1,     1,    16,    16,     5,    49,    50,    21,
    21,    16,    43,     6,    26,    49,    50,    51,    60,    18,
    42,     6,    31,    32,    23,    43,    60,    42,    81,    28,
    42,    42,    44,    44,     1,    39,     1,    42,     5,    44,
     5,    39,    16,    42,    45,    83,    29,    14,    90,    14,
    88,    13,    19,    29,    19,    89,    90,    91,    43,    24,
   102,   103,   104,   105,    98,    99,   100,   101,    42,     1,
    44,   112,     1,     5,   108,    42,     5,    42,    40,    43,
    43,    42,    11,    12,    43,    17,    15,    43,    43,    43,
    22,    20,   126,   127,   128,    27,    25,    43,     1,     1,
    29,    30,     5,     5,     1,   147,     1,    43,     5,   151,
    42,    21,   146,    42,    17,    43,    18,     0,    13,    22,
    -1,    23,   113,    -1,     1,   159,    -1,    -1,   170,    24,
    25,    26,    27,    28,    -1,   169,    13,    -1,    -1,    42,
    42,    -1,     1,    -1,    -1,    42,    41,    24,    25,    26,
    27,    28,    -1,    -1,    13,    29,    30,    31,    32,    11,
    12,    -1,    -1,    15,    41,    24,    25,    26,    27,    28,
    11,    12,     0,     1,    15,    -1,    -1,    29,    30,    20,
     0,     1,    41,    -1,    25,    13,    -1,    -1,    29,    30,
    42,    -1,    -1,    13,    -1,    -1,    24,    25,    26,    27,
    28,    42,    -1,    -1,    24,    25,    26,    27,    28,    21,
    29,    30,    31,    32,    26,    -1,    29,    30,    31,    32,
    29,    30,    31,    32,    -1,    -1,    -1,    46,    -1,    -1,
    42,    -1,    44,    46,    -1,    -1,    45,    29,    30,    31,
    32,    29,    30,    31,    32,    29,    30,    31,    32,    29,
    30,    31,    32,    45,    -1,    -1,    -1,    45,    -1,    -1,
    -1,    45,    -1,    43,    29,    30,    31,    32,    29,    30,
    31,    32,    29,    30,    31,    32,    -1,    -1,    43,    -1,
    -1,    -1,    43,    -1,    -1,    -1,    43,    29,    30,    31,
    32,    13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,
    -1,    43
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
#line 126 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyerrok; /* error recovery */ ;
    break;}
case 10:
#line 136 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyerrok; /* error recovery */ ;
    break;}
case 12:
#line 141 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyerr(info,2) << "semicolon expected"; yyerrok; ;
    break;}
case 13:
#line 144 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_current_child(info,yyvsp[-1].identifier()); ;
    break;}
case 14:
#line 145 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_to_parent(info); ;
    break;}
case 15:
#line 146 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_current_child(info,yyvsp[-2].identifier(),yyvsp[-1].identifier());;
    break;}
case 16:
#line 147 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_to_parent(info); ;
    break;}
case 17:
#line 151 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_flag(),info); ;
    break;}
case 18:
#line 152 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ flag_prop_declaration_finish(yyvsp[-2].prop_flag(),yyvsp[0].tok(),info); ;
    break;}
case 19:
#line 155 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.flag() = yyvsp[0].flag(); ;
    break;}
case 20:
#line 156 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_flag(info) = yyvsp[0].meta_op_flag(info)(); ;
    break;}
case 21:
#line 157 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 22:
#line 161 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_scalar(),info); ;
    break;}
case 23:
#line 162 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ scalar_prop_declaration_finish(yyvsp[-2].prop_scalar(),yyvsp[0].tok(),info); ;
    break;}
case 24:
#line 165 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[0].scalar(); ;
    break;}
case 25:
#line 166 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 26:
#line 167 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 27:
#line 171 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_vector(),info); ;
    break;}
case 28:
#line 172 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ vector_prop_declaration_finish(yyvsp[-2].prop_vector(),yyvsp[0].tok(),info); ;
    break;}
case 29:
#line 175 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = yyvsp[0].vector(); ;
    break;}
case 30:
#line 176 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[0].meta_op_vector(info)(); ;
    break;}
case 31:
#line 177 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 32:
#line 181 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_matrix(),info); ;
    break;}
case 33:
#line 182 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ matrix_prop_declaration_finish(yyvsp[-2].prop_matrix(),yyvsp[0].tok(),info); ;
    break;}
case 34:
#line 185 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.matrix() = yyvsp[0].matrix(); ;
    break;}
case 35:
#line 186 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_matrix(info) = yyvsp[0].meta_op_matrix(info)(); ;
    break;}
case 36:
#line 187 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 37:
#line 191 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_string(),info); ;
    break;}
case 38:
#line 192 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ string_prop_declaration_finish(yyvsp[-2].prop_string(),yyvsp[0].tok(),info); ;
    break;}
case 39:
#line 195 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.string() = yyvsp[0].string(); ;
    break;}
case 40:
#line 196 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_string(info) = yyvsp[0].meta_op_string(info)(); ;
    break;}
case 41:
#line 197 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 42:
#line 202 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.flag() = yyvsp[-1].flag(); ;
    break;}
case 43:
#line 203 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.flag() = yyvsp[0].flag(); ;
    break;}
case 44:
#line 207 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-2].scalar() + yyvsp[0].scalar(); ;
    break;}
case 45:
#line 208 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-2].scalar() - yyvsp[0].scalar(); ;
    break;}
case 46:
#line 209 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-2].scalar() * yyvsp[0].scalar(); ;
    break;}
case 47:
#line 210 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-2].scalar() / yyvsp[0].scalar(); ;
    break;}
case 48:
#line 211 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = -yyvsp[0].scalar(); ;
    break;}
case 49:
#line 212 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[0].scalar(); ;
    break;}
case 50:
#line 213 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-1].scalar(); ;
    break;}
case 51:
#line 214 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = sin(yyvsp[-1].scalar()); ;
    break;}
case 52:
#line 215 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = sqrt(yyvsp[-1].scalar()); ;
    break;}
case 53:
#line 216 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[0].scalar(); ;
    break;}
case 54:
#line 220 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = yyvsp[-2].vector() + yyvsp[0].vector(); ;
    break;}
case 55:
#line 222 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = values::Vector( yyvsp[-5].scalar(), yyvsp[-3].scalar(), yyvsp[-1].scalar() ); ;
    break;}
case 56:
#line 223 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = yyvsp[-1].vector(); ;
    break;}
case 57:
#line 224 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = yyvsp[0].vector(); ;
    break;}
case 58:
#line 228 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.matrix() = yyvsp[-1].matrix(); ;
    break;}
case 59:
#line 229 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.matrix() = yyvsp[0].matrix(); ;
    break;}
case 60:
#line 233 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.string() = yyvsp[-1].string(); ;
    break;}
case 61:
#line 234 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.string() = yyvsp[0].string(); ;
    break;}
case 62:
#line 238 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_flag(info) = yyvsp[-1].op_flag(info);;
    break;}
case 63:
#line 239 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_flag(info) = yyvsp[0].prop_flag();;
    break;}
case 64:
#line 240 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_flag(info) = yyvsp[0].op_flag(info);;
    break;}
case 65:
#line 245 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() + yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 66:
#line 246 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() - yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 67:
#line 247 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() * yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 68:
#line 248 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() / yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 69:
#line 249 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = -yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 70:
#line 250 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 71:
#line 251 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-1].meta_op_scalar(info)(); ;
    break;}
case 72:
#line 252 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = sqrt(yyvsp[-1].meta_op_scalar(info)()); ;
    break;}
case 73:
#line 254 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = solve::const_op(yyvsp[0].scalar(),msg_consultant(info));;
    break;}
case 74:
#line 255 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[0].prop_scalar(); ;
    break;}
case 75:
#line 256 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[0].op_scalar(info); ;
    break;}
case 76:
#line 260 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[-2].meta_op_vector(info)() + yyvsp[0].meta_op_vector(info)(); ;
    break;}
case 77:
#line 262 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ /*$$ = combine_Vector( $2, $4, $6 ); not supported yet*/ 
	yyerr(info) << "vector creation from operands not supported yet!";
	assert(0); ;
    break;}
case 78:
#line 265 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[-1].op_vector(info); ;
    break;}
case 79:
#line 266 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[0].prop_vector(); ;
    break;}
case 80:
#line 267 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[0].op_vector(info); ;
    break;}
case 81:
#line 271 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_matrix(info) = yyvsp[-1].op_matrix(info);;
    break;}
case 82:
#line 272 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_matrix(info) = yyvsp[0].prop_matrix();;
    break;}
case 83:
#line 273 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_matrix(info) = yyvsp[0].op_matrix(info);;
    break;}
case 84:
#line 277 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_string(info) = yyvsp[-1].op_string(info);;
    break;}
case 85:
#line 278 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_string(info) = yyvsp[0].prop_string();;
    break;}
case 86:
#line 279 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_string(info) = yyvsp[0].op_string(info);;
    break;}
case 91:
#line 287 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyerr(info,2) << "operator expected"; yyerrok; ;
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
#line 290 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"

    int parse_adl( Prop_Tree_Node *node, message::Message_Consultant *c, 
		   std::string filename, pass_type pass )
    {
      adlparser_info info(c);
      info.set_max_old_positions(MAX_OLD_POSITIONS);
      info.id_resolver = &info.res_property;
      
      info.open_file( filename );
      info.set_new_tree_node( node );
      info.set_pass(pass);
      int ret = yyparse( static_cast<void*>(&info) );
      info.close_file();
      
      return ret;
    }

  } // close namespace adlparser
} // close namespace anitmt

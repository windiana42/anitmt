
/*  A Bison parser, made from /home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	TAFD_include	257
#define	TAFD_declaration	258
#define	TAFD_base_types	259
#define	TAFD_serial	260
#define	TAFD_type	261
#define	TAFD_node	262
#define	TAFD_provides	263
#define	TAFD_extends	264
#define	TAFD_properties	265
#define	TAFD_aliases	266
#define	TAFD_operands	267
#define	TAFD_common	268
#define	TAFD_constraints	269
#define	TAFD_solvers	270
#define	TAFD_actions	271
#define	TAFD_push	272
#define	TAFD_default	273
#define	TAFD_contains	274
#define	TAFD_max1	275
#define	TAFD_min1	276
#define	TAFD_provide	277
#define	TAFD_resulting	278
#define	TAFD_requires	279
#define	TAFD_this	280
#define	TAFD_prev	281
#define	TAFD_next	282
#define	TAFD_first	283
#define	TAFD_last	284
#define	TAFD_parent	285
#define	TAFD_child	286
#define	TAFD_first_child	287
#define	TAFD_last_child	288
#define	TAFD_start_param	289
#define	TAFD_end_param	290
#define	TAFD_true	291
#define	TAFD_false	292
#define	TAFD_ERROR	293
#define	TAFD_IS_EQUAL	294
#define	TAFD_NOT_EQUAL	295
#define	TAFD_MORE_EQUAL	296
#define	TAFD_LESS_EQUAL	297
#define	TAFD_NS_CONCAT	298
#define	TAFD_CODE	299
#define	TAFD_IDENTIFIER	300
#define	TAFD_SCALAR	301
#define	TAFD_QSTRING	302
#define	UMINUS	303

#line 19 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"

#include <iostream>
#include <string>

#include <message/message.hpp>

#include "token.hpp"
#include "parsinfo.hpp"
#include "afdbase.hpp"

#include "parser_functions.hpp"	// help functions for the parser
				// including nessessary defines 

#define MAX_OLD_POSITIONS 	10

// open namespaces
namespace funcgen
{

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		355
#define	YYFLAG		-32768
#define	YYNTBASE	66

#define YYTRANSLATE(x) ((unsigned)(x) <= 303 ? yytranslate[x] : 158)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    61,
    62,    53,    51,    60,    52,    63,    54,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    56,    49,
    59,    50,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    64,     2,    65,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    57,     2,    58,     2,     2,     2,     2,     2,
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
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    55
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     6,     8,    10,    12,    17,    22,    23,
    26,    31,    32,    40,    42,    46,    48,    52,    55,    56,
    64,    65,    67,    68,    71,    78,    79,    88,    89,    92,
    94,    98,    99,   102,   104,   108,   110,   111,   114,   116,
   118,   120,   122,   124,   126,   128,   130,   135,   136,   139,
   140,   147,   151,   152,   155,   158,   163,   164,   167,   172,
   177,   178,   181,   182,   189,   193,   194,   197,   200,   201,
   207,   208,   215,   216,   223,   224,   226,   227,   230,   232,
   234,   236,   241,   242,   245,   248,   253,   254,   257,   258,
   265,   270,   272,   276,   281,   282,   285,   286,   287,   299,
   300,   301,   313,   314,   323,   325,   327,   331,   336,   337,
   340,   345,   346,   348,   349,   351,   352,   359,   360,   363,
   364,   376,   377,   380,   382,   385,   387,   391,   400,   404,
   411,   420,   421,   424,   425,   428,   430,   434,   438,   442,
   444,   454,   463,   464,   467,   471,   475,   479,   483,   487,
   491,   493,   495,   499,   503,   507,   511,   515,   520,   522,
   526,   530,   534,   541,   543,   545,   549,   553,   557,   563,
   569,   576,   578,   580,   582,   584,   586
};

static const short yyrhs[] = {    -1,
    66,    67,     0,    68,     0,    69,     0,    76,     0,    81,
     0,     3,     4,    48,    56,     0,     5,    57,    70,    58,
     0,     0,    70,    71,     0,    46,    59,    73,    56,     0,
     0,    46,    59,    57,    72,    74,    58,    56,     0,    46,
     0,    73,    44,    46,     0,    75,     0,    74,    60,    75,
     0,    46,    46,     0,     0,    78,     7,    46,    77,    57,
    79,    58,     0,     0,     6,     0,     0,    79,    80,     0,
     9,    46,    61,    46,    62,    56,     0,     0,     8,    46,
    82,    83,    85,    57,    88,    58,     0,     0,    10,    84,
     0,    46,     0,    84,    60,    46,     0,     0,     9,    86,
     0,    87,     0,    86,    60,    87,     0,    46,     0,     0,
    88,    89,     0,    90,     0,    96,     0,    99,     0,   105,
     0,   107,     0,   109,     0,   132,     0,   137,     0,    11,
    57,    91,    58,     0,     0,    91,    92,     0,     0,     7,
    46,    93,    57,    94,    58,     0,    46,    46,    56,     0,
     0,    94,    95,     0,    46,    56,     0,    12,    57,    97,
    58,     0,     0,    97,    98,     0,    46,    59,    46,    56,
     0,    13,    57,   100,    58,     0,     0,   100,   101,     0,
     0,     7,    46,   102,    57,   103,    58,     0,    46,    46,
    56,     0,     0,   103,   104,     0,    46,    56,     0,     0,
    14,   106,    57,   112,    58,     0,     0,    29,   111,   108,
    57,   112,    58,     0,     0,    30,   111,   110,    57,   112,
    58,     0,     0,    46,     0,     0,   112,   113,     0,   114,
     0,   117,     0,   122,     0,    15,    57,   115,    58,     0,
     0,   115,   116,     0,   151,    56,     0,    16,    57,   118,
    58,     0,     0,   118,   119,     0,     0,    46,   120,    61,
   121,    62,    56,     0,    46,    59,   152,    56,     0,   153,
     0,   121,    60,   153,     0,    17,    57,   123,    58,     0,
     0,   123,   124,     0,     0,     0,    19,    61,   130,    60,
   125,   153,    60,   126,   152,    62,    56,     0,     0,     0,
    18,    61,   130,    60,   127,   153,    60,   128,   153,    62,
    56,     0,     0,    46,    61,   130,    60,   129,   131,    62,
    56,     0,    47,     0,   153,     0,   131,    60,   153,     0,
    20,    57,   133,    58,     0,     0,   133,   134,     0,   135,
   136,    46,    56,     0,     0,    21,     0,     0,    22,     0,
     0,    23,    46,   138,    57,   139,    58,     0,     0,   139,
   140,     0,     0,    24,    46,    61,    46,    46,    62,   141,
   142,    57,   145,    58,     0,     0,    25,   143,     0,   144,
     0,   143,   144,     0,    46,     0,    32,    63,    46,     0,
    32,    63,    46,    63,    46,    61,    46,    62,     0,    26,
    63,    46,     0,    26,    63,    46,    61,    46,    62,     0,
    26,    63,    46,    63,    46,    61,    46,    62,     0,     0,
   146,   147,     0,     0,   147,   148,     0,    45,     0,    64,
   149,    65,     0,    64,    47,    65,     0,    64,     1,    65,
     0,    46,     0,    32,    63,    46,    63,    46,    61,    46,
    46,    62,     0,    26,    63,   150,    46,    61,    46,    46,
    62,     0,     0,    46,    63,     0,   152,    40,   152,     0,
   152,    41,   152,     0,   152,    42,   152,     0,   152,    43,
   152,     0,   152,    50,   152,     0,   152,    49,   152,     0,
   153,     0,    47,     0,    61,   152,    62,     0,   152,    51,
   152,     0,   152,    52,   152,     0,   152,    53,   152,     0,
   152,    54,   152,     0,    46,    61,   152,    62,     0,    46,
     0,   155,    63,    46,     0,   154,    63,    35,     0,   154,
    63,    36,     0,    46,    63,    46,    61,    46,    62,     0,
   156,     0,   157,     0,   155,    63,   157,     0,   154,    63,
    27,     0,   154,    63,    28,     0,    32,    63,    46,    63,
    29,     0,    32,    63,    46,    63,    30,     0,    32,    63,
    46,    64,    47,    65,     0,    27,     0,    28,     0,    31,
     0,    33,     0,    34,     0,    32,    64,    47,    65,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    95,    97,    99,   101,   102,   103,   105,   108,   111,   112,
   114,   117,   119,   121,   123,   125,   127,   129,   134,   137,
   139,   140,   142,   143,   145,   149,   152,   154,   155,   157,
   159,   161,   162,   164,   165,   167,   170,   171,   173,   175,
   176,   177,   178,   179,   180,   181,   183,   186,   187,   188,
   191,   192,   195,   196,   198,   201,   204,   205,   207,   211,
   214,   215,   216,   219,   220,   223,   224,   226,   229,   232,
   234,   237,   239,   242,   244,   245,   247,   248,   250,   252,
   253,   255,   258,   259,   261,   264,   267,   268,   270,   273,
   275,   277,   280,   283,   286,   287,   289,   292,   294,   297,
   299,   301,   304,   306,   309,   312,   313,   317,   320,   321,
   323,   327,   328,   330,   331,   333,   336,   338,   339,   341,
   344,   346,   347,   349,   351,   353,   356,   358,   360,   362,
   364,   367,   368,   370,   371,   373,   375,   376,   377,   379,
   381,   384,   388,   389,   391,   393,   394,   395,   396,   397,
   399,   401,   402,   403,   404,   405,   406,   407,   409,   413,
   415,   416,   418,   422,   424,   425,   427,   429,   430,   432,
   434,   437,   439,   440,   441,   442,   443
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","TAFD_include",
"TAFD_declaration","TAFD_base_types","TAFD_serial","TAFD_type","TAFD_node","TAFD_provides",
"TAFD_extends","TAFD_properties","TAFD_aliases","TAFD_operands","TAFD_common",
"TAFD_constraints","TAFD_solvers","TAFD_actions","TAFD_push","TAFD_default",
"TAFD_contains","TAFD_max1","TAFD_min1","TAFD_provide","TAFD_resulting","TAFD_requires",
"TAFD_this","TAFD_prev","TAFD_next","TAFD_first","TAFD_last","TAFD_parent","TAFD_child",
"TAFD_first_child","TAFD_last_child","TAFD_start_param","TAFD_end_param","TAFD_true",
"TAFD_false","TAFD_ERROR","TAFD_IS_EQUAL","TAFD_NOT_EQUAL","TAFD_MORE_EQUAL",
"TAFD_LESS_EQUAL","TAFD_NS_CONCAT","TAFD_CODE","TAFD_IDENTIFIER","TAFD_SCALAR",
"TAFD_QSTRING","'<'","'>'","'+'","'-'","'*'","'/'","UMINUS","';'","'{'","'}'",
"'='","','","'('","')'","'.'","'['","']'","statements","statement","include_declaration",
"base_types_declaration","base_type_statements","base_type_statement","@1","CXX_identifier",
"base_type_structure","base_type_structure_element","type_declaration","@2",
"opt_serial","provider_type_statements","provider_type_statement","node_declaration",
"@3","opt_extends","extend_list","opt_provides","provided_type_list","provided_type",
"node_statements","node_statement","properties_declaration","property_types",
"property_type","@4","property_names","property_name","aliases_declaration",
"alias_statements","alias_statement","operands_declaration","operand_types",
"operand_type","@5","operand_names","operand_name","common_declaration","@6",
"first_declaration","@7","last_declaration","@8","opt_provider_type","solve_system_statements",
"solve_system_statement","constraints_declaration","constraint_statements","constraint_statement",
"solvers_declaration","solver_statements","solver_statement","@9","solver_parameter_list",
"actions_declaration","action_statements","action_statement","@10","@11","@12",
"@13","@14","priority_level","action_parameter_list","contains_declaration",
"contain_statements","contain_statement","opt_max1","opt_min1","provide_declaration",
"@15","result_code_definitions","result_code_definition","@16","opt_requires",
"essentials_list","essential","result_code_block","@17","code_statements","code_statement",
"result_reference","opt_res_ref_provider","bool_expression","expression","property_reference",
"provider_type","node_reference","local_node_identifier","node_identifier", NULL
};
#endif

static const short yyr1[] = {     0,
    66,    66,    67,    67,    67,    67,    68,    69,    70,    70,
    71,    72,    71,    73,    73,    74,    74,    75,    77,    76,
    78,    78,    79,    79,    80,    82,    81,    83,    83,    84,
    84,    85,    85,    86,    86,    87,    88,    88,    89,    89,
    89,    89,    89,    89,    89,    89,    90,    91,    91,    93,
    92,    92,    94,    94,    95,    96,    97,    97,    98,    99,
   100,   100,   102,   101,   101,   103,   103,   104,   106,   105,
   108,   107,   110,   109,   111,   111,   112,   112,   113,   113,
   113,   114,   115,   115,   116,   117,   118,   118,   120,   119,
   119,   121,   121,   122,   123,   123,   125,   126,   124,   127,
   128,   124,   129,   124,   130,   131,   131,   132,   133,   133,
   134,   135,   135,   136,   136,   138,   137,   139,   139,   141,
   140,   142,   142,   143,   143,   144,   144,   144,   144,   144,
   144,   146,   145,   147,   147,   148,   148,   148,   148,   149,
   149,   149,   150,   150,   151,   151,   151,   151,   151,   151,
   152,   152,   152,   152,   152,   152,   152,   152,   153,   153,
   153,   153,   154,   155,   155,   155,   156,   156,   156,   156,
   156,   157,   157,   157,   157,   157,   157
};

static const short yyr2[] = {     0,
     0,     2,     1,     1,     1,     1,     4,     4,     0,     2,
     4,     0,     7,     1,     3,     1,     3,     2,     0,     7,
     0,     1,     0,     2,     6,     0,     8,     0,     2,     1,
     3,     0,     2,     1,     3,     1,     0,     2,     1,     1,
     1,     1,     1,     1,     1,     1,     4,     0,     2,     0,
     6,     3,     0,     2,     2,     4,     0,     2,     4,     4,
     0,     2,     0,     6,     3,     0,     2,     2,     0,     5,
     0,     6,     0,     6,     0,     1,     0,     2,     1,     1,
     1,     4,     0,     2,     2,     4,     0,     2,     0,     6,
     4,     1,     3,     4,     0,     2,     0,     0,    11,     0,
     0,    11,     0,     8,     1,     1,     3,     4,     0,     2,
     4,     0,     1,     0,     1,     0,     6,     0,     2,     0,
    11,     0,     2,     1,     2,     1,     3,     8,     3,     6,
     8,     0,     2,     0,     2,     1,     3,     3,     3,     1,
     9,     8,     0,     2,     3,     3,     3,     3,     3,     3,
     1,     1,     3,     3,     3,     3,     3,     4,     1,     3,
     3,     3,     6,     1,     1,     3,     3,     3,     5,     5,
     6,     1,     1,     1,     1,     1,     4
};

static const short yydefact[] = {     1,
    21,     0,     0,    22,     0,     2,     3,     4,     5,     0,
     6,     0,     9,    26,     0,     0,     0,    28,    19,     7,
     0,     8,    10,     0,    32,     0,     0,    30,    29,     0,
     0,    23,    14,    12,     0,     0,    36,    33,    34,    37,
     0,     0,     0,    11,    31,     0,     0,     0,    20,    24,
     0,     0,    16,    15,    35,     0,     0,     0,    69,     0,
     0,    75,    75,    27,    38,    39,    40,    41,    42,    43,
    44,    45,    46,     0,    18,     0,     0,    48,    57,    61,
     0,   109,   116,    76,    71,    73,     0,    13,    17,     0,
     0,     0,    77,   112,     0,     0,     0,     0,     0,     0,
    47,    49,     0,    56,    58,     0,     0,    60,    62,     0,
   113,   108,   110,   114,   118,    77,    77,     0,    50,     0,
     0,    63,     0,     0,     0,     0,    70,    78,    79,    80,
    81,   115,     0,     0,     0,     0,    25,     0,    52,     0,
     0,    65,    83,    87,    95,     0,     0,   117,   119,    72,
    74,    53,    59,    66,     0,     0,     0,   111,     0,     0,
     0,   172,   173,   174,     0,   175,   176,   159,   152,    82,
     0,    84,     0,     0,   151,     0,     0,   164,   165,    89,
    86,    88,     0,     0,     0,    94,    96,     0,     0,    51,
    54,     0,    64,    67,     0,     0,     0,     0,     0,    85,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    55,    68,
     0,     0,     0,     0,   153,   145,   146,   147,   148,   150,
   149,   154,   155,   156,   157,   167,   168,   161,   162,     0,
   160,   166,     0,     0,   105,     0,     0,     0,     0,     0,
     0,   177,   158,     0,    91,   159,     0,    92,   100,    97,
   103,   120,   169,   170,     0,     0,     0,     0,     0,     0,
     0,   122,   171,   163,    93,    90,     0,     0,     0,   106,
     0,     0,   101,    98,     0,     0,     0,     0,   126,   123,
   124,   132,     0,     0,   107,   104,     0,     0,   125,     0,
   134,     0,     0,   129,   127,   121,   133,     0,     0,     0,
     0,     0,   136,     0,   135,   102,    99,     0,     0,     0,
     0,     0,     0,   140,     0,     0,   130,     0,     0,   139,
     0,     0,   138,   137,     0,     0,     0,     0,     0,   131,
   128,   144,     0,     0,     0,     0,     0,     0,     0,     0,
   142,     0,   141,     0,     0
};

static const short yydefgoto[] = {     1,
     6,     7,     8,    17,    23,    42,    35,    52,    53,     9,
    26,    10,    41,    50,    11,    18,    25,    29,    31,    38,
    39,    47,    65,    66,    90,   102,   138,   160,   191,    67,
    91,   105,    68,    92,   109,   141,   161,   194,    69,    81,
    70,    96,    71,    97,    85,   110,   128,   129,   155,   172,
   130,   156,   182,   214,   257,   131,   157,   187,   270,   294,
   269,   293,   271,   246,   279,    72,    94,   113,   114,   133,
    73,    95,   134,   149,   272,   282,   290,   291,   300,   301,
   307,   315,   326,   338,   173,   174,   175,   176,   177,   178,
   179
};

static const short yypact[] = {-32768,
   169,     0,   -44,-32768,   -17,-32768,-32768,-32768,-32768,    90,
-32768,    15,-32768,-32768,    57,    17,     9,   116,-32768,-32768,
    51,-32768,-32768,   115,   154,   119,    76,-32768,   113,   136,
   137,-32768,-32768,-32768,    16,   149,-32768,   138,-32768,-32768,
    -4,   150,   151,-32768,-32768,   136,    -2,   153,-32768,-32768,
   155,    20,-32768,-32768,-32768,   143,   145,   146,-32768,   147,
   160,   161,   161,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   144,-32768,   152,   150,-32768,-32768,-32768,
   156,-32768,-32768,-32768,-32768,-32768,   163,-32768,-32768,    -6,
    37,    -5,-32768,    27,   157,   158,   162,   148,   165,   166,
-32768,-32768,   164,-32768,-32768,   170,   171,-32768,-32768,    -9,
-32768,-32768,-32768,   196,-32768,-32768,-32768,   168,-32768,   172,
   174,-32768,   173,   175,   176,   177,-32768,-32768,-32768,-32768,
-32768,-32768,   179,    -7,    -1,     8,-32768,   178,-32768,   180,
   181,-32768,-32768,-32768,-32768,   183,   184,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,    43,    38,    40,-32768,   182,    53,
    59,-32768,-32768,-32768,   101,-32768,-32768,    39,-32768,-32768,
    60,-32768,   185,    95,-32768,   186,   187,-32768,-32768,   167,
-32768,-32768,   190,   191,   192,-32768,-32768,   194,   188,-32768,
-32768,   189,-32768,-32768,   200,   195,    60,   201,    88,-32768,
    60,    60,    60,    60,    60,    60,    60,    60,    60,    60,
   132,    81,    60,   193,   208,   208,   208,   202,-32768,-32768,
   107,   197,   100,   198,-32768,   133,   133,   133,   133,   133,
   133,    66,    66,-32768,-32768,-32768,-32768,-32768,-32768,   199,
-32768,-32768,   127,    97,-32768,   204,   205,   206,   207,   159,
   209,-32768,-32768,   211,-32768,   210,    19,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   203,   212,    97,   214,    97,    97,
    97,   233,-32768,-32768,-32768,-32768,   215,   216,    56,-32768,
    36,   220,-32768,-32768,    97,   222,   217,   218,-32768,    36,
-32768,-32768,    97,    60,-32768,-32768,   221,   225,-32768,   224,
-32768,   223,   104,    71,   226,-32768,   -42,   227,   228,   240,
   241,   242,-32768,    18,-32768,-32768,-32768,   229,   231,   232,
   230,   234,   235,-32768,   236,   237,-32768,   244,   248,-32768,
   250,   253,-32768,-32768,   238,   243,   245,   257,   246,-32768,
-32768,-32768,   249,   258,   260,   251,   261,   265,   252,   267,
-32768,   254,-32768,   272,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   247,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   269,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   255,    74,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   -24,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -69,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,  -171,  -224,-32768,-32768,-32768,
    10
};


#define	YYLAST		324


static const short yytable[] = {   199,
    99,   106,   313,    12,    48,   124,   125,   126,    56,    57,
    58,    59,    13,   124,   125,   126,   147,    60,   321,   258,
    61,   314,   124,   125,   126,   223,    62,    63,    14,   226,
   227,   228,   229,   230,   231,   232,   233,   234,   235,   100,
   107,   243,   275,   322,   277,   278,   280,   111,   127,   323,
   148,   101,   108,    49,    21,    64,   150,   183,   184,    43,
   295,   287,    16,   324,   325,   151,    22,   288,   302,   162,
   163,    44,    20,   164,   165,   166,   167,    76,   267,    77,
   268,   289,   103,   180,   112,   185,   162,   163,   168,   169,
   164,   165,   166,   167,   104,   181,    15,   186,   189,   197,
   170,   198,    19,   171,   192,   168,   169,   162,   163,    27,
   190,   164,   240,   166,   167,   285,   193,   286,   209,   210,
   171,    33,   303,   162,   163,    24,   241,   164,   165,   166,
   167,   310,    34,   311,   201,   202,   203,   204,   207,   208,
   209,   210,   256,   205,   206,   207,   208,   209,   210,   225,
   207,   208,   209,   210,   207,   208,   209,   210,   236,   237,
    28,   253,    30,   195,   196,   309,   238,   239,   354,   250,
   251,     2,    36,     3,     4,    32,     5,   207,   208,   209,
   210,    37,   255,   207,   208,   209,   210,   263,   264,   135,
   136,   247,   248,    40,    45,    51,    54,    46,    74,    78,
    75,    79,    80,    82,    87,    83,    84,    88,    98,   118,
   119,   120,    93,   115,   116,   122,   123,   132,   117,   140,
   299,   242,   121,   137,   146,   213,     0,   139,   142,   159,
     0,   143,   144,   145,   152,   153,     0,   154,   158,   218,
   200,   222,   188,   219,   220,   221,   224,   249,   211,   212,
   215,   216,   217,   244,   245,   265,   266,   281,   254,     0,
     0,   252,   196,   259,   260,   261,   304,   273,   262,   276,
   305,   355,   198,   274,   283,   284,   292,   296,     0,   297,
   298,   306,   316,   317,   308,   318,   319,   320,   312,   335,
   327,   328,   329,   336,   330,   337,   331,   332,   339,   340,
   333,   334,   343,   346,   341,   347,   349,   342,   344,   345,
   350,   348,   352,   351,    55,   353,     0,    86,     0,     0,
     0,     0,     0,    89
};

static const short yycheck[] = {   171,
     7,     7,    45,     4,     9,    15,    16,    17,    11,    12,
    13,    14,    57,    15,    16,    17,    24,    20,     1,   244,
    23,    64,    15,    16,    17,   197,    29,    30,    46,   201,
   202,   203,   204,   205,   206,   207,   208,   209,   210,    46,
    46,   213,   267,    26,   269,   270,   271,    21,    58,    32,
    58,    58,    58,    58,    46,    58,    58,    18,    19,    44,
   285,    26,    48,    46,    47,    58,    58,    32,   293,    27,
    28,    56,    56,    31,    32,    33,    34,    58,    60,    60,
    62,    46,    46,    46,    58,    46,    27,    28,    46,    47,
    31,    32,    33,    34,    58,    58,     7,    58,    46,    61,
    58,    63,    46,    61,    46,    46,    47,    27,    28,    59,
    58,    31,    32,    33,    34,    60,    58,    62,    53,    54,
    61,    46,   294,    27,    28,    10,    46,    31,    32,    33,
    34,    61,    57,    63,    40,    41,    42,    43,    51,    52,
    53,    54,    46,    49,    50,    51,    52,    53,    54,    62,
    51,    52,    53,    54,    51,    52,    53,    54,    27,    28,
    46,    62,     9,    63,    64,    62,    35,    36,     0,    63,
    64,     3,    60,     5,     6,    57,     8,    51,    52,    53,
    54,    46,    56,    51,    52,    53,    54,    29,    30,   116,
   117,   216,   217,    57,    46,    46,    46,    60,    46,    57,
    46,    57,    57,    57,    61,    46,    46,    56,    46,    62,
    46,    46,    57,    57,    57,    46,    46,    22,    57,    46,
   290,   212,    59,    56,    46,    59,    -1,    56,    56,    46,
    -1,    57,    57,    57,    57,    56,    -1,    57,    56,    46,
    56,    47,    61,    56,    56,    46,    46,    46,    63,    63,
    61,    61,    61,    61,    47,    47,    46,    25,    61,    -1,
    -1,    65,    64,    60,    60,    60,    46,    65,    62,    56,
    46,     0,    63,    62,    60,    60,    57,    56,    -1,    63,
    63,    58,    56,    56,    62,    46,    46,    46,    63,    46,
    62,    61,    61,    46,    65,    46,    63,    63,    46,    62,
    65,    65,    46,    46,    62,    46,    46,    63,    63,    61,
    46,    61,    46,    62,    46,    62,    -1,    63,    -1,    -1,
    -1,    -1,    -1,    77
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

case 11:
#line 116 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ declare_base_type( info, yyvsp[-3].string, yyvsp[-1].string ); ;
    break;}
case 12:
#line 118 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ declare_base_type_structure( info, yyvsp[-2].string ); ;
    break;}
case 14:
#line 122 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.string = yyvsp[0].string;
    break;}
case 15:
#line 123 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.string = yyvsp[-2].string + "::" + yyvsp[0].string;
    break;}
case 18:
#line 131 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ base_type_structure_element( info, yyvsp[-1].string, yyvsp[0].string ); ;
    break;}
case 19:
#line 136 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ start_provider_type_declaration( info, yyvsp[-2].u.boolean, yyvsp[0].string ); ;
    break;}
case 21:
#line 139 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ yyval.u.boolean = false; ;
    break;}
case 22:
#line 140 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ yyval.u.boolean = true; ;
    break;}
case 25:
#line 147 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ add_provided_result_type( info, yyvsp[-4].string, yyvsp[-2].string ); ;
    break;}
case 26:
#line 151 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ start_node_declaration( info, yyvsp[0].string ); ;
    break;}
case 30:
#line 158 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_extends( info, yyvsp[0].string ); ;
    break;}
case 31:
#line 159 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_extends( info, yyvsp[0].string ); ;
    break;}
case 36:
#line 168 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_provides( info, yyvsp[0].string ); ;
    break;}
case 50:
#line 190 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_property_type( info, yyvsp[0].string ); ;
    break;}
case 52:
#line 193 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_declare_property( info, yyvsp[-2].string, yyvsp[-1].string ); ;
    break;}
case 55:
#line 199 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_declare_property( info, yyvsp[-1].string ); ;
    break;}
case 59:
#line 209 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_declare_alias( info, yyvsp[-3].string, yyvsp[-1].string ); ;
    break;}
case 63:
#line 218 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_operand_type( info, yyvsp[0].string ); ;
    break;}
case 65:
#line 221 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_declare_operand( info, yyvsp[-2].string, yyvsp[-1].string ); ;
    break;}
case 68:
#line 227 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_declare_operand( info, yyvsp[-1].string ); ;
    break;}
case 69:
#line 231 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_common_declaration( info ); ;
    break;}
case 71:
#line 236 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_first_declaration( info, yyvsp[0].string ); ;
    break;}
case 73:
#line 241 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_last_declaration( info, yyvsp[0].string ); ;
    break;}
case 75:
#line 244 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.string = "";;
    break;}
case 76:
#line 245 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.string = yyvsp[0].string;;
    break;}
case 85:
#line 262 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_solve_constraint( info, yyvsp[-1].u.exp ); ;
    break;}
case 89:
#line 272 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_solver( info, yyvsp[0].string ); ;
    break;}
case 90:
#line 274 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_finish_solver( info ); ;
    break;}
case 91:
#line 275 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_solve_expression( info,yyvsp[-3].string,yyvsp[-1].u.exp ); ;
    break;}
case 92:
#line 279 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_solver_parameter( info ); ;
    break;}
case 93:
#line 281 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_solver_parameter( info ); ;
    break;}
case 97:
#line 291 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_action( info, "default", yyvsp[-1].u.scalar ); ;
    break;}
case 98:
#line 293 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_action_parameter_ref( info ); ;
    break;}
case 99:
#line 295 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_action_parameter_exp( info, yyvsp[-2].u.exp );
	node_finish_action( info ); ;
    break;}
case 100:
#line 298 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_action( info, "push", yyvsp[-1].u.scalar ); ;
    break;}
case 101:
#line 300 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_action_parameter_ref( info ); ;
    break;}
case 102:
#line 302 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_action_parameter_ref( info ); // store second parameter
	node_finish_action( info ); ;
    break;}
case 103:
#line 305 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_action( info, yyvsp[-3].string, yyvsp[-1].u.scalar ); ;
    break;}
case 104:
#line 307 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_finish_action( info ); ;
    break;}
case 107:
#line 314 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_action_parameter_ref( info ); ;
    break;}
case 111:
#line 325 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_contains( info, yyvsp[-3].u.boolean, yyvsp[-2].u.boolean, yyvsp[-1].string ); ;
    break;}
case 112:
#line 327 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.boolean = false; ;
    break;}
case 113:
#line 328 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.boolean = true; ;
    break;}
case 114:
#line 330 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.boolean = false; ;
    break;}
case 115:
#line 331 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.boolean = true; ;
    break;}
case 116:
#line 335 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_provide( info, yyvsp[0].string ); ;
    break;}
case 120:
#line 343 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_result_code( info, yyvsp[-4].string, yyvsp[-2].string, yyvsp[-1].string ); ;
    break;}
case 126:
#line 355 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_result_essential_prop( info, yyvsp[0].string ); ;
    break;}
case 127:
#line 357 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_result_essential_child_result( info, yyvsp[0].string ); ;
    break;}
case 128:
#line 359 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_result_essential_child_result( info, yyvsp[-5].string, yyvsp[-3].string, yyvsp[-1].string ); ;
    break;}
case 129:
#line 361 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_result_essential_result( info, yyvsp[0].string ); ;
    break;}
case 130:
#line 363 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_result_essential_result( info, "", yyvsp[-3].string, yyvsp[-1].string ); ;
    break;}
case 131:
#line 365 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_result_essential_result( info, yyvsp[-5].string, yyvsp[-3].string, yyvsp[-1].string ); ;
    break;}
case 132:
#line 368 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ start_code_block(info); ;
    break;}
case 133:
#line 368 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ finish_code_block(info); ;
    break;}
case 137:
#line 375 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ /*insert here*/ continue_code_mode(info); ;
    break;}
case 138:
#line 376 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ continue_code_mode(info); ;
    break;}
case 139:
#line 377 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ continue_code_mode(info); ;
    break;}
case 140:
#line 380 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ res_ref_property(info,yyvsp[0].string); ;
    break;}
case 141:
#line 383 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ res_ref_child(info,yyvsp[-6].string,yyvsp[-4].string,yyvsp[-2].string,yyvsp[-1].string); ;
    break;}
case 142:
#line 386 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ res_ref_this(info,yyvsp[-5].string,yyvsp[-4].string,yyvsp[-2].string,yyvsp[-1].string); ;
    break;}
case 143:
#line 388 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ yyval.string = ""; ;
    break;}
case 144:
#line 389 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ yyval.string = yyvsp[-1].string; ;
    break;}
case 145:
#line 392 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = bool_expr(yyvsp[-2].u.exp,"==",yyvsp[0].u.exp);;
    break;}
case 146:
#line 393 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = bool_expr(yyvsp[-2].u.exp,"!=",yyvsp[0].u.exp);;
    break;}
case 147:
#line 394 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = bool_expr(yyvsp[-2].u.exp,">=",yyvsp[0].u.exp);;
    break;}
case 148:
#line 395 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = bool_expr(yyvsp[-2].u.exp,"<=",yyvsp[0].u.exp);;
    break;}
case 149:
#line 396 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = bool_expr(yyvsp[-2].u.exp,">",yyvsp[0].u.exp);;
    break;}
case 150:
#line 397 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = bool_expr(yyvsp[-2].u.exp,"<",yyvsp[0].u.exp);;
    break;}
case 151:
#line 400 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr_from_ref(info);;
    break;}
case 152:
#line 401 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr_scalar(yyvsp[0].u.scalar);;
    break;}
case 153:
#line 402 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr(yyvsp[-1].u.exp);;
    break;}
case 154:
#line 403 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr(yyvsp[-2].u.exp,"+",yyvsp[0].u.exp);;
    break;}
case 155:
#line 404 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr(yyvsp[-2].u.exp,"-",yyvsp[0].u.exp);;
    break;}
case 156:
#line 405 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr(yyvsp[-2].u.exp,"*",yyvsp[0].u.exp);;
    break;}
case 157:
#line 406 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr(yyvsp[-2].u.exp,"/",yyvsp[0].u.exp);;
    break;}
case 158:
#line 407 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr_function(yyvsp[-3].string,yyvsp[-1].u.exp);;
    break;}
case 159:
#line 411 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_prop_or_op(info, yyvsp[0].string); ;
    break;}
case 160:
#line 413 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_prop(info,yyvsp[0].string); ;
    break;}
case 161:
#line 415 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_start_param(info); ;
    break;}
case 162:
#line 416 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_end_param(info); ;
    break;}
case 163:
#line 420 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_provider_type(info,yyvsp[-5].string,yyvsp[-3].string,yyvsp[-1].string); ;
    break;}
case 167:
#line 428 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_local_prev(info); ;
    break;}
case 168:
#line 429 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_local_next(info); ;
    break;}
case 169:
#line 431 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_local_child_first(info,yyvsp[-2].string); ;
    break;}
case 170:
#line 433 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_local_child_last(info,yyvsp[-2].string); ;
    break;}
case 171:
#line 435 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_local_child(info,yyvsp[-3].string,yyvsp[-1].u.scalar); ;
    break;}
case 172:
#line 438 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_prev(info); ;
    break;}
case 173:
#line 439 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_next(info); ;
    break;}
case 174:
#line 440 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_parent(info); ;
    break;}
case 175:
#line 441 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_first_child(info); ;
    break;}
case 176:
#line 442 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_last_child(info); ;
    break;}
case 177:
#line 443 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_child(info, yyvsp[-1].u.scalar); ;
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
#line 446 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"

  int parse_afd( AFD_Root *afd, message::Message_Consultant *c, 
		 std::string filename )
  {
    afd_info info(afd,c);
    info.set_max_old_positions(MAX_OLD_POSITIONS);
     
    info.open_file( filename );
    int ret = yyparse( static_cast<void*>(&info) );
    info.close_file();
      
    return ret;
  }
} // close namespace afd

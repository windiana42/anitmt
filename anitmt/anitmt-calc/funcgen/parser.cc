
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



#define	YYFINAL		347
#define	YYFLAG		-32768
#define	YYNTBASE	66

#define YYTRANSLATE(x) ((unsigned)(x) <= 303 ? yytranslate[x] : 158)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    59,
    60,    51,    49,    58,    50,    61,    52,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    54,    65,
    57,    64,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    62,     2,    63,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    55,     2,    56,     2,     2,     2,     2,     2,
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
    47,    48,    53
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     6,     8,    10,    12,    17,    22,    23,
    26,    31,    32,    40,    42,    46,    48,    52,    55,    56,
    64,    65,    67,    68,    71,    78,    79,    88,    89,    92,
    94,    98,    99,   102,   104,   108,   110,   111,   114,   116,
   118,   120,   122,   124,   126,   128,   130,   135,   136,   139,
   140,   147,   148,   151,   154,   159,   160,   163,   168,   173,
   174,   177,   178,   185,   186,   189,   192,   193,   199,   200,
   207,   208,   215,   216,   218,   219,   222,   224,   226,   228,
   233,   234,   237,   240,   245,   246,   249,   250,   257,   262,
   264,   268,   273,   274,   277,   278,   279,   291,   292,   293,
   305,   306,   315,   317,   319,   323,   328,   329,   332,   337,
   338,   340,   341,   343,   344,   351,   352,   355,   356,   368,
   369,   372,   374,   377,   379,   383,   392,   396,   403,   412,
   413,   416,   417,   420,   422,   426,   430,   434,   436,   445,
   453,   454,   457,   461,   465,   469,   473,   477,   481,   483,
   485,   489,   493,   497,   501,   505,   510,   512,   516,   520,
   524,   531,   533,   535,   539,   543,   547,   553,   559,   566,
   568,   570,   572,   574,   576
};

static const short yyrhs[] = {    -1,
    66,    67,     0,    68,     0,    69,     0,    76,     0,    81,
     0,     3,     4,    48,    54,     0,     5,    55,    70,    56,
     0,     0,    70,    71,     0,    46,    57,    73,    54,     0,
     0,    46,    57,    55,    72,    74,    56,    54,     0,    46,
     0,    73,    44,    46,     0,    75,     0,    74,    58,    75,
     0,    46,    46,     0,     0,    78,     7,    46,    77,    55,
    79,    56,     0,     0,     6,     0,     0,    79,    80,     0,
     9,    46,    59,    46,    60,    54,     0,     0,     8,    46,
    82,    83,    85,    55,    88,    56,     0,     0,    10,    84,
     0,    46,     0,    84,    58,    46,     0,     0,     9,    86,
     0,    87,     0,    86,    58,    87,     0,    46,     0,     0,
    88,    89,     0,    90,     0,    96,     0,    99,     0,   105,
     0,   107,     0,   109,     0,   132,     0,   137,     0,    11,
    55,    91,    56,     0,     0,    91,    92,     0,     0,     7,
    46,    93,    55,    94,    56,     0,     0,    94,    95,     0,
    46,    54,     0,    12,    55,    97,    56,     0,     0,    97,
    98,     0,    46,    57,    46,    54,     0,    13,    55,   100,
    56,     0,     0,   100,   101,     0,     0,     7,    46,   102,
    55,   103,    56,     0,     0,   103,   104,     0,    46,    54,
     0,     0,    14,   106,    55,   112,    56,     0,     0,    29,
   111,   108,    55,   112,    56,     0,     0,    30,   111,   110,
    55,   112,    56,     0,     0,    46,     0,     0,   112,   113,
     0,   114,     0,   117,     0,   122,     0,    15,    55,   115,
    56,     0,     0,   115,   116,     0,   151,    54,     0,    16,
    55,   118,    56,     0,     0,   118,   119,     0,     0,    46,
   120,    59,   121,    60,    54,     0,    46,    57,   152,    54,
     0,   153,     0,   121,    58,   153,     0,    17,    55,   123,
    56,     0,     0,   123,   124,     0,     0,     0,    19,    59,
   130,    58,   125,   153,    58,   126,   152,    60,    54,     0,
     0,     0,    18,    59,   130,    58,   127,   153,    58,   128,
   153,    60,    54,     0,     0,    46,    59,   130,    58,   129,
   131,    60,    54,     0,    47,     0,   153,     0,   131,    58,
   153,     0,    20,    55,   133,    56,     0,     0,   133,   134,
     0,   135,   136,    46,    54,     0,     0,    21,     0,     0,
    22,     0,     0,    23,    46,   138,    55,   139,    56,     0,
     0,   139,   140,     0,     0,    24,    46,    59,    46,    46,
    60,   141,   142,    55,   145,    56,     0,     0,    25,   143,
     0,   144,     0,   143,   144,     0,    46,     0,    32,    61,
    46,     0,    32,    61,    46,    61,    46,    59,    46,    60,
     0,    26,    61,    46,     0,    26,    61,    46,    59,    46,
    60,     0,    26,    61,    46,    61,    46,    59,    46,    60,
     0,     0,   146,   147,     0,     0,   147,   148,     0,    45,
     0,    62,   149,    63,     0,    62,    47,    63,     0,    62,
     1,    63,     0,    46,     0,    32,    61,    46,    61,    46,
    59,    46,    60,     0,    26,    61,   150,    46,    59,    46,
    60,     0,     0,    46,    61,     0,   152,    40,   152,     0,
   152,    41,   152,     0,   152,    42,   152,     0,   152,    43,
   152,     0,   152,    64,   152,     0,   152,    65,   152,     0,
   153,     0,    47,     0,    59,   152,    60,     0,   152,    49,
   152,     0,   152,    50,   152,     0,   152,    51,   152,     0,
   152,    52,   152,     0,    46,    59,   152,    60,     0,    46,
     0,   155,    61,    46,     0,   154,    61,    35,     0,   154,
    61,    36,     0,    46,    61,    46,    59,    46,    60,     0,
   156,     0,   157,     0,   155,    61,   157,     0,   154,    61,
    27,     0,   154,    61,    28,     0,    32,    61,    46,    61,
    29,     0,    32,    61,    46,    61,    30,     0,    32,    61,
    46,    62,    47,    63,     0,    27,     0,    28,     0,    31,
     0,    33,     0,    34,     0,    32,    62,    47,    63,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    94,    96,    98,   100,   101,   102,   104,   107,   110,   111,
   113,   116,   118,   120,   122,   124,   126,   128,   133,   136,
   138,   139,   141,   142,   144,   148,   151,   153,   154,   156,
   158,   160,   161,   163,   164,   166,   169,   170,   172,   174,
   175,   176,   177,   178,   179,   180,   182,   185,   186,   187,
   190,   192,   193,   195,   198,   201,   202,   204,   208,   211,
   212,   213,   216,   218,   219,   221,   224,   227,   229,   232,
   234,   237,   239,   240,   242,   243,   245,   247,   248,   250,
   253,   254,   256,   259,   262,   263,   265,   268,   270,   272,
   275,   278,   281,   282,   284,   287,   289,   292,   294,   296,
   299,   301,   304,   307,   308,   312,   315,   316,   318,   322,
   323,   325,   326,   328,   331,   333,   334,   336,   339,   341,
   342,   344,   346,   348,   351,   353,   355,   357,   359,   362,
   363,   365,   366,   368,   370,   371,   372,   374,   376,   378,
   381,   382,   384,   386,   387,   388,   389,   390,   392,   394,
   395,   396,   397,   398,   399,   400,   402,   406,   408,   409,
   411,   415,   417,   418,   420,   422,   423,   425,   427,   430,
   432,   433,   434,   435,   436
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
"TAFD_QSTRING","'+'","'-'","'*'","'/'","UMINUS","';'","'{'","'}'","'='","','",
"'('","')'","'.'","'['","']'","'>'","'<'","statements","statement","include_declaration",
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
    92,    94,    94,    95,    96,    97,    97,    98,    99,   100,
   100,   102,   101,   103,   103,   104,   106,   105,   108,   107,
   110,   109,   111,   111,   112,   112,   113,   113,   113,   114,
   115,   115,   116,   117,   118,   118,   120,   119,   119,   121,
   121,   122,   123,   123,   125,   126,   124,   127,   128,   124,
   129,   124,   130,   131,   131,   132,   133,   133,   134,   135,
   135,   136,   136,   138,   137,   139,   139,   141,   140,   142,
   142,   143,   143,   144,   144,   144,   144,   144,   144,   146,
   145,   147,   147,   148,   148,   148,   148,   149,   149,   149,
   150,   150,   151,   151,   151,   151,   151,   151,   152,   152,
   152,   152,   152,   152,   152,   152,   153,   153,   153,   153,
   154,   155,   155,   155,   156,   156,   156,   156,   156,   157,
   157,   157,   157,   157,   157
};

static const short yyr2[] = {     0,
     0,     2,     1,     1,     1,     1,     4,     4,     0,     2,
     4,     0,     7,     1,     3,     1,     3,     2,     0,     7,
     0,     1,     0,     2,     6,     0,     8,     0,     2,     1,
     3,     0,     2,     1,     3,     1,     0,     2,     1,     1,
     1,     1,     1,     1,     1,     1,     4,     0,     2,     0,
     6,     0,     2,     2,     4,     0,     2,     4,     4,     0,
     2,     0,     6,     0,     2,     2,     0,     5,     0,     6,
     0,     6,     0,     1,     0,     2,     1,     1,     1,     4,
     0,     2,     2,     4,     0,     2,     0,     6,     4,     1,
     3,     4,     0,     2,     0,     0,    11,     0,     0,    11,
     0,     8,     1,     1,     3,     4,     0,     2,     4,     0,
     1,     0,     1,     0,     6,     0,     2,     0,    11,     0,
     2,     1,     2,     1,     3,     8,     3,     6,     8,     0,
     2,     0,     2,     1,     3,     3,     3,     1,     8,     7,
     0,     2,     3,     3,     3,     3,     3,     3,     1,     1,
     3,     3,     3,     3,     3,     4,     1,     3,     3,     3,
     6,     1,     1,     3,     3,     3,     5,     5,     6,     1,
     1,     1,     1,     1,     4
};

static const short yydefact[] = {     1,
    21,     0,     0,    22,     0,     2,     3,     4,     5,     0,
     6,     0,     9,    26,     0,     0,     0,    28,    19,     7,
     0,     8,    10,     0,    32,     0,     0,    30,    29,     0,
     0,    23,    14,    12,     0,     0,    36,    33,    34,    37,
     0,     0,     0,    11,    31,     0,     0,     0,    20,    24,
     0,     0,    16,    15,    35,     0,     0,     0,    67,     0,
     0,    73,    73,    27,    38,    39,    40,    41,    42,    43,
    44,    45,    46,     0,    18,     0,     0,    48,    56,    60,
     0,   107,   114,    74,    69,    71,     0,    13,    17,     0,
     0,     0,    75,   110,     0,     0,     0,     0,     0,    47,
    49,     0,    55,    57,     0,    59,    61,     0,   111,   106,
   108,   112,   116,    75,    75,     0,    50,     0,    62,     0,
     0,     0,    68,    76,    77,    78,    79,   113,     0,     0,
     0,     0,    25,     0,     0,     0,    81,    85,    93,     0,
     0,   115,   117,    70,    72,    52,    58,    64,     0,     0,
     0,   109,     0,     0,     0,   170,   171,   172,     0,   173,
   174,   157,   150,    80,     0,    82,     0,     0,   149,     0,
     0,   162,   163,    87,    84,    86,     0,     0,     0,    92,
    94,     0,     0,    51,    53,     0,    63,    65,     0,     0,
     0,     0,     0,    83,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    54,    66,     0,     0,     0,     0,   151,   143,
   144,   145,   146,   152,   153,   154,   155,   147,   148,   165,
   166,   159,   160,     0,   158,   164,     0,     0,   103,     0,
     0,     0,     0,     0,     0,   175,   156,     0,    89,   157,
     0,    90,    98,    95,   101,   118,   167,   168,     0,     0,
     0,     0,     0,     0,     0,   120,   169,   161,    91,    88,
     0,     0,     0,   104,     0,     0,    99,    96,     0,     0,
     0,     0,   124,   121,   122,   130,     0,     0,   105,   102,
     0,     0,   123,     0,   132,     0,     0,   127,   125,   119,
   131,     0,     0,     0,     0,     0,   134,     0,   133,   100,
    97,     0,     0,     0,     0,     0,     0,   138,     0,     0,
   128,     0,     0,   137,     0,     0,   136,   135,     0,     0,
     0,     0,     0,   129,   126,   142,     0,     0,     0,     0,
     0,     0,   140,     0,   139,     0,     0
};

static const short yydefgoto[] = {     1,
     6,     7,     8,    17,    23,    42,    35,    52,    53,     9,
    26,    10,    41,    50,    11,    18,    25,    29,    31,    38,
    39,    47,    65,    66,    90,   101,   134,   154,   185,    67,
    91,   104,    68,    92,   107,   136,   155,   188,    69,    81,
    70,    96,    71,    97,    85,   108,   124,   125,   149,   166,
   126,   150,   176,   208,   251,   127,   151,   181,   264,   288,
   263,   287,   265,   240,   273,    72,    94,   111,   112,   129,
    73,    95,   130,   143,   266,   276,   284,   285,   294,   295,
   301,   309,   320,   332,   167,   168,   169,   170,   171,   172,
   173
};

static const short yypact[] = {-32768,
   132,     1,   -39,-32768,   -33,-32768,-32768,-32768,-32768,    40,
-32768,    20,-32768,-32768,    38,    50,   -42,    81,-32768,-32768,
    52,-32768,-32768,    51,   109,    78,   -35,-32768,    91,   136,
   108,-32768,-32768,-32768,     5,   147,-32768,   137,-32768,-32768,
    -3,   148,   150,-32768,-32768,   136,    43,   151,-32768,-32768,
   152,    13,-32768,-32768,-32768,   144,   145,   146,-32768,   149,
   156,   157,   157,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   153,-32768,   154,   148,-32768,-32768,-32768,
   155,-32768,-32768,-32768,-32768,-32768,   159,-32768,-32768,    -6,
    21,    -4,-32768,   -11,   160,   161,   162,   163,   165,-32768,
-32768,   164,-32768,-32768,   167,-32768,-32768,    -8,-32768,-32768,
-32768,   184,-32768,-32768,-32768,   166,-32768,   168,-32768,   169,
   170,   171,-32768,-32768,-32768,-32768,-32768,-32768,   172,   -12,
     2,     6,-32768,   173,   175,   176,-32768,-32768,-32768,   178,
   181,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    47,    39,
    46,-32768,   174,    54,    59,-32768,-32768,-32768,    97,-32768,
-32768,    37,-32768,-32768,    80,-32768,   180,    79,-32768,   177,
   179,-32768,-32768,   182,-32768,-32768,   183,   185,   186,-32768,
-32768,   189,   187,-32768,-32768,   192,-32768,-32768,   190,   196,
    80,   191,   101,-32768,    80,    80,    80,    80,    80,    80,
    80,    80,    80,    80,    89,    55,    80,   188,   201,   201,
   201,   203,-32768,-32768,   122,   193,   105,   194,-32768,   129,
   129,   129,   129,   134,   134,-32768,-32768,   129,   129,-32768,
-32768,-32768,-32768,   195,-32768,-32768,   121,   114,-32768,   197,
   200,   202,   199,   158,   204,-32768,-32768,   206,-32768,   205,
    76,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   198,   207,
   114,   208,   114,   114,   114,   225,-32768,-32768,-32768,-32768,
   210,   211,   104,-32768,    44,   209,-32768,-32768,   114,   216,
   212,   213,-32768,    44,-32768,-32768,   114,    80,-32768,-32768,
   217,   219,-32768,   215,-32768,   218,   117,   115,   214,-32768,
   -21,   222,   223,   226,   233,   234,-32768,    14,-32768,-32768,
-32768,   221,   224,   227,   228,   229,   231,-32768,   230,   232,
-32768,   236,   238,-32768,   239,   241,-32768,-32768,   237,   240,
   235,   242,   243,-32768,-32768,-32768,   244,   248,   252,   246,
   247,   253,-32768,   249,-32768,   254,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   130,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   255,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,   245,    75,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   -19,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -75,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,  -165,  -236,-32768,-32768,-32768,
    16
};


#define	YYLAST		309


static const short yytable[] = {   193,
    99,   252,   105,    21,    12,    48,   120,   121,   122,   109,
    33,   141,    14,    22,   315,    13,   120,   121,   122,    34,
   120,   121,   122,   307,   269,   217,   271,   272,   274,   220,
   221,   222,   223,   224,   225,   226,   227,   228,   229,   316,
   308,   237,   289,   142,   110,   317,    15,   123,    43,   100,
   296,   106,    49,    56,    57,    58,    59,   144,    44,   318,
   319,   145,    60,   177,   178,    61,   102,    16,    76,   281,
    77,    62,    63,   156,   157,   282,   103,   158,   159,   160,
   161,   156,   157,    19,   174,   158,   234,   160,   161,   283,
    24,   179,   162,   163,   175,   191,    28,   192,    64,   183,
   235,   180,   164,    20,   186,   165,   156,   157,    27,   184,
   158,   159,   160,   161,   187,   230,   231,    30,   195,   196,
   197,   198,   297,   232,   233,   162,   163,   199,   200,   201,
   202,   346,    32,   261,     2,   262,     3,     4,   165,     5,
   156,   157,   203,   204,   158,   159,   160,   161,    36,   199,
   200,   201,   202,   199,   200,   201,   202,   189,   190,   250,
   219,   279,    40,   280,   247,   199,   200,   201,   202,   199,
   200,   201,   202,   304,   249,   305,   303,   199,   200,   201,
   202,    37,   244,   245,   201,   202,   257,   258,   131,   132,
   241,   242,    45,    51,    46,    54,    74,    75,    78,    79,
    80,    83,    84,    82,    98,   128,    89,    88,   293,    93,
   117,    87,   119,   135,   113,   114,   115,   140,     0,   133,
   118,   236,   116,   137,   138,   139,   153,   146,   147,     0,
   148,   152,   182,   194,   212,   215,   218,   205,   207,   206,
   213,   209,   216,   210,   211,   214,   238,   239,   243,   275,
   259,   260,   248,   347,   253,   246,   190,   254,   256,   255,
   267,   270,   298,   286,   299,   192,   268,   277,   278,   290,
   300,   312,   291,   292,   306,   310,   311,   302,   313,   314,
   321,   329,   322,   330,   331,   323,   333,   337,     0,   325,
   324,   326,   327,   340,   328,   336,   334,   341,   344,   335,
    55,     0,   339,   338,   342,     0,   343,    86,   345
};

static const short yycheck[] = {   165,
     7,   238,     7,    46,     4,     9,    15,    16,    17,    21,
    46,    24,    46,    56,     1,    55,    15,    16,    17,    55,
    15,    16,    17,    45,   261,   191,   263,   264,   265,   195,
   196,   197,   198,   199,   200,   201,   202,   203,   204,    26,
    62,   207,   279,    56,    56,    32,     7,    56,    44,    56,
   287,    56,    56,    11,    12,    13,    14,    56,    54,    46,
    47,    56,    20,    18,    19,    23,    46,    48,    56,    26,
    58,    29,    30,    27,    28,    32,    56,    31,    32,    33,
    34,    27,    28,    46,    46,    31,    32,    33,    34,    46,
    10,    46,    46,    47,    56,    59,    46,    61,    56,    46,
    46,    56,    56,    54,    46,    59,    27,    28,    57,    56,
    31,    32,    33,    34,    56,    27,    28,     9,    40,    41,
    42,    43,   288,    35,    36,    46,    47,    49,    50,    51,
    52,     0,    55,    58,     3,    60,     5,     6,    59,     8,
    27,    28,    64,    65,    31,    32,    33,    34,    58,    49,
    50,    51,    52,    49,    50,    51,    52,    61,    62,    46,
    60,    58,    55,    60,    60,    49,    50,    51,    52,    49,
    50,    51,    52,    59,    54,    61,    60,    49,    50,    51,
    52,    46,    61,    62,    51,    52,    29,    30,   114,   115,
   210,   211,    46,    46,    58,    46,    46,    46,    55,    55,
    55,    46,    46,    55,    46,    22,    77,    54,   284,    55,
    46,    59,    46,    46,    55,    55,    55,    46,    -1,    54,
    57,   206,    60,    55,    55,    55,    46,    55,    54,    -1,
    55,    54,    59,    54,    46,    46,    46,    61,    57,    61,
    54,    59,    47,    59,    59,    54,    59,    47,    46,    25,
    47,    46,    59,     0,    58,    63,    62,    58,    60,    58,
    63,    54,    46,    55,    46,    61,    60,    58,    58,    54,
    56,    46,    61,    61,    61,    54,    54,    60,    46,    46,
    60,    46,    59,    46,    46,    59,    46,    46,    -1,    61,
    63,    61,    63,    46,    63,    61,    60,    46,    46,    60,
    46,    -1,    59,    61,    59,    -1,    60,    63,    60
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
#line 115 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ declare_base_type( info, yyvsp[-3].string, yyvsp[-1].string ); ;
    break;}
case 12:
#line 117 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ declare_base_type_structure( info, yyvsp[-2].string ); ;
    break;}
case 14:
#line 121 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.string = yyvsp[0].string;
    break;}
case 15:
#line 122 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.string = yyvsp[-2].string + "::" + yyvsp[0].string;
    break;}
case 18:
#line 130 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ base_type_structure_element( info, yyvsp[-1].string, yyvsp[0].string ); ;
    break;}
case 19:
#line 135 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ start_provider_type_declaration( info, yyvsp[-2].u.boolean, yyvsp[0].string ); ;
    break;}
case 21:
#line 138 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ yyval.u.boolean = false; ;
    break;}
case 22:
#line 139 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ yyval.u.boolean = true; ;
    break;}
case 25:
#line 146 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ add_provided_result_type( info, yyvsp[-4].string, yyvsp[-2].string ); ;
    break;}
case 26:
#line 150 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ start_node_declaration( info, yyvsp[0].string ); ;
    break;}
case 30:
#line 157 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_extends( info, yyvsp[0].string ); ;
    break;}
case 31:
#line 158 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_extends( info, yyvsp[0].string ); ;
    break;}
case 36:
#line 167 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_provides( info, yyvsp[0].string ); ;
    break;}
case 50:
#line 189 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_property_type( info, yyvsp[0].string ); ;
    break;}
case 54:
#line 196 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_declare_property( info, yyvsp[-1].string ); ;
    break;}
case 58:
#line 206 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_declare_alias( info, yyvsp[-3].string, yyvsp[-1].string ); ;
    break;}
case 62:
#line 215 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_operand_type( info, yyvsp[0].string ); ;
    break;}
case 66:
#line 222 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_declare_operand( info, yyvsp[-1].string ); ;
    break;}
case 67:
#line 226 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_common_declaration( info ); ;
    break;}
case 69:
#line 231 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_first_declaration( info, yyvsp[0].string ); ;
    break;}
case 71:
#line 236 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_last_declaration( info, yyvsp[0].string ); ;
    break;}
case 73:
#line 239 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.string = "";;
    break;}
case 74:
#line 240 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.string = yyvsp[0].string;;
    break;}
case 83:
#line 257 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_solve_constraint( info, yyvsp[-1].u.exp ); ;
    break;}
case 87:
#line 267 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_solver( info, yyvsp[0].string ); ;
    break;}
case 88:
#line 269 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_finish_solver( info ); ;
    break;}
case 89:
#line 270 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_solve_expression( info,yyvsp[-3].string,yyvsp[-1].u.exp ); ;
    break;}
case 90:
#line 274 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_solver_parameter( info ); ;
    break;}
case 91:
#line 276 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_solver_parameter( info ); ;
    break;}
case 95:
#line 286 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_action( info, "default", yyvsp[-1].u.scalar ); ;
    break;}
case 96:
#line 288 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_action_parameter_ref( info ); ;
    break;}
case 97:
#line 290 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_action_parameter_exp( info, yyvsp[-2].u.exp );
	node_finish_action( info ); ;
    break;}
case 98:
#line 293 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_action( info, "push", yyvsp[-1].u.scalar ); ;
    break;}
case 99:
#line 295 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_action_parameter_ref( info ); ;
    break;}
case 100:
#line 297 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_action_parameter_ref( info ); // store second parameter
	node_finish_action( info ); ;
    break;}
case 101:
#line 300 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_action( info, yyvsp[-3].string, yyvsp[-1].u.scalar ); ;
    break;}
case 102:
#line 302 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_finish_action( info ); ;
    break;}
case 105:
#line 309 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_add_action_parameter_ref( info ); ;
    break;}
case 109:
#line 320 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_contains( info, yyvsp[-3].u.boolean, yyvsp[-2].u.boolean, yyvsp[-1].string ); ;
    break;}
case 110:
#line 322 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.boolean = false; ;
    break;}
case 111:
#line 323 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.boolean = true; ;
    break;}
case 112:
#line 325 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.boolean = false; ;
    break;}
case 113:
#line 326 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.boolean = true; ;
    break;}
case 114:
#line 330 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_provide( info, yyvsp[0].string ); ;
    break;}
case 118:
#line 338 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_start_result_code( info, yyvsp[-4].string, yyvsp[-2].string, yyvsp[-1].string ); ;
    break;}
case 124:
#line 350 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_result_essential_prop( info, yyvsp[0].string ); ;
    break;}
case 125:
#line 352 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_result_essential_child_result( info, yyvsp[0].string ); ;
    break;}
case 126:
#line 354 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_result_essential_child_result( info, yyvsp[-5].string, yyvsp[-3].string, yyvsp[-1].string ); ;
    break;}
case 127:
#line 356 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_result_essential_result( info, yyvsp[0].string ); ;
    break;}
case 128:
#line 358 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_result_essential_result( info, "", yyvsp[-3].string, yyvsp[-1].string ); ;
    break;}
case 129:
#line 360 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ node_result_essential_result( info, yyvsp[-5].string, yyvsp[-3].string, yyvsp[-1].string ); ;
    break;}
case 130:
#line 363 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ start_code_block(info); ;
    break;}
case 131:
#line 363 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ finish_code_block(info); ;
    break;}
case 135:
#line 370 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ /*insert here*/ continue_code_mode(info); ;
    break;}
case 136:
#line 371 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ continue_code_mode(info); ;
    break;}
case 137:
#line 372 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ continue_code_mode(info); ;
    break;}
case 138:
#line 375 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ res_ref_property(info,yyvsp[0].string); ;
    break;}
case 139:
#line 377 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ res_ref_child(info,yyvsp[-5].string,yyvsp[-3].string,yyvsp[-1].string); ;
    break;}
case 140:
#line 379 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ res_ref_this(info,yyvsp[-4].string,yyvsp[-3].string,yyvsp[-1].string); ;
    break;}
case 141:
#line 381 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ yyval.string = ""; ;
    break;}
case 142:
#line 382 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ yyval.string = yyvsp[-1].string; ;
    break;}
case 143:
#line 385 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = bool_expr(yyvsp[-2].u.exp,"==",yyvsp[0].u.exp);;
    break;}
case 144:
#line 386 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = bool_expr(yyvsp[-2].u.exp,"!=",yyvsp[0].u.exp);;
    break;}
case 145:
#line 387 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = bool_expr(yyvsp[-2].u.exp,">=",yyvsp[0].u.exp);;
    break;}
case 146:
#line 388 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = bool_expr(yyvsp[-2].u.exp,"<=",yyvsp[0].u.exp);;
    break;}
case 147:
#line 389 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = bool_expr(yyvsp[-2].u.exp,">",yyvsp[0].u.exp);;
    break;}
case 148:
#line 390 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = bool_expr(yyvsp[-2].u.exp,"<",yyvsp[0].u.exp);;
    break;}
case 149:
#line 393 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr_from_ref(info);;
    break;}
case 150:
#line 394 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr_scalar(yyvsp[0].u.scalar);;
    break;}
case 151:
#line 395 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr(yyvsp[-1].u.exp);;
    break;}
case 152:
#line 396 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr(yyvsp[-2].u.exp,"+",yyvsp[0].u.exp);;
    break;}
case 153:
#line 397 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr(yyvsp[-2].u.exp,"-",yyvsp[0].u.exp);;
    break;}
case 154:
#line 398 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr(yyvsp[-2].u.exp,"*",yyvsp[0].u.exp);;
    break;}
case 155:
#line 399 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr(yyvsp[-2].u.exp,"/",yyvsp[0].u.exp);;
    break;}
case 156:
#line 400 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{yyval.u.exp = expr_function(yyvsp[-3].string,yyvsp[-1].u.exp);;
    break;}
case 157:
#line 404 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_prop_or_op(info, yyvsp[0].string); ;
    break;}
case 158:
#line 406 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_prop(info,yyvsp[0].string); ;
    break;}
case 159:
#line 408 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_start_param(info); ;
    break;}
case 160:
#line 409 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_end_param(info); ;
    break;}
case 161:
#line 413 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_provider_type(info,yyvsp[-5].string,yyvsp[-3].string,yyvsp[-1].string); ;
    break;}
case 165:
#line 421 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_local_prev(info); ;
    break;}
case 166:
#line 422 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_local_next(info); ;
    break;}
case 167:
#line 424 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_local_child_first(info,yyvsp[-2].string); ;
    break;}
case 168:
#line 426 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_local_child_last(info,yyvsp[-2].string); ;
    break;}
case 169:
#line 428 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_local_child(info,yyvsp[-3].string,yyvsp[-1].u.scalar); ;
    break;}
case 170:
#line 431 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_prev(info); ;
    break;}
case 171:
#line 432 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_next(info); ;
    break;}
case 172:
#line 433 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_parent(info); ;
    break;}
case 173:
#line 434 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_first_child(info); ;
    break;}
case 174:
#line 435 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
{ ref_node_last_child(info); ;
    break;}
case 175:
#line 436 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"
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
#line 439 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/funcgen/./parser.yy"

  int parse_afd( AFD_Manager *afd, message::Message_Consultant *c, 
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

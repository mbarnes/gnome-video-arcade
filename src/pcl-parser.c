/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     END_OF_FILE = 0,
     FILE_INPUT = 258,
     EVAL_INPUT = 259,
     SINGLE_INPUT = 260,
     KW_AND = 261,
     KW_AS = 262,
     KW_ASSERT = 263,
     KW_BREAK = 264,
     KW_CLASS = 265,
     KW_CONTINUE = 266,
     KW_CUT = 267,
     KW_DEF = 268,
     KW_DEL = 269,
     KW_ELIF = 270,
     KW_ELSE = 271,
     KW_EXCEPT = 272,
     KW_EXEC = 273,
     KW_FINALLY = 274,
     KW_FOR = 275,
     KW_FROM = 276,
     KW_GLOBAL = 277,
     KW_IF = 278,
     KW_IMPORT = 279,
     KW_IN = 280,
     KW_IS = 281,
     KW_IS_NOT = 282,
     KW_NOT = 283,
     KW_NOT_IN = 284,
     KW_OR = 285,
     KW_PASS = 286,
     KW_PREDICATE = 287,
     KW_PRINT = 288,
     KW_RAISE = 289,
     KW_RETURN = 290,
     KW_STATIC = 291,
     KW_TRY = 292,
     KW_WHILE = 293,
     KW_YIELD = 294,
     OP_INPLACE_ADD = 295,
     OP_INPLACE_SUB = 296,
     OP_INPLACE_MUL = 297,
     OP_INPLACE_DIV = 298,
     OP_INPLACE_MOD = 299,
     OP_INPLACE_POW = 300,
     OP_INPLACE_FLD = 301,
     OP_INPLACE_LSH = 302,
     OP_INPLACE_RSH = 303,
     OP_INPLACE_AND = 304,
     OP_INPLACE_XOR = 305,
     OP_INPLACE_OR = 306,
     OP_FLD = 307,
     OP_LSH = 308,
     OP_RSH = 309,
     OP_POW = 310,
     OP_LE = 311,
     OP_GE = 312,
     OP_EQ = 313,
     OP_NE = 314,
     DEDENT = 315,
     INDENT = 316,
     NEWLINE = 317,
     NUMBER = 318,
     NAME = 319,
     STRING = 320
   };
#endif
/* Tokens.  */
#define END_OF_FILE 0
#define FILE_INPUT 258
#define EVAL_INPUT 259
#define SINGLE_INPUT 260
#define KW_AND 261
#define KW_AS 262
#define KW_ASSERT 263
#define KW_BREAK 264
#define KW_CLASS 265
#define KW_CONTINUE 266
#define KW_CUT 267
#define KW_DEF 268
#define KW_DEL 269
#define KW_ELIF 270
#define KW_ELSE 271
#define KW_EXCEPT 272
#define KW_EXEC 273
#define KW_FINALLY 274
#define KW_FOR 275
#define KW_FROM 276
#define KW_GLOBAL 277
#define KW_IF 278
#define KW_IMPORT 279
#define KW_IN 280
#define KW_IS 281
#define KW_IS_NOT 282
#define KW_NOT 283
#define KW_NOT_IN 284
#define KW_OR 285
#define KW_PASS 286
#define KW_PREDICATE 287
#define KW_PRINT 288
#define KW_RAISE 289
#define KW_RETURN 290
#define KW_STATIC 291
#define KW_TRY 292
#define KW_WHILE 293
#define KW_YIELD 294
#define OP_INPLACE_ADD 295
#define OP_INPLACE_SUB 296
#define OP_INPLACE_MUL 297
#define OP_INPLACE_DIV 298
#define OP_INPLACE_MOD 299
#define OP_INPLACE_POW 300
#define OP_INPLACE_FLD 301
#define OP_INPLACE_LSH 302
#define OP_INPLACE_RSH 303
#define OP_INPLACE_AND 304
#define OP_INPLACE_XOR 305
#define OP_INPLACE_OR 306
#define OP_FLD 307
#define OP_LSH 308
#define OP_RSH 309
#define OP_POW 310
#define OP_LE 311
#define OP_GE 312
#define OP_EQ 313
#define OP_NE 314
#define DEDENT 315
#define INDENT 316
#define NEWLINE 317
#define NUMBER 318
#define NAME 319
#define STRING 320




/* Copy the first part of user declarations.  */
#line 19 "pcl-parser.y"

#include "pcl.h"
#include "pcl-node.h"
#include "pcl-opcode.h"

#define YYERROR_VERBOSE 1
#define YYFPRINTF stream_to_debug
#define YYPRINT(stream, type, value)    yyprint (stream, type, value)

/* Bison scans for location symbols ('@n') before preprocessing macros are
 * expanded, so the caller must pass a location symbol to the macro, even
 * though it's almost always '@$'. */
#define NEW_TREE_NODE(type, loc) \
        g_node_new (pcl_node_new ( \
        (type), (loc).first_line, (loc).first_column))
#define NEW_TREE_NODE_FROM_LIST(type, list, loc) \
        adopt_children (NEW_TREE_NODE ((type), (loc)), (list))

/* Lexer Declarations */
extern gint yylex (void);
extern gint yyerror (const gchar *);


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 47 "pcl-parser.y"
{
        gint flag;
        gint oper;
        GList *list;
        GNode *node;
}
/* Line 193 of yacc.c.  */
#line 258 "pcl-parser.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */
#line 252 "pcl-parser.y"

static GNode *adopt_children (GNode *parent, GList *children);
static GNode *adopt_name (GNode *parent, GNode *name);
static void normalize_conjunction (GNode *conjunct);
static GList *process_arguments (GNode *pa, GNode *ka, GNode *epa, GNode *eka);
static GList *process_parameters (GNode *pp, GNode *dp, GNode *epp, GNode *ekp);
static void stream_to_debug (FILE *stream, const gchar *format, ...);
static GNode *validate_target (GNode *target);
static void yyprint (FILE *stream, gint type, YYSTYPE value);


/* Line 216 of yacc.c.  */
#line 293 "pcl-parser.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
    YYLTYPE yyls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  99
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1339

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  120
/* YYNRULES -- Number of rules.  */
#define YYNRULES  338
/* YYNRULES -- Number of states.  */
#define YYNSTATES  623

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   320

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    82,    78,     2,
      67,    68,    71,    79,    70,    80,    84,    81,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    66,    73,
      74,    72,    75,     2,    69,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    85,     2,    86,    77,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    87,    76,    88,    83,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,    10,    13,    17,    22,    23,    29,
      34,    43,    51,    59,    66,    68,    71,    79,    85,    89,
      91,   101,   108,   115,   119,   127,   132,   137,   139,   147,
     152,   157,   159,   165,   168,   171,   177,   181,   183,   187,
     191,   197,   199,   203,   205,   207,   209,   211,   213,   215,
     217,   219,   221,   223,   225,   227,   229,   231,   233,   235,
     239,   241,   246,   250,   252,   254,   256,   258,   260,   262,
     264,   266,   269,   275,   281,   286,   292,   298,   303,   307,
     315,   322,   329,   335,   341,   346,   351,   358,   363,   367,
     369,   372,   376,   382,   387,   390,   392,   394,   396,   402,
     404,   408,   410,   414,   416,   419,   422,   424,   427,   431,
     433,   435,   437,   439,   441,   443,   445,   447,   449,   451,
     453,   457,   459,   463,   465,   469,   471,   475,   479,   481,
     485,   489,   491,   495,   499,   503,   507,   509,   512,   515,
     518,   520,   524,   526,   528,   530,   532,   534,   538,   543,
     550,   559,   561,   562,   564,   565,   567,   568,   570,   572,
     574,   576,   578,   580,   582,   584,   588,   591,   601,   608,
     615,   619,   627,   632,   637,   639,   647,   652,   657,   659,
     665,   668,   671,   677,   681,   683,   687,   691,   697,   708,
     718,   727,   735,   745,   754,   762,   769,   773,   778,   782,
     784,   786,   788,   790,   794,   796,   798,   802,   804,   807,
     809,   811,   813,   815,   817,   819,   821,   823,   825,   827,
     829,   831,   833,   835,   837,   841,   845,   847,   849,   851,
     853,   856,   860,   865,   869,   873,   878,   882,   888,   894,
     899,   905,   911,   916,   920,   925,   931,   936,   941,   944,
     946,   948,   950,   953,   959,   964,   969,   972,   977,   980,
     982,   984,   986,   989,   994,   997,  1000,  1005,  1008,  1011,
    1015,  1019,  1021,  1023,  1025,  1027,  1029,  1031,  1033,  1035,
    1037,  1039,  1041,  1043,  1045,  1048,  1054,  1058,  1060,  1062,
    1066,  1069,  1074,  1081,  1085,  1088,  1093,  1099,  1107,  1112,
    1116,  1118,  1124,  1128,  1132,  1134,  1140,  1144,  1146,  1148,
    1152,  1154,  1161,  1165,  1169,  1171,  1178,  1183,  1186,  1188,
    1192,  1195,  1197,  1201,  1204,  1206,  1210,  1213,  1215,  1219,
    1223,  1229,  1232,  1234,  1238,  1240,  1242,  1243,  1245
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      90,     0,    -1,     3,   106,    -1,     4,   199,    62,    -1,
       5,    62,    -1,     5,   105,    62,    -1,     5,   102,   208,
      62,    -1,    -1,    10,    64,    92,    66,   104,    -1,    67,
     200,   207,    68,    -1,    94,    13,    64,    67,    97,    68,
      66,   104,    -1,    94,    13,    64,    67,    68,    66,   104,
      -1,    13,    64,    67,    97,    68,    66,   104,    -1,    13,
      64,    67,    68,    66,   104,    -1,    95,    -1,    94,    95,
      -1,    69,    96,    67,   143,   207,    68,    62,    -1,    69,
      96,    67,    68,    62,    -1,    69,    96,    62,    -1,   198,
      -1,    98,    70,    99,    70,    71,    64,    70,    55,    64,
      -1,    98,    70,    99,    70,    71,    64,    -1,    98,    70,
      99,    70,    55,    64,    -1,    98,    70,    99,    -1,    98,
      70,    71,    64,    70,    55,    64,    -1,    98,    70,    71,
      64,    -1,    98,    70,    55,    64,    -1,    98,    -1,    99,
      70,    71,    64,    70,    55,    64,    -1,    99,    70,    71,
      64,    -1,    99,    70,    55,    64,    -1,    99,    -1,    71,
      64,    70,    55,    64,    -1,    71,    64,    -1,    55,    64,
      -1,    98,    70,    99,    70,    64,    -1,    99,    70,    64,
      -1,    64,    -1,    98,    70,    64,    -1,    64,    72,   117,
      -1,    99,    70,    64,    72,   117,    -1,   105,    -1,   102,
     208,    62,    -1,   176,    -1,   177,    -1,   179,    -1,   181,
      -1,   183,    -1,   185,    -1,   186,    -1,   103,    -1,   187,
      -1,   188,    -1,   193,    -1,   194,    -1,   195,    -1,   116,
      -1,   197,    -1,   101,    -1,   102,    73,   101,    -1,   199,
      -1,    62,    61,   106,    60,    -1,   102,   208,    62,    -1,
     107,    -1,   110,    -1,   111,    -1,   115,    -1,    91,    -1,
      93,    -1,   146,    -1,   100,    -1,   106,   100,    -1,    23,
     119,    66,   104,   108,    -1,    23,   119,    66,   104,   109,
      -1,    23,   119,    66,   104,    -1,    15,   117,    66,   104,
     108,    -1,    15,   117,    66,   104,   109,    -1,    15,   117,
      66,   104,    -1,    16,    66,   104,    -1,    20,   204,    25,
     199,    66,   104,   109,    -1,    20,   204,    25,   199,    66,
     104,    -1,    37,    66,   104,   113,   109,   114,    -1,    37,
      66,   104,   113,   109,    -1,    37,    66,   104,   113,   114,
      -1,    37,    66,   104,   113,    -1,    37,    66,   104,   114,
      -1,    17,   117,    70,   206,    66,   104,    -1,    17,   117,
      66,   104,    -1,    17,    66,   104,    -1,   112,    -1,   113,
     112,    -1,    19,    66,   104,    -1,    38,   117,    66,   104,
     109,    -1,    38,   117,    66,   104,    -1,    35,   199,    -1,
      35,    -1,   118,    -1,   119,    -1,   118,    23,   119,    16,
     119,    -1,   120,    -1,   119,    30,   120,    -1,   121,    -1,
     120,     6,   121,    -1,   122,    -1,    28,   121,    -1,   125,
     123,    -1,   125,    -1,   124,   125,    -1,   123,   124,   125,
      -1,    74,    -1,    56,    -1,    58,    -1,    59,    -1,    57,
      -1,    75,    -1,    25,    -1,    29,    -1,    26,    -1,    27,
      -1,   126,    -1,   125,    76,   126,    -1,   127,    -1,   126,
      77,   127,    -1,   128,    -1,   127,    78,   128,    -1,   129,
      -1,   128,    53,   129,    -1,   128,    54,   129,    -1,   130,
      -1,   129,    79,   130,    -1,   129,    80,   130,    -1,   131,
      -1,   130,    71,   131,    -1,   130,    81,   131,    -1,   130,
      82,   131,    -1,   130,    52,   131,    -1,   132,    -1,    80,
     131,    -1,    79,   131,    -1,    83,   131,    -1,   133,    -1,
     133,    55,   131,    -1,   140,    -1,   134,    -1,   135,    -1,
     136,    -1,   182,    -1,   133,    84,    64,    -1,   133,    85,
     199,    86,    -1,   133,    85,   137,    66,   138,    86,    -1,
     133,    85,   137,    66,   138,    66,   139,    86,    -1,   117,
      -1,    -1,   117,    -1,    -1,   117,    -1,    -1,    64,    -1,
      63,    -1,    65,    -1,   141,    -1,   142,    -1,   170,    -1,
     175,    -1,   165,    -1,    67,   199,    68,    -1,    67,    68,
      -1,   144,    70,   145,    70,    71,   117,    70,    55,   117,
      -1,   144,    70,   145,    70,    71,   117,    -1,   144,    70,
     145,    70,    55,   117,    -1,   144,    70,   145,    -1,   144,
      70,    71,   117,    70,    55,   117,    -1,   144,    70,    71,
     117,    -1,   144,    70,    55,   117,    -1,   144,    -1,   145,
      70,    71,   117,    70,    55,   117,    -1,   145,    70,    71,
     117,    -1,   145,    70,    55,   117,    -1,   145,    -1,    71,
     117,    70,    55,   117,    -1,    71,   117,    -1,    55,   117,
      -1,   144,    70,   145,    70,   117,    -1,   145,    70,   117,
      -1,   117,    -1,   144,    70,   117,    -1,    64,    72,   117,
      -1,   145,    70,    64,    72,   117,    -1,    94,    32,    64,
      67,    97,    68,    70,   147,    66,   148,    -1,    94,    32,
      64,    67,    68,    70,   147,    66,   148,    -1,    94,    32,
      64,    67,    97,    68,    66,   148,    -1,    94,    32,    64,
      67,    68,    66,   148,    -1,    32,    64,    67,    97,    68,
      70,   147,    66,   148,    -1,    32,    64,    67,    68,    70,
     147,    66,   148,    -1,    32,    64,    67,    97,    68,    66,
     148,    -1,    32,    64,    67,    68,    66,   148,    -1,    64,
      72,   199,    -1,    62,    61,   156,    60,    -1,   150,   208,
      62,    -1,   187,    -1,   193,    -1,   196,    -1,   149,    -1,
     150,    73,   149,    -1,   159,    -1,   161,    -1,   153,   208,
      62,    -1,   151,    -1,   152,   151,    -1,   164,    -1,   176,
      -1,   177,    -1,   179,    -1,   184,    -1,   185,    -1,   186,
      -1,   103,    -1,   187,    -1,   188,    -1,   193,    -1,   194,
      -1,   195,    -1,   196,    -1,   153,    -1,   154,    73,   153,
      -1,   150,   208,    62,    -1,    93,    -1,   146,    -1,   157,
      -1,   155,    -1,   156,   155,    -1,     6,    66,   158,    -1,
      62,    61,   152,    60,    -1,   154,   208,    62,    -1,    30,
      66,   160,    -1,    62,    61,   156,    60,    -1,   150,   208,
      62,    -1,    23,   119,    66,   158,   162,    -1,    23,   119,
      66,   158,   163,    -1,    23,   119,    66,   158,    -1,    15,
     117,    66,   158,   162,    -1,    15,   117,    66,   158,   163,
      -1,    15,   117,    66,   158,    -1,    16,    66,   158,    -1,
      20,   204,    25,   199,    -1,    67,   117,   166,   169,    68,
      -1,    67,   117,   166,    68,    -1,    20,   204,    25,   201,
      -1,    23,   119,    -1,   166,    -1,   167,    -1,   168,    -1,
     169,   168,    -1,    85,   117,   171,   174,    86,    -1,    85,
     117,   171,    86,    -1,    85,   200,   207,    86,    -1,    85,
      86,    -1,    20,   204,    25,   201,    -1,    23,   119,    -1,
     171,    -1,   172,    -1,   173,    -1,   174,   173,    -1,    87,
     203,   207,    88,    -1,    87,    88,    -1,     8,   117,    -1,
       8,   117,    70,   117,    -1,   178,   199,    -1,   199,    72,
      -1,   178,   199,    72,    -1,   206,   180,   199,    -1,    40,
      -1,    41,    -1,    42,    -1,    43,    -1,    44,    -1,    45,
      -1,    46,    -1,    47,    -1,    48,    -1,    49,    -1,    50,
      -1,    51,    -1,     9,    -1,   133,   165,    -1,   133,    67,
     143,   207,    68,    -1,   133,    67,    68,    -1,    11,    -1,
      12,    -1,    14,   205,   207,    -1,    18,   133,    -1,    18,
     133,    25,   117,    -1,    18,   133,    25,   117,    70,   117,
      -1,    22,   192,   207,    -1,    24,   190,    -1,    21,   191,
      24,   189,    -1,    21,   191,    24,   189,    70,    -1,    21,
     191,    24,    67,   189,   207,    68,    -1,    21,   191,    24,
      71,    -1,    64,     7,    64,    -1,    64,    -1,   189,    70,
      64,     7,    64,    -1,   189,    70,    64,    -1,   191,     7,
      64,    -1,   191,    -1,   190,    70,   191,     7,    64,    -1,
     190,    70,   191,    -1,   198,    -1,    64,    -1,   192,    70,
      64,    -1,    31,    -1,    33,    54,   117,    70,   200,   207,
      -1,    33,    54,   117,    -1,    33,   200,   207,    -1,    33,
      -1,    34,   117,    70,   117,    70,   117,    -1,    34,   117,
      70,   117,    -1,    34,   117,    -1,    34,    -1,    36,   192,
     207,    -1,    39,   199,    -1,    64,    -1,   198,    84,    64,
      -1,   200,   207,    -1,   117,    -1,   200,    70,   117,    -1,
     202,   207,    -1,   119,    -1,   202,    70,   119,    -1,   117,
      66,   117,    -1,   203,    70,   117,    66,   117,    -1,   205,
     207,    -1,   206,    -1,   205,    70,   206,    -1,   133,    -1,
      70,    -1,    -1,    73,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   266,   266,   272,   278,   284,   291,   299,   307,   317,
     325,   333,   344,   352,   366,   371,   379,   387,   392,   399,
     416,   425,   434,   443,   453,   461,   469,   477,   486,   494,
     502,   510,   519,   526,   533,   540,   546,   555,   559,   566,
     572,   581,   585,   589,   590,   591,   592,   593,   594,   595,
     596,   597,   598,   599,   600,   601,   602,   603,   607,   611,
     618,   626,   630,   637,   638,   639,   640,   641,   642,   643,
     647,   648,   655,   662,   669,   678,   685,   692,   701,   708,
     717,   727,   735,   743,   751,   759,   768,   775,   781,   790,
     794,   811,   818,   825,   834,   839,   847,   851,   852,   862,
     863,   877,   878,   892,   893,   911,   916,   920,   925,   933,
     937,   941,   945,   949,   953,   957,   961,   965,   969,   976,
     977,   993,   994,  1010,  1011,  1027,  1028,  1041,  1057,  1058,
    1071,  1087,  1088,  1101,  1114,  1127,  1143,  1144,  1150,  1156,
    1165,  1166,  1176,  1177,  1178,  1179,  1180,  1184,  1201,  1210,
    1222,  1239,  1241,  1247,  1249,  1255,  1257,  1263,  1264,  1265,
    1266,  1270,  1271,  1272,  1273,  1277,  1281,  1288,  1298,  1307,
    1316,  1326,  1334,  1342,  1350,  1359,  1367,  1375,  1383,  1392,
    1399,  1406,  1413,  1419,  1428,  1432,  1439,  1445,  1454,  1464,
    1477,  1487,  1499,  1509,  1521,  1530,  1545,  1554,  1558,  1565,
    1566,  1567,  1571,  1575,  1582,  1583,  1584,  1588,  1592,  1599,
    1600,  1601,  1602,  1603,  1604,  1605,  1606,  1607,  1608,  1609,
    1610,  1611,  1612,  1616,  1620,  1627,  1628,  1632,  1636,  1643,
    1644,  1651,  1658,  1664,  1673,  1680,  1685,  1693,  1701,  1709,
    1718,  1726,  1734,  1743,  1750,  1761,  1767,  1776,  1785,  1793,
    1794,  1798,  1802,  1809,  1816,  1822,  1827,  1834,  1843,  1851,
    1852,  1856,  1860,  1867,  1872,  1879,  1889,  1902,  1911,  1922,
    1936,  1962,  1966,  1970,  1974,  1978,  1982,  1986,  1990,  1994,
    1998,  2002,  2006,  2013,  2020,  2026,  2034,  2042,  2049,  2056,
    2068,  2075,  2082,  2092,  2100,  2106,  2112,  2119,  2126,  2135,
    2141,  2148,  2154,  2164,  2170,  2177,  2183,  2193,  2200,  2204,
    2211,  2218,  2224,  2229,  2235,  2243,  2250,  2257,  2264,  2274,
    2282,  2290,  2294,  2301,  2319,  2323,  2330,  2349,  2353,  2360,
    2366,  2375,  2393,  2397,  2404,  2412,  2417,  2423,  2428
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined", "FILE_INPUT", "EVAL_INPUT",
  "SINGLE_INPUT", "\"and\"", "\"as\"", "\"assert\"", "\"break\"",
  "\"class\"", "\"continue\"", "\"cut\"", "\"def\"", "\"del\"", "\"elif\"",
  "\"else\"", "\"except\"", "\"exec\"", "\"finally\"", "\"for\"",
  "\"from\"", "\"global\"", "\"if\"", "\"import\"", "\"in\"", "\"is\"",
  "\"is not\"", "\"not\"", "\"not in\"", "\"or\"", "\"pass\"",
  "\"predicate\"", "\"print\"", "\"raise\"", "\"return\"", "\"static\"",
  "\"try\"", "\"while\"", "\"yield\"", "\"+=\"", "\"-=\"", "\"*=\"",
  "\"/=\"", "\"%=\"", "\"**=\"", "\"//=\"", "\"<<=\"", "\">>=\"", "\"&=\"",
  "\"^=\"", "\"|-\"", "\"//\"", "\"<<\"", "\">>\"", "\"**\"", "\"<=\"",
  "\">=\"", "\"==\"", "\"!=\"", "DEDENT", "INDENT", "NEWLINE",
  "\"number\"", "\"name\"", "\"string\"", "':'", "'('", "')'", "'@'",
  "','", "'*'", "'='", "';'", "'<'", "'>'", "'|'", "'^'", "'&'", "'+'",
  "'-'", "'/'", "'%'", "'~'", "'.'", "'['", "']'", "'{'", "'}'", "$accept",
  "start", "class_definition", "inheritance", "function_definition",
  "decorators", "decorator", "decorator_name", "parameter_list",
  "parameters", "default_parameters", "statement", "simple_statement",
  "simple_statement_list", "expression_statement", "suite",
  "compound_statement", "statement_list", "if_statement", "elif_clause",
  "else_clause", "for_statement", "try_statement", "except_clause",
  "except_clause_list", "finally_clause", "while_statement",
  "return_statement", "expression", "conditional", "or_test", "and_test",
  "not_test", "comparison", "comp_list", "comp_operator", "or_expr",
  "xor_expr", "and_expr", "shift_expr", "a_expr", "m_expr", "u_expr",
  "power", "primary", "attribute", "subscription", "slicing",
  "lower_bound", "upper_bound", "stride", "atom", "enclosure",
  "parenth_form", "argument_list", "positional_arguments",
  "keyword_arguments", "predicate_definition", "predicate_goal",
  "predicate_suite", "simple_predicate_statement",
  "simple_predicate_statement_list", "predicate_term",
  "predicate_term_list", "predicate_simple_term",
  "predicate_simple_term_list", "predicate_statement",
  "predicate_statement_list", "predicate_conjunction",
  "predicate_conjunction_suite", "predicate_disjunction",
  "predicate_disjunction_suite", "predicate_if_statement",
  "predicate_elif_clause", "predicate_else_clause",
  "predicate_for_statement", "generator_expression",
  "generator_for_clause", "generator_if_clause", "generator_clause",
  "generator_clause_list", "list_display", "list_for_clause",
  "list_if_clause", "list_clause", "list_clause_list", "dict_display",
  "assert_statement", "assignment_statement", "target_assignment_list",
  "augmented_assignment_statement", "augmented_assignment_operator",
  "break_statement", "call", "continue_statement", "cut_statement",
  "delete_statement", "execute_statement", "global_statement",
  "import_statement", "import_list", "module_list", "module",
  "identifier_list", "pass_statement", "print_statement",
  "raise_statement", "static_statement", "yield_statement", "dotted_name",
  "expression_or_tuple", "expression_list", "or_test_or_tuple",
  "or_test_list", "key_datum_list", "target_or_tuple", "target_list",
  "target", "opt_comma", "opt_semi", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,    58,    40,    41,    64,
      44,    42,    61,    59,    60,    62,   124,    94,    38,    43,
      45,    47,    37,   126,    46,    91,    93,   123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    89,    90,    90,    90,    90,    90,    90,    91,    92,
      93,    93,    93,    93,    94,    94,    95,    95,    95,    96,
      97,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    98,    98,    99,
      99,   100,   100,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   102,   102,
     103,   104,   104,   105,   105,   105,   105,   105,   105,   105,
     106,   106,   107,   107,   107,   108,   108,   108,   109,   110,
     110,   111,   111,   111,   111,   111,   112,   112,   112,   113,
     113,   114,   115,   115,   116,   116,   117,   118,   118,   119,
     119,   120,   120,   121,   121,   122,   122,   123,   123,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   125,
     125,   126,   126,   127,   127,   128,   128,   128,   129,   129,
     129,   130,   130,   130,   130,   130,   131,   131,   131,   131,
     132,   132,   133,   133,   133,   133,   133,   134,   135,   136,
     136,   137,   137,   138,   138,   139,   139,   140,   140,   140,
     140,   141,   141,   141,   141,   142,   142,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   144,   144,   145,   145,   146,   146,
     146,   146,   146,   146,   146,   146,   147,   148,   148,   149,
     149,   149,   150,   150,   151,   151,   151,   152,   152,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   154,   154,   155,   155,   155,   155,   156,
     156,   157,   158,   158,   159,   160,   160,   161,   161,   161,
     162,   162,   162,   163,   164,   165,   165,   166,   167,   168,
     168,   169,   169,   170,   170,   170,   170,   171,   172,   173,
     173,   174,   174,   175,   175,   176,   176,   177,   178,   178,
     179,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   181,   182,   182,   182,   183,   184,   185,
     186,   186,   186,   187,   188,   188,   188,   188,   188,   189,
     189,   189,   189,   190,   190,   190,   190,   191,   192,   192,
     193,   194,   194,   194,   194,   195,   195,   195,   195,   196,
     197,   198,   198,   199,   200,   200,   201,   202,   202,   203,
     203,   204,   205,   205,   206,   207,   207,   208,   208
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     2,     3,     4,     0,     5,     4,
       8,     7,     7,     6,     1,     2,     7,     5,     3,     1,
       9,     6,     6,     3,     7,     4,     4,     1,     7,     4,
       4,     1,     5,     2,     2,     5,     3,     1,     3,     3,
       5,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     4,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     5,     5,     4,     5,     5,     4,     3,     7,
       6,     6,     5,     5,     4,     4,     6,     4,     3,     1,
       2,     3,     5,     4,     2,     1,     1,     1,     5,     1,
       3,     1,     3,     1,     2,     2,     1,     2,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     3,     1,     3,     1,     3,     3,     1,     3,
       3,     1,     3,     3,     3,     3,     1,     2,     2,     2,
       1,     3,     1,     1,     1,     1,     1,     3,     4,     6,
       8,     1,     0,     1,     0,     1,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     9,     6,     6,
       3,     7,     4,     4,     1,     7,     4,     4,     1,     5,
       2,     2,     5,     3,     1,     3,     3,     5,    10,     9,
       8,     7,     9,     8,     7,     6,     3,     4,     3,     1,
       1,     1,     1,     3,     1,     1,     3,     1,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     3,     1,     1,     1,     1,
       2,     3,     4,     3,     3,     4,     3,     5,     5,     4,
       5,     5,     4,     3,     4,     5,     4,     4,     2,     1,
       1,     1,     2,     5,     4,     4,     2,     4,     2,     1,
       1,     1,     2,     4,     2,     2,     4,     2,     2,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     5,     3,     1,     1,     3,
       2,     4,     6,     3,     2,     4,     5,     7,     4,     3,
       1,     5,     3,     3,     1,     5,     3,     1,     1,     3,
       1,     6,     3,     3,     1,     6,     4,     2,     1,     3,
       2,     1,     3,     2,     1,     3,     2,     1,     3,     3,
       5,     2,     1,     3,     1,     1,     0,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       7,     0,     0,     0,     0,     0,   283,     0,   287,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   310,     0,
     314,   318,    95,     0,     0,     0,   158,   157,   159,     0,
       0,     0,     0,     0,     0,     0,    67,    68,     0,    14,
      70,    58,   338,    50,    41,     2,    63,    64,    65,    66,
      56,   324,    96,    97,    99,   101,   103,   106,   119,   121,
     123,   125,   128,   131,   136,   140,   143,   144,   145,   142,
     160,   161,    69,   164,   162,   163,    43,    44,     0,    45,
      46,   146,    47,    48,    49,    51,    52,    53,    54,    55,
      57,    60,   336,     0,   140,     0,     4,   338,     0,     1,
     265,     0,     0,   334,   336,   332,   290,     0,   336,   321,
       0,   307,   308,   336,     0,   294,   304,   104,     0,     0,
     336,   317,    94,     0,     0,   320,   166,   324,     0,     0,
      19,   138,   137,   139,   256,   324,   336,   264,     0,   336,
       0,     0,    15,   337,     0,    71,     0,     0,     0,   115,
     117,   118,   116,   110,   113,   111,   112,   109,   114,     0,
     105,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   152,   284,   267,   268,   335,
     323,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,     0,     3,     0,     5,     0,     0,     0,
       0,   335,   289,     0,     0,   331,     0,     0,   335,   293,
       0,     0,     0,     0,   312,   313,     0,     0,   338,     0,
       0,     0,     0,   165,    18,     0,     0,     0,     0,     0,
     335,     0,     0,     0,    59,    42,     0,   100,   102,   120,
       0,   107,   122,   124,   126,   127,   129,   130,   135,   132,
     133,   134,   141,     0,   157,   286,     0,   184,   336,   174,
     178,   147,   324,     0,     0,   269,   325,   270,     6,   266,
     336,     0,     0,    37,     0,     0,     0,    27,    31,   333,
     291,     0,   300,     0,   298,   295,   322,   309,    74,   306,
     303,     0,     0,     0,   316,     0,     0,     0,     0,    89,
      84,    85,    93,     0,     0,   246,   249,   250,   251,     0,
       0,   184,   336,     0,     0,   254,   259,   260,   261,     0,
     255,   329,     0,   263,     0,     0,     0,   108,   181,     0,
     180,   335,     0,     0,     0,   154,   148,     0,     8,    34,
       0,     0,    33,     0,     0,     0,     0,     0,     0,   336,
     296,     0,     0,    72,    73,     0,     0,     0,     0,   336,
       0,     0,    62,     0,     0,     0,    82,    90,    83,    92,
       0,   248,   245,   252,    17,     0,     0,   258,   253,   262,
       0,     0,     0,     0,     0,    98,   186,     0,   285,     0,
       0,   185,   170,     0,   157,     0,   183,   153,     0,     9,
      39,    13,     0,     0,     0,    38,     0,    23,     0,    36,
       0,   292,    80,   299,   335,     0,   302,     0,     0,   305,
       0,     0,   195,   202,   338,   199,   200,   201,     0,     0,
       0,     0,   311,   315,    61,    88,     0,     0,    91,    81,
     327,   247,   336,     0,   257,   330,     0,     0,     0,     0,
       0,     0,   173,   172,     0,   177,     0,   176,   156,   149,
       0,    12,    26,    25,     0,    30,     0,    29,    79,   297,
       0,     0,    78,   336,     0,   337,     0,     0,     0,   194,
       0,    87,     0,   335,   326,    16,    11,     0,   191,     0,
       0,     0,   179,     0,     0,     0,   182,   187,     0,   155,
       0,    32,     0,     0,    35,     0,    40,     0,   301,    77,
     319,     0,   226,   227,   338,   229,     0,   228,   203,   198,
     196,   193,     0,     0,   328,    10,     0,   190,     0,     0,
     169,   168,     0,   150,     0,    22,    21,     0,    75,    76,
       0,     0,   197,   230,   192,    86,   189,     0,   171,     0,
     175,    24,     0,    28,   288,     0,     0,   216,   223,   338,
     231,   209,   210,   211,   212,   213,   214,   215,   217,   218,
     219,   220,   221,   222,   225,   188,     0,     0,     0,     0,
     337,     0,   167,    20,     0,     0,     0,   207,     0,   338,
     204,   205,   224,   233,   244,     0,     0,   232,   208,   337,
       0,     0,     0,   338,   234,   206,   239,     0,     0,     0,
       0,   237,   238,     0,   236,     0,     0,   235,     0,   243,
     242,   240,   241
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,    36,   199,    37,    38,    39,   129,   276,   277,
     278,    40,    41,   218,    43,   219,    44,    45,    46,   353,
     354,    47,    48,   299,   300,   301,    49,    50,    51,    52,
      53,    54,    55,    56,   160,   161,    57,    58,    59,    60,
      61,    62,    63,    64,    94,    66,    67,    68,   263,   398,
     500,    69,    70,    71,   258,   259,   260,    72,   429,   422,
     423,   424,   587,   588,   558,   559,   515,   516,   517,   560,
     590,   604,   591,   611,   612,   561,    73,   222,   307,   308,
     309,    74,   316,   317,   318,   319,    75,    76,    77,    78,
      79,   193,    80,    81,    82,   565,    83,    84,    85,    86,
     285,   115,   110,   113,    87,    88,    89,   427,    90,   111,
      91,    92,   441,   442,   139,   107,   108,    93,   180,   144
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -520
static const yytype_int16 yypact[] =
{
     257,   786,  1170,   626,    43,  1170,  -520,   -10,  -520,     8,
     468,   468,   468,    11,    29,  1170,    11,  1170,  -520,    42,
     343,  1170,  1170,    -9,  1170,  1170,  -520,  -520,  -520,   588,
      11,   668,   668,   668,   617,   506,  -520,  -520,    79,  -520,
    -520,  -520,    -8,  -520,  -520,   786,  -520,  -520,  -520,  -520,
    -520,  -520,    55,   107,   180,  -520,  -520,  1213,    70,   113,
      62,   208,   160,  -520,  -520,  1254,  -520,  -520,  -520,  -520,
    -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,  1170,  -520,
    -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,
    -520,   126,   134,   787,   148,   155,  -520,    -8,   184,  -520,
     166,   206,   212,   121,   237,  -520,    83,   268,   237,  -520,
     286,   233,  -520,   255,    23,   260,   329,  -520,   278,  1170,
     134,   277,  -520,   916,   285,  -520,  -520,   334,   289,    82,
     233,  -520,  -520,  -520,  -520,   339,   134,  -520,   295,   293,
     306,   322,  -520,  1024,   307,  -520,  1170,  1170,  1170,  -520,
    -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,   668,
    1218,   668,   668,   668,   668,   668,   668,   668,   668,   668,
     668,   668,   668,   697,   331,  1170,  -520,   324,  -520,  1170,
    -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,
    -520,  -520,  -520,  1170,  -520,   341,  -520,  1170,  1170,   335,
     203,   468,  -520,  1170,  1170,  -520,   108,   345,   350,  -520,
     916,    11,   352,   228,   349,  -520,  1170,   360,    -8,   305,
     916,   468,    50,  -520,  -520,   830,   468,    27,   338,  1170,
    1170,   346,   362,   370,  -520,  -520,   153,   180,  -520,    70,
     668,   351,   113,    62,   208,   208,   160,   160,  -520,  -520,
    -520,  -520,  -520,  1170,   368,  -520,  1170,   334,   371,   372,
     373,  -520,   378,   380,   365,  -520,  -520,  -520,  -520,  -520,
     134,   916,   389,   387,   395,   399,   397,   401,   402,  -520,
     408,   403,   460,   417,  -520,   412,  -520,  -520,   282,   477,
    -520,   119,   419,  1170,   418,   786,   427,  1141,   424,  -520,
     363,  -520,   479,   472,  1170,  -520,  -520,  -520,  -520,    71,
     436,  -520,   371,   480,  1170,  -520,  -520,  -520,  -520,    41,
    -520,  -520,   438,  -520,   248,   250,  1170,   351,  -520,  1170,
     439,  -520,   443,   905,  1051,  1170,  -520,   444,  -520,  -520,
    1170,   916,   447,   448,   174,   279,  1170,   916,   449,   451,
     454,  1170,   453,  -520,  -520,   461,    38,   462,   127,   134,
    1170,   706,  -520,   916,   141,   916,   505,  -520,  -520,  -520,
    1170,   107,  -520,  -520,  -520,   469,  1170,   107,  -520,  -520,
    1170,   470,   474,   173,   475,   107,  -520,   473,  -520,  1170,
    1170,  -520,   478,  1170,   484,  1170,  -520,  -520,    46,  -520,
    -520,  -520,   490,   916,   485,   387,   488,   487,   494,   489,
     495,  -520,   479,  -520,   454,   492,   555,   497,   916,  -520,
      29,   503,  -520,  -520,   493,  -520,  -520,  -520,   496,   501,
      38,   462,  -520,  -520,  -520,  -520,   916,   468,  -520,  -520,
     107,  -520,   507,   513,  -520,  -520,   916,   510,    38,   462,
     183,  1170,  -520,   511,  1107,  -520,  1170,   514,  1170,  -520,
     516,  -520,  -520,   518,   284,  -520,  1170,   522,  -520,  -520,
     519,   916,  -520,   255,   246,   204,   533,  1170,    38,  -520,
     530,  -520,   531,  1170,  -520,  -520,  -520,   916,  -520,   532,
      38,   462,  -520,   544,  1170,  1170,  -520,  -520,   546,  -520,
     517,  -520,   547,   542,   489,   545,  -520,   553,  -520,   282,
    -520,   548,  -520,  -520,   493,  -520,    45,  -520,  -520,  -520,
    -520,  -520,    38,   916,   107,  -520,    38,  -520,   552,  1170,
    -520,   540,  1170,  -520,   556,  -520,   541,   558,  -520,  -520,
    1062,   550,  -520,  -520,  -520,  -520,  -520,    38,  -520,   564,
    -520,  -520,   568,  -520,  -520,   468,   563,  -520,  -520,   554,
    -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,  -520,
    -520,  -520,  -520,  -520,  -520,  -520,  1170,   561,   603,   986,
    1132,   567,  -520,  -520,  1170,  1170,   565,  -520,   856,   557,
    -520,  -520,  -520,  -520,  -520,    91,   156,  -520,  -520,  -520,
     571,  1062,   581,   493,  -520,  -520,   313,   246,   600,  1170,
     604,  -520,  -520,   244,  -520,   606,  1062,  -520,  1062,  -520,
     313,  -520,  -520
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -520,  -520,  -520,  -520,  -461,  -520,   628,  -520,  -171,  -520,
     325,   -41,   534,    14,  -492,  -181,   671,   381,  -520,   169,
    -279,  -520,  -520,   379,  -520,  -237,  -520,  -520,    -2,  -520,
      -3,   536,     3,  -520,  -520,   525,  -115,   527,   537,   524,
     201,   223,    -7,  -520,    -1,  -520,  -520,  -520,  -520,  -520,
    -520,  -520,  -520,  -520,   467,  -520,   361,  -433,  -346,  -392,
     226,  -437,   110,  -520,  -196,  -520,  -482,   100,  -520,  -519,
    -520,  -520,  -520,    88,    90,  -520,     1,  -173,  -520,   409,
    -520,  -520,   577,  -520,   404,  -520,  -520,  -478,  -460,  -520,
    -332,  -520,  -520,  -520,  -520,  -520,  -316,  -203,  -122,  -168,
     459,  -520,     2,   301,    25,  -141,   -78,   -72,  -520,   692,
       6,    -4,   374,  -520,  -520,  -220,   726,    -5,   -68,   -83
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -335
static const yytype_int16 yytable[] =
{
      65,   303,    65,   100,   145,   105,   313,   105,    95,   103,
     106,   103,   114,   512,   195,    42,   120,    97,   116,   121,
     117,   366,   124,   369,   131,   132,   133,   127,   122,   288,
     136,   125,   135,   138,   543,   128,   202,   514,   479,   302,
     205,   513,   292,    99,    65,   209,   241,   226,   557,   306,
     314,   511,   215,   147,   101,   512,   488,   123,     9,    42,
      14,   226,   562,   368,   314,   143,   176,    14,   228,    18,
     221,   231,   102,   304,   420,   109,    18,    19,   146,   514,
     563,   420,   606,   513,   177,   480,   521,   557,   557,   210,
     338,   221,   140,   112,   304,   176,   557,   619,   527,   620,
     421,   562,   562,   489,   176,   542,   118,   176,   203,   557,
     562,   141,   458,   315,    30,   164,   165,   214,   305,   563,
     563,   147,    65,   562,   557,   327,   557,   378,   563,   439,
     544,   543,   459,   468,   546,   296,   306,   147,   562,   372,
     562,   563,    65,   236,   224,   528,   512,   162,    30,   225,
     173,   238,   512,   382,   384,   575,   563,   601,   563,   603,
     401,   248,   249,   250,   251,   252,   412,   174,   175,   326,
     514,   257,   282,   262,   513,   283,   514,   266,    14,   284,
     513,   264,   435,   147,   438,   356,   148,    18,   173,   357,
     332,   163,   420,   430,   270,   269,   279,   431,   178,   267,
     103,   280,   337,   172,   179,   174,   175,   436,   564,    65,
     281,   437,   168,   289,   294,   173,   105,   194,   602,    65,
     103,   105,   461,   311,   566,   103,    14,   321,   322,   404,
     539,   169,   174,   175,   425,    18,   197,   472,   405,   448,
     420,   170,   171,   449,   375,   406,   196,   564,   564,   490,
     511,   328,   511,   491,   330,   481,   564,     9,   272,     9,
       1,     2,     3,   566,   566,   486,    14,   273,    14,   564,
      65,   274,   566,   198,   275,    18,    19,    18,    19,   200,
     420,   415,   420,   272,   564,   566,   564,   166,   167,   359,
     509,   432,   273,   204,    65,   364,   291,   351,   352,   275,
     566,   371,   566,   272,   617,   272,   525,   201,   425,    42,
     206,   377,   273,    30,   273,    30,   381,   207,   383,   275,
     145,   275,   297,   385,   298,   208,   425,   386,   609,   610,
     211,   391,   396,   397,   408,   578,   212,   567,   400,   503,
      65,   476,   545,   409,   411,   213,    65,   216,   504,   417,
     410,   220,   425,   425,   221,   505,   425,   223,   433,   226,
      65,   229,    65,   230,    65,   244,   245,   440,   425,   235,
     232,    17,   569,   440,   484,    42,   567,   567,   445,   352,
     297,   426,   298,   589,   592,   567,   233,   452,   453,   246,
     247,   455,   589,   457,   425,   261,   265,   119,   567,   571,
     425,   271,    65,   268,   425,   510,    26,    27,    28,   286,
      29,   569,   569,   567,   287,   567,   290,    65,   568,   293,
     569,   295,    31,    32,   320,   425,    33,   159,    34,   324,
      35,   541,   482,   569,   323,    65,   103,   325,   571,   571,
     329,   331,   333,   334,  -151,    65,   335,   571,   569,   492,
     569,   336,   496,   339,   497,   426,   499,   568,   568,   340,
     571,   341,   572,   342,   506,   343,   568,   348,   573,   347,
      65,   344,   345,   426,   425,   571,   581,   571,   346,   568,
     524,   282,   350,   520,   355,   425,    65,   358,   360,   362,
     365,   425,   530,   531,   568,   352,   568,   370,   374,   426,
     426,   572,   572,   426,   380,   376,   600,   573,   573,   387,
     572,   388,   399,   413,   403,   426,   573,   402,   416,   418,
     608,   414,    65,   572,   298,   419,   428,   548,   451,   573,
     550,    26,    27,    28,    17,    29,   446,   443,   572,    65,
     572,   426,   447,   450,   573,   460,   573,   426,   454,   462,
     105,   426,   463,    34,   103,    35,   456,   464,   465,   467,
     469,   466,   470,   471,   474,   570,   475,   478,   477,    26,
      27,    28,   426,    29,   582,   485,   487,   483,    65,    65,
     501,   493,   595,   508,   498,    31,    32,    65,   502,    33,
     594,    34,   507,    35,   137,   519,   522,   523,   526,   529,
      65,   532,   534,   533,   570,   570,   535,   615,   537,   536,
     549,   552,   574,   570,   540,    65,    17,    65,   547,   576,
     551,   426,   553,   577,   579,   583,   570,   580,   584,   593,
     599,   596,   426,   605,     5,     6,     7,     8,   426,     9,
      10,   570,   607,   570,    11,    17,    12,    13,    14,    15,
      16,    26,    27,    28,    17,    29,   126,    18,    19,    20,
      21,    22,   614,    23,    24,    25,   142,    31,    32,   407,
     616,    33,   618,    34,    98,    35,   361,   234,   538,   367,
      26,    27,    28,   237,    29,   240,   239,   243,    96,    26,
      27,    28,   312,    29,   392,    30,    31,    32,   598,   242,
      33,   518,    34,   134,    35,    31,    32,   613,   621,    33,
     622,    34,   227,    35,     5,     6,     7,     8,   373,     9,
      10,   473,   130,   379,    11,    17,    12,    13,    14,    15,
      16,    26,    27,    28,    17,    29,   104,    18,    19,    20,
      21,    22,   349,    23,    24,    25,     0,    31,    32,     0,
     444,    33,   253,    34,     0,    35,     0,     0,     0,     0,
      26,   254,    28,     0,    29,   255,   434,     0,   256,    26,
      27,    28,     0,    29,     0,    30,    31,    32,     0,     0,
      33,     0,    34,     0,    35,    31,    32,     0,     0,    33,
       0,    34,     0,    35,     5,     6,     7,     8,     0,     9,
      10,     0,     0,     0,    11,     0,    12,    13,    14,    15,
      16,     0,     0,     0,    17,     0,     0,    18,    19,    20,
      21,    22,     0,    23,    24,    25,     0,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    26,
      27,    28,     0,    29,     0,    30,     0,     0,    17,     0,
       0,     0,     0,     0,     5,    31,    32,     0,   554,    33,
      10,    34,     0,    35,    11,     0,   555,    13,    14,   585,
      16,     0,     0,     0,    17,   253,   586,    18,     0,    20,
      21,     0,   420,    26,   254,    28,     0,    29,   310,     0,
       0,   256,     0,     0,     0,     0,     0,     0,     0,    31,
      32,     0,     0,    33,     0,    34,   597,    35,     0,    26,
      27,    28,     0,    29,     5,     6,     0,     8,     0,     0,
      10,     0,     0,    17,    11,    31,    32,    13,    14,    33,
      16,    34,     0,    35,    17,     0,     0,    18,     0,    20,
      21,    22,     0,     0,     0,    25,     0,     0,     0,     0,
     389,     0,     0,     0,     0,     0,     0,     0,    26,   254,
      28,     0,    29,     0,     0,     0,   390,     0,   217,    26,
      27,    28,     0,    29,    31,    32,     0,     0,    33,     0,
      34,     0,    35,     0,     5,    31,    32,     0,   554,    33,
      10,    34,     0,    35,    11,     0,   555,    13,    14,   585,
      16,     0,     0,     0,    17,     0,   586,    18,     0,    20,
      21,     0,   420,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     0,     8,     0,     0,    10,     0,
       0,     0,    11,     0,     0,    13,    14,     0,    16,    26,
      27,    28,    17,    29,     0,    18,     0,    20,    21,    22,
       0,     0,     0,    25,     0,    31,    32,     0,     0,    33,
       5,    34,     0,    35,   554,     0,    10,     0,     0,    17,
      11,     0,   555,    13,    14,     0,    16,    26,    27,    28,
      17,    29,     0,    18,     0,    20,    21,     0,   420,     0,
       0,     0,     0,    31,    32,     0,   393,    33,     0,    34,
       0,    35,     0,     0,    26,   394,    28,     0,    29,     0,
       0,     0,   395,     0,   556,    26,    27,    28,     0,    29,
      31,    32,     0,     0,    33,    17,    34,     0,    35,     0,
       5,    31,    32,     0,   554,    33,    10,    34,     0,    35,
      11,     0,   555,    13,    14,     0,    16,     0,     0,     0,
      17,     0,   494,    18,     0,    20,    21,     0,   420,    17,
      26,   394,    28,     0,    29,     0,     0,     0,   495,     0,
       0,     0,     0,     0,     0,     0,    31,    32,     0,     0,
      33,     0,    34,     0,    35,    26,    27,    28,    17,    29,
       0,     0,     0,     0,    26,    27,    28,   363,    29,     0,
       0,    31,    32,     0,     0,    33,     0,    34,     0,    35,
      31,    32,     0,     0,    33,     0,    34,     0,    35,     0,
       0,     0,     0,    26,    27,    28,     0,    29,   149,   150,
     151,     0,   152,   149,   150,   151,     0,   152,     0,    31,
      32,     0,     0,    33,     0,    34,     0,    35,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   153,
     154,   155,   156,     0,   153,   154,   155,   156,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   157,   158,   159,
       0,     0,   157,   158,  -334,  -334,  -334,  -334,  -334,  -334,
    -334,  -334,  -334,  -334,  -334,  -334,     0,     0,     0,   172,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   173,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   174,   175
};

static const yytype_int16 yycheck[] =
{
       1,   221,     3,     5,    45,    10,   226,    12,     2,    10,
      11,    12,    15,   474,    97,     1,    20,     3,    16,    21,
      17,   300,    24,   302,    31,    32,    33,    29,    22,   210,
      34,    25,    34,    35,   516,    29,   104,   474,   430,   220,
     108,   474,   213,     0,    45,   113,   161,    20,   540,   222,
      23,     6,   120,    30,    64,   516,   448,    66,    13,    45,
      22,    20,   540,   300,    23,    73,    65,    22,   136,    31,
      20,   139,    64,    23,    36,    64,    31,    32,    23,   516,
     540,    36,   601,   516,    78,   431,   478,   579,   580,    66,
     271,    20,    13,    64,    23,    94,   588,   616,   490,   618,
      62,   579,   580,   449,   103,    60,    64,   106,    25,   601,
     588,    32,    66,    86,    69,    53,    54,   119,    68,   579,
     580,    30,   123,   601,   616,   240,   618,    86,   588,   366,
     522,   613,    86,   412,   526,   218,   309,    30,   616,    68,
     618,   601,   143,   146,    62,   491,   607,    77,    69,    67,
      67,   148,   613,   324,   325,   547,   616,    66,   618,   596,
     341,   168,   169,   170,   171,   172,   347,    84,    85,    16,
     607,   173,    64,   175,   607,    67,   613,   179,    22,    71,
     613,   175,   363,    30,   365,    66,     6,    31,    67,    70,
     258,    78,    36,    66,   198,   197,   201,    70,    72,   193,
     201,   203,   270,    55,    70,    84,    85,    66,   540,   210,
     204,    70,    52,   211,   216,    67,   221,    62,    62,   220,
     221,   226,   403,   225,   540,   226,    22,   229,   230,    55,
     509,    71,    84,    85,   356,    31,    70,   418,    64,    66,
      36,    81,    82,    70,   312,    71,    62,   579,   580,    66,
       6,   253,     6,    70,   256,   436,   588,    13,    55,    13,
       3,     4,     5,   579,   580,   446,    22,    64,    22,   601,
     271,    68,   588,    67,    71,    31,    32,    31,    32,    67,
      36,   349,    36,    55,   616,   601,   618,    79,    80,   293,
     471,   359,    64,    25,   295,   297,    68,    15,    16,    71,
     616,   304,   618,    55,    60,    55,   487,    70,   430,   295,
      24,   314,    64,    69,    64,    69,    68,    84,    68,    71,
     361,    71,    17,   326,    19,    70,   448,   329,    15,    16,
      70,   333,   334,   335,    55,   555,     7,   540,   340,    55,
     341,   424,   523,    64,   346,    67,   347,    70,    64,   351,
      71,    66,   474,   475,    20,    71,   478,    68,   360,    20,
     361,    66,   363,    70,   365,   164,   165,   370,   490,    62,
      64,    28,   540,   376,   442,   361,   579,   580,   380,    16,
      17,   356,    19,   579,   580,   588,    64,   389,   390,   166,
     167,   393,   588,   395,   516,    64,    72,    54,   601,   540,
     522,    66,   403,    62,   526,   473,    63,    64,    65,    64,
      67,   579,   580,   616,    64,   618,    64,   418,   540,    70,
     588,    61,    79,    80,    86,   547,    83,    76,    85,    67,
      87,   514,   437,   601,    88,   436,   437,    67,   579,   580,
      72,    70,    70,    70,    66,   446,    66,   588,   616,   451,
     618,    86,   454,    64,   456,   430,   458,   579,   580,    72,
     601,    66,   540,    64,   466,    68,   588,     7,   540,    66,
     471,    70,    70,   448,   596,   616,   559,   618,    70,   601,
     483,    64,    70,   477,     7,   607,   487,    68,    70,    62,
      66,   613,   494,   495,   616,    16,   618,    25,    62,   474,
     475,   579,   580,   478,    66,    25,   589,   579,   580,    70,
     588,    68,    68,    64,    66,   490,   588,    70,    64,    66,
     603,    70,   523,   601,    19,    64,    64,   529,    55,   601,
     532,    63,    64,    65,    28,    67,    66,    68,   616,   540,
     618,   516,    68,    68,   616,    55,   618,   522,    70,    64,
     555,   526,    64,    85,   555,    87,    72,    70,    64,    64,
      68,    72,     7,    66,    61,   540,    73,    66,    72,    63,
      64,    65,   547,    67,   576,    62,    66,    70,   579,   580,
      64,    70,   585,    64,    70,    79,    80,   588,    70,    83,
     584,    85,    70,    87,    88,    62,    66,    66,    66,    55,
     601,    55,    55,    86,   579,   580,    64,   609,    55,    64,
      70,    70,    62,   588,    66,   616,    28,   618,    66,    55,
      64,   596,    64,    55,    61,    64,   601,    73,    25,    62,
      73,    66,   607,    62,     8,     9,    10,    11,   613,    13,
      14,   616,    61,   618,    18,    28,    20,    21,    22,    23,
      24,    63,    64,    65,    28,    67,    68,    31,    32,    33,
      34,    35,    62,    37,    38,    39,    38,    79,    80,   344,
      66,    83,    66,    85,     3,    87,   295,   143,   509,   300,
      63,    64,    65,   147,    67,   160,   159,   163,    62,    63,
      64,    65,   225,    67,   333,    69,    79,    80,   588,   162,
      83,   475,    85,    86,    87,    79,    80,   607,   620,    83,
     620,    85,   135,    87,     8,     9,    10,    11,   309,    13,
      14,   420,    30,   319,    18,    28,    20,    21,    22,    23,
      24,    63,    64,    65,    28,    67,    10,    31,    32,    33,
      34,    35,   283,    37,    38,    39,    -1,    79,    80,    -1,
     376,    83,    55,    85,    -1,    87,    -1,    -1,    -1,    -1,
      63,    64,    65,    -1,    67,    68,    60,    -1,    71,    63,
      64,    65,    -1,    67,    -1,    69,    79,    80,    -1,    -1,
      83,    -1,    85,    -1,    87,    79,    80,    -1,    -1,    83,
      -1,    85,    -1,    87,     8,     9,    10,    11,    -1,    13,
      14,    -1,    -1,    -1,    18,    -1,    20,    21,    22,    23,
      24,    -1,    -1,    -1,    28,    -1,    -1,    31,    32,    33,
      34,    35,    -1,    37,    38,    39,    -1,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      64,    65,    -1,    67,    -1,    69,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    -1,     8,    79,    80,    -1,    12,    83,
      14,    85,    -1,    87,    18,    -1,    20,    21,    22,    23,
      24,    -1,    -1,    -1,    28,    55,    30,    31,    -1,    33,
      34,    -1,    36,    63,    64,    65,    -1,    67,    68,    -1,
      -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    79,
      80,    -1,    -1,    83,    -1,    85,    60,    87,    -1,    63,
      64,    65,    -1,    67,     8,     9,    -1,    11,    -1,    -1,
      14,    -1,    -1,    28,    18,    79,    80,    21,    22,    83,
      24,    85,    -1,    87,    28,    -1,    -1,    31,    -1,    33,
      34,    35,    -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,
      65,    -1,    67,    -1,    -1,    -1,    71,    -1,    62,    63,
      64,    65,    -1,    67,    79,    80,    -1,    -1,    83,    -1,
      85,    -1,    87,    -1,     8,    79,    80,    -1,    12,    83,
      14,    85,    -1,    87,    18,    -1,    20,    21,    22,    23,
      24,    -1,    -1,    -1,    28,    -1,    30,    31,    -1,    33,
      34,    -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     8,     9,    -1,    11,    -1,    -1,    14,    -1,
      -1,    -1,    18,    -1,    -1,    21,    22,    -1,    24,    63,
      64,    65,    28,    67,    -1,    31,    -1,    33,    34,    35,
      -1,    -1,    -1,    39,    -1,    79,    80,    -1,    -1,    83,
       8,    85,    -1,    87,    12,    -1,    14,    -1,    -1,    28,
      18,    -1,    20,    21,    22,    -1,    24,    63,    64,    65,
      28,    67,    -1,    31,    -1,    33,    34,    -1,    36,    -1,
      -1,    -1,    -1,    79,    80,    -1,    55,    83,    -1,    85,
      -1,    87,    -1,    -1,    63,    64,    65,    -1,    67,    -1,
      -1,    -1,    71,    -1,    62,    63,    64,    65,    -1,    67,
      79,    80,    -1,    -1,    83,    28,    85,    -1,    87,    -1,
       8,    79,    80,    -1,    12,    83,    14,    85,    -1,    87,
      18,    -1,    20,    21,    22,    -1,    24,    -1,    -1,    -1,
      28,    -1,    55,    31,    -1,    33,    34,    -1,    36,    28,
      63,    64,    65,    -1,    67,    -1,    -1,    -1,    71,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    79,    80,    -1,    -1,
      83,    -1,    85,    -1,    87,    63,    64,    65,    28,    67,
      -1,    -1,    -1,    -1,    63,    64,    65,    66,    67,    -1,
      -1,    79,    80,    -1,    -1,    83,    -1,    85,    -1,    87,
      79,    80,    -1,    -1,    83,    -1,    85,    -1,    87,    -1,
      -1,    -1,    -1,    63,    64,    65,    -1,    67,    25,    26,
      27,    -1,    29,    25,    26,    27,    -1,    29,    -1,    79,
      80,    -1,    -1,    83,    -1,    85,    -1,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      57,    58,    59,    -1,    56,    57,    58,    59,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      -1,    -1,    74,    75,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    -1,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    84,    85
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,    90,     8,     9,    10,    11,    13,
      14,    18,    20,    21,    22,    23,    24,    28,    31,    32,
      33,    34,    35,    37,    38,    39,    63,    64,    65,    67,
      69,    79,    80,    83,    85,    87,    91,    93,    94,    95,
     100,   101,   102,   103,   105,   106,   107,   110,   111,   115,
     116,   117,   118,   119,   120,   121,   122,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   140,
     141,   142,   146,   165,   170,   175,   176,   177,   178,   179,
     181,   182,   183,   185,   186,   187,   188,   193,   194,   195,
     197,   199,   200,   206,   133,   199,    62,   102,   105,     0,
     117,    64,    64,   133,   205,   206,   133,   204,   205,    64,
     191,   198,    64,   192,   119,   190,   191,   121,    64,    54,
     200,   117,   199,    66,   117,   199,    68,   117,   199,    96,
     198,   131,   131,   131,    86,   117,   200,    88,   117,   203,
      13,    32,    95,    73,   208,   100,    23,    30,     6,    25,
      26,    27,    29,    56,    57,    58,    59,    74,    75,    76,
     123,   124,    77,    78,    53,    54,    79,    80,    52,    71,
      81,    82,    55,    67,    84,    85,   165,   199,    72,    70,
     207,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   180,    62,   208,    62,    70,    67,    92,
      67,    70,   207,    25,    25,   207,    24,    84,    70,   207,
      66,    70,     7,    67,   117,   207,    70,    62,   102,   104,
      66,    20,   166,    68,    62,    67,    20,   171,   207,    66,
      70,   207,    64,    64,   101,    62,   119,   120,   121,   126,
     124,   125,   127,   128,   129,   129,   130,   130,   131,   131,
     131,   131,   131,    55,    64,    68,    71,   117,   143,   144,
     145,    64,   117,   137,   199,    72,   117,   199,    62,   117,
     200,    66,    55,    64,    68,    71,    97,    98,    99,   206,
     117,   199,    64,    67,    71,   189,    64,    64,   104,   191,
      64,    68,    97,    70,   117,    61,   208,    17,    19,   112,
     113,   114,   104,   204,    23,    68,   166,   167,   168,   169,
      68,   117,   143,   204,    23,    86,   171,   172,   173,   174,
      86,   117,   117,    88,    67,    67,    16,   125,   117,    72,
     117,    70,   207,    70,    70,    66,    86,   207,   104,    64,
      72,    66,    64,    68,    70,    70,    70,    66,     7,   189,
      70,    15,    16,   108,   109,     7,    66,    70,    68,   200,
      70,   106,    62,    66,   117,    66,   109,   112,   114,   109,
      25,   119,    68,   168,    62,   207,    25,   119,    86,   173,
      66,    68,    97,    68,    97,   119,   117,    70,    68,    55,
      71,   117,   145,    55,    64,    71,   117,   117,   138,    68,
     117,   104,    70,    66,    55,    64,    71,    99,    55,    64,
      71,   117,   104,    64,    70,   207,    64,   117,    66,    64,
      36,    62,   148,   149,   150,   187,   193,   196,    64,   147,
      66,    70,   207,   117,    60,   104,    66,    70,   104,   114,
     119,   201,   202,    68,   201,   117,    66,    68,    66,    70,
      68,    55,   117,   117,    70,   117,    72,   117,    66,    86,
      55,   104,    64,    64,    70,    64,    72,    64,   109,    68,
       7,    66,   104,   192,    61,    73,   208,    72,    66,   148,
     147,   104,   206,    70,   207,    62,   104,    66,   148,   147,
      66,    70,   117,    70,    55,    71,   117,   117,    70,   117,
     139,    64,    70,    55,    64,    71,   117,    70,    64,   104,
     207,     6,    93,   146,   150,   155,   156,   157,   149,    62,
     199,   148,    66,    66,   119,   104,    66,   148,   147,    55,
     117,   117,    55,    86,    55,    64,    64,    55,   108,   109,
      66,   208,    60,   155,   148,   104,   148,    66,   117,    70,
     117,    64,    70,    64,    12,    20,    62,   103,   153,   154,
     158,   164,   176,   177,   179,   184,   185,   186,   187,   188,
     193,   194,   195,   196,    62,   148,    55,    55,   204,    61,
      73,   208,   117,    64,    25,    23,    30,   151,   152,   153,
     159,   161,   153,    62,   199,   119,    66,    60,   151,    73,
     208,    66,    62,   150,   160,    62,   158,    61,   208,    15,
      16,   162,   163,   156,    62,   117,    66,    60,    66,   158,
     158,   162,   163
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;
/* Location data for the look-ahead symbol.  */
YYLTYPE yylloc;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;

  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[2];

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;
#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 0;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
	YYSTACK_RELOCATE (yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 267 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_FILE_INPUT, (yyvsp[(2) - (2)].list), (yyloc));
                yylval.node = (yyval.node);  /* for lexer to grab */
        }
    break;

  case 3:
#line 273 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_EVAL_INPUT, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (3)].node));
                yylval.node = (yyval.node);  /* for lexer to grab */
        }
    break;

  case 4:
#line 279 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_EVAL_INPUT, (yyloc));
                yylval.node = (yyval.node);  /* for lexer to grab */
                YYACCEPT;
        }
    break;

  case 5:
#line 285 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_SINGLE_INPUT, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (3)].node));
                yylval.node = (yyval.node);  /* for lexer to grab */
                YYACCEPT;
        }
    break;

  case 6:
#line 292 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_SINGLE_INPUT, (yyvsp[(2) - (4)].list), (yyloc));
                yylval.node = (yyval.node);  /* for lexer to grab */
                YYACCEPT;
        }
    break;

  case 7:
#line 299 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_FILE_INPUT, (yyloc));
                yylval.node = (yyval.node);  /* for lexer to grab */
                YYACCEPT;
        }
    break;

  case 8:
#line 308 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_CLASS, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (5)].node));
                adopt_name ((yyval.node), (yyvsp[(2) - (5)].node));
        }
    break;

  case 9:
#line 318 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_TUPLE, (yyvsp[(2) - (4)].list), (yyloc));
                PCL_NODE_FLAGS ((yyval.node)) |= (yyvsp[(3) - (4)].flag);
        }
    break;

  case 10:
#line 326 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_FUNCTION, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(8) - (8)].node));
                adopt_children ((yyval.node), (yyvsp[(5) - (8)].list));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (8)].node));
                adopt_name ((yyval.node), (yyvsp[(3) - (8)].node));
        }
    break;

  case 11:
#line 334 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_FUNCTION, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(7) - (7)].node));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (7)].node));
                adopt_name ((yyval.node), (yyvsp[(3) - (7)].node));
        }
    break;

  case 12:
#line 345 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_FUNCTION, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(7) - (7)].node));
                adopt_children ((yyval.node), (yyvsp[(4) - (7)].list));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                adopt_name ((yyval.node), (yyvsp[(2) - (7)].node));
        }
    break;

  case 13:
#line 353 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_FUNCTION, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(6) - (6)].node));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                adopt_name ((yyval.node), (yyvsp[(2) - (6)].node));
        }
    break;

  case 14:
#line 367 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_DECORATORS, (yyloc));
                g_node_append ((yyval.node), (yyvsp[(1) - (1)].node));
        }
    break;

  case 15:
#line 372 "pcl-parser.y"
    {
                (yyval.node) = (yyvsp[(1) - (2)].node);
                g_node_append ((yyval.node), (yyvsp[(2) - (2)].node));
        }
    break;

  case 16:
#line 380 "pcl-parser.y"
    {
                if ((yyvsp[(4) - (7)].list) == NULL)
                        YYABORT;  /* syntax error occurred */
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_CALL, (yyvsp[(4) - (7)].list), (yyloc));
                PCL_NODE_FLAGS ((yyval.node)) |= (yyvsp[(5) - (7)].flag);
                g_node_prepend ((yyval.node), (yyvsp[(2) - (7)].node));
        }
    break;

  case 17:
#line 388 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_CALL, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (5)].node));
        }
    break;

  case 18:
#line 393 "pcl-parser.y"
    {
                (yyval.node) = (yyvsp[(2) - (3)].node);
        }
    break;

  case 19:
#line 400 "pcl-parser.y"
    {
                if (g_list_next ((yyvsp[(1) - (1)].list)) != NULL)
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_ATTRIBUTE, (yyloc));
                        adopt_children ((yyval.node), (yyvsp[(1) - (1)].list));
                }
                else
                {
                        (yyval.node) = (yyvsp[(1) - (1)].list)->data;
                        (yyvsp[(1) - (1)].list) = g_list_delete_link ((yyvsp[(1) - (1)].list), (yyvsp[(1) - (1)].list));
                        g_assert ((yyvsp[(1) - (1)].list) == NULL);
                }
        }
    break;

  case 20:
#line 417 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(1) - (9)].list), (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(3) - (9)].list), (yyloc)),
                        (yyvsp[(6) - (9)].node), (yyvsp[(9) - (9)].node));
        }
    break;

  case 21:
#line 426 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(1) - (6)].list), (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(3) - (6)].list), (yyloc)),
                        (yyvsp[(6) - (6)].node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 22:
#line 435 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(1) - (6)].list), (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(3) - (6)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)), (yyvsp[(6) - (6)].node));
        }
    break;

  case 23:
#line 444 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(1) - (3)].list), (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(3) - (3)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 24:
#line 454 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(1) - (7)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                         (yyvsp[(4) - (7)].node), (yyvsp[(7) - (7)].node));
        }
    break;

  case 25:
#line 462 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(1) - (4)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        (yyvsp[(4) - (4)].node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 26:
#line 470 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(1) - (4)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)), (yyvsp[(4) - (4)].node));
        }
    break;

  case 27:
#line 478 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(1) - (1)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 28:
#line 487 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(1) - (7)].list), (yyloc)),
                        (yyvsp[(4) - (7)].node), (yyvsp[(7) - (7)].node));
        }
    break;

  case 29:
#line 495 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(1) - (4)].list), (yyloc)),
                        (yyvsp[(4) - (4)].node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 30:
#line 503 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(1) - (4)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)), (yyvsp[(4) - (4)].node));
        }
    break;

  case 31:
#line 511 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, (yyvsp[(1) - (1)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 32:
#line 520 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        (yyvsp[(2) - (5)].node), (yyvsp[(5) - (5)].node));
        }
    break;

  case 33:
#line 527 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        (yyvsp[(2) - (2)].node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 34:
#line 534 "pcl-parser.y"
    {
                (yyval.list) = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)), (yyvsp[(2) - (2)].node));
        }
    break;

  case 35:
#line 541 "pcl-parser.y"
    {
                yylloc.first_column = (yylsp[(5) - (5)]).first_column;
                yyerror ("non-default argument follows default argument");
                YYABORT;
        }
    break;

  case 36:
#line 547 "pcl-parser.y"
    {
                yylloc.first_column = (yylsp[(3) - (3)]).first_column;
                yyerror ("non-default argument follows default argument");
                YYABORT;
        }
    break;

  case 37:
#line 556 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 38:
#line 560 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 39:
#line 567 "pcl-parser.y"
    {
                (yyval.list) = NULL;
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(1) - (3)].node));
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 40:
#line 573 "pcl-parser.y"
    {
                (yyval.list) = (yyvsp[(1) - (5)].list);
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(3) - (5)].node));
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(5) - (5)].node));
        }
    break;

  case 41:
#line 582 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 58:
#line 608 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 59:
#line 612 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 60:
#line 619 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_EXPRESSION, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (1)].node));
        }
    break;

  case 61:
#line 627 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_SUITE, (yyvsp[(3) - (4)].list), (yyloc));
        }
    break;

  case 62:
#line 631 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_SUITE, (yyvsp[(1) - (3)].list), (yyloc));
        }
    break;

  case 71:
#line 649 "pcl-parser.y"
    {
                (yyval.list) = g_list_concat ((yyvsp[(2) - (2)].list), (yyvsp[(1) - (2)].list));
        }
    break;

  case 72:
#line 656 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (5)].node));
        }
    break;

  case 73:
#line 663 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (5)].node));
        }
    break;

  case 74:
#line 670 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 75:
#line 679 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (5)].node));
        }
    break;

  case 76:
#line 686 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (5)].node));
        }
    break;

  case 77:
#line 693 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 78:
#line 702 "pcl-parser.y"
    {
                (yyval.node) = (yyvsp[(3) - (3)].node);
        }
    break;

  case 79:
#line 710 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_FOR, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(7) - (7)].node));
                g_node_prepend ((yyval.node), (yyvsp[(6) - (7)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (7)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (7)].node));
        }
    break;

  case 80:
#line 718 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_FOR, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(6) - (6)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (6)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (6)].node));
        }
    break;

  case 81:
#line 728 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_TRY_EXCEPT, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(6) - (6)].node));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (6)].node));
                adopt_children ((yyval.node), (yyvsp[(4) - (6)].list));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (6)].node));
        }
    break;

  case 82:
#line 736 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_TRY_EXCEPT, (yyloc));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (5)].node));
                adopt_children ((yyval.node), (yyvsp[(4) - (5)].list));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (5)].node));
        }
    break;

  case 83:
#line 744 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_TRY_EXCEPT, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (5)].node));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                adopt_children ((yyval.node), (yyvsp[(4) - (5)].list));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (5)].node));
        }
    break;

  case 84:
#line 752 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_TRY_EXCEPT, (yyloc));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                adopt_children ((yyval.node), (yyvsp[(4) - (4)].list));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (4)].node));
        }
    break;

  case 85:
#line 760 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_TRY_FINALLY, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (4)].node));
        }
    break;

  case 86:
#line 769 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_EXCEPT, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(6) - (6)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (6)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (6)].node));
        }
    break;

  case 87:
#line 776 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_EXCEPT, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 88:
#line 782 "pcl-parser.y"
    {
                /* Just one child so that we can easily detect it. */
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_EXCEPT, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 89:
#line 791 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 90:
#line 795 "pcl-parser.y"
    {
                /* An expression-less 'except' clause must be last.  Make
                 * sure the previous 'except' clause has an expression. */
                GNode *previous = g_list_first ((yyvsp[(1) - (2)].list))->data;
                if (g_node_n_children (previous) == 1)
                {
                        yylloc.first_line = PCL_NODE_LINENO (previous);
                        yyerror ("default 'except' must be last");
                        YYABORT;
                }

                (yyval.list) = g_list_prepend ((yyvsp[(1) - (2)].list), (yyvsp[(2) - (2)].node));
        }
    break;

  case 91:
#line 812 "pcl-parser.y"
    {
                (yyval.node) = (yyvsp[(3) - (3)].node);
        }
    break;

  case 92:
#line 819 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_WHILE, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (5)].node));
        }
    break;

  case 93:
#line 826 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_WHILE, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 94:
#line 835 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_RETURN, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (2)].node));
        }
    break;

  case 95:
#line 840 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_RETURN, (yyloc));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 98:
#line 853 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_CONDITIONAL, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (5)].node));
        }
    break;

  case 100:
#line 864 "pcl-parser.y"
    {
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_TEST_OR)
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_TEST_OR, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
                g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 102:
#line 879 "pcl-parser.y"
    {
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_TEST_AND)
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_TEST_AND, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
                g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 104:
#line 894 "pcl-parser.y"
    {
                /* Check for 'not not expression' and simplify. */
                if (PCL_NODE_TYPE ((yyvsp[(2) - (2)].node)) == PCL_NODE_TYPE_TEST_NOT)
                {
                        (yyval.node) = g_node_first_child ((yyvsp[(2) - (2)].node));
                        pcl_node_destroy (PCL_NODE ((yyvsp[(2) - (2)].node)));
                        g_node_destroy ((yyvsp[(2) - (2)].node));
                }
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_TEST_NOT, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(2) - (2)].node));
                }
        }
    break;

  case 105:
#line 912 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_COMPARISON, (yyvsp[(2) - (2)].list), (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (2)].node));
        }
    break;

  case 107:
#line 921 "pcl-parser.y"
    {
                PCL_NODE_OPCODE ((yyvsp[(2) - (2)].node)) = (yyvsp[(1) - (2)].oper);
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(2) - (2)].node));
        }
    break;

  case 108:
#line 926 "pcl-parser.y"
    {
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = (yyvsp[(2) - (3)].oper);
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 109:
#line 934 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_COMPARISON_LT;
        }
    break;

  case 110:
#line 938 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_COMPARISON_LE;
        }
    break;

  case 111:
#line 942 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_COMPARISON_EQ;
        }
    break;

  case 112:
#line 946 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_COMPARISON_NE;
        }
    break;

  case 113:
#line 950 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_COMPARISON_GE;
        }
    break;

  case 114:
#line 954 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_COMPARISON_GT;
        }
    break;

  case 115:
#line 958 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_COMPARISON_IN;
        }
    break;

  case 116:
#line 962 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_COMPARISON_NOT_IN;
        }
    break;

  case 117:
#line 966 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_COMPARISON_IS;
        }
    break;

  case 118:
#line 970 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_COMPARISON_IS_NOT;
        }
    break;

  case 120:
#line 978 "pcl-parser.y"
    {
                g_assert (PCL_NODE_OPCODE ((yyvsp[(1) - (3)].node)) == 0);
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_BINARY_OPER)
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = PCL_OPCODE_BINARY_OR;
                g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 122:
#line 995 "pcl-parser.y"
    {
                g_assert (PCL_NODE_OPCODE ((yyvsp[(1) - (3)].node)) == 0);
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_BINARY_OPER)
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = PCL_OPCODE_BINARY_XOR;
                g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 124:
#line 1012 "pcl-parser.y"
    {
                g_assert (PCL_NODE_OPCODE ((yyvsp[(1) - (3)].node)) == 0);
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_BINARY_OPER)
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = PCL_OPCODE_BINARY_AND;
                g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 126:
#line 1029 "pcl-parser.y"
    {
                g_assert (PCL_NODE_OPCODE ((yyvsp[(1) - (3)].node)) == 0);
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_BINARY_OPER)
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = PCL_OPCODE_BINARY_LSH;
                g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 127:
#line 1042 "pcl-parser.y"
    {
                g_assert (PCL_NODE_OPCODE ((yyvsp[(1) - (3)].node)) == 0);
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_BINARY_OPER)
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = PCL_OPCODE_BINARY_RSH;
                g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 129:
#line 1059 "pcl-parser.y"
    {
                g_assert (PCL_NODE_OPCODE ((yyvsp[(1) - (3)].node)) == 0);
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_BINARY_OPER)
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = PCL_OPCODE_BINARY_ADD;
                g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 130:
#line 1072 "pcl-parser.y"
    {
                g_assert (PCL_NODE_OPCODE ((yyvsp[(1) - (3)].node)) == 0);
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_BINARY_OPER)
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = PCL_OPCODE_BINARY_SUB;
                g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 132:
#line 1089 "pcl-parser.y"
    {
                g_assert (PCL_NODE_OPCODE ((yyvsp[(1) - (3)].node)) == 0);
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_BINARY_OPER)
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = PCL_OPCODE_BINARY_MUL;
                g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 133:
#line 1102 "pcl-parser.y"
    {
                g_assert (PCL_NODE_OPCODE ((yyvsp[(1) - (3)].node)) == 0);
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_BINARY_OPER)
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = PCL_OPCODE_BINARY_DIV;
                g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 134:
#line 1115 "pcl-parser.y"
    {
                g_assert (PCL_NODE_OPCODE ((yyvsp[(1) - (3)].node)) == 0);
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_BINARY_OPER)
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = PCL_OPCODE_BINARY_MOD;
                g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 135:
#line 1128 "pcl-parser.y"
    {
                g_assert (PCL_NODE_OPCODE ((yyvsp[(1) - (3)].node)) == 0);
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_BINARY_OPER)
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = PCL_OPCODE_BINARY_FLD;
                g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 137:
#line 1145 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_UNARY_OPER, (yyloc));
                PCL_NODE_OPCODE ((yyvsp[(2) - (2)].node)) = PCL_OPCODE_UNARY_NEG;
                g_node_prepend ((yyval.node), (yyvsp[(2) - (2)].node));
        }
    break;

  case 138:
#line 1151 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_UNARY_OPER, (yyloc));
                PCL_NODE_OPCODE ((yyvsp[(2) - (2)].node)) = PCL_OPCODE_UNARY_POS;
                g_node_prepend ((yyval.node), (yyvsp[(2) - (2)].node));
        }
    break;

  case 139:
#line 1157 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_UNARY_OPER, (yyloc));
                PCL_NODE_OPCODE ((yyvsp[(2) - (2)].node)) = PCL_OPCODE_UNARY_INV;
                g_node_prepend ((yyval.node), (yyvsp[(2) - (2)].node));
        }
    break;

  case 141:
#line 1167 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, (yyloc));
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = PCL_OPCODE_BINARY_POW;
                g_node_prepend ((yyval.node), (yyvsp[(3) - (3)].node));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
        }
    break;

  case 147:
#line 1185 "pcl-parser.y"
    {
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_ATTRIBUTE)
                {
                        (yyval.node) = (yyvsp[(1) - (3)].node);
                        g_node_append ((yyval.node), (yyvsp[(3) - (3)].node));
                }
                else
                {
                        (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_ATTRIBUTE, (yyloc));
                        g_node_prepend ((yyval.node), (yyvsp[(3) - (3)].node));
                        g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
                }
        }
    break;

  case 148:
#line 1202 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_SUBSCRIPT, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (4)].node));
        }
    break;

  case 149:
#line 1211 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_SLICING, (yyloc));
                if ((yyvsp[(3) - (6)].node) == NULL)
                        (yyvsp[(3) - (6)].node) = NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc));
                if ((yyvsp[(5) - (6)].node) == NULL)
                        (yyvsp[(5) - (6)].node) = NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (6)].node));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (6)].node));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (6)].node));
        }
    break;

  case 150:
#line 1223 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_SLICING, (yyloc));
                if ((yyvsp[(3) - (8)].node) == NULL)
                        (yyvsp[(3) - (8)].node) = NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc));
                if ((yyvsp[(5) - (8)].node) == NULL)
                        (yyvsp[(5) - (8)].node) = NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc));
                if ((yyvsp[(7) - (8)].node) == NULL)
                        (yyvsp[(7) - (8)].node) = NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(7) - (8)].node));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (8)].node));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (8)].node));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (8)].node));
        }
    break;

  case 152:
#line 1241 "pcl-parser.y"
    {
                (yyval.node) = NULL;
        }
    break;

  case 154:
#line 1249 "pcl-parser.y"
    {
                (yyval.node) = NULL;
        }
    break;

  case 156:
#line 1257 "pcl-parser.y"
    {
                (yyval.node) = NULL;
        }
    break;

  case 165:
#line 1278 "pcl-parser.y"
    {
                (yyval.node) = (yyvsp[(2) - (3)].node);
        }
    break;

  case 166:
#line 1282 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_TUPLE, (yyloc));
        }
    break;

  case 167:
#line 1290 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(1) - (9)].list), (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(3) - (9)].list), (yyloc)),
                        (yyvsp[(6) - (9)].node), (yyvsp[(9) - (9)].node));
        }
    break;

  case 168:
#line 1299 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(1) - (6)].list), (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(3) - (6)].list), (yyloc)),
                        (yyvsp[(6) - (6)].node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 169:
#line 1308 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(1) - (6)].list), (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(3) - (6)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)), (yyvsp[(6) - (6)].node));
        }
    break;

  case 170:
#line 1317 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(1) - (3)].list), (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(3) - (3)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 171:
#line 1327 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(1) - (7)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        (yyvsp[(4) - (7)].node), (yyvsp[(7) - (7)].node));
        }
    break;

  case 172:
#line 1335 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(1) - (4)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        (yyvsp[(4) - (4)].node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 173:
#line 1343 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(1) - (4)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)), (yyvsp[(4) - (4)].node));
        }
    break;

  case 174:
#line 1351 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(1) - (1)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 175:
#line 1360 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(1) - (7)].list), (yyloc)),
                        (yyvsp[(4) - (7)].node), (yyvsp[(7) - (7)].node));
        }
    break;

  case 176:
#line 1368 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(1) - (4)].list), (yyloc)),
                        (yyvsp[(4) - (4)].node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 177:
#line 1376 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(1) - (4)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)), (yyvsp[(4) - (4)].node));
        }
    break;

  case 178:
#line 1384 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, (yyvsp[(1) - (1)].list), (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 179:
#line 1393 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        (yyvsp[(2) - (5)].node), (yyvsp[(5) - (5)].node));
        }
    break;

  case 180:
#line 1400 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        (yyvsp[(2) - (2)].node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 181:
#line 1407 "pcl-parser.y"
    {
                (yyval.list) = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)), (yyvsp[(2) - (2)].node));
        }
    break;

  case 182:
#line 1414 "pcl-parser.y"
    {
                yylloc.first_column = (yylsp[(5) - (5)]).first_column;
                yyerror ("non-keyword argument after keyword argument");
                YYABORT;
        }
    break;

  case 183:
#line 1420 "pcl-parser.y"
    {
                yylloc.first_column = (yylsp[(3) - (3)]).first_column;
                yyerror ("non-keyword argument after keyword argument");
                YYABORT;
        }
    break;

  case 184:
#line 1429 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 185:
#line 1433 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 186:
#line 1440 "pcl-parser.y"
    {
                (yyval.list) = NULL;
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(1) - (3)].node));
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 187:
#line 1446 "pcl-parser.y"
    {
                (yyval.list) = (yyvsp[(1) - (5)].list);
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(3) - (5)].node));
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(5) - (5)].node));
        }
    break;

  case 188:
#line 1456 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(10) - (10)].node));
                g_node_prepend ((yyval.node), (yyvsp[(8) - (10)].node));
                adopt_children ((yyval.node), (yyvsp[(5) - (10)].list));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (10)].node));
                adopt_name ((yyval.node), (yyvsp[(3) - (10)].node));
        }
    break;

  case 189:
#line 1466 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(9) - (9)].node));
                g_node_prepend ((yyval.node), (yyvsp[(7) - (9)].node));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (9)].node));
                adopt_name ((yyval.node), (yyvsp[(3) - (9)].node));
        }
    break;

  case 190:
#line 1479 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(8) - (8)].node));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                adopt_children ((yyval.node), (yyvsp[(5) - (8)].list));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (8)].node));
                adopt_name ((yyval.node), (yyvsp[(3) - (8)].node));
        }
    break;

  case 191:
#line 1488 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(7) - (7)].node));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (7)].node));
                adopt_name ((yyval.node), (yyvsp[(3) - (7)].node));
        }
    break;

  case 192:
#line 1501 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(9) - (9)].node));
                g_node_prepend ((yyval.node), (yyvsp[(7) - (9)].node));
                adopt_children ((yyval.node), (yyvsp[(4) - (9)].list));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                adopt_name ((yyval.node), (yyvsp[(2) - (9)].node));
        }
    break;

  case 193:
#line 1510 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(8) - (8)].node));
                g_node_prepend ((yyval.node), (yyvsp[(6) - (8)].node));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                adopt_name ((yyval.node), (yyvsp[(2) - (8)].node));
        }
    break;

  case 194:
#line 1522 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(7) - (7)].node));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                adopt_children ((yyval.node), (yyvsp[(4) - (7)].list));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                adopt_name ((yyval.node), (yyvsp[(2) - (7)].node));
        }
    break;

  case 195:
#line 1531 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(6) - (6)].node));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                adopt_name ((yyval.node), (yyvsp[(2) - (6)].node));
        }
    break;

  case 196:
#line 1546 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_ASSIGN, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (3)].node));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
        }
    break;

  case 197:
#line 1555 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_SUITE, (yyvsp[(3) - (4)].list), (yyloc));
        }
    break;

  case 198:
#line 1559 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_SUITE, (yyvsp[(1) - (3)].list), (yyloc));
        }
    break;

  case 202:
#line 1572 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 203:
#line 1576 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 207:
#line 1589 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 208:
#line 1593 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (2)].list), (yyvsp[(2) - (2)].node));
        }
    break;

  case 223:
#line 1617 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 224:
#line 1621 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 226:
#line 1629 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 227:
#line 1633 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 228:
#line 1637 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 230:
#line 1645 "pcl-parser.y"
    {
                (yyval.list) = g_list_concat ((yyvsp[(2) - (2)].list), (yyvsp[(1) - (2)].list));
        }
    break;

  case 231:
#line 1652 "pcl-parser.y"
    {
                (yyval.node) = (yyvsp[(3) - (3)].node);
        }
    break;

  case 232:
#line 1659 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_CONJUNCTION, (yyvsp[(3) - (4)].list), (yyloc));
                normalize_conjunction ((yyval.node));
        }
    break;

  case 233:
#line 1665 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_CONJUNCTION, (yyvsp[(1) - (3)].list), (yyloc));
                normalize_conjunction ((yyval.node));
        }
    break;

  case 234:
#line 1674 "pcl-parser.y"
    {
                (yyval.node) = (yyvsp[(3) - (3)].node);
        }
    break;

  case 235:
#line 1681 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_DISJUNCTION, (yyvsp[(3) - (4)].list), (yyloc));
        }
    break;

  case 236:
#line 1686 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_DISJUNCTION, (yyvsp[(1) - (3)].list), (yyloc));
        }
    break;

  case 237:
#line 1695 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (5)].node));
        }
    break;

  case 238:
#line 1703 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (5)].node));
        }
    break;

  case 239:
#line 1710 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 240:
#line 1720 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (5)].node));
        }
    break;

  case 241:
#line 1728 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(5) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (5)].node));
        }
    break;

  case 242:
#line 1735 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 243:
#line 1744 "pcl-parser.y"
    {
                (yyval.node) = (yyvsp[(3) - (3)].node);
        }
    break;

  case 244:
#line 1751 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_FOR, (yyloc));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (
                        PCL_NODE_TYPE_CONJUNCTION, (yyloc)));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 245:
#line 1762 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_GENERATOR, (yyvsp[(4) - (5)].list), (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (5)].node));
        }
    break;

  case 246:
#line 1768 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_GENERATOR, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 247:
#line 1777 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_FOR, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 248:
#line 1786 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (2)].node));
        }
    break;

  case 251:
#line 1799 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 252:
#line 1803 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (2)].list), (yyvsp[(2) - (2)].node));
        }
    break;

  case 253:
#line 1810 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_COMPREHENSION, (yyvsp[(4) - (5)].list), (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (5)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (5)].node));
        }
    break;

  case 254:
#line 1817 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_COMPREHENSION, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 255:
#line 1823 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_LIST, (yyvsp[(2) - (4)].list), (yyloc));
                PCL_NODE_FLAGS ((yyval.node)) |= (yyvsp[(3) - (4)].flag);
        }
    break;

  case 256:
#line 1828 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_LIST, (yyloc));
        }
    break;

  case 257:
#line 1835 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_FOR, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 258:
#line 1844 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IF, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (2)].node));
        }
    break;

  case 261:
#line 1857 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 262:
#line 1861 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (2)].list), (yyvsp[(2) - (2)].node));
        }
    break;

  case 263:
#line 1868 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_DICT, (yyvsp[(2) - (4)].list), (yyloc));
                PCL_NODE_FLAGS ((yyval.node)) |= ((yyvsp[(3) - (4)].flag) | PCL_NODE_FLAG_PAIRS);
        }
    break;

  case 264:
#line 1873 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_DICT, (yyloc));
        }
    break;

  case 265:
#line 1880 "pcl-parser.y"
    {
                /* The RAISE_EXCEPTION opcode takes three arguments.
                 * The first is AssertionError, the others are None.
                 * Queue them up here to keep the compiler simple. */
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_ASSERT, (yyloc));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (2)].node));
        }
    break;

  case 266:
#line 1890 "pcl-parser.y"
    {
                /* The RAISE_EXCEPTION opcode takes three arguments.
                 * The first is AssertionError, followed by $4 and None.
                 * Queue them up here to keep the compiler simple. */
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_ASSERT, (yyloc));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 267:
#line 1903 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_ASSIGN, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (2)].node));
                adopt_children ((yyval.node), (yyvsp[(1) - (2)].list));
        }
    break;

  case 268:
#line 1912 "pcl-parser.y"
    {
                /* Can't use target_or_tuple here because it causes a conflict
                 * with expression statements.  So we have to validate the
                 * expressions ourselves. */
                yylloc.first_column = (yylsp[(1) - (2)]).first_column;
                if (validate_target ((yyvsp[(1) - (2)].node)) == NULL)
                        YYABORT;
                yylloc.first_column = (yylsp[(2) - (2)]).first_column;
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (2)].node));
        }
    break;

  case 269:
#line 1923 "pcl-parser.y"
    {
                /* Can't use target_or_tuple here because it causes a conflict
                 * with expression statements.  So we have to validate the
                 * expressions ourselves. */
                yylloc.first_column = (yylsp[(2) - (3)]).first_column;
                if (validate_target ((yyvsp[(2) - (3)].node)) == NULL)
                        YYABORT;
                yylloc.first_column = (yylsp[(3) - (3)]).first_column;
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (3)].list), (yyvsp[(2) - (3)].node));
        }
    break;

  case 270:
#line 1937 "pcl-parser.y"
    {
                /* Some extra restrictions apply to the target. */
                if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_LIST)
                {
                        yylloc.first_column = (yylsp[(1) - (3)]).first_column;
                        yyerror ("augmented assign to list literal "
                                 "not possible");
                        YYABORT;
                }
                else if (PCL_NODE_TYPE ((yyvsp[(1) - (3)].node)) == PCL_NODE_TYPE_TUPLE)
                {
                        yylloc.first_column = (yylsp[(1) - (3)]).first_column;
                        yyerror ("augmented assign to tuple literal "
                                 "not possible");
                        YYABORT;
                }

                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_AUG_ASSIGN, (yyloc));
                PCL_NODE_OPCODE ((yyvsp[(3) - (3)].node)) = (yyvsp[(2) - (3)].oper);
                g_node_prepend ((yyval.node), (yyvsp[(3) - (3)].node));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
        }
    break;

  case 271:
#line 1963 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_INPLACE_ADD;
        }
    break;

  case 272:
#line 1967 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_INPLACE_SUB;
        }
    break;

  case 273:
#line 1971 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_INPLACE_MUL;
        }
    break;

  case 274:
#line 1975 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_INPLACE_DIV;
        }
    break;

  case 275:
#line 1979 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_INPLACE_MOD;
        }
    break;

  case 276:
#line 1983 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_INPLACE_POW;
        }
    break;

  case 277:
#line 1987 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_INPLACE_FLD;
        }
    break;

  case 278:
#line 1991 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_INPLACE_LSH;
        }
    break;

  case 279:
#line 1995 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_INPLACE_RSH;
        }
    break;

  case 280:
#line 1999 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_INPLACE_AND;
        }
    break;

  case 281:
#line 2003 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_INPLACE_XOR;
        }
    break;

  case 282:
#line 2007 "pcl-parser.y"
    {
                (yyval.oper) = PCL_OPCODE_INPLACE_OR;
        }
    break;

  case 283:
#line 2014 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_BREAK, (yyloc));
        }
    break;

  case 284:
#line 2021 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_CALL, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (2)].node));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (2)].node));
        }
    break;

  case 285:
#line 2027 "pcl-parser.y"
    {
                if ((yyvsp[(3) - (5)].list) == NULL)
                        YYABORT;  /* syntax error occurred */
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_CALL, (yyvsp[(3) - (5)].list), (yyloc));
                PCL_NODE_FLAGS ((yyval.node)) |= (yyvsp[(4) - (5)].flag);
                g_node_prepend ((yyval.node), (yyvsp[(1) - (5)].node));
        }
    break;

  case 286:
#line 2035 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_CALL, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(1) - (3)].node));
        }
    break;

  case 287:
#line 2043 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_CONTINUE, (yyloc));
        }
    break;

  case 288:
#line 2050 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_CUT, (yyloc));
        }
    break;

  case 289:
#line 2057 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_DEL, (yyvsp[(2) - (3)].list), (yyloc));
                PCL_NODE_FLAGS ((yyval.node)) |= (yyvsp[(3) - (3)].flag);
        }
    break;

  case 290:
#line 2069 "pcl-parser.y"
    {
                /* Third argument will be duped from the second. */
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_EXEC, (yyloc));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (2)].node));
        }
    break;

  case 291:
#line 2076 "pcl-parser.y"
    {
                /* Third argument will be duped from the second. */
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_EXEC, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 292:
#line 2083 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_EXEC, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(6) - (6)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (6)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (6)].node));
        }
    break;

  case 293:
#line 2093 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_GLOBAL, (yyvsp[(2) - (3)].list), (yyloc));
                PCL_NODE_FLAGS ((yyval.node)) |= (yyvsp[(3) - (3)].flag);
        }
    break;

  case 294:
#line 2101 "pcl-parser.y"
    {
                /* Even # children indicates module import. */
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_IMPORT, (yyvsp[(2) - (2)].list), (yyloc));
                PCL_NODE_FLAGS ((yyval.node)) |= PCL_NODE_FLAG_PAIRS;
        }
    break;

  case 295:
#line 2107 "pcl-parser.y"
    {
                /* Odd # children indicates 'from' form of import. */
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_IMPORT, (yyvsp[(4) - (4)].list), (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 296:
#line 2113 "pcl-parser.y"
    {
                yylloc.first_column = (yylsp[(5) - (5)]).first_column;
                yyerror ("trailing comma not allowed "
                        "without surrounding parentheses");
                YYABORT;
        }
    break;

  case 297:
#line 2120 "pcl-parser.y"
    {
                /* Odd # children indicates 'from' form of import. */
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_IMPORT, (yyvsp[(5) - (7)].list), (yyloc));
                PCL_NODE_FLAGS ((yyval.node)) |= (yyvsp[(6) - (7)].flag);
                g_node_prepend ((yyval.node), (yyvsp[(2) - (7)].node));
        }
    break;

  case 298:
#line 2127 "pcl-parser.y"
    {
                /* One child indicates 'from <module> import *'. */
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_IMPORT, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 299:
#line 2136 "pcl-parser.y"
    {
                (yyval.list) = NULL;
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(1) - (3)].node));
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 300:
#line 2142 "pcl-parser.y"
    {
                (yyval.list) = NULL;
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(1) - (1)].node));
                (yyval.list) = g_list_prepend ((yyval.list), NEW_TREE_NODE (
                        PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 301:
#line 2149 "pcl-parser.y"
    {
                (yyval.list) = (yyvsp[(1) - (5)].list);
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(3) - (5)].node));
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(5) - (5)].node));
        }
    break;

  case 302:
#line 2155 "pcl-parser.y"
    {
                (yyval.list) = (yyvsp[(1) - (3)].list);
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(3) - (3)].node));
                (yyval.list) = g_list_prepend ((yyval.list), NEW_TREE_NODE (
                        PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 303:
#line 2165 "pcl-parser.y"
    {
                (yyval.list) = NULL;
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(1) - (3)].node));
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 304:
#line 2171 "pcl-parser.y"
    {
                (yyval.list) = NULL;
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(1) - (1)].node));
                (yyval.list) = g_list_prepend ((yyval.list), NEW_TREE_NODE (
                        PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 305:
#line 2178 "pcl-parser.y"
    {
                (yyval.list) = (yyvsp[(1) - (5)].list);
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(3) - (5)].node));
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(5) - (5)].node));
        }
    break;

  case 306:
#line 2184 "pcl-parser.y"
    {
                (yyval.list) = (yyvsp[(1) - (3)].list);
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(3) - (3)].node));
                (yyval.list) = g_list_prepend ((yyval.list), NEW_TREE_NODE (
                        PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 307:
#line 2194 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_MODULE, (yyvsp[(1) - (1)].list), (yyloc));
        }
    break;

  case 308:
#line 2201 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 309:
#line 2205 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 310:
#line 2212 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_PASS, (yyloc));
        }
    break;

  case 311:
#line 2219 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_PRINT, (yyvsp[(5) - (6)].list), (yyloc));
                PCL_NODE_FLAGS ((yyval.node)) |= (yyvsp[(6) - (6)].flag);
                g_node_prepend ((yyval.node), (yyvsp[(3) - (6)].node));
        }
    break;

  case 312:
#line 2225 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_PRINT, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(3) - (3)].node));
        }
    break;

  case 313:
#line 2230 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_PRINT, (yyvsp[(2) - (3)].list), (yyloc));
                PCL_NODE_FLAGS ((yyval.node)) |= (yyvsp[(3) - (3)].flag);
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 314:
#line 2236 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_PRINT, (yyloc));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 315:
#line 2244 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_RAISE, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(6) - (6)].node));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (6)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (6)].node));
        }
    break;

  case 316:
#line 2251 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_RAISE, (yyloc));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), (yyvsp[(4) - (4)].node));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (4)].node));
        }
    break;

  case 317:
#line 2258 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_RAISE, (yyloc));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (2)].node));
        }
    break;

  case 318:
#line 2265 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_RAISE, (yyloc));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
                g_node_prepend ((yyval.node), NEW_TREE_NODE (PCL_NODE_TYPE_NONE, (yyloc)));
        }
    break;

  case 319:
#line 2275 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_STATIC, (yyvsp[(2) - (3)].list), (yyloc));
                PCL_NODE_FLAGS ((yyval.node)) |= (yyvsp[(3) - (3)].flag);
        }
    break;

  case 320:
#line 2283 "pcl-parser.y"
    {
                (yyval.node) = NEW_TREE_NODE (PCL_NODE_TYPE_YIELD, (yyloc));
                g_node_prepend ((yyval.node), (yyvsp[(2) - (2)].node));
        }
    break;

  case 321:
#line 2291 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 322:
#line 2295 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 323:
#line 2302 "pcl-parser.y"
    {
                if (g_list_next ((yyvsp[(1) - (2)].list)) != NULL || (yyvsp[(2) - (2)].flag))
                {
                        (yyval.node) = NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_TUPLE, (yyvsp[(1) - (2)].list), (yyloc));
                        PCL_NODE_FLAGS ((yyval.node)) |= (yyvsp[(2) - (2)].flag);
                }
                else
                {
                        (yyval.node) = (yyvsp[(1) - (2)].list)->data;
                        (yyvsp[(1) - (2)].list) = g_list_delete_link ((yyvsp[(1) - (2)].list), (yyvsp[(1) - (2)].list));
                        g_assert ((yyvsp[(1) - (2)].list) == NULL);
                }
        }
    break;

  case 324:
#line 2320 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 325:
#line 2324 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 326:
#line 2331 "pcl-parser.y"
    {
                /* excludes conditionals from list */
                if (g_list_next ((yyvsp[(1) - (2)].list)) != NULL || (yyvsp[(2) - (2)].flag))
                {
                        (yyval.node) = NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_TUPLE, (yyvsp[(1) - (2)].list), (yyloc));
                        PCL_NODE_FLAGS ((yyval.node)) |= (yyvsp[(2) - (2)].flag);
                }
                else
                {
                        (yyval.node) = (yyvsp[(1) - (2)].list)->data;
                        (yyvsp[(1) - (2)].list) = g_list_delete_link ((yyvsp[(1) - (2)].list), (yyvsp[(1) - (2)].list));
                        g_assert ((yyvsp[(1) - (2)].list) == NULL);
                }
        }
    break;

  case 327:
#line 2350 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 328:
#line 2354 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 329:
#line 2361 "pcl-parser.y"
    {
                (yyval.list) = NULL;
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(1) - (3)].node));
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 330:
#line 2367 "pcl-parser.y"
    {
                (yyval.list) = (yyvsp[(1) - (5)].list);
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(3) - (5)].node));
                (yyval.list) = g_list_prepend ((yyval.list), (yyvsp[(5) - (5)].node));
        }
    break;

  case 331:
#line 2376 "pcl-parser.y"
    {
                if (g_list_next ((yyvsp[(1) - (2)].list)) != NULL || (yyvsp[(2) - (2)].flag))
                {
                        (yyval.node) = NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_TUPLE, (yyvsp[(1) - (2)].list), (yyloc));
                        PCL_NODE_FLAGS ((yyval.node)) |= (yyvsp[(2) - (2)].flag);
                }
                else
                {
                        (yyval.node) = (yyvsp[(1) - (2)].list)->data;
                        (yyvsp[(1) - (2)].list) = g_list_delete_link ((yyvsp[(1) - (2)].list), (yyvsp[(1) - (2)].list));
                        g_assert ((yyvsp[(1) - (2)].list) == NULL);
                }
        }
    break;

  case 332:
#line 2394 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend (NULL, (yyvsp[(1) - (1)].node));
        }
    break;

  case 333:
#line 2398 "pcl-parser.y"
    {
                (yyval.list) = g_list_prepend ((yyvsp[(1) - (3)].list), (yyvsp[(3) - (3)].node));
        }
    break;

  case 334:
#line 2405 "pcl-parser.y"
    {
                if (((yyval.node) = validate_target ((yyvsp[(1) - (1)].node))) == NULL)
                        YYABORT;
        }
    break;

  case 335:
#line 2413 "pcl-parser.y"
    {
                (yyval.flag) = PCL_NODE_FLAG_COMMA;
        }
    break;

  case 336:
#line 2417 "pcl-parser.y"
    {
                (yyval.flag) = 0;
        }
    break;

  case 337:
#line 2424 "pcl-parser.y"
    {
                (yyval.flag) = 1;
        }
    break;

  case 338:
#line 2428 "pcl-parser.y"
    {
                (yyval.flag) = 0;
        }
    break;


/* Line 1267 of yacc.c.  */
#line 4777 "pcl-parser.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 2433 "pcl-parser.y"


static GNode *
adopt_children (GNode *parent, GList *children)
{
        GList *child = children;
        g_assert (parent != NULL);
        while (child != NULL)
        {
                GNode *node = child->data;
                g_node_prepend (parent, node);
                child = g_list_next (child);
        }
        g_list_free (children);
        return parent;
}

static GNode *
adopt_name (GNode *parent, GNode *name)
{
        g_assert (parent != NULL && name != NULL);
        g_assert (PCL_NODE_FLAGS (name) & PCL_NODE_FLAG_INTERNED);
        PCL_NODE_STRING (parent) = PCL_NODE_STRING (name);
        PCL_NODE_FLAGS (parent) |= PCL_NODE_FLAG_INTERNED;
        pcl_node_destroy (PCL_NODE (name));
        g_node_destroy (name);
        return parent;
}

static void  /* helper for normalize_conjunction */
copy_to_disjunct (GNode *compound, GNode *original)
{
        GNode *child, *conjunct, *copy;

        switch (PCL_NODE_TYPE (compound))
        {
                case PCL_NODE_TYPE_DISJUNCTION:
                        conjunct = g_node_first_child (compound);
                        g_assert (conjunct != NULL);
                        break;
                case PCL_NODE_TYPE_IF:
                        conjunct = g_node_nth_child (compound, 1);
                        g_assert (conjunct != NULL);
                        break;
                case PCL_NODE_TYPE_FOR:
                        conjunct = g_node_last_child (compound);
                        g_assert (conjunct != NULL);
                        break;
                default:
                        conjunct = NULL;  /* silence compiler warning */
                        g_assert_not_reached ();
        }

        child = g_node_last_child (conjunct);
        g_assert (child != NULL);

        switch (PCL_NODE_TYPE (child))
        {
                case PCL_NODE_TYPE_DISJUNCTION:
                case PCL_NODE_TYPE_FOR:
                case PCL_NODE_TYPE_IF:
                        copy_to_disjunct (child, original);
                        break;
                default:
                        g_node_append (conjunct, original);
                        break;
        }

        conjunct = g_node_next_sibling (conjunct);
        if (conjunct != NULL && PCL_NODE_TYPE (conjunct) == PCL_NODE_TYPE_IF)
                conjunct = g_node_nth_child (conjunct, 1);

        while (conjunct != NULL)
        {
                g_assert (PCL_NODE_TYPE (conjunct) ==
                        PCL_NODE_TYPE_CONJUNCTION);

                copy = g_node_copy_deep (original,
                       (GCopyFunc) pcl_node_copy, NULL);

                child = g_node_last_child (conjunct);
                g_assert (child != NULL);

                switch (PCL_NODE_TYPE (child))
                {
                        case PCL_NODE_TYPE_DISJUNCTION:
                        case PCL_NODE_TYPE_FOR:
                        case PCL_NODE_TYPE_IF:
                                copy_to_disjunct (child, copy);
                                break;
                        default:
                                g_node_append (conjunct, copy);
                                break;
                }

                conjunct = g_node_next_sibling (conjunct);
                if (conjunct != NULL &&
                                PCL_NODE_TYPE (conjunct) == PCL_NODE_TYPE_IF)
                        conjunct = g_node_nth_child (conjunct, 1);
        }
}

static gboolean  /* helper for normalize_conjunction */
normalize_cut (GNode *node)
{
        GNode *ancestor = node;
        gint depth = 0;

        /* Only interested in cut nodes that we tagged. */
        if (PCL_NODE_TYPE (node) != PCL_NODE_TYPE_CUT)
                return FALSE;
        if (PCL_NODE_OPCODE (node) >= 0)
                return FALSE;

        /* Count the number of ancestor nodes that are conjunctions
         * (excluding the root node).  This is the call stack depth
         * from the conjunction that this cut is bound to.  Replace
         * the cut node's tag value with the call stack depth. */
        while (!G_NODE_IS_ROOT (ancestor))
        {
                if (PCL_NODE_TYPE (ancestor) == PCL_NODE_TYPE_CONJUNCTION)
                        depth++;
                ancestor = ancestor->parent;
        }
        PCL_NODE_OPCODE (node) = depth;

        return FALSE;
}

static void
normalize_conjunction (GNode *node)
{
        /* "Normalizing" a conjunction involves rearranging the parse tree
         * such that only the last child of a conjunction node may be a
         * disjunction or for-loop, and any nested conjunctions are also
         * normalized.  This process may involve replicating portions of the
         * conjunction's parse tree, which is fairly expensive. */

        GNode *ii, *jj, *dest, *temp;

        g_assert (G_NODE_IS_ROOT (node));

        ii = g_node_last_child (node);

        /* Append a "pass" node if the last child is a "for" node.  This gives
         * us something to collect and put in the "for" node's conjunction,
         * since an empty conjunction is illegal. */
        if (ii != NULL && PCL_NODE_TYPE (ii) == PCL_NODE_TYPE_FOR)
        {
                temp = g_node_new (pcl_node_new (PCL_NODE_TYPE_PASS, 0, 0));
                ii = g_node_append (node, temp);
        }

        while (ii != NULL)
        {
                switch (PCL_NODE_TYPE (ii))
                {
                        case PCL_NODE_TYPE_CUT:
                                /* Tag it so we can find it again later. */
                                PCL_NODE_OPCODE (ii) = -1;
                                break;

                        case PCL_NODE_TYPE_DISJUNCTION:
                        case PCL_NODE_TYPE_IF:
                                jj = g_node_next_sibling (ii);
                                while (jj != NULL)
                                {
                                        temp = jj;
                                        jj = g_node_next_sibling (jj);
                                        g_node_unlink (temp);
                                        copy_to_disjunct (ii, temp);
                                }
                                break;

                        case PCL_NODE_TYPE_FOR:
                                dest = g_node_last_child (ii);
                                jj = g_node_next_sibling (ii);
                                while (jj != NULL)
                                {
                                        temp = jj;
                                        jj = g_node_next_sibling (jj);
                                        g_node_unlink (temp);
                                        g_node_prepend (dest, temp);
                                }
                                g_node_reverse_children (dest);
                                break;

                        default:
                                break;
                }
                ii = g_node_prev_sibling (ii);
        }

        g_node_traverse (
                node, G_IN_ORDER, G_TRAVERSE_ALL, -1,
                (GNodeTraverseFunc) normalize_cut, NULL);
}

static GList *
process_arguments (GNode *pa, GNode *ka, GNode *epa, GNode *eka)
{
        GList *arguments = NULL;
        guint na = g_node_n_children (pa);
        guint nk = g_node_n_children (ka) / 2;

        /* Abbreviations:
         * pa  = Positional Arguments             foo(a, b, c)
         * ka  = Keyword Arguments                foo(x=a, y=b, z=c)
         * epa = Extended Positional Arguments    foo(*args)
         * eka = Extended Keyword Arguments       foo(**kwds)
         * na  = # Positional Arguments
         * nk  = # Keyword Arguments (in pairs)
         */

        if (na > 255 || nk > 255)
        {
                yyerror ("more than 255 arguments");
                return NULL;
        }

        if (nk > 1)
        {
                GNode *child = g_node_first_child (ka);
                gboolean found_dup = FALSE;
                const gchar *name;
                GData *data;
                
                g_datalist_init (&data);
                while (child != NULL)
                {
                        name = PCL_NODE_STRING (child);
                        if (g_datalist_get_data (&data, name) != NULL)
                        {
                                found_dup = TRUE;
                                break;
                        }
                        g_datalist_set_data (&data, name, child);
                        child = g_node_next_sibling (child);
                        child = g_node_next_sibling (child);
                }
                g_datalist_clear (&data);

                if (found_dup)
                {
                        yyerror ("duplicate keyword argument");
                        return NULL;
                }
        }

        if (PCL_NODE_TYPE (ka) == PCL_NODE_TYPE_ARGUMENTS)
                PCL_NODE_FLAGS (ka) |= PCL_NODE_FLAG_PAIRS;

        arguments = g_list_prepend (arguments, pa);
        arguments = g_list_prepend (arguments, ka);
        arguments = g_list_prepend (arguments, epa);
        arguments = g_list_prepend (arguments, eka);

        return arguments;
}

static GList *
process_parameters (GNode *pp, GNode *dp, GNode *epp, GNode *ekp)
{
        GList *parameters = NULL;

        /* Abbreviations:
         * pp  = Positional Parameters            def foo(x)
         * dp  = Default Parameters               def foo(x=0)
         * epp = Extended Positional Parameters   def foo(*x)
         * ekp = Extended Keyword Parameters      def foo(**x)
         */

        if (PCL_NODE_TYPE (dp) == PCL_NODE_TYPE_PARAMETERS)
                PCL_NODE_FLAGS (dp) |= PCL_NODE_FLAG_PAIRS;

        parameters = g_list_prepend (parameters, pp);
        parameters = g_list_prepend (parameters, dp);
        parameters = g_list_prepend (parameters, epp);
        parameters = g_list_prepend (parameters, ekp);

        return parameters;
}

static void
stream_to_debug (FILE *stream, const gchar *format, ...)
{
        /* Intercept message segments bound for `stream' and emit a
         * debug-level logging message when we see a newline. */

        static gchar buffer[512];
        static gchar *end = buffer + sizeof (buffer);
        static gchar *next = buffer;
        va_list args;
        gchar *cp;

        va_start (args, format);
        g_assert (next < end);
        next += g_vsnprintf (next, end - next, format, args);
        cp = strchr (buffer, '\n');
        if (cp != NULL)
        {
                *cp++ = '\0';
                g_debug ("%s", buffer);
                next = g_stpcpy (buffer, cp);
        }
        va_end (args);
}

static GNode *
validate_target (GNode *target)
{
        GNode *child;

        switch (PCL_NODE_TYPE (target))
        {
                case PCL_NODE_TYPE_ATTRIBUTE:
                case PCL_NODE_TYPE_SLICING:
                case PCL_NODE_TYPE_SUBSCRIPT:
                        break;

                case PCL_NODE_TYPE_CALL:
                        /* XXX Also: can't delete function call */
                        yyerror ("can't assign to function call");
                        return NULL;

                case PCL_NODE_TYPE_COMPARISON:
                case PCL_NODE_TYPE_UNARY_OPER:
                case PCL_NODE_TYPE_BINARY_OPER:
                case PCL_NODE_TYPE_TEST_AND:
                case PCL_NODE_TYPE_TEST_NOT:
                case PCL_NODE_TYPE_TEST_OR:
                        yyerror ("can't assign to operator");
                        return NULL;

                case PCL_NODE_TYPE_COMPREHENSION:
                        yyerror ("can't assign to list comprehension");
                        return NULL;

                case PCL_NODE_TYPE_CONDITIONAL:
                        yyerror ("can't assign to conditional");
                        return NULL;

                case PCL_NODE_TYPE_GENERATOR:
                        yyerror ("can't assign to generator expression");
                        return NULL;

                case PCL_NODE_TYPE_LIST:
                case PCL_NODE_TYPE_TUPLE:
                        child = g_node_first_child (target);
                        if (child == NULL)
                        {
                                if (PCL_NODE_TYPE (target) ==
                                                PCL_NODE_TYPE_LIST)
                                        yyerror ("can't assign to []");
                                else
                                        yyerror ("can't assign to ()");
                                return NULL;
                        }
                        while (child != NULL)
                        {
                                validate_target (child);
                                child = g_node_next_sibling (child);
                        }
                        break;

                case PCL_NODE_TYPE_NAME:
                {
                        const gchar *name = PCL_NODE_STRING (target);
                        if (strcmp (name, "__debug__") == 0)
                        {
                                yyerror ("can't assign to __debug__");
                                return NULL;
                        }
                        break;
                }

                case PCL_NODE_TYPE_FLOAT:
                case PCL_NODE_TYPE_IMAGINARY:
                case PCL_NODE_TYPE_INTEGER:
                case PCL_NODE_TYPE_STRING:
                        yyerror ("can't assign to literal");
                        return NULL;

                default:
                        g_printerr ("%s: forgot to account for %s", G_STRFUNC,
                                pcl_node_type_name (PCL_NODE_TYPE (target)));
                        g_assert_not_reached ();
        }

        return target;
}

static void
yyprint (FILE *stream, gint type, YYSTYPE value)
{
        if (type == NAME || type == NUMBER || type == STRING)
                YYFPRINTF (stream, "%s", PCL_NODE_STRING (value.node));
}


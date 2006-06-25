/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 47 "pcl-parser.y"
{
        gint flag;
        gint oper;
        GList *list;
        GNode *node;
}
/* Line 1529 of yacc.c.  */
#line 188 "pcl-parser.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

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

extern YYLTYPE yylloc;

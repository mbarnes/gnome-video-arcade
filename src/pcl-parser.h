#ifndef BISON_PCL_PARSER_H
# define BISON_PCL_PARSER_H

#ifndef YYSTYPE
typedef union {
        gint flag;
        gint oper;
        GList *list;
        GNode *node;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif

#ifndef YYLTYPE
typedef struct yyltype
{
  int first_line;
  int first_column;

  int last_line;
  int last_column;
} yyltype;

# define YYLTYPE yyltype
# define YYLTYPE_IS_TRIVIAL 1
#endif

# define	FILE_INPUT	257
# define	EVAL_INPUT	258
# define	SINGLE_INPUT	259
# define	KW_AND	260
# define	KW_AS	261
# define	KW_ASSERT	262
# define	KW_BREAK	263
# define	KW_CLASS	264
# define	KW_CONTINUE	265
# define	KW_CUT	266
# define	KW_DEF	267
# define	KW_DEL	268
# define	KW_ELIF	269
# define	KW_ELSE	270
# define	KW_EXCEPT	271
# define	KW_EXEC	272
# define	KW_FINALLY	273
# define	KW_FOR	274
# define	KW_FROM	275
# define	KW_GLOBAL	276
# define	KW_IF	277
# define	KW_IMPORT	278
# define	KW_IN	279
# define	KW_IS	280
# define	KW_IS_NOT	281
# define	KW_NOT	282
# define	KW_NOT_IN	283
# define	KW_OR	284
# define	KW_PASS	285
# define	KW_PREDICATE	286
# define	KW_PRINT	287
# define	KW_RAISE	288
# define	KW_RETURN	289
# define	KW_STATIC	290
# define	KW_TRY	291
# define	KW_WHILE	292
# define	KW_YIELD	293
# define	OP_INPLACE_ADD	294
# define	OP_INPLACE_SUB	295
# define	OP_INPLACE_MUL	296
# define	OP_INPLACE_DIV	297
# define	OP_INPLACE_MOD	298
# define	OP_INPLACE_POW	299
# define	OP_INPLACE_FLD	300
# define	OP_INPLACE_LSH	301
# define	OP_INPLACE_RSH	302
# define	OP_INPLACE_AND	303
# define	OP_INPLACE_XOR	304
# define	OP_INPLACE_OR	305
# define	OP_FLD	306
# define	OP_LSH	307
# define	OP_RSH	308
# define	OP_POW	309
# define	OP_LE	310
# define	OP_GE	311
# define	OP_EQ	312
# define	OP_NE	313
# define	DEDENT	314
# define	INDENT	315
# define	NEWLINE	316
# define	END_OF_FILE	0
# define	NUMBER	317
# define	NAME	318
# define	STRING	319


extern YYSTYPE yylval;

#endif /* not BISON_PCL_PARSER_H */

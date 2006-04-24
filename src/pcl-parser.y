/* PCL - Predicate Constraint Language
 * Copyright (C) 2006 The Boeing Company
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

%{
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
%}

%debug
%defines
%locations
%verbose

%union {
        gint flag;
        gint oper;
        GList *list;
        GNode *node;
};

/* Start Tokens */
%token FILE_INPUT
%token EVAL_INPUT
%token SINGLE_INPUT

/* Keywords */
%token KW_AND                   "and"
%token KW_AS                    "as"
%token KW_ASSERT                "assert"
%token KW_BREAK                 "break"
%token KW_CLASS                 "class"
%token KW_CONTINUE              "continue"
%token KW_CUT                   "cut"
%token KW_DEF                   "def"
%token KW_DEL                   "del"
%token KW_ELIF                  "elif"
%token KW_ELSE                  "else"
%token KW_EXCEPT                "except"
%token KW_EXEC                  "exec"
%token KW_FINALLY               "finally"
%token KW_FOR                   "for"
%token KW_FROM                  "from"
%token KW_GLOBAL                "global"
%token KW_IF                    "if"
%token KW_IMPORT                "import"
%token KW_IN                    "in"
%token KW_IS                    "is"
%token KW_IS_NOT                "is not"
%token KW_NOT                   "not"
%token KW_NOT_IN                "not in"
%token KW_OR                    "or"
%token KW_PASS                  "pass"
%token KW_PREDICATE             "predicate"
%token KW_PRINT                 "print"
%token KW_RAISE                 "raise"
%token KW_RETURN                "return"
%token KW_STATIC                "static"
%token KW_TRY                   "try"
%token KW_WHILE                 "while"
%token KW_YIELD                 "yield"

/* Operators */
%token OP_INPLACE_ADD           "+="
%token OP_INPLACE_SUB           "-="
%token OP_INPLACE_MUL           "*="
%token OP_INPLACE_DIV           "/="
%token OP_INPLACE_MOD           "%="
%token OP_INPLACE_POW           "**="
%token OP_INPLACE_FLD           "//="
%token OP_INPLACE_LSH           "<<="
%token OP_INPLACE_RSH           ">>="
%token OP_INPLACE_AND           "&="
%token OP_INPLACE_XOR           "^="
%token OP_INPLACE_OR            "|-"
%token OP_FLD                   "//"
%token OP_LSH                   "<<"
%token OP_RSH                   ">>"
%token OP_POW                   "**"
%token OP_LE                    "<="
%token OP_GE                    ">="
%token OP_EQ                    "=="
%token OP_NE                    "!="

/* Whitespace */
%token DEDENT
%token INDENT
%token NEWLINE

/* End of File */
%token END_OF_FILE 0            "end of file"

%token <node> NUMBER            "number"
%token <node> NAME              "name"
%token <node> STRING            "string"

%type <node> start
%type <node> class_definition
%type <node> inheritance
%type <node> function_definition
%type <node> decorators
%type <node> decorator
%type <node> decorator_name
%type <list> parameter_list
%type <list> parameters
%type <list> default_parameters
%type <list> statement
%type <node> simple_statement
%type <list> simple_statement_list
%type <node> expression_statement
%type <node> suite
%type <node> compound_statement
%type <list> statement_list
%type <node> if_statement
%type <node> elif_clause
%type <node> else_clause
%type <node> for_statement
%type <node> try_statement
%type <node> except_clause
%type <list> except_clause_list
%type <node> finally_clause
%type <node> while_statement
%type <node> return_statement
%type <node> expression
%type <node> conditional
%type <node> or_test
%type <node> and_test
%type <node> not_test
%type <node> or_expr
%type <node> xor_expr
%type <node> and_expr
%type <node> comparison
%type <list> comp_list
%type <oper> comp_operator
%type <node> shift_expr
%type <node> a_expr
%type <node> m_expr
%type <node> u_expr
%type <node> power
%type <node> primary
%type <node> attribute
%type <node> subscription
%type <node> slicing
%type <node> lower_bound
%type <node> upper_bound
%type <node> stride
%type <node> atom
%type <node> enclosure
%type <node> parenth_form
%type <list> argument_list
%type <list> positional_arguments
%type <list> keyword_arguments

%type <node> predicate_definition
%type <node> predicate_goal
%type <node> predicate_suite
%type <node> simple_predicate_statement
%type <list> simple_predicate_statement_list
%type <node> predicate_term
%type <list> predicate_term_list
%type <node> predicate_simple_term
%type <list> predicate_simple_term_list
%type <list> predicate_statement
%type <list> predicate_statement_list
%type <node> predicate_conjunction
%type <node> predicate_conjunction_suite
%type <node> predicate_disjunction
%type <node> predicate_disjunction_suite
%type <node> predicate_if_statement
%type <node> predicate_elif_clause
%type <node> predicate_else_clause
%type <node> predicate_for_statement
%type <node> generator_expression
%type <node> generator_for_clause
%type <node> generator_if_clause
%type <node> generator_clause
%type <list> generator_clause_list
%type <node> list_display
%type <node> list_for_clause
%type <node> list_if_clause
%type <node> list_clause
%type <list> list_clause_list
%type <node> dict_display

%type <node> assert_statement
%type <node> assignment_statement
%type <list> target_assignment_list
%type <node> augmented_assignment_statement
%type <oper> augmented_assignment_operator
%type <node> break_statement
%type <node> call
%type <node> continue_statement
%type <node> cut_statement
%type <node> delete_statement
%type <node> execute_statement
%type <node> global_statement
%type <node> import_statement
%type <list> import_list
%type <list> module_list
%type <node> module
%type <list> identifier_list
%type <node> pass_statement
%type <node> print_statement
%type <node> raise_statement
%type <node> static_statement
%type <node> yield_statement
%type <list> dotted_name
%type <node> expression_or_tuple
%type <list> expression_list
%type <node> or_test_or_tuple
%type <list> or_test_list
%type <list> key_datum_list
%type <node> target_or_tuple
%type <list> target_list
%type <node> target

%type <flag> opt_comma
%type <flag> opt_semi

%{
static GNode *adopt_children (GNode *parent, GList *children);
static GNode *adopt_name (GNode *parent, GNode *name);
static void normalize_conjunction (GNode *conjunct);
static GList *process_arguments (GNode *pa, GNode *ka, GNode *epa, GNode *eka);
static GList *process_parameters (GNode *pp, GNode *dp, GNode *epp, GNode *ekp);
static void stream_to_debug (FILE *stream, const gchar *format, ...);
static GNode *validate_target (GNode *target);
static void yyprint (FILE *stream, gint type, YYSTYPE value);
%}

%%

start
        : FILE_INPUT statement_list
        {
                $$ = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_FILE_INPUT, $2, @$);
                yylval.node = $$;  /* for lexer to grab */
        }
        | EVAL_INPUT expression_or_tuple NEWLINE
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_EVAL_INPUT, @$);
                g_node_prepend ($$, $2);
                yylval.node = $$;  /* for lexer to grab */
        }
        | SINGLE_INPUT NEWLINE
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_EVAL_INPUT, @$);
                yylval.node = $$;  /* for lexer to grab */
                YYACCEPT;
        }
        | SINGLE_INPUT compound_statement NEWLINE
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_SINGLE_INPUT, @$);
                g_node_prepend ($$, $2);
                yylval.node = $$;  /* for lexer to grab */
                YYACCEPT;
        }
        | SINGLE_INPUT simple_statement_list opt_semi NEWLINE
        {
                $$ = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_SINGLE_INPUT, $2, @$);
                yylval.node = $$;  /* for lexer to grab */
                YYACCEPT;
        }
        |
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_FILE_INPUT, @$);
                yylval.node = $$;  /* for lexer to grab */
                YYACCEPT;
        }
        ;

class_definition
        : KW_CLASS NAME inheritance ':' suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_CLASS, @$);
                g_node_prepend ($$, $5);
                g_node_prepend ($$, $3);
                adopt_name ($$, $2);
        }
        ;

inheritance
        : '(' expression_list opt_comma ')'
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_TUPLE, $2, @$);
                PCL_NODE_FLAGS ($$) |= $3;
        }
        ;

function_definition
        : decorators KW_DEF NAME '(' parameter_list ')' ':' suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_FUNCTION, @$);
                g_node_prepend ($$, $8);
                adopt_children ($$, $5);
                g_node_prepend ($$, $1);
                adopt_name ($$, $3);
        }
        | decorators KW_DEF NAME '(' ')' ':' suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_FUNCTION, @$);
                g_node_prepend ($$, $7);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, $1);
                adopt_name ($$, $3);
        }
        | KW_DEF NAME '(' parameter_list ')' ':' suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_FUNCTION, @$);
                g_node_prepend ($$, $7);
                adopt_children ($$, $4);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                adopt_name ($$, $2);
        }
        | KW_DEF NAME '(' ')' ':' suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_FUNCTION, @$);
                g_node_prepend ($$, $6);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                adopt_name ($$, $2);
        }
        ;

decorators
        : decorator
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_DECORATORS, @$);
                g_node_append ($$, $1);
        }
        | decorators decorator
        {
                $$ = $1;
                g_node_append ($$, $2);
        }
        ;

decorator
        : '@' decorator_name '(' argument_list opt_comma ')' NEWLINE
        {
                if ($4 == NULL)
                        YYABORT;  /* syntax error occurred */
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_CALL, $4, @$);
                PCL_NODE_FLAGS ($$) |= $5;
                g_node_prepend ($$, $2);
        }
        | '@' decorator_name '(' ')' NEWLINE
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_CALL, @$);
                g_node_prepend ($$, $2);
        }
        | '@' decorator_name NEWLINE
        {
                $$ = $2;
        }
        ;

decorator_name
        : dotted_name
        {
                if (g_list_next ($1) != NULL)
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_ATTRIBUTE, @$);
                        adopt_children ($$, $1);
                }
                else
                {
                        $$ = $1->data;
                        $1 = g_list_delete_link ($1, $1);
                        g_assert ($1 == NULL);
                }
        }
        ;

parameter_list
        : parameters ',' default_parameters ',' '*' NAME ',' OP_POW NAME
        {
                $$ = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $1, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $3, @$),
                        $6, $9);
        }
        | parameters ',' default_parameters ',' '*' NAME
        {
                $$ = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $1, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $3, @$),
                        $6, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | parameters ',' default_parameters ',' OP_POW NAME
        {
                $$ = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $1, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $3, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$), $6);
        }
        | parameters ',' default_parameters
        {
                $$ = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $1, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $3, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | parameters ',' '*' NAME ',' OP_POW NAME
        {
                $$ = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $1, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                         $4, $7);
        }
        | parameters ',' '*' NAME
        {
                $$ = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $1, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        $4, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | parameters ',' OP_POW NAME
        {
                $$ = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $1, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$), $4);
        }
        | parameters
        {
                $$ = process_parameters (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $1, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | default_parameters ',' '*' NAME ',' OP_POW NAME
        {
                $$ = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $1, @$),
                        $4, $7);
        }
        | default_parameters ',' '*' NAME
        {
                $$ = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $1, @$),
                        $4, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | default_parameters ',' OP_POW NAME
        {
                $$ = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $1, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$), $4);
        }
        | default_parameters
        {
                $$ = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_PARAMETERS, $1, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | '*' NAME ',' OP_POW NAME
        {
                $$ = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        $2, $5);
        }
        | '*' NAME
        {
                $$ = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        $2, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | OP_POW NAME
        {
                $$ = process_parameters (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$), $2);
        }
        | parameters ',' default_parameters ',' NAME
        {
                yylloc.first_column = @5.first_column;
                yyerror ("non-default argument follows default argument");
                YYABORT;
        }
        | default_parameters ',' NAME
        {
                yylloc.first_column = @3.first_column;
                yyerror ("non-default argument follows default argument");
                YYABORT;
        }
        ;

parameters
        : NAME
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | parameters ',' NAME
        {
                $$ = g_list_prepend ($1, $3);
        }
        ;

default_parameters
        : NAME '=' expression
        {
                $$ = NULL;
                $$ = g_list_prepend ($$, $1);
                $$ = g_list_prepend ($$, $3);
        }
        | default_parameters ',' NAME '=' expression
        {
                $$ = $1;
                $$ = g_list_prepend ($$, $3);
                $$ = g_list_prepend ($$, $5);
        }
        ;

statement
        : compound_statement
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | simple_statement_list opt_semi NEWLINE
        ;

simple_statement
        : assert_statement
        | assignment_statement
        | augmented_assignment_statement
        | break_statement
        | continue_statement
        | delete_statement
        | execute_statement
        | expression_statement
        | global_statement
        | import_statement
        | pass_statement
        | print_statement
        | raise_statement
        | return_statement
        | yield_statement
        ;

simple_statement_list
        : simple_statement
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | simple_statement_list ';' simple_statement
        {
                $$ = g_list_prepend ($1, $3);
        }
        ;

expression_statement
        : expression_or_tuple
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_EXPRESSION, @$);
                g_node_prepend ($$, $1);
        }
        ;

suite
        : NEWLINE INDENT statement_list DEDENT
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_SUITE, $3, @$);
        }
        | simple_statement_list opt_semi NEWLINE
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_SUITE, $1, @$);
        }
        ;

compound_statement
        : if_statement
        | for_statement
        | try_statement
        | while_statement
        | class_definition
        | function_definition
        | predicate_definition
        ;

statement_list
        : statement
        | statement_list statement
        {
                $$ = g_list_concat ($2, $1);
        }
        ;

if_statement
        : KW_IF or_test ':' suite elif_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $5);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_IF or_test ':' suite else_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $5);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_IF or_test ':' suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        ;

elif_clause
        : KW_ELIF expression ':' suite elif_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $5);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_ELIF expression ':' suite else_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $5);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_ELIF expression ':' suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        ;

else_clause
        : KW_ELSE ':' suite
        {
                $$ = $3;
        }
        ;

for_statement
        : KW_FOR target_or_tuple KW_IN expression_or_tuple ':' suite
          else_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_FOR, @$);
                g_node_prepend ($$, $7);
                g_node_prepend ($$, $6);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_FOR target_or_tuple KW_IN expression_or_tuple ':' suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_FOR, @$);
                g_node_prepend ($$, $6);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        ;

try_statement
        : KW_TRY ':' suite except_clause_list else_clause finally_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_TRY_EXCEPT, @$);
                g_node_prepend ($$, $6);
                g_node_prepend ($$, $5);
                adopt_children ($$, $4);
                g_node_prepend ($$, $3);
        }
        | KW_TRY ':' suite except_clause_list else_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_TRY_EXCEPT, @$);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, $5);
                adopt_children ($$, $4);
                g_node_prepend ($$, $3);
        }
        | KW_TRY ':' suite except_clause_list finally_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_TRY_EXCEPT, @$);
                g_node_prepend ($$, $5);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                adopt_children ($$, $4);
                g_node_prepend ($$, $3);
        }
        | KW_TRY ':' suite except_clause_list
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_TRY_EXCEPT, @$);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                adopt_children ($$, $4);
                g_node_prepend ($$, $3);
        }
        | KW_TRY ':' suite finally_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_TRY_FINALLY, @$);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $3);
        }
        ;

except_clause
        : KW_EXCEPT expression ',' target ':' suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_EXCEPT, @$);
                g_node_prepend ($$, $6);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_EXCEPT expression ':' suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_EXCEPT, @$);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_EXCEPT ':' suite
        {
                /* Just one child so that we can easily detect it. */
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_EXCEPT, @$);
                g_node_prepend ($$, $3);
        }
        ;

except_clause_list
        : except_clause
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | except_clause_list except_clause
        {
                /* An expression-less 'except' clause must be last.  Make
                 * sure the previous 'except' clause has an expression. */
                GNode *previous = g_list_first ($1)->data;
                if (g_node_n_children (previous) == 1)
                {
                        yylloc.first_line = PCL_NODE_LINENO (previous);
                        yyerror ("default 'except' must be last");
                        YYABORT;
                }

                $$ = g_list_prepend ($1, $2);
        }
        ;

finally_clause
        : KW_FINALLY ':' suite
        {
                $$ = $3;
        }
        ;

while_statement
        : KW_WHILE expression ':' suite else_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_WHILE, @$);
                g_node_prepend ($$, $5);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_WHILE expression ':' suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_WHILE, @$);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        ;

return_statement
        : KW_RETURN expression_or_tuple
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_RETURN, @$);
                g_node_prepend ($$, $2);
        }
        | KW_RETURN
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_RETURN, @$);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        ;

expression
        : conditional
        ;

conditional
        : or_test
        | conditional KW_IF or_test KW_ELSE or_test
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_CONDITIONAL, @$);
                g_node_prepend ($$, $5);
                g_node_prepend ($$, $1);
                g_node_prepend ($$, $3);
        }
        ;

or_test
        : and_test
        | or_test KW_OR and_test
        {
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_TEST_OR)
                        $$ = $1;
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_TEST_OR, @$);
                        g_node_prepend ($$, $1);
                }
                g_node_append ($$, $3);
        }
        ;

and_test
        : not_test
        | and_test KW_AND not_test
        {
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_TEST_AND)
                        $$ = $1;
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_TEST_AND, @$);
                        g_node_prepend ($$, $1);
                }
                g_node_append ($$, $3);
        }
        ;

not_test
        : comparison
        | KW_NOT not_test
        {
                /* Check for 'not not expression' and simplify. */
                if (PCL_NODE_TYPE ($2) == PCL_NODE_TYPE_TEST_NOT)
                {
                        $$ = g_node_first_child ($2);
                        pcl_node_destroy (PCL_NODE ($2));
                        g_node_destroy ($2);
                }
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_TEST_NOT, @$);
                        g_node_prepend ($$, $2);
                }
        }
        ;

comparison
        : or_expr comp_list
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_COMPARISON, $2, @$);
                g_node_prepend ($$, $1);
        }
        | or_expr
        ;

comp_list
        : comp_operator or_expr
        {
                PCL_NODE_OPCODE ($2) = $1;
                $$ = g_list_prepend (NULL, $2);
        }
        | comp_list comp_operator or_expr
        {
                PCL_NODE_OPCODE ($3) = $2;
                $$ = g_list_prepend ($1, $3);
        }
        ;

comp_operator
        : '<'
        {
                $$ = PCL_OPCODE_COMPARISON_LT;
        }
        | OP_LE
        {
                $$ = PCL_OPCODE_COMPARISON_LE;
        }
        | OP_EQ
        {
                $$ = PCL_OPCODE_COMPARISON_EQ;
        }
        | OP_NE
        {
                $$ = PCL_OPCODE_COMPARISON_NE;
        }
        | OP_GE
        {
                $$ = PCL_OPCODE_COMPARISON_GE;
        }
        | '>'
        {
                $$ = PCL_OPCODE_COMPARISON_GT;
        }
        | KW_IN
        {
                $$ = PCL_OPCODE_COMPARISON_IN;
        }
        | KW_NOT_IN
        {
                $$ = PCL_OPCODE_COMPARISON_NOT_IN;
        }
        | KW_IS
        {
                $$ = PCL_OPCODE_COMPARISON_IS;
        }
        | KW_IS_NOT
        {
                $$ = PCL_OPCODE_COMPARISON_IS_NOT;
        }
        ;

or_expr
        : xor_expr
        | or_expr '|' xor_expr
        {
                g_assert (PCL_NODE_OPCODE ($1) == 0);
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_BINARY_OPER)
                        $$ = $1;
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, @$);
                        g_node_prepend ($$, $1);
                }
                PCL_NODE_OPCODE ($3) = PCL_OPCODE_BINARY_OR;
                g_node_append ($$, $3);
        }
        ;

xor_expr
        : and_expr
        | xor_expr '^' and_expr
        {
                g_assert (PCL_NODE_OPCODE ($1) == 0);
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_BINARY_OPER)
                        $$ = $1;
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, @$);
                        g_node_prepend ($$, $1);
                }
                PCL_NODE_OPCODE ($3) = PCL_OPCODE_BINARY_XOR;
                g_node_append ($$, $3);
        }
        ;

and_expr
        : shift_expr
        | and_expr '&' shift_expr
        {
                g_assert (PCL_NODE_OPCODE ($1) == 0);
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_BINARY_OPER)
                        $$ = $1;
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, @$);
                        g_node_prepend ($$, $1);
                }
                PCL_NODE_OPCODE ($3) = PCL_OPCODE_BINARY_AND;
                g_node_append ($$, $3);
        }
        ;

shift_expr
        : a_expr
        | shift_expr OP_LSH a_expr
        {
                g_assert (PCL_NODE_OPCODE ($1) == 0);
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_BINARY_OPER)
                        $$ = $1;
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, @$);
                        g_node_prepend ($$, $1);
                }
                PCL_NODE_OPCODE ($3) = PCL_OPCODE_BINARY_LSH;
                g_node_append ($$, $3);
        }
        | shift_expr OP_RSH a_expr
        {
                g_assert (PCL_NODE_OPCODE ($1) == 0);
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_BINARY_OPER)
                        $$ = $1;
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, @$);
                        g_node_prepend ($$, $1);
                }
                PCL_NODE_OPCODE ($3) = PCL_OPCODE_BINARY_RSH;
                g_node_append ($$, $3);
        }
        ;

a_expr
        : m_expr
        | a_expr '+' m_expr
        {
                g_assert (PCL_NODE_OPCODE ($1) == 0);
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_BINARY_OPER)
                        $$ = $1;
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, @$);
                        g_node_prepend ($$, $1);
                }
                PCL_NODE_OPCODE ($3) = PCL_OPCODE_BINARY_ADD;
                g_node_append ($$, $3);
        }
        | a_expr '-' m_expr
        {
                g_assert (PCL_NODE_OPCODE ($1) == 0);
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_BINARY_OPER)
                        $$ = $1;
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, @$);
                        g_node_prepend ($$, $1);
                }
                PCL_NODE_OPCODE ($3) = PCL_OPCODE_BINARY_SUB;
                g_node_append ($$, $3);
        }
        ;

m_expr
        : u_expr
        | m_expr '*' u_expr
        {
                g_assert (PCL_NODE_OPCODE ($1) == 0);
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_BINARY_OPER)
                        $$ = $1;
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, @$);
                        g_node_prepend ($$, $1);
                }
                PCL_NODE_OPCODE ($3) = PCL_OPCODE_BINARY_MUL;
                g_node_append ($$, $3);
        }
        | m_expr '/' u_expr
        {
                g_assert (PCL_NODE_OPCODE ($1) == 0);
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_BINARY_OPER)
                        $$ = $1;
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, @$);
                        g_node_prepend ($$, $1);
                }
                PCL_NODE_OPCODE ($3) = PCL_OPCODE_BINARY_DIV;
                g_node_append ($$, $3);
        }
        | m_expr '%' u_expr
        {
                g_assert (PCL_NODE_OPCODE ($1) == 0);
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_BINARY_OPER)
                        $$ = $1;
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, @$);
                        g_node_prepend ($$, $1);
                }
                PCL_NODE_OPCODE ($3) = PCL_OPCODE_BINARY_MOD;
                g_node_append ($$, $3);
        }
        | m_expr OP_FLD u_expr
        {
                g_assert (PCL_NODE_OPCODE ($1) == 0);
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_BINARY_OPER)
                        $$ = $1;
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, @$);
                        g_node_prepend ($$, $1);
                }
                PCL_NODE_OPCODE ($3) = PCL_OPCODE_BINARY_FLD;
                g_node_append ($$, $3);
        }
        ;

u_expr
        : power
        | '-' u_expr
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_UNARY_OPER, @$);
                PCL_NODE_OPCODE ($2) = PCL_OPCODE_UNARY_NEG;
                g_node_prepend ($$, $2);
        }
        | '+' u_expr
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_UNARY_OPER, @$);
                PCL_NODE_OPCODE ($2) = PCL_OPCODE_UNARY_POS;
                g_node_prepend ($$, $2);
        }
        | '~' u_expr
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_UNARY_OPER, @$);
                PCL_NODE_OPCODE ($2) = PCL_OPCODE_UNARY_INV;
                g_node_prepend ($$, $2);
        }
        ;

power
        : primary
        | primary OP_POW u_expr
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_BINARY_OPER, @$);
                PCL_NODE_OPCODE ($3) = PCL_OPCODE_BINARY_POW;
                g_node_prepend ($$, $3);
                g_node_prepend ($$, $1);
        }
        ;

primary
        : atom
        | attribute
        | subscription
        | slicing
        | call
        ;

attribute
        : primary '.' NAME
        {
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_ATTRIBUTE)
                {
                        $$ = $1;
                        g_node_append ($$, $3);
                }
                else
                {
                        $$ = NEW_TREE_NODE (PCL_NODE_TYPE_ATTRIBUTE, @$);
                        g_node_prepend ($$, $3);
                        g_node_prepend ($$, $1);
                }
        }
        ;

subscription
        : primary '[' expression_or_tuple ']'
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_SUBSCRIPT, @$);
                g_node_prepend ($$, $3);
                g_node_prepend ($$, $1);
        }
        ;

slicing
        : primary '[' lower_bound ':' upper_bound ']'
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_SLICING, @$);
                if ($3 == NULL)
                        $3 = NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$);
                if ($5 == NULL)
                        $5 = NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, $5);
                g_node_prepend ($$, $3);
                g_node_prepend ($$, $1);
        }
        | primary '[' lower_bound ':' upper_bound ':' stride ']'
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_SLICING, @$);
                if ($3 == NULL)
                        $3 = NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$);
                if ($5 == NULL)
                        $5 = NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$);
                if ($7 == NULL)
                        $7 = NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$);
                g_node_prepend ($$, $7);
                g_node_prepend ($$, $5);
                g_node_prepend ($$, $3);
                g_node_prepend ($$, $1);
        }
        ;

lower_bound
        : expression
        |
        {
                $$ = NULL;
        }
        ;

upper_bound
        : expression
        |
        {
                $$ = NULL;
        }
        ;

stride
        : expression
        |
        {
                $$ = NULL;
        }
        ;

atom
        : NAME
        | NUMBER
        | STRING
        | enclosure
        ;

enclosure
        : parenth_form
        | list_display
        | dict_display
        | generator_expression
        ;

parenth_form
        : '(' expression_or_tuple ')'
        {
                $$ = $2;
        }
        | '(' ')'
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_TUPLE, @$);
        }
        ;

argument_list
        : positional_arguments ',' keyword_arguments ',' '*' expression
          ',' OP_POW expression
        {
                $$ = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $1, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $3, @$),
                        $6, $9);
        }
        | positional_arguments ',' keyword_arguments ',' '*' expression
        {
                $$ = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $1, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $3, @$),
                        $6, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | positional_arguments ',' keyword_arguments ',' OP_POW expression
        {
                $$ = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $1, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $3, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$), $6);
        }
        | positional_arguments ',' keyword_arguments
        {
                $$ = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $1, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $3, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | positional_arguments ',' '*' expression ',' OP_POW expression
        {
                $$ = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $1, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        $4, $7);
        }
        | positional_arguments ',' '*' expression
        {
                $$ = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $1, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        $4, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | positional_arguments ',' OP_POW expression
        {
                $$ = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $1, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$), $4);
        }
        | positional_arguments
        {
                $$ = process_arguments (
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $1, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | keyword_arguments ',' '*' expression ',' OP_POW expression
        {
                $$ = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $1, @$),
                        $4, $7);
        }
        | keyword_arguments ',' '*' expression
        {
                $$ = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $1, @$),
                        $4, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | keyword_arguments ',' OP_POW expression
        {
                $$ = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $1, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$), $4);
        }
        | keyword_arguments
        {
                $$ = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_ARGUMENTS, $1, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | '*' expression ',' OP_POW expression
        {
                $$ = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        $2, $5);
        }
        | '*' expression
        {
                $$ = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        $2, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | OP_POW expression
        {
                $$ = process_arguments (
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$),
                        NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$), $2);
        }
        | positional_arguments ',' keyword_arguments ',' expression
        {
                yylloc.first_column = @5.first_column;
                yyerror ("non-keyword argument after keyword argument");
                YYABORT;
        }
        | keyword_arguments ',' expression
        {
                yylloc.first_column = @3.first_column;
                yyerror ("non-keyword argument after keyword argument");
                YYABORT;
        }
        ;

positional_arguments
        : expression
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | positional_arguments ',' expression
        {
                $$ = g_list_prepend ($1, $3);
        }
        ;

keyword_arguments
        : NAME '=' expression
        {
                $$ = NULL;
                $$ = g_list_prepend ($$, $1);
                $$ = g_list_prepend ($$, $3);
        }
        | keyword_arguments ',' NAME '=' expression
        {
                $$ = $1;
                $$ = g_list_prepend ($$, $3);
                $$ = g_list_prepend ($$, $5);
        }
        ;

predicate_definition
        : decorators KW_PREDICATE NAME '(' parameter_list ')' ','
          predicate_goal ':' predicate_suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, @$);
                g_node_prepend ($$, $10);
                g_node_prepend ($$, $8);
                adopt_children ($$, $5);
                g_node_prepend ($$, $1);
                adopt_name ($$, $3);
        }
        | decorators KW_PREDICATE NAME '(' ')' ',' predicate_goal ':'
          predicate_suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, @$);
                g_node_prepend ($$, $9);
                g_node_prepend ($$, $7);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, $1);
                adopt_name ($$, $3);
        }
        | decorators KW_PREDICATE NAME '(' parameter_list ')' ':'
          predicate_suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, @$);
                g_node_prepend ($$, $8);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                adopt_children ($$, $5);
                g_node_prepend ($$, $1);
                adopt_name ($$, $3);
        }
        | decorators KW_PREDICATE NAME '(' ')' ':' predicate_suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, @$);
                g_node_prepend ($$, $7);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, $1);
                adopt_name ($$, $3);
        }
        | KW_PREDICATE NAME '(' parameter_list ')' ',' predicate_goal ':'
          predicate_suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, @$);
                g_node_prepend ($$, $9);
                g_node_prepend ($$, $7);
                adopt_children ($$, $4);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                adopt_name ($$, $2);
        }
        | KW_PREDICATE NAME '(' ')' ',' predicate_goal ':' predicate_suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, @$);
                g_node_prepend ($$, $8);
                g_node_prepend ($$, $6);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                adopt_name ($$, $2);
        }
        | KW_PREDICATE NAME '(' parameter_list ')' ':' predicate_suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, @$);
                g_node_prepend ($$, $7);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                adopt_children ($$, $4);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                adopt_name ($$, $2);
        }
        | KW_PREDICATE NAME '(' ')' ':' predicate_suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_PREDICATE, @$);
                g_node_prepend ($$, $6);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                adopt_name ($$, $2);
        }
        ;

predicate_goal
        : NAME '=' expression_or_tuple
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_ASSIGN, @$);
                g_node_prepend ($$, $3);
                g_node_prepend ($$, $1);
        }
        ;

predicate_suite
        : NEWLINE INDENT predicate_statement_list DEDENT
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_SUITE, $3, @$);
        }
        | simple_predicate_statement_list opt_semi NEWLINE
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_SUITE, $1, @$);
        }
        ;

simple_predicate_statement
        : global_statement
        | pass_statement
        | static_statement
        ;

simple_predicate_statement_list
        : simple_predicate_statement
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | simple_predicate_statement_list ';' simple_predicate_statement
        {
                $$ = g_list_prepend ($1, $3);
        }
        ;

predicate_term
        : predicate_disjunction
        | predicate_if_statement
        | predicate_simple_term opt_semi NEWLINE
        ;

predicate_term_list
        : predicate_term
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | predicate_term_list predicate_term
        {
                $$ = g_list_prepend ($1, $2);
        }
        ;

predicate_simple_term
        : predicate_for_statement
        | assert_statement
        | assignment_statement
        | augmented_assignment_statement
        | cut_statement
        | delete_statement
        | execute_statement
        | expression_statement
        | global_statement
        | import_statement
        | pass_statement
        | print_statement
        | raise_statement
        | static_statement
        ;

predicate_simple_term_list
        : predicate_simple_term
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | predicate_simple_term_list ';' predicate_simple_term
        {
                $$ = g_list_prepend ($1, $3);
        }
        ;

predicate_statement
        : simple_predicate_statement_list opt_semi NEWLINE
        | function_definition
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | predicate_definition
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | predicate_conjunction
        {
                $$ = g_list_prepend (NULL, $1);
        }
        ;

predicate_statement_list
        : predicate_statement
        | predicate_statement_list predicate_statement
        {
                $$ = g_list_concat ($2, $1);
        }
        ;

predicate_conjunction
        : KW_AND ':' predicate_conjunction_suite
        {
                $$ = $3;
        }
        ;

predicate_conjunction_suite
        : NEWLINE INDENT predicate_term_list DEDENT
        {
                $$ = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_CONJUNCTION, $3, @$);
                normalize_conjunction ($$);
        }
        | predicate_simple_term_list opt_semi NEWLINE
        {
                $$ = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_CONJUNCTION, $1, @$);
                normalize_conjunction ($$);
        }
        ;

predicate_disjunction
        : KW_OR ':' predicate_disjunction_suite
        {
                $$ = $3;
        }
        ;

predicate_disjunction_suite
        : NEWLINE INDENT predicate_statement_list DEDENT
        {
                $$ = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_DISJUNCTION, $3, @$);
        }
        | simple_predicate_statement_list opt_semi NEWLINE
        {
                $$ = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_DISJUNCTION, $1, @$);
        }
        ;

predicate_if_statement
        : KW_IF or_test ':' predicate_conjunction_suite
          predicate_elif_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $5);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_IF or_test ':' predicate_conjunction_suite
          predicate_else_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $5);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_IF or_test ':' predicate_conjunction_suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        ;

predicate_elif_clause
        : KW_ELIF expression ':' predicate_conjunction_suite
          predicate_elif_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $5);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_ELIF expression ':' predicate_conjunction_suite
          predicate_else_clause
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $5);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_ELIF expression ':' predicate_conjunction_suite
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        ;

predicate_else_clause
        : KW_ELSE ':' predicate_conjunction_suite
        {
                $$ = $3;
        }
        ;

predicate_for_statement
        : KW_FOR target_or_tuple KW_IN expression_or_tuple
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_FOR, @$);
                g_node_prepend ($$, NEW_TREE_NODE (
                        PCL_NODE_TYPE_CONJUNCTION, @$));
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        ;

generator_expression
        : '(' expression generator_for_clause generator_clause_list ')'
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_GENERATOR, $4, @$);
                g_node_prepend ($$, $3);
                g_node_prepend ($$, $2);
        }
        | '(' expression generator_for_clause ')'
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_GENERATOR, @$);
                g_node_prepend ($$, $3);
                g_node_prepend ($$, $2);
        }
        ;

generator_for_clause
        : KW_FOR target_or_tuple KW_IN or_test_or_tuple
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_FOR, @$);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        ;

generator_if_clause
        : KW_IF or_test
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $2);
        }
        ;

generator_clause
        : generator_for_clause
        | generator_if_clause
        ;

generator_clause_list
        : generator_clause
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | generator_clause_list generator_clause
        {
                $$ = g_list_prepend ($1, $2);
        }
        ;

list_display
        : '[' expression list_for_clause list_clause_list ']'
        {
                $$ = NEW_TREE_NODE_FROM_LIST (
                        PCL_NODE_TYPE_COMPREHENSION, $4, @$);
                g_node_prepend ($$, $3);
                g_node_prepend ($$, $2);
        }
        | '[' expression list_for_clause ']'
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_COMPREHENSION, @$);
                g_node_prepend ($$, $3);
                g_node_prepend ($$, $2);
        }
        | '[' expression_list opt_comma ']'
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_LIST, $2, @$);
                PCL_NODE_FLAGS ($$) |= $3;
        }
        | '[' ']'
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_LIST, @$);
        }
        ;

list_for_clause
        : KW_FOR target_or_tuple KW_IN or_test_or_tuple
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_FOR, @$);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        ;

list_if_clause
        : KW_IF or_test
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IF, @$);
                g_node_prepend ($$, $2);
        }
        ;

list_clause
        : list_for_clause
        | list_if_clause
        ;

list_clause_list
        : list_clause
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | list_clause_list list_clause
        {
                $$ = g_list_prepend ($1, $2);
        }
        ;

dict_display
        : '{' key_datum_list opt_comma '}'
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_DICT, $2, @$);
                PCL_NODE_FLAGS ($$) |= ($3 | PCL_NODE_FLAG_PAIRS);
        }
        | '{' '}'
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_DICT, @$);
        }
        ;

assert_statement
        : KW_ASSERT expression
        {
                /* The RAISE_EXCEPTION opcode takes three arguments.
                 * The first is AssertionError, the others are None.
                 * Queue them up here to keep the compiler simple. */
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_ASSERT, @$);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, $2);
        }
        | KW_ASSERT expression ',' expression
        {
                /* The RAISE_EXCEPTION opcode takes three arguments.
                 * The first is AssertionError, followed by $4 and None.
                 * Queue them up here to keep the compiler simple. */
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_ASSERT, @$);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        ;

assignment_statement
        : target_assignment_list expression_or_tuple
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_ASSIGN, @$);
                g_node_prepend ($$, $2);
                adopt_children ($$, $1);
        }
        ;

target_assignment_list
        : expression_or_tuple '='
        {
                /* Can't use target_or_tuple here because it causes a conflict
                 * with expression statements.  So we have to validate the
                 * expressions ourselves. */
                yylloc.first_column = @1.first_column;
                if (validate_target ($1) == NULL)
                        YYABORT;
                yylloc.first_column = @2.first_column;
                $$ = g_list_prepend (NULL, $1);
        }
        | target_assignment_list expression_or_tuple '='
        {
                /* Can't use target_or_tuple here because it causes a conflict
                 * with expression statements.  So we have to validate the
                 * expressions ourselves. */
                yylloc.first_column = @2.first_column;
                if (validate_target ($2) == NULL)
                        YYABORT;
                yylloc.first_column = @3.first_column;
                $$ = g_list_prepend ($1, $2);
        }
        ;

augmented_assignment_statement
        : target augmented_assignment_operator expression_or_tuple
        {
                /* Some extra restrictions apply to the target. */
                if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_LIST)
                {
                        yylloc.first_column = @1.first_column;
                        yyerror ("augmented assign to list literal "
                                 "not possible");
                        YYABORT;
                }
                else if (PCL_NODE_TYPE ($1) == PCL_NODE_TYPE_TUPLE)
                {
                        yylloc.first_column = @1.first_column;
                        yyerror ("augmented assign to tuple literal "
                                 "not possible");
                        YYABORT;
                }

                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_AUG_ASSIGN, @$);
                PCL_NODE_OPCODE ($3) = $2;
                g_node_prepend ($$, $3);
                g_node_prepend ($$, $1);
        }
        ;

augmented_assignment_operator
        : OP_INPLACE_ADD
        {
                $$ = PCL_OPCODE_INPLACE_ADD;
        }
        | OP_INPLACE_SUB
        {
                $$ = PCL_OPCODE_INPLACE_SUB;
        }
        | OP_INPLACE_MUL
        {
                $$ = PCL_OPCODE_INPLACE_MUL;
        }
        | OP_INPLACE_DIV
        {
                $$ = PCL_OPCODE_INPLACE_DIV;
        }
        | OP_INPLACE_MOD
        {
                $$ = PCL_OPCODE_INPLACE_MOD;
        }
        | OP_INPLACE_POW
        {
                $$ = PCL_OPCODE_INPLACE_POW;
        }
        | OP_INPLACE_FLD
        {
                $$ = PCL_OPCODE_INPLACE_FLD;
        }
        | OP_INPLACE_LSH
        {
                $$ = PCL_OPCODE_INPLACE_LSH;
        }
        | OP_INPLACE_RSH
        {
                $$ = PCL_OPCODE_INPLACE_RSH;
        }
        | OP_INPLACE_AND
        {
                $$ = PCL_OPCODE_INPLACE_AND;
        }
        | OP_INPLACE_XOR
        {
                $$ = PCL_OPCODE_INPLACE_XOR;
        }
        | OP_INPLACE_OR
        {
                $$ = PCL_OPCODE_INPLACE_OR;
        }
        ; 

break_statement
        : KW_BREAK
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_BREAK, @$);
        }
        ;

call
        : primary generator_expression
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_CALL, @$);
                g_node_prepend ($$, $2);
                g_node_prepend ($$, $1);
        }
        | primary '(' argument_list opt_comma ')'
        {
                if ($3 == NULL)
                        YYABORT;  /* syntax error occurred */
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_CALL, $3, @$);
                PCL_NODE_FLAGS ($$) |= $4;
                g_node_prepend ($$, $1);
        }
        | primary '(' ')'
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_CALL, @$);
                g_node_prepend ($$, $1);
        }
        ;

continue_statement
        : KW_CONTINUE
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_CONTINUE, @$);
        }
        ;

cut_statement
        : KW_CUT
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_CUT, @$);
        }
        ;

delete_statement
        : KW_DEL target_list opt_comma
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_DEL, $2, @$);
                PCL_NODE_FLAGS ($$) |= $3;
        }
        ;

/* XXX Python grammar uses 'expression' for the first argument of an 'exec'
       statement, but this introduces a shift/reduce conflict with "x in y"
       expressions (which aren't valid for that argument anyway).  So we use
       'primary' for the first argument instead. */
execute_statement
        : KW_EXEC primary
        {
                /* Third argument will be duped from the second. */
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_EXEC, @$);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, $2);
        }
        | KW_EXEC primary KW_IN expression
        {
                /* Third argument will be duped from the second. */
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_EXEC, @$);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_EXEC primary KW_IN expression ',' expression
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_EXEC, @$);
                g_node_prepend ($$, $6);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        ;

global_statement
        : KW_GLOBAL identifier_list opt_comma
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_GLOBAL, $2, @$);
                PCL_NODE_FLAGS ($$) |= $3;
        }
        ;

import_statement
        : KW_IMPORT module_list
        {
                /* Even # children indicates module import. */
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_IMPORT, $2, @$);
                PCL_NODE_FLAGS ($$) |= PCL_NODE_FLAG_PAIRS;
        }
        | KW_FROM module KW_IMPORT import_list
        {
                /* Odd # children indicates 'from' form of import. */
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_IMPORT, $4, @$);
                g_node_prepend ($$, $2);
        }
        | KW_FROM module KW_IMPORT import_list ','
        {
                yylloc.first_column = @5.first_column;
                yyerror ("trailing comma not allowed "
                        "without surrounding parentheses");
                YYABORT;
        }
        | KW_FROM module KW_IMPORT '(' import_list opt_comma ')'
        {
                /* Odd # children indicates 'from' form of import. */
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_IMPORT, $5, @$);
                PCL_NODE_FLAGS ($$) |= $6;
                g_node_prepend ($$, $2);
        }
        | KW_FROM module KW_IMPORT '*'
        {
                /* One child indicates 'from <module> import *'. */
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_IMPORT, @$);
                g_node_prepend ($$, $2);
        }
        ;

import_list
        : NAME KW_AS NAME
        {
                $$ = NULL;
                $$ = g_list_prepend ($$, $1);
                $$ = g_list_prepend ($$, $3);
        }
        | NAME
        {
                $$ = NULL;
                $$ = g_list_prepend ($$, $1);
                $$ = g_list_prepend ($$, NEW_TREE_NODE (
                        PCL_NODE_TYPE_NONE, @$));
        }
        | import_list ',' NAME KW_AS NAME
        {
                $$ = $1;
                $$ = g_list_prepend ($$, $3);
                $$ = g_list_prepend ($$, $5);
        }
        | import_list ',' NAME
        {
                $$ = $1;
                $$ = g_list_prepend ($$, $3);
                $$ = g_list_prepend ($$, NEW_TREE_NODE (
                        PCL_NODE_TYPE_NONE, @$));
        }
        ;

module_list
        : module KW_AS NAME
        {
                $$ = NULL;
                $$ = g_list_prepend ($$, $1);
                $$ = g_list_prepend ($$, $3);
        }
        | module
        {
                $$ = NULL;
                $$ = g_list_prepend ($$, $1);
                $$ = g_list_prepend ($$, NEW_TREE_NODE (
                        PCL_NODE_TYPE_NONE, @$));
        }
        | module_list ',' module KW_AS NAME
        {
                $$ = $1;
                $$ = g_list_prepend ($$, $3);
                $$ = g_list_prepend ($$, $5);
        }
        | module_list ',' module
        {
                $$ = $1;
                $$ = g_list_prepend ($$, $3);
                $$ = g_list_prepend ($$, NEW_TREE_NODE (
                        PCL_NODE_TYPE_NONE, @$));
        }
        ;

module
        : dotted_name
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_MODULE, $1, @$);
        }
        ;

identifier_list
        : NAME
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | identifier_list ',' NAME
        {
                $$ = g_list_prepend ($1, $3);
        }
        ;

pass_statement
        : KW_PASS
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_PASS, @$);
        }
        ;

print_statement
        : KW_PRINT OP_RSH expression ',' expression_list opt_comma
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_PRINT, $5, @$);
                PCL_NODE_FLAGS ($$) |= $6;
                g_node_prepend ($$, $3);
        }
        | KW_PRINT OP_RSH expression
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_PRINT, @$);
                g_node_prepend ($$, $3);
        }
        | KW_PRINT expression_list opt_comma
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_PRINT, $2, @$);
                PCL_NODE_FLAGS ($$) |= $3;
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        | KW_PRINT
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_PRINT, @$);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        ;

raise_statement
        : KW_RAISE expression ',' expression ',' expression
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_RAISE, @$);
                g_node_prepend ($$, $6);
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_RAISE expression ',' expression
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_RAISE, @$);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, $4);
                g_node_prepend ($$, $2);
        }
        | KW_RAISE expression
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_RAISE, @$);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, $2);
        }
        | KW_RAISE
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_RAISE, @$);
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
                g_node_prepend ($$, NEW_TREE_NODE (PCL_NODE_TYPE_NONE, @$));
        }
        ;

static_statement
        : KW_STATIC identifier_list opt_comma
        {
                $$ = NEW_TREE_NODE_FROM_LIST (PCL_NODE_TYPE_STATIC, $2, @$);
                PCL_NODE_FLAGS ($$) |= $3;
        }
        ;

yield_statement
        : KW_YIELD expression_or_tuple
        {
                $$ = NEW_TREE_NODE (PCL_NODE_TYPE_YIELD, @$);
                g_node_prepend ($$, $2);
        }
        ;

dotted_name
        : NAME
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | dotted_name '.' NAME
        {
                $$ = g_list_prepend ($1, $3);
        }
        ;

expression_or_tuple
        : expression_list opt_comma
        {
                if (g_list_next ($1) != NULL || $2)
                {
                        $$ = NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_TUPLE, $1, @$);
                        PCL_NODE_FLAGS ($$) |= $2;
                }
                else
                {
                        $$ = $1->data;
                        $1 = g_list_delete_link ($1, $1);
                        g_assert ($1 == NULL);
                }
        }
        ;

expression_list
        : expression
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | expression_list ',' expression
        {
                $$ = g_list_prepend ($1, $3);
        }
        ;

or_test_or_tuple
        : or_test_list opt_comma
        {
                /* excludes conditionals from list */
                if (g_list_next ($1) != NULL || $2)
                {
                        $$ = NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_TUPLE, $1, @$);
                        PCL_NODE_FLAGS ($$) |= $2;
                }
                else
                {
                        $$ = $1->data;
                        $1 = g_list_delete_link ($1, $1);
                        g_assert ($1 == NULL);
                }
        }
        ;

or_test_list
        : or_test
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | or_test_list ',' or_test
        {
                $$ = g_list_prepend ($1, $3);
        }
        ;

key_datum_list
        : expression ':' expression
        {
                $$ = NULL;
                $$ = g_list_prepend ($$, $1);
                $$ = g_list_prepend ($$, $3);
        }
        | key_datum_list ',' expression ':' expression
        {
                $$ = $1;
                $$ = g_list_prepend ($$, $3);
                $$ = g_list_prepend ($$, $5);
        }
        ;

target_or_tuple
        : target_list opt_comma
        {
                if (g_list_next ($1) != NULL || $2)
                {
                        $$ = NEW_TREE_NODE_FROM_LIST (
                                PCL_NODE_TYPE_TUPLE, $1, @$);
                        PCL_NODE_FLAGS ($$) |= $2;
                }
                else
                {
                        $$ = $1->data;
                        $1 = g_list_delete_link ($1, $1);
                        g_assert ($1 == NULL);
                }
        }
        ;

target_list
        : target
        {
                $$ = g_list_prepend (NULL, $1);
        }
        | target_list ',' target
        {
                $$ = g_list_prepend ($1, $3);
        }
        ;

target
        : primary
        {
                if (($$ = validate_target ($1)) == NULL)
                        YYABORT;
        }
        ;

opt_comma
        : ','
        {
                $$ = PCL_NODE_FLAG_COMMA;
        }
        |
        {
                $$ = 0;
        }
        ;

opt_semi
        : ';'
        {
                $$ = 1;
        }
        |
        {
                $$ = 0;
        }
        ;

%%

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

static void
normalize_conjunction (GNode *node)
{
        /* "Normalizing" a conjunction involves rearranging the parse tree
         * such that only the last child of a conjunction node may be a
         * disjunction or for-loop, and any nested conjunctions are also
         * normalized.  This process may involve replicating portions of the
         * conjunction's parse tree, which is fairly expensive. */

        GNode *ii, *jj, *dest, *temp;

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
                if (PCL_NODE_TYPE (ii) == PCL_NODE_TYPE_DISJUNCTION ||
                        PCL_NODE_TYPE (ii) == PCL_NODE_TYPE_IF)
                {
                        jj = g_node_next_sibling (ii);
                        while (jj != NULL)
                        {
                                temp = jj;
                                jj = g_node_next_sibling (jj);
                                g_node_unlink (temp);
                                copy_to_disjunct (ii, temp);
                        }
                }
                if (PCL_NODE_TYPE (ii) == PCL_NODE_TYPE_FOR)
                {
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
                }
                ii = g_node_prev_sibling (ii);
        }
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

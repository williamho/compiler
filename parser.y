/* parser.y by William Ho */
%error-verbose
%debug
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "declarations.h"

extern int yylex();
extern int yyparse(void);
extern char *filename;

int cur_scope;
char type_flags[TS_COUNT];
%}

%union{
	char cval;
	char *sval;
	
	struct num {
		int ntype;  // int, long, long long, float, double, long double
		int is_unsigned;
		unsigned long long ival;
		long double rval;
	} num;
	
	struct generic_node *lval;
}

%token <cval> CHARLIT
%token <sval> STRING IDENT TYPENAME
%token <num> NUMBER

/*
%type <num.ival> primary_expr cast_expr unary_expr mult_expr add_expr shift_expr
%type <num.ival> rel_expr eq_expr and_expr xor_expr or_expr log_and_expr
%type <num.ival> log_or_expr cond_expr postfix_expr asgn_expr expr
*/

%token SIZEOF INLINE
%token INDSEL PLUSPLUS MINUSMINUS SHL SHR LTEQ GTEQ EQEQ NOTEQ
%token LOGAND LOGOR TIMESEQ DIVEQ MODEQ PLUSEQ
%token MINUSEQ SHLEQ SHREQ ANDEQ
%token XOREQ OREQ TYPEDEF_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER CONST VOLATILE RESTRICT 
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token _BOOL _COMPLEX _IMAGINARY 
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%start translation_unit
%%

/* +===========+
   | TOP LEVEL |
   +===========+ */

translation_unit
	:external_decl
	|translation_unit external_decl
	;

external_decl
	:function_definition
	|decl
	;

function_definition
	:decl_specs declarator decl_list compound_stmt
	|decl_specs declarator compound_stmt
	|declarator decl_list compound_stmt
	|declarator compound_stmt
	;

/* +==============+
   | DECLARATIONS |
   +==============+ */
	
decl
	:decl_specs ';'
	|decl_specs init_declarator_list ';'
	;

decl_specs
	:storage_class_spec
	|storage_class_spec decl_specs
	|type_spec
	|type_spec decl_specs
	|type_qual
	|type_qual decl_specs
	|INLINE
	|INLINE decl_specs
	;

init_declarator_list
	:init_declarator
	|init_declarator_list ',' init_declarator
	;

init_declarator
	:declarator
	|declarator '=' initializer
	;

storage_class_spec
	:TYPEDEF
	|EXTERN
	|STATIC
	|AUTO
	|REGISTER
	;
	
type_spec	// Set flags and check for illegal type_spec combinations after
	:VOID { type_flags[TS_VOID]++; }
	|CHAR { type_flags[TS_CHAR]++; }
	|SHORT { type_flags[TS_SHORT]++; }
	|INT { type_flags[TS_INT]++; }
	|LONG { type_flags[TS_LONG]++; }
	|FLOAT { type_flags[TS_FLOAT]++; }
	|DOUBLE { type_flags[TS_DOUBLE]++; }
	|SIGNED { type_flags[TS_SIGNED]++; }
	|UNSIGNED { type_flags[TS_UNSIGNED]++; }
	|_BOOL { type_flags[TS_BOOL]++; }
	|_COMPLEX { type_flags[TS_COMPLEX]++; }
	|struct_or_union_spec { type_flags[TS_STRUCT]++; }
	|enum_spec { type_flags[TS_ENUM]++; }
	|TYPEDEF_NAME { type_flags[TS_TYPENAME]++; }
	;

struct_or_union_spec
	:struct_or_union IDENT '{' struct_decl_list '}'
	|struct_or_union '{' struct_decl_list '}'
	|struct_or_union IDENT
	;

struct_or_union
	:STRUCT
	|UNION
	;

struct_decl_list
	:struct_decl
	|struct_decl_list struct_decl
	;

struct_decl
	:spec_qual_list struct_declarator_list ';'
	;

spec_qual_list
	:type_spec spec_qual_list
	|type_spec
	|type_qual spec_qual_list
	|type_qual
	;

struct_declarator_list
	:struct_declarator
	|struct_declarator_list ',' struct_declarator
	;

struct_declarator
	:declarator
	|':' const_expr
	|declarator ':' const_expr
	;

enum_spec
	:ENUM '{' enumerator_list '}'
	|ENUM IDENT '{' enumerator_list '}'
	|ENUM IDENT
	;

enumerator_list
	:enumerator
	|enumerator_list ',' enumerator
	;

enumerator
	:IDENT
	|IDENT '=' const_expr
	;

type_qual
	:CONST
	|RESTRICT
	|VOLATILE
	;

declarator
	:pointer direct_declarator
	|direct_declarator
	;

direct_declarator
	:IDENT
	|'(' declarator ')'
	|direct_declarator '[' const_expr ']'
	|direct_declarator '[' ']'
	|direct_declarator '(' param_type_list ')'
	|direct_declarator '(' IDENT_list ')'
	|direct_declarator '(' ')'
	;

pointer
	:'*'
	|'*' type_qual_list
	|'*' pointer
	|'*' type_qual_list pointer
	;

type_qual_list
	:type_qual
	|type_qual_list type_qual
	;


param_type_list
	:param_list
	|param_list ',' ELLIPSIS
	;

param_list
	:param_decl
	|param_list ',' param_decl
	;

param_decl
	:decl_specs declarator
	|decl_specs abstract_declarator
	|decl_specs
	;

IDENT_list
	:IDENT
	|IDENT_list ',' IDENT
	;

type_name
	:spec_qual_list
	|spec_qual_list abstract_declarator
	;

abstract_declarator
	:pointer
	|direct_abstract_declarator
	|pointer direct_abstract_declarator
	;

direct_abstract_declarator
	:'(' abstract_declarator ')'
	|'[' ']'
	|'[' const_expr ']'
	|direct_abstract_declarator '[' ']'
	|direct_abstract_declarator '[' const_expr ']'
	|'(' ')'
	|'(' param_type_list ')'
	|direct_abstract_declarator '(' ')'
	|direct_abstract_declarator '(' param_type_list ')'
	;

initializer
	:asgn_expr
	|'{' initializer_list '}'
	|'{' initializer_list ',' '}'
	;

initializer_list
	:initializer
	|initializer_list ',' initializer
	;
	
/* +=============+
   | EXPRESSIONS |
   +=============+ */
	
primary_expr
	:IDENT
	|NUMBER
	|STRING
	|'(' expr ')'
	;

postfix_expr
	:primary_expr
	|postfix_expr '[' expr ']'
	|postfix_expr '(' ')'
	|postfix_expr '(' arg_expr_list ')'
	|postfix_expr '.' IDENT
	|postfix_expr INDSEL IDENT
	|postfix_expr PLUSPLUS
	|postfix_expr MINUSMINUS
	;

arg_expr_list
	:asgn_expr
	|arg_expr_list ',' asgn_expr
	;

unary_expr
	:postfix_expr
	|PLUSPLUS unary_expr
	|MINUSMINUS unary_expr
	|'&' cast_expr
	|'*' cast_expr
	|'+' cast_expr
	|'-' cast_expr
	|'~' cast_expr
	|'!' cast_expr
	|SIZEOF unary_expr
	|SIZEOF '(' type_name ')'
	;

cast_expr
	:unary_expr
	|'(' type_name ')' cast_expr
	;

mult_expr
	:cast_expr
	|mult_expr '*' cast_expr
	|mult_expr '/' cast_expr
	|mult_expr '%' cast_expr
	;

add_expr
	:mult_expr
	|add_expr '+' mult_expr
	|add_expr '-' mult_expr
	;

shift_expr
	:add_expr
	|shift_expr SHL add_expr
	|shift_expr SHR add_expr
	;

rel_expr
	:shift_expr
	|rel_expr '<' shift_expr
	|rel_expr '>' shift_expr
	|rel_expr LTEQ shift_expr
	|rel_expr GTEQ shift_expr
	;

eq_expr
	:rel_expr
	|eq_expr EQEQ rel_expr
	|eq_expr NOTEQ rel_expr
	;

and_expr
	:eq_expr
	|and_expr '&' eq_expr
	;

xor_expr
	:and_expr
	|xor_expr '^' and_expr
	;

or_expr
	:xor_expr
	|or_expr '|' xor_expr
	;

log_and_expr
	:or_expr
	|log_and_expr LOGAND or_expr
	;

log_or_expr
	:log_and_expr
	|log_or_expr LOGOR log_and_expr
	;

cond_expr
	:log_or_expr
	|log_or_expr '?' expr ':' cond_expr
	;

asgn_expr
	:cond_expr
	|unary_expr '=' asgn_expr
	|unary_expr DIVEQ asgn_expr
	|unary_expr MODEQ asgn_expr
	|unary_expr PLUSEQ asgn_expr
	|unary_expr MINUSEQ asgn_expr
	|unary_expr SHLEQ asgn_expr
	|unary_expr SHREQ asgn_expr
	|unary_expr ANDEQ asgn_expr
	|unary_expr XOREQ asgn_expr
	|unary_expr OREQ asgn_expr
	;

expr
	:asgn_expr
	|expr ',' asgn_expr
	;

const_expr
	:cond_expr
	;

/* +============+
   | STATEMENTS |
   +============+ */
	
stmt
	:labeled_stmt
	|compound_stmt
	|expr_stmt
	|selection_stmt
	|iteration_stmt
	|jump_stmt
	;

labeled_stmt
	:IDENT ':' stmt
	|CASE const_expr ':' stmt
	|DEFAULT ':' stmt
	;

compound_stmt
	:'{' '}'
	|'{' stmt_list '}'
	|'{' decl_list '}'
	|'{' decl_list stmt_list '}'
	;

decl_list
	:decl
	|decl_list decl
	;

stmt_list
	:stmt
	|stmt_list stmt
	;

expr_stmt
	:';'
	|expr ';'
	;

selection_stmt
	:IF '(' expr ')' stmt
	|IF '(' expr ')' stmt ELSE stmt
	|SWITCH '(' expr ')' stmt
	;

iteration_stmt
	:WHILE '(' expr ')' stmt
	|DO stmt WHILE '(' expr ')' ';'
	|FOR '(' expr_stmt expr_stmt ')' stmt
	|FOR '(' expr_stmt expr_stmt expr ')' stmt
	;

jump_stmt
	:GOTO IDENT ';'
	|CONTINUE ';'
	|BREAK ';'
	|RETURN ';'
	|RETURN expr ';'
	;
	
%%
main() {
	yyparse();
	return 0;
}

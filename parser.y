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
%}

%union{
	char cval;
	char *sval;
	char spec;
	
	struct num {
		int ntype;  // int, long, long long, float, double, long double
		int is_unsigned;
		unsigned long long ival;
		long double rval;
	} num;
	
	struct decl_spec {
		char type_flags[TS_COUNT];
		char storage_flags[SC_COUNT];
	} flags;
	
	struct generic_node *lval;
}

%token <cval> CHARLIT
%token <sval> STRING IDENT TYPENAME
%token <num> NUMBER
%type <flags> decl_specs
%type <spec> type_spec storage_class_spec 

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
	:decl_specs ';' {} // Declaring structs without initializing
	|decl_specs init_declarator_list ';' {
		check_type_specs($1.type_flags);
		check_storage_classes($1.storage_flags);
		// do stuff
		print_decl_info($1.type_flags, $1.storage_flags);
	}
	;

decl_specs
	:storage_class_spec { 
		memset($$.storage_flags,0,SC_COUNT); 
		$$.storage_flags[$1]++; 
	}
	|storage_class_spec decl_specs { 
		$$ = $2; 
		$$.storage_flags[$1]++; 
	}
	|type_spec { 
		memset($$.type_flags,0,TS_COUNT); 
		$$.type_flags[$1]++; 
	}
	|type_spec decl_specs { 
		$$ = $2; 
		$$.type_flags[$1]++; 
	}
	|type_qual {}
	|type_qual decl_specs {}
	|INLINE {}
	|INLINE decl_specs {}
	;

init_declarator_list
	:init_declarator {
		
	}
	|init_declarator_list ',' init_declarator
	;

init_declarator
	:declarator
	|declarator '=' initializer // Not implemented
	;

storage_class_spec
	:TYPEDEF { $$ = SC_TYPEDEF; }
	|EXTERN { $$ = SC_EXTERN; }
	|STATIC { $$ = SC_STATIC; }
	|AUTO { $$ = SC_AUTO; }
	|REGISTER { $$ = SC_REGISTER; }
	;
	
type_spec	// Set flags and check for illegal type_spec combinations after
	:VOID { $$ = TS_VOID; }
	|CHAR { $$ = TS_CHAR; }
	|SHORT { $$ = TS_SHORT; }
	|INT { $$ = TS_INT; }
	|LONG { $$ = TS_LONG; }
	|FLOAT { $$ = TS_FLOAT; }
	|DOUBLE { $$ = TS_DOUBLE; }
	|SIGNED { $$ = TS_SIGNED; }
	|UNSIGNED { $$ = TS_UNSIGNED; }
	|_BOOL { $$ = TS_BOOL; }
	|_COMPLEX { $$ = TS_COMPLEX; }
	|struct_or_union_spec { $$ = TS_STRUCT; }
	|enum_spec { $$ = TS_ENUM; }
	|TYPEDEF_NAME { $$ = TS_TYPENAME; }
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
	:CONST {}
	|RESTRICT {}
	|VOLATILE {}
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
	//:cond_expr
	:NUMBER
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
	|'{' decl_or_stmt_list '}'
	;

decl_or_stmt_list
	:decl
	|stmt
	|decl_or_stmt_list decl
	|decl_or_stmt_list stmt
	;
	
decl_list
	:decl
	|decl_list decl
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

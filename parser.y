%error-verbose
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex();
extern int yyparse(void);
%}

%code requires {
	enum ntype {
		N_INT, N_LONG, N_LONGLONG, N_FLOAT, N_DOUBLE, N_LONGDOUBLE
	};

	struct num {
		int ntype;  /* 0: int,   1: long,   2: long long, 
					   3: float, 4: double, 5: long double */
		int is_unsigned;
		long long int ival;
		long double rval;
	};
}

%union{
	char cval;
	char *sval;
	struct num num;
}

%token <cval> CHARLIT
%token <sval> STRING IDENT
%token <num> NUMBER

%token INDSEL PLUSPLUS MINUSMINUS SHL SHR LTEQ GTEQ EQEQ NOTEQ LOGAND LOGOR 
%token ELLIPSIS TIMESEQ DIVEQ MODEQ PLUSEQ MINUSEQ SHLEQ SHREQ ANDEQ OREQ 
%token XOREQ AUTO BREAK CASE CHAR CONST CONTINUE DEFAULT DO DOUBLE ELSE ENUM 
%token EXTERN FLOAT FOR GOTO IF INLINE INT LONG REGISTER RESTRICT RETURN SHORT 
%token SIGNED SIZEOF STATIC STRUCT SWITCH TYPEDEF UNION UNSIGNED VOID VOLATILE 
%token WHILE _BOOL _COMPLEX _IMAGINARY

%nonassoc IDENT CHARLIT STRING


%%
start: 
	expr ';'
	|decl ';'
	|start expr ';'
	|start decl ';'
	;

primary_expr:
	IDENT
	|NUMBER
	|CHARLIT
	|STRING
	|'(' expr ')'
	;

postfix_expr:
	primary_expr
	|postfix_expr '[' expr ']'
	|postfix_expr '.' IDENT
	|postfix_expr INDSEL IDENT
	|postfix_expr '(' ')'
	|postfix_expr '(' expr_list ')'
	|postfix_expr PLUSPLUS
	|postfix_expr MINUSMINUS
	|'(' INT ')' '{' init_list ',' '}'
	|'(' INT ')' '{' init_list '}'
	;
	
expr_list:
	asgn_expr
	|expr_list ',' asgn_expr
	;

init:
	asgn_expr
	|'{' init_list '}'
	|'{' init_list ',' '}'
	;
	
init_list:
	init
	|init_list ',' init
	|designation init
	|init_list ',' designation init
	;
	
designation:
	desig_list '='
	;
	
desig_list:
	designator
	|desig_list designator
	;
	
designator:
	'[' NUMBER ']'
	|'[' CHARLIT ']'
	|'[' STRING ']'
	|'.' IDENT
	;

cast_expr:
	unary_expr
	|'(' INT ')' cast_expr
	;
	
unary_expr:
	postfix_expr
	|SIZEOF '(' INT ')'
	|SIZEOF unary_expr
	|'-' cast_expr
	|'+' cast_expr
	|'!' cast_expr
	|'~' cast_expr
	|'&' cast_expr
	|'*' cast_expr
	|PLUSPLUS cast_expr
	|MINUSMINUS cast_expr
	;

mult_expr:
	cast_expr
	|mult_expr '*' cast_expr
	|mult_expr '/' cast_expr
	|mult_expr '%' cast_expr
	;


add_expr:
	mult_expr
	|add_expr '+' mult_expr
	|add_expr '-' mult_expr
	;
	
shift_expr:
	add_expr
	|shift_expr SHL add_expr
	|shift_expr SHR add_expr
	;
	
rel_expr:
	shift_expr
	|rel_expr '<' shift_expr
	|rel_expr '>' shift_expr
	|rel_expr LTEQ shift_expr
	|rel_expr GTEQ shift_expr
	;

eq_expr:
	rel_expr
	|eq_expr EQEQ rel_expr
	|eq_expr NOTEQ rel_expr
	;
	
and_expr:
	eq_expr
	|and_expr '&' eq_expr
	;

xor_expr:
	and_expr
	|xor_expr '^' and_expr
	;

or_expr:
	xor_expr
	|or_expr '|' xor_expr
	;
	
log_and_expr:
	or_expr
	|log_and_expr LOGAND or_expr
	;

log_or_expr:
	log_and_expr
	|log_or_expr LOGOR log_and_expr
	;
	
cond_expr:
	log_or_expr
	|log_or_expr '?' expr ':' cond_expr
	;
	
asgn_expr:
	cond_expr
	|unary_expr asgn_op asgn_expr
	;
	
asgn_op:
	'=' 
	|TIMESEQ 
	|DIVEQ 
	|MODEQ 
	|PLUSEQ
	|MINUSEQ
	|SHLEQ 
	|SHREQ 
	|ANDEQ 
	|OREQ 
	|XOREQ
	;
	
expr:
	asgn_expr
	|expr ',' asgn_expr
	;
	
decl:
	INT ident_list
	;
	
ident_list:
	IDENT
	|ident_list ',' IDENT
	;
	
%%
main() {
	yyparse();
	return 0;
}

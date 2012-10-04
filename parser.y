/* parser.y by William Ho */
%error-verbose
%debug
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
}

%union{
	char cval;
	char *sval;
	
	struct num {
		int ntype;  /* 0: int,   1: long,   2: long long, 
					   3: float, 4: double, 5: long double */
		int is_unsigned;
		long long int ival;
		long double rval;
	} num;
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

%type <num.ival> primary_expr cast_expr unary_expr mult_expr add_expr shift_expr
%type <num.ival> rel_expr eq_expr and_expr xor_expr or_expr log_and_expr
%type <num.ival> log_or_expr cond_expr postfix_expr asgn_expr expr

%nonassoc IDENT CHARLIT STRING
%right '='
%left '+' '-'
%left '*' '/'
%left '(' ')'

%%
start
	:expr ';' { yyerror("%lld",$1); }
	|decl ';'
	|start expr ';' { yyerror("%lld",$2); }
	|start decl ';'
	;

primary_expr
	:IDENT { $$ = 3; /* get the value from the symbol table */ }
	|NUMBER { 
		if ($1.ntype >= N_FLOAT) {
			yyerror("warning: Truncating real number %Lg to integer %lld",$1.rval,(long long)$1.rval); 
			$$ = (long long)$1.rval;
		}
		else
			$$ = $1.ival;
	}
	|CHARLIT { $$ = (long long)$1; }
	|STRING { $$ = (long long)$1; }
	|'(' expr ')' { $$ = (long long)$2; }
	;

postfix_expr
	:primary_expr
	|postfix_expr '[' expr ']' {  
		yyerror("warning: arrays not implemented");
		$$ = 1; // change later
	}
	|postfix_expr '.' IDENT {
		yyerror("warning: structs/unions not implemented");
		$$ = 1; // change later
	}
	|postfix_expr INDSEL IDENT
	|postfix_expr '(' ')'
	|postfix_expr '(' expr_list ')'
	|postfix_expr PLUSPLUS {/*increment after*/}
	|postfix_expr MINUSMINUS
	|'(' INT ')' '{' init_list ',' '}' {$$ = 1; /* change later*/}
	|'(' INT ')' '{' init_list '}' {$$ = 1; /* change later*/}
	;
	
expr_list
	:asgn_expr
	|expr_list ',' asgn_expr
	;

init
	:asgn_expr
	|'{' init_list '}'
	|'{' init_list ',' '}'
	;
	
init_list
	:init
	|init_list ',' init
	|designation init
	|init_list ',' designation init
	;
	
designation
	:desig_list '='
	;
	
desig_list
	:designator
	|desig_list designator
	;
	
designator
	:'[' NUMBER ']'
	|'[' CHARLIT ']'
	|'[' STRING ']'
	|'.' IDENT
	;

cast_expr
	:unary_expr
	|'(' INT ')' cast_expr { $$ = (long long) $4; /* change later */ }
	;
	
unary_expr
	:postfix_expr
	|SIZEOF '(' INT ')' { $$ = sizeof(long long); /* change later */ }
	|SIZEOF unary_expr  { $$ = sizeof($2); /* change later */ }
	|'-' cast_expr { $$ = -$2; }
	|'+' cast_expr { $$ = $2; }
	|'!' cast_expr { $$ = !$2; }
	|'~' cast_expr { $$ = ~$2; }
	|'&' cast_expr { $$ = (long long)&$2; /*address*/}
	|'*' cast_expr { $$ = $2; /* pointers don't exist yet */}
	|PLUSPLUS cast_expr { $$ = $2+1; /* inc the value in symtable */ }
	|MINUSMINUS cast_expr { $$ = $2-1; /* likewise */ }
	;

mult_expr
	:mult_expr '*' cast_expr { $$ = $1 * $3; }
	|mult_expr '/' cast_expr { $$ = $1 / $3; }
	|mult_expr '%' cast_expr { $$ = $1 % $3; }
	|cast_expr
	;


add_expr
	:add_expr '+' mult_expr { $$ = $1 + $3; }
	|add_expr '-' mult_expr { $$ = $1 - $3; }
	|mult_expr
	;
	
shift_expr
	:shift_expr SHL add_expr { $$ = $1 << $3; }
	|shift_expr SHR add_expr { $$ = $1 >> $3; }
	|add_expr
	;
	
rel_expr
	:rel_expr '<' shift_expr { $$ = $1 < $3; }
	|rel_expr '>' shift_expr { $$ = $1 > $3; }
	|rel_expr LTEQ shift_expr { $$ = $1 <= $3; }
	|rel_expr GTEQ shift_expr { $$ = $1 >= $3; }
	|shift_expr
	;

eq_expr
	:eq_expr EQEQ rel_expr { $$ = $1 == $3; }
	|eq_expr NOTEQ rel_expr { $$ = $1 != $3; }
	|rel_expr
	;
	
and_expr
	:and_expr '&' eq_expr { $$ = $1 & $3; }
	|eq_expr
	;

xor_expr
	:xor_expr '^' and_expr { $$ = $1 ^ $3; }
	|and_expr
	;

or_expr
	:or_expr '|' xor_expr { $$ = $1 | $3; }
	|xor_expr
	;
	
log_and_expr
	:log_and_expr LOGAND or_expr { $$ = $1 && $3; }
	|or_expr
	;

log_or_expr
	:log_or_expr LOGOR log_and_expr { $$ = $1 || $3; }
	|log_and_expr
	;
	
cond_expr
	:log_or_expr '?' expr ':' cond_expr {
		$$ = $1 ? $3 : $5;
	}
	|log_or_expr
	;
	
asgn_expr
	:cond_expr
	|unary_expr '=' asgn_expr { /* modify value in symtable */ } 
	|unary_expr TIMESEQ asgn_expr
	|unary_expr DIVEQ asgn_expr
	|unary_expr MODEQ asgn_expr
	|unary_expr PLUSEQ asgn_expr
	|unary_expr MINUSEQ asgn_expr
	|unary_expr SHLEQ asgn_expr
	|unary_expr SHREQ asgn_expr
	|unary_expr ANDEQ asgn_expr
	|unary_expr OREQ asgn_expr
	|unary_expr XOREQ asgn_expr
	;
	
expr
	:expr ',' asgn_expr { $$ = $3; } 
	|asgn_expr
	;
	
decl
	:INT ident_list
	;
	
ident_list
	:ident_list ',' IDENT
	|IDENT { 
		// check if exists in symbol table, if not, add it
	}
	;
	
%%
main() {
	yyparse();
	return 0;
}

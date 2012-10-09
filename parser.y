/* parser.y by William Ho */
%error-verbose
%debug
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

extern int yylex();
extern int yyparse(void);
extern char *filename;

int scope;
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
		long long ival;
		long double rval;
	} num;
	
	struct symbol *lval;
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

%type <lval> lvalue

%%

start
	:top_level_decl { scope=S_FILE; }
	|start top_level_decl { scope=S_FILE; }
	;

top_level_decl
	:decl
	|func_def
	;

	
stmt
	:compound_stmt
	|expr ';' { yyerror("exprval=%lld",$1); }
	|';'
	;
	
compound_stmt
	:'{' '}'
	|'{' { new_symtable(scope); scope=S_BLOCK; } decl_or_stmt_list '}' { remove_symtable(); }
	;

decl_or_stmt_list
	:decl_list stmt_list
	|decl_list
	|stmt_list
	;
	
func_def
	:func_def_spec { scope=S_FUNC; } compound_stmt
	;
	
func_def_spec
	:IDENT '(' ')' { // To be added: prototype scope
		if (new_sym($1)) 
			scope = S_FUNC;
		else {
			struct symbol *sym = get_sym($1);
			yyerror("error: redefinition of '%s' previously declared at %s %d", $1, sym->fname, sym->line);
			// later, distinguish between namespaces
		}
	}
	;

decl_list
	:decl_list decl
	|decl
	;
	
stmt_list
	:stmt_list stmt
	|stmt
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

primary_expr
	:IDENT {
		struct symbol *sym = get_sym($1);
		if (sym)
			$$ = sym->val;
		else {
			$$ = 0;
			yyerror("error: undeclared identifier '%s'", $1);	
		}
	}
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
	:postfix_expr '[' expr ']' {  
		yyerror("warning: arrays not implemented");
		$$ = 0; // change later
	}
	|postfix_expr '.' IDENT {
		yyerror("warning: structs/unions not implemented");
		$$ = 0; // change later
	}
	|postfix_expr INDSEL IDENT {
		yyerror("warning: structs/unions not implemented");
		$$ = 0; // change later
	}
	|postfix_expr '(' ')'
	|postfix_expr '(' expr_list ')'
	|lvalue PLUSPLUS { $$ = get_sym_p($1); set_sym_p($1,$$+1); }
	|lvalue MINUSMINUS { $$ = get_sym_p($1); set_sym_p($1,$$-1); }
	|'(' INT ')' '{' init_list ',' '}' {$$ = 1; /* change later*/}
	|'(' INT ')' '{' init_list '}' {$$ = 1; /* change later*/}
	|primary_expr
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
	|PLUSPLUS lvalue { $$ = get_sym_p($2)+1; set_sym_p($2,$$); }
	|MINUSMINUS lvalue { $$ = get_sym_p($2)-1; set_sym_p($2,$$); }
	;

cast_expr
	:unary_expr
	|'(' INT ')' cast_expr { $$ = (long long) $4; /* change later */ }
	;
	
mult_expr
	:mult_expr '*' cast_expr { $$ = $1 * $3; }
	|mult_expr '%' cast_expr { $$ = $1 % $3; }
	|mult_expr '/' cast_expr { 
		if (!$3) {
			yyerror("error: divide by 0");
			$$ = 0; //technically, undefined
		}
		else
			$$ = $1 / $3; 
	}
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

lvalue
	:IDENT { 
		if (!($$ = get_sym($1)))
			yyerror("error: undeclared identifier '%s'", $1);
	}
	;
	
asgn_expr
	:cond_expr
	|lvalue '=' asgn_expr { $$ = $3; set_sym_p($1,$$); } 
	|lvalue TIMESEQ asgn_expr { $$ = $1->val * $3; set_sym_p($1,$$); } 
	|lvalue DIVEQ asgn_expr { $$ = $1->val / $3; set_sym_p($1,$$); } 
	|lvalue MODEQ asgn_expr { $$ = $1->val % $3; set_sym_p($1,$$); } 
	|lvalue PLUSEQ asgn_expr { $$ = $1->val + $3; set_sym_p($1,$$); } 
	|lvalue MINUSEQ asgn_expr { $$ = $1->val - $3; set_sym_p($1,$$); } 
	|lvalue SHLEQ asgn_expr { $$ = $1->val << $3; set_sym_p($1,$$); } 
	|lvalue SHREQ asgn_expr { $$ = $1->val >> $3; set_sym_p($1,$$); } 
	|lvalue ANDEQ asgn_expr { $$ = $1->val & $3; set_sym_p($1,$$); } 
	|lvalue OREQ asgn_expr { $$ = $1->val | $3; set_sym_p($1,$$); } 
	|lvalue XOREQ asgn_expr { $$ = $1->val ^ $3; set_sym_p($1,$$); } 
	;
	
expr
	:expr ',' asgn_expr { $$ = $3; } 
	|asgn_expr
	;
	
decl
	:INT ident_list ';'
	;
	
ident_list
	:ident_list ',' IDENT { 
		if (!new_sym($3)) {
			struct symbol *sym = get_sym($3);
			yyerror("error: redeclaration of '%s' previously declared at %s %d", $3, sym->fname, sym->line);
		}
	}
	|IDENT { 
		if (!new_sym($1)) {
			struct symbol *sym = get_sym($1);
			yyerror("error: redeclaration of '%s' previously declared at %s %d", $1, sym->fname, sym->line);
		}
	}
	;
	
%%
main() {
	yyparse();
	return 0;
}

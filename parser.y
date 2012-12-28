/* parser.y by William Ho */
%error-verbose
%debug
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "symtable.h"
#include "declarations.h"
#include "statements.h"
#include "expressions.h"
#include "quads.h"
#include "globals.h"
#include "target.h"
#include "file_info.h"

#define CHECK_ARR_SIZE(n) \
if (n.ntype >= N_FLOAT) { \
	yyerror("invalid array size: %Lg",n.rval);\
	n.ival = (unsigned long long) n.rval;\
}

int yylex();
int yyparse(void);
int line_num;
char filename[MAX_STR_LEN];
struct symtable *cur_symtable;
int func_counter;
struct block *first_bb;
struct block *cur_bb;
struct block *newest_bb;
char *cur_func;

int cur_scope;
int tmp_counter; // number of local/tmp vars in a function
struct string_lit *strings;
struct global *globals;
struct func_list *funcs;

// options
char show_ast, show_decl, show_quads, show_target;
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
	
	struct decl_spec *specs;
	struct declarator *declarator;
	
	struct declarator_list decl_list;
	struct expr_node *expr;
	struct stmt_node *stmt;
}

%token <cval> CHARLIT
%token <sval> STRING IDENT TYPEDEF_NAME
%token <num> NUMBER 

%type <specs> decl_specs decl_spec type_spec storage_class_spec type_qual 
	spec_qual_list struct_or_union_spec
%type <num> const_expr
%type <declarator> direct_declarator declarator pointer init_declarator 
	struct_declarator abstract_declarator direct_abstract_declarator
%type <decl_list> init_declarator_list struct_declarator_list struct_decl
	struct_decl_list
%type <expr> primary_expr postfix_expr arg_expr_list unary_expr 
	cast_expr mult_expr add_expr shift_expr rel_expr eq_expr and_expr 
	xor_expr or_expr log_and_expr log_or_expr cond_expr asgn_expr expr
%type <stmt> expr_stmt selection_stmt iteration_stmt jump_stmt stmt_list
	compound_stmt stmt decl_or_stmt_list 

%expect 1

%token SIZEOF INLINE
%token INDSEL PLUSPLUS MINUSMINUS SHL SHR LTEQ GTEQ EQEQ NOTEQ
%token LOGAND LOGOR TIMESEQ DIVEQ MODEQ PLUSEQ
%token MINUSEQ SHLEQ SHREQ ANDEQ
%token XOREQ OREQ

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
	:decl_specs declarator {
		struct declarator_list *dl = malloc(sizeof(struct declarator_list));
		new_declarator_list(dl,$2);
		new_declaration($1,dl);
		new_function(cur_func);
	} compound_stmt
	//|decl_specs declarator decl_list compound_stmt // K&R
	|declarator decl_list compound_stmt
	|declarator {
		cur_func = ((struct symbol *)$1->top)->id;
		struct declarator_list *dl = malloc(sizeof(struct declarator_list));
		new_declarator_list(dl,$1);
		new_declaration(new_spec(TS,TS_INT),dl);
		new_function(cur_func);
	} compound_stmt  // return type int
	;

/* +==============+
   | DECLARATIONS |
   +==============+ */
   
decl
	:decl_specs ';' {
		new_declaration($1,0);
	} 
	|decl_specs init_declarator_list ';' {
		new_declaration($1,&$2);
	}
	;

decl_specs
	:decl_spec
	|decl_specs decl_spec {
		$2->next = $1; // Add decl_spec to linked list of specs
		$$ = $2;
	}
	;

decl_spec
	:storage_class_spec
	|type_spec
	|type_qual { yywarn("type qualifiers not implemented"); }
	|INLINE { yywarn("inline not implemented"); } // Not implemented
	;
	
init_declarator_list
	:init_declarator {
		new_declarator_list(&$$,$1);
	}
	|init_declarator_list ',' init_declarator {
		add_declarator_list(&$$,&$1,$3);
	}
	;

init_declarator
	:declarator { $1->deepest->nodetype = N_VAR; $$ = $1; }
	|declarator '=' initializer // Not implemented
	;

storage_class_spec
	:TYPEDEF { $$ = new_spec(SC,SC_TYPEDEF); }
	|EXTERN { $$ = new_spec(SC,SC_EXTERN); }
	|STATIC { $$ = new_spec(SC,SC_STATIC); }
	|AUTO { $$ = new_spec(SC,SC_AUTO); }
	|REGISTER { $$ = new_spec(SC,SC_REGISTER); }
	;
	
type_spec
	:VOID { $$ = new_spec(TS,TS_VOID); }
	|CHAR { $$ = new_spec(TS,TS_CHAR); }
	|SHORT { $$ = new_spec(TS,TS_SHORT); }
	|INT { $$ = new_spec(TS,TS_INT); }
	|LONG { $$ = new_spec(TS,TS_LONG); }
	|FLOAT { $$ = new_spec(TS,TS_FLOAT); }
	|DOUBLE { $$ = new_spec(TS,TS_DOUBLE); }
	|SIGNED { $$ = new_spec(TS,TS_SIGNED); }
	|UNSIGNED { $$ = new_spec(TS,TS_UNSIGNED); }
	|_BOOL { $$ = new_spec(TS,TS_BOOL); }
	|_COMPLEX { $$ = new_spec(TS,TS_COMPLEX); }
	|struct_or_union_spec { $$ = $1; }
	|enum_spec { $$ = new_spec(TS,TS_ENUM); }
	|TYPEDEF_NAME { $$ = new_typename_spec($1); }
	;
	
struct_or_union_spec
	:struct_or_union IDENT { 
			new_symtable(S_STRUCT);
			if (show_decl)
				printf("struct %s declaration at %s:%d {\n", 
					$2,filename,line_num); 
		} 
		'{' struct_decl_list '}' { 
			$$ = new_spec(TS,TS_STRUCT);
			$$->node = (struct generic_node *)new_struct($2,1);
			if (show_decl)
				printf("}\n");
	}
	|struct_or_union {  // unnamed struct
		new_symtable(S_STRUCT);
		if (show_decl)
			printf("struct declaration at %s:%d {\n",filename,line_num); 
		} 
		'{' struct_decl_list '}' {
			$$ = new_spec(TS,TS_STRUCT);
			$$->node = (struct generic_node *)new_struct(0,1);
			if (show_decl)
				printf("}\n");
	}
	|struct_or_union IDENT {
		// Check if struct/union exists. If not, incomplete declaration.
		$$ = new_spec(TS,TS_STRUCT);
		$$->node = (struct generic_node *)get_sym($2,NS_STRUCT_TAG,0);
		
		// If struct tag doesn't already exist, add it as incomplete
		if (!$$->node) {
			new_symtable(S_STRUCT);
			$$->node = (struct generic_node *)new_struct($2,0);
		}
	}
	;

struct_or_union
	:STRUCT
	|UNION { yywarn("unions not implemented"); }
	;

struct_decl_list
	:struct_decl
	|struct_decl_list struct_decl
	;

struct_decl
	:spec_qual_list struct_declarator_list ';' { 
		new_declaration($1,&$2);
	}
	;

spec_qual_list
	:type_spec spec_qual_list {
		$2->next = $1; // Add type_spec to linked list of specs
		$$ = $2;
	}
	|type_spec 
	|type_qual spec_qual_list { yywarn("type qualifiers not implemented"); }
	|type_qual { yywarn("type qualifiers not implemented"); }
	;

struct_declarator_list
	// same as init_decl_list
	:struct_declarator {
		new_declarator_list(&$$,$1);
	}
	|struct_declarator_list ',' struct_declarator {
		add_declarator_list(&$$,&$1,$3);
	}
	;

struct_declarator
	:declarator { $1->deepest->nodetype = N_STRUCT_MEM; $$ = $1; }
	|':' const_expr { yywarn("bit fields not implemented"); }
	|declarator ':' const_expr { yywarn("bit fields not implemented"); }
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
	:CONST { $$ = new_spec(TQ,TQ_CONST); }
	|RESTRICT { $$ = new_spec(TQ,TQ_RESTRICT); }
	|VOLATILE { $$ = new_spec(TQ,TQ_VOLATILE); }
	;

declarator
	:pointer direct_declarator {
		$$ = new_declarator($1->top);
		((struct ptr_node *)$2->top)->to = $1->deepest;
		$$->deepest = $2->deepest;
		free($1);
		free($2);
	}
	|direct_declarator { $$ = $1; }
	;
	
direct_declarator
	:IDENT {
		$$ = new_declarator((struct generic_node *)new_sym($1));
	}
	|'(' declarator ')' { $$=$2; }
	|direct_declarator '[' const_expr ']' {
		CHECK_ARR_SIZE($3);
		$$ = $1;
		add_declarator($$,new_arr_node($3.ival));
	}
	|direct_declarator '[' ']' {
		$$ = $1;
		add_declarator($$,new_arr_node(0));
	}
	
	// functions
	|direct_declarator '(' param_type_list ')' { 
		$$ = $1;
		$$->deepest->nodetype = N_FUNC;
	}
	|direct_declarator '(' ident_list ')' {
		$$ = $1; 
		$$->deepest->nodetype = N_FUNC;
	}
	|direct_declarator '(' ')' {
		$$ = $1; 
		$$->deepest->nodetype = N_FUNC;
	}
	;
	
pointer 
	:'*' { $$ = new_declarator((struct generic_node *)new_ptr_node()); }
	|'*' type_qual_list { yywarn("type qualifiers not implemented"); } 
	|'*' pointer {
		$$ = $2;
		((struct ptr_node *) $$->top)->to = new_ptr_node();
		$$->top = ((struct ptr_node *)$$->top)->to;
	}
	|'*' type_qual_list pointer { yywarn("type qualifiers not implemented"); } 
	;

type_qual_list
	:type_qual
	|type_qual_list type_qual
	;

// Functions
param_type_list
	:param_list
	|param_list ',' ELLIPSIS
	;

param_list
	:param_decl
	|param_list ',' param_decl
	;

param_decl
	:decl_specs declarator {
		struct declarator_list *dl = malloc(sizeof(struct declarator_list));
		new_declarator_list(dl,$2);
		new_declaration($1,dl);
	}
	//|decl_specs abstract_declarator
	//|decl_specs // prototypes
	;

ident_list
	:IDENT
	|ident_list ',' IDENT
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
	:'(' abstract_declarator ')' { $$ = $2; }
	// arrays
	|'[' ']' { 
		$$ = new_declarator(new_arr_node(0)); 
	}
	|'[' const_expr ']' { 
		CHECK_ARR_SIZE($2);
		$$ = new_declarator(new_arr_node($2.ival)); 
	}
	|direct_abstract_declarator '[' ']' { 
		$$ = $1;
		add_declarator($$,new_arr_node(0));
	}
	|direct_abstract_declarator '[' const_expr ']' {
		CHECK_ARR_SIZE($3);
		$$ = $1;
		add_declarator($$,new_arr_node($3.ival));	
	}
	
	// functions
	|'(' ')' {}
	|'(' param_type_list ')' {}
	|direct_abstract_declarator '(' ')'
	|direct_abstract_declarator '(' param_type_list ')'
	;

initializer // not implemented
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
	:IDENT { 
		// should work for functions not yet defined
		struct symbol *s;
		if (!get_sym($1,NS_OTHER,0)) {
			s =  malloc(sizeof(struct symbol));
			s = (struct symbol *)new_func_node();
			s->id = $1;
			s->file = "<external>";
			$$ = new_sym_node(s); 
		}
		else
			$$ = new_sym_node(get_sym($1,NS_OTHER,0)); 
	}
	|NUMBER { $$ = new_const_node($1.ival); }
	|STRING { $$ = new_string_node($1); }
	|'(' expr ')' { $$ = $2; }
	;

postfix_expr
	:primary_expr
	/*|postfix_expr '[' expr ']' { $$ = new_array_access_node($1,$3); }*/
	|postfix_expr '[' expr ']' { 
		struct expr_node *tmp = new_binary_node('+',$1,$3); 
		$$ = new_unary_node('*',tmp); // deref
	}
	|postfix_expr '(' ')' { $$ = new_func_call_node($1,0); }
	|postfix_expr '(' arg_expr_list ')' { $$ = new_func_call_node($1,$3); }
	|postfix_expr '.' IDENT { yywarn("structs not implemented"); }
	|postfix_expr INDSEL IDENT { yywarn("structs not implemented"); }
	|postfix_expr PLUSPLUS { $$ = new_unary_node(E_POSTINC,$1); }
	|postfix_expr MINUSMINUS { $$ = new_unary_node(E_POSTDEC,$1); }
	;

arg_expr_list
	:asgn_expr { $$ = new_func_arg_list($1); }
	|arg_expr_list ',' asgn_expr { add_func_arg($1,$3); $$ = $1; }
	;

unary_expr
	:postfix_expr
	|PLUSPLUS unary_expr { $$ = new_unary_node(E_PREINC,$2); }
	|MINUSMINUS unary_expr { $$ = new_unary_node(E_PREDEC,$2); }
	|'&' cast_expr { $$ = new_unary_node('&',$2); }
	|'*' cast_expr { $$ = new_unary_node('*',$2); }
	|'+' cast_expr { $$ = new_unary_node('+',$2); }
	|'-' cast_expr { $$ = new_unary_node('-',$2); }
	|'~' cast_expr { $$ = new_unary_node('~',$2); }
	|'!' cast_expr { $$ = new_unary_node('!',$2); }
	|SIZEOF unary_expr { yywarn("sizeof not implemented"); }
	|SIZEOF '(' type_name ')' { yywarn("sizeof not implemented"); }
	;

cast_expr
	:unary_expr
	|'(' type_name ')' cast_expr { yywarn("casts not implemented"); }
	;

mult_expr
	:cast_expr
	|mult_expr '*' cast_expr { $$ = new_binary_node('*',$1,$3); }
	|mult_expr '/' cast_expr { $$ = new_binary_node('/',$1,$3); }
	|mult_expr '%' cast_expr { $$ = new_binary_node('%',$1,$3); }
	;

add_expr
	:mult_expr
	|add_expr '+' mult_expr { $$ = new_binary_node('+',$1,$3); }
	|add_expr '-' mult_expr { $$ = new_binary_node('-',$1,$3); }
	;

shift_expr
	:add_expr
	|shift_expr SHL add_expr { $$ = new_binary_node(SHL,$1,$3); }
	|shift_expr SHR add_expr { $$ = new_binary_node(SHR,$1,$3); }
	;

rel_expr
	:shift_expr
	|rel_expr '<' shift_expr { $$ = new_binary_node('<',$1,$3); }
	|rel_expr '>' shift_expr { $$ = new_binary_node('>',$1,$3); }
	|rel_expr LTEQ shift_expr { $$ = new_binary_node(LTEQ,$1,$3); }
	|rel_expr GTEQ shift_expr { $$ = new_binary_node(GTEQ,$1,$3); }
	;

eq_expr
	:rel_expr
	|eq_expr EQEQ rel_expr { $$ = new_binary_node(EQEQ,$1,$3); }
	|eq_expr NOTEQ rel_expr { $$ = new_binary_node(NOTEQ,$1,$3); }
	;

and_expr
	:eq_expr
	|and_expr '&' eq_expr { $$ = new_binary_node('&',$1,$3); }
	;

xor_expr
	:and_expr
	|xor_expr '^' and_expr { $$ = new_binary_node('^',$1,$3); }
	;

or_expr
	:xor_expr
	|or_expr '|' xor_expr { $$ = new_binary_node('|',$1,$3); }
	;

log_and_expr
	:or_expr
	|log_and_expr LOGAND or_expr { $$ = new_binary_node(LOGAND,$1,$3); }
	;

log_or_expr
	:log_and_expr
	|log_or_expr LOGOR log_and_expr { $$ = new_binary_node(LOGOR,$1,$3); }
	;

cond_expr
	:log_or_expr
	|log_or_expr '?' expr ':' cond_expr { yywarn("cond_expr not implemented"); }
	;

asgn_expr
	:cond_expr
	|unary_expr '=' asgn_expr { $$ = new_asgn_node($1,$3); }
	|unary_expr TIMESEQ asgn_expr { $$ = new_asgn('*',$1,$3); }
	|unary_expr DIVEQ asgn_expr { $$ = new_asgn('/',$1,$3);	}
	|unary_expr MODEQ asgn_expr { $$ = new_asgn('%',$1,$3); }
	|unary_expr PLUSEQ asgn_expr { $$ = new_asgn('+',$1,$3); }
	|unary_expr MINUSEQ asgn_expr { $$ = new_asgn('-',$1,$3); }
	|unary_expr SHLEQ asgn_expr { $$ = new_asgn(SHL,$1,$3); }
	|unary_expr SHREQ asgn_expr { $$ = new_asgn(SHR,$1,$3); }
	|unary_expr ANDEQ asgn_expr { $$ = new_asgn('&',$1,$3); }
	|unary_expr XOREQ asgn_expr { $$ = new_asgn('^',$1,$3); }
	|unary_expr OREQ asgn_expr { $$ = new_asgn('|',$1,$3); }
	;

expr
	:asgn_expr 
	|expr ',' asgn_expr { $$ = new_binary_node(',',$1,$3); }
	;

const_expr
	//:cond_expr
	:NUMBER //{ $$ = new_const_node($1); }
	;

/* +============+
   | STATEMENTS |
   +============+ */
	
stmt
	:compound_stmt
	|expr_stmt
//	|labeled_stmt
	|selection_stmt
	|iteration_stmt
	|jump_stmt
	;

/*
labeled_stmt
	:IDENT ':' stmt { yywarn("labels not implemented"); }
	|CASE const_expr ':' stmt { yywarn("switch not implemented"); }
	|DEFAULT ':' stmt { yywarn("switch not implemented"); }
	;
*/

compound_stmt
	:'{' '}' {
		$$ = new_jump_stmt(RETURN);
		stmt_list_to_quads($$);
		funcs->last->num_locals = tmp_counter;
	}
	|'{' { 
	// If compound statement encountered in file scope, it must be a function
		if (cur_symtable->scope_type == S_FILE)
			new_symtable(S_FUNC); 
		else
			new_symtable(S_BLOCK); 
	} decl_or_stmt_list '}' { 
		$$ = $3;
		if(cur_symtable->scope_type == S_FUNC) {
			add_stmt_list($$,new_jump_stmt(RETURN));
			if (show_ast) {
				printf("AST dump for function %s\n",cur_func);
				print_stmts($3,0); 
			}
			stmt_list_to_quads($3);
		}
		funcs->last->num_locals = tmp_counter;
		remove_symtable(); 
	}
	;

decl_or_stmt_list
	:decl_list { $$ = 0; }
	|stmt_list
	|decl_list stmt_list { $$ = $2; }
	;

stmt_list
	:stmt
	|stmt_list stmt { add_stmt_list($1,$2); $$ = $1; }
	;
	
decl_list
	:decl
	|decl_list decl
	;

expr_stmt
	:';' {}
	|expr ';' {
		$$ = new_stmt_list($1);
	}
	;

selection_stmt
	:IF '(' expr ')' stmt { $$ = new_if($3,$5,0); }
	|IF '(' expr ')' stmt ELSE stmt { $$ = new_if($3,$5,$7); }
	|SWITCH '(' expr ')' stmt { yywarn("switch not implemented"); }
	;

iteration_stmt
	:WHILE '(' expr ')' stmt {
		$$ = new_while($3,$5);	
	}
	|DO stmt WHILE '(' expr ')' ';' { yywarn("do loop not implemented"); }
	|FOR '(' expr_stmt expr_stmt ')' stmt { 
		$$ = new_for($3,$4,0,$6); 
	}
	|FOR '(' expr_stmt expr_stmt expr ')' stmt { 
		$$ = new_for($3,$4,$5,$7); 
	}
	;

jump_stmt
	:GOTO IDENT ';' { yywarn("goto not implemented"); }
	|CONTINUE ';'  { $$ = new_jump_stmt(CONTINUE); }
	|BREAK ';' { $$ = new_jump_stmt(BREAK); }
	|RETURN ';' { $$ = new_stmt_list(0); $$->nodetype = RETURN; }
	|RETURN expr ';' { $$ = new_stmt_list($2); $$->nodetype = RETURN; }
	;
	
%%
void set_options(int argc, char *argv[]) {
	char c;
	while ((c = getopt(argc, argv, "adqtT")) != -1) {
		switch(c) {
		case 'a': // AST
			show_ast = 1;
			break;
		case 'd': // Declarations
			show_decl = 1;
			break;
		case 'q': // Quads
			show_quads = 1;
			break;
		case 't': // Target
			show_target = 1;
			break;
		case 'T':
			show_target = 2;
			break;
		default:
			fprintf(stderr,"Unknown option %c\n",c);
			break;
		}
	}
	if (!show_ast && !show_decl && !show_quads && !show_target)
		show_target = 1;
}

main(int argc, char *argv[]) {
	set_options(argc,argv);
	cur_symtable = new_file("<stdin>");
	strings = calloc(1,sizeof(struct string_lit));
	strings->last = strings;
	globals = calloc(1,sizeof(struct global));
	globals->last = globals;
	funcs = calloc(1,sizeof(struct func_list));
	funcs->last = funcs;

	yyparse();
	putchar('\n');

	if (show_quads) 
		print_all_quads();

	if (show_target)
		print_target_code();
	return 0;
}


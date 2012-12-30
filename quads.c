/* quads.c by William Ho  */
#include "quads.h"
#include "globals.h"
#include "symtable.h"
#include "statements.h"
#include "expressions.h"
#include "declarations.h"
#include "y.tab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int func_counter = 0;
int block_counter = 1;
int tmp_counter = 1;
struct block *first_bb = 0;
struct block *cur_bb = 0;
struct block *newest_bb = 0;
struct func_list *funcs;
struct string_lit *strings;
struct global *globals;
struct loop *cur_loop;
int lval = 0;

void stmt_list_to_quads(struct stmt_node *stmt) {
	if (!stmt)
		return;

	if (!cur_bb) {
		newest_bb = 0;
		first_bb = cur_bb = newest_bb = new_block();
		funcs->last->bb = first_bb;
	}

	do
		stmt_to_quad(stmt);
	while (stmt = stmt->next);
}

struct quad *stmt_to_quad(struct stmt_node *stmt) {
	struct for_node *for_node = (struct for_node *)stmt;
	struct if_node *if_node = (struct if_node *)stmt;
	struct while_node *while_node = (struct while_node *)stmt;

	if (!stmt)
		return;

	switch(stmt->nodetype) {
	case ';': // expression
		expr_to_node(stmt->expr);
		break;
	case FOR:
		gen_for(stmt);
		break;
	case IF:
		gen_if(stmt);
		break;
	case WHILE:
		gen_while(stmt);
		break;
	case RETURN:
		if (stmt->expr)
			new_quad(Q_RETURN,0,expr_to_node(stmt->expr),0);
		else
			new_quad(Q_RETURN,0,0,0);
		break;
	case BREAK:
		new_quad(Q_BR,0,(struct generic_node *)cur_loop->after,0);
		break;
	case CONTINUE:
		new_quad(Q_BR,0,(struct generic_node *)cur_loop->body,0);
		break;
	}
}

void gen_if(struct stmt_node *stmt) {
	struct if_node *node = (struct if_node *)stmt;
	struct block *bt, *bf, *bn;
	bt = new_block();
	bf = new_block();

	if (node->otherwise)
		bn = new_block();
	else
		bn = bf;
	
	gen_condexpr(node->check,bt,bf);
	cur_bb = bt;
	stmt_list_to_quads(node->then);
	link_bb(cur_bb,bn);

	if (node->otherwise) {
		cur_bb = bf;
		stmt_list_to_quads(node->otherwise);
		link_bb(cur_bb,bn);
	}
	cur_bb = bn;
}

void new_loop(struct block *body, struct block *after) {
	struct loop *l = malloc(sizeof(struct loop));
	l->body = body;
	l->after = after;
	l->prev = cur_loop;
	cur_loop = l;
}

void end_loop() {
	struct loop *l = cur_loop;
	if (cur_loop) {
		cur_loop = cur_loop->prev;
		free(l);
	}
}

void gen_while(struct stmt_node *stmt) {
	struct while_node *node = (struct while_node *)stmt;

	struct block *check, *body, *after;
	check = new_block();
	body = new_block();
	after = new_block();
	new_loop(body,after);

	cur_bb = check;
	gen_condexpr(node->check,body,after);
	cur_bb = body;
	stmt_list_to_quads(node->body);
	link_bb(cur_bb,check);
	cur_bb = after;
	end_loop();
}

void gen_for(struct stmt_node *stmt) {
	struct for_node *node = (struct for_node *)stmt;

	struct block *cond, *body, *incr, *after;
	cond = new_block();
	body = new_block();
	incr = new_block();
	after = new_block();

	stmt_list_to_quads(node->init);
	new_loop(incr,after);
	cur_bb = cond;
	gen_condexpr(node->cond->expr,body,after);
	cur_bb = body;
	stmt_list_to_quads(node->body);
	link_bb(cur_bb,incr);
	cur_bb = incr;
	stmt_list_to_quads(new_stmt_list(node->incr));
	link_bb(cur_bb,cond);
	cur_bb = after;
	end_loop();
}

void link_bb(struct block *bb1, struct block *bb2) {
	new_quad(Q_BR,0,(struct generic_node *)bb2,0);
}

void set_tmp_bool(int opcode, struct generic_node *tmp) {
	struct block *bt, *bf, *bn;

	bt = new_block();
	bf = new_block();
	bn = new_block();
	new_quad(opcode,0,(struct generic_node *)bt,(struct generic_node *)bf);
	cur_bb = bt;
	new_quad(Q_MOV,tmp,new_const_node_q(1),0);
	link_bb(cur_bb,bn);
	cur_bb = bf;
	new_quad(Q_MOV,tmp,new_const_node_q(0),0);
	link_bb(cur_bb,bn);
	cur_bb = bn;
}

gen_condexpr(struct expr_node *e, struct block *bt, 
	struct block *bf) 
{
	struct generic_node *node = expr_to_node(e);
	struct binary_node *bn = (struct binary_node *)e;
	int opcode;

	// evaluate the expression and set a value to 0 or 1 and check that
	new_quad(Q_CMP,0,node,new_const_node_q(0));
	new_quad(Q_BRNE,0,(struct generic_node *)bt,(struct generic_node *)bf);
	return;

	// not used
	if (e->nodetype == E_BINARY) {
		switch(bn->type) {
		case '>': opcode = Q_BRGT; break;
		case '<': opcode = Q_BRLT; break;
		case GTEQ: opcode = Q_BRGE; break;
		case LTEQ: opcode = Q_BRLE; break;
		case EQEQ: opcode = Q_BREQ; break;
		case NOTEQ: opcode = Q_BRNE; break;
		case LOGAND: break; //TODO
		case LOGOR: break; 
		default:
			new_quad(Q_CMP,0,node,new_const_node_q(0));
			opcode = Q_BRNE;
		}
	}
	else {
		new_quad(Q_CMP,0,node,new_const_node_q(0));
		opcode = Q_BRNE;
	}
	new_quad(opcode,0,(struct generic_node *)bt,(struct generic_node *)bf);
}

struct generic_node *expr_to_node(struct expr_node *expr) {
	struct generic_node *dest, *src1, *src2, *tmp;
	struct symbol *sym = ((struct sym_node *)expr)->sym;
	struct array_access_node *arrnode = (struct array_access_node *)expr;
	int mode = 0;

	switch(expr->nodetype) {
	case E_ASGN:
		dest = gen_lval(((struct asgn_node *)expr)->lval,&mode);
		src1 = expr_to_node(((struct asgn_node *)expr)->rval);

		if (mode) // indirect
			new_quad(Q_STORE,0,src1,dest);
		else
			new_quad(Q_MOV,dest,src1,0);
		return src1;
	case E_UNARY:
		return unary_to_node(expr);
		break;
	case E_BINARY:
		return binary_to_node(expr);
		break;
	case STRING:
		return new_string(((struct string_node *)expr)->str);
		break;
	case NUMBER:
		return new_const_node_q(((struct const_node *)expr)->val);
	case IDENT:
		if (sym->id[0] != '%' && sym->scope->scope_type != S_FILE)
			rename_sym(sym);
		if (sym->type->nodetype == N_ARR) {
			struct symbol *arr;
			tmp = new_tmp_node();
			arr = malloc(sizeof(struct symbol));
			*arr = *sym;
			arr->id = ((struct symbol *)tmp)->id;
			free(tmp);

			new_quad(Q_LEA,(struct generic_node *)arr,
				(struct generic_node *)sym,0);
			return (struct generic_node *)arr;
		}
		return (struct generic_node *)sym;
	case E_FUNC_CALL:
		return get_func_args(expr);
		break;
	}
}

struct generic_node *gen_lval(struct expr_node *node, int *mode) {
	struct symbol *sym;
	struct unary_node *unode;
	switch(node->nodetype) {
	case NUMBER:
		yyerror("Invalid lvalue"); 
		return expr_to_node(node);
	case IDENT:
		sym = (struct symbol *)node;
		return expr_to_node(node);
	case E_UNARY:
		unode = (struct unary_node *)node;
		if (unode->type == '*') { // dereference
			*mode = 1; // indirect
			return expr_to_node(unode->child);
		}
		return unary_to_node(node);
		break;
	default:
		*mode = 1;
		return expr_to_node(node);
	}
}

struct generic_node *get_func_args(struct expr_node *f) {
	struct func_call_node *node = (struct func_call_node *)f;
	struct func_arg_node *arg;
	struct generic_node *dest;
	int num_args = 0;

	arg = node->first_arg;
	if (arg) {
		do 
			num_args++; // count args
		while (arg = arg->next);

		new_quad(Q_ARG_BEGIN,0,new_const_node_q(num_args),0);

		arg = node->first_arg;
		do 
			new_quad(Q_FUNC_ARG,0,expr_to_node(arg->val),0);
		while (arg = arg->next);
	}

	new_quad(Q_FUNC_CALL,dest = new_tmp_node(),
		(struct generic_node *)(((struct sym_node *)(node->func))->sym),0);
	return dest;
}

struct symbol *rename_sym(struct symbol *sym) {
	char *tmp_name;
	tmp_name = malloc(16);
	if (sym->type->nodetype == N_ARR)
		tmp_counter += get_size_of_arr(sym->type)/4;
	sprintf(tmp_name,"%%T%d",tmp_counter++);
	sym->id = tmp_name;

	return sym;
}

struct block *new_block() {
	struct block *block = malloc(sizeof(struct block));
	char *tmp_name = malloc(16);
	sprintf(tmp_name,".BB%d.%d",func_counter,block_counter++);
	block->nodetype = N_VAR;
	block->id = tmp_name;
	block->first = block->last = 0;
	block->next = 0;

	if (newest_bb) {
		newest_bb->next = block;
		newest_bb = block;
	}
	return block;
}

void print_bb(struct block *bb) {
	printf("%s:\n",bb->id);
}

struct generic_node *new_tmp_node() {
	struct symbol *node = malloc(sizeof(struct symbol));
	char *tmp_name = malloc(16);
	sprintf(tmp_name,"%%T%d",tmp_counter++);
	node->id = tmp_name;
	node->nodetype = N_INT;
	return (struct generic_node *)node;
}

struct generic_node *new_const_node_q(int val) {
	struct symbol *node = malloc(sizeof(struct symbol));
	char *const_name = malloc(10); 
	sprintf(const_name,"%d",val);
	node->id = const_name;
	node->nodetype = N_CONST;
	return (struct generic_node *)node;
}

struct generic_node *unary_to_node(struct expr_node *expr) {
	struct generic_node *dest, *src;
	struct unary_node *e = (struct unary_node *)expr;

	src = expr_to_node(e->child);
	dest = new_tmp_node();

	switch(e->type) {
	case E_PREINC:
		new_quad(Q_INC,0,src,0);
		new_quad(Q_MOV,dest,src,0);
		break;
	case E_PREDEC:
		new_quad(Q_DEC,0,src,0);
		new_quad(Q_MOV,dest,src,0);
		break;
	case E_POSTINC:
		new_quad(Q_MOV,dest,src,0);
		new_quad(Q_INC,0,src,0);
		break;
	case E_POSTDEC:
		new_quad(Q_MOV,dest,src,0);
		new_quad(Q_DEC,0,src,0);
		break;
	case '&':
		new_quad(Q_LEA,dest,src,0);
		break;
	case '!':
		new_quad(Q_LOGNOT,dest,src,0);
		break;
	case '*':
		new_quad(Q_LOAD,dest,src,0);
		break;
	case '+': // do nothing
		return src;
		break;
	case '-':
		new_quad(Q_MUL,dest,src,new_const_node_q(-1));
		break;
	case '~':
		new_quad(Q_NOT,dest,src,0);
		break;
	}
	return dest;
}

struct generic_node *ptr_arithmetic(int opcode, struct generic_node *dest, 
	struct generic_node *src1, struct generic_node *src2) 
{
	struct generic_node *tmp, *tmp2;
	struct symbol *sym;
	int type1, type2, type_tmp;

	type1 = (src1->nodetype==N_CONST || src1->nodetype==N_INT) ? 
			N_INT : ((struct symbol *)src1)->type->nodetype;
	type2 = (src2->nodetype==N_CONST || src2->nodetype==N_INT) ? 
			N_INT : ((struct symbol *)src2)->type->nodetype;

	// normal arithmetic
	if (type1 == N_INT && type2 == N_INT)
	{
		new_quad(opcode,dest,src1,src2);
		return dest;
	}
	if (type1 == N_INT && (type2 == N_ARR || type2 == N_PTR))
	{ 
		tmp = src1; src1 = src2; src2 = tmp; // swap nodes
		type_tmp = type1; type1 = type2; type2 = type_tmp; // swap types
	}
	if ((type1 == N_PTR || type1 == N_ARR) && type2 == N_INT) {
		tmp = new_tmp_node();
		tmp->nodetype = N_PTR;

		tmp2 = src1;
		sym = (struct symbol *)src1;
		if (((struct ptr_node *) // if array of arrays
			sym->type)->to->nodetype == N_ARR)
		{
			yyerror("multidimensional arrays not implemented");
			struct symbol *arr;
			arr = malloc(sizeof(struct symbol));
			*arr = *sym;
			arr->id = ((struct symbol *)tmp)->id;
			arr->type = ((struct ptr_node *)sym->type)->to;
			tmp = new_tmp_node();

			new_quad(Q_MUL,tmp,src2,new_const_node_q(4)); // sizeof ptr
			new_quad(opcode,dest,tmp2,tmp);
			return expr_to_node(new_sym_node(arr));

			exit(-1);
			new_quad(Q_MUL,tmp,src2,new_const_node_q(
				get_size_of_arr((struct arr_node *)((struct arr_node *)
				((struct symbol *)src1)->type)->base)));  // ugh
		}
		else {
			new_quad(Q_MUL,tmp,src2,new_const_node_q(4)); // sizeof ptr
		}

		new_quad(opcode,dest,tmp2,tmp);
		return dest;
	}
}

int get_size_of_arr(struct arr_node *a) {
	if (a->base->nodetype == N_INT || a->base->nodetype == N_PTR) 
		return a->size * 4;
	else if (a->base->nodetype == N_ARR)
		return a->size * get_size_of_arr((struct arr_node *)(a->base));
}

struct generic_node *binary_to_node(struct expr_node *expr) {
	struct generic_node *dest, *src1, *src2;
	struct binary_node *e = (struct binary_node *)expr;

	src1 = expr_to_node(e->left);
	src2 = expr_to_node(e->right);
	dest = new_tmp_node();

	switch(e->type) {
	case '*':
		new_quad(Q_MUL,dest,src1,src2);
		break;
	case '/':
		new_quad(Q_DIV,dest,src1,src2);
		break;
	case '%':
		new_quad(Q_MOD,dest,src1,src2);
		break;
	case '+':
		ptr_arithmetic(Q_ADD,dest,src1,src2);
		break;
	case '-':
		ptr_arithmetic(Q_SUB,dest,src1,src2);
		break;
	case SHL: // not implemented
		new_quad(Q_SHL,dest,src1,src2);
		break;
	case SHR: // not implemented
		new_quad(Q_SHR,dest,src1,src2);
		break;
	case '>':
		new_quad(Q_CMP,0,src1,src2);
		set_tmp_bool(Q_BRGT,dest);
		break;
	case '<':
		new_quad(Q_CMP,0,src1,src2);
		set_tmp_bool(Q_BRLT,dest);
		break;
	case GTEQ:
		new_quad(Q_CMP,0,src1,src2);
		set_tmp_bool(Q_BRGE,dest);
		break;
	case LTEQ:
		new_quad(Q_CMP,0,src1,src2);
		set_tmp_bool(Q_BRLE,dest);
		break;
	case EQEQ:
		new_quad(Q_CMP,0,src1,src2);
		set_tmp_bool(Q_BREQ,dest);
		break;
	case NOTEQ:
		new_quad(Q_CMP,0,src1,src2);
		set_tmp_bool(Q_BRNE,dest);
		break;
	case '&':
		new_quad(Q_AND,dest,src1,src2);
		break;
	case '^':
		new_quad(Q_XOR,dest,src1,src2);
		break;
	case '|':
		new_quad(Q_OR,dest,src1,src2);
		break;

	struct stmt_node *stmt;
	// Short circuit operators
	case LOGAND:
		new_quad(Q_LOGAND,dest,src1,src2);
		break;
	case LOGOR:
		new_quad(Q_LOGOR,dest,src1,src2);
		break;

	// Not implemented
	case ',': 
		yyerror("comma operator not implented");
		break;
	}
	return dest;
}

struct quad *new_quad(int opcode, struct generic_node *r, 
	struct generic_node *s1,struct generic_node *s2)
{
	struct quad *q = malloc(sizeof(struct quad));
	q->opcode = opcode;
	q->result = r;
	q->src1 = s1;
	q->src2 = s2;
	q->next = 0;

	if (!cur_bb->first)
		cur_bb->first = q;
	if (cur_bb->last)
		cur_bb->last->next = q;
	cur_bb->last = q;
	/*emit(q);*/
	return q;
}

void print_quads(struct block *bb) {
	/*struct block *bb = first_bb;*/
	struct quad *q;

	do {
		putchar('\n');
		print_bb(bb);
		if (!(q = bb->first))
			continue;

		do 
			emit(q);	
		while (q = q->next);
	}
	while (bb = bb->next);
}

struct func_list *new_function(char *name) {
	struct func_list *fl;

	cur_bb = 0;
	func_counter++;
	block_counter = 1;
	tmp_counter = 1;

	fl = malloc(sizeof(struct func_list));
	fl->next = 0; 
	fl->last = fl;
	fl->id = name;
	funcs->last->next = fl;
	funcs->last = fl;
	return fl;
}

void emit(struct quad *q) {
	struct symbol *r, *s1, *s2;
	r = (struct symbol *)q->result;
	s1 = (struct symbol *)q->src1;
	s2 = (struct symbol *)q->src2;

	if (r)
		printf("%s = ",r->id);
	if (q->opcode)
		printf("%s ",opcode_string(q->opcode));
	if (s1)
		printf("%s",s1->id);
	if (s2)
		printf(",%s",s2->id);
	putchar('\n');
}

char *opcode_string(int opcode) {
	switch(opcode) {
	case Q_MOV: 	return "MOV";
	case Q_LOAD: 	return "LOAD";
	case Q_LEA: 	return "LEA";
	case Q_STORE: 	return "STORE";
	case Q_BR: 	return "BR";
	case Q_CMP: 	return "CMP";
	case Q_BRGT: 	return "BRGT";
	case Q_BRLT: 	return "BRLT";
	case Q_BRGE: 	return "BRGE";
	case Q_BRLE: 	return "BRLE";
	case Q_BREQ: 	return "BREQ";
	case Q_BRNE: 	return "BRNE";
	case Q_INC: 	return "INC";
	case Q_DEC: 	return "DEC";
	case Q_ADD: 	return "ADD";
	case Q_SUB: 	return "SUB";
	case Q_MUL: 	return "MUL";
	case Q_DIV: 	return "DIV";
	case Q_MOD: 	return "MOD";
	case Q_AND: 	return "AND";
	case Q_XOR: 	return "XOR";
	case Q_OR: 	return "OR";
	case Q_LOGAND: 	return "LOGAND";
	case Q_LOGOR: 	return "LOGOR";
	case Q_NOT: 	return "NOT";
	case Q_LOGNOT: 	return "LOGNOT";
	case Q_SHL: 	return "SHL";
	case Q_SHR: 	return "SHR";
	case Q_FUNC_CALL: 	return "CALL";
	case Q_FUNC_ARG: 	return "ARG";
	case Q_ARG_BEGIN: 	return "ARGBEGIN";
	case Q_RETURN: 	return "RETURN";
	default: 	return "";
	}
}

print_all_quads() {
	// print globals
	struct global *g = globals;
	struct symbol *sym;
	while(g = g->next) {
		sym = (struct symbol *)g->var;
		printf("GLOBAL %s ",sym->id);
		if (sym->type->nodetype == N_ARR)
			printf("%d",get_size_of_arr((struct arr_node *)(sym->type)));
		else
			printf("4");
		putchar('\n');
	}
	putchar('\n');

	struct string_lit *s = strings;
	while(s = s->next) {
		printf("STRING .LC%d \"%s\"\n",s->num,s->str);
	}
	putchar('\n');

	// go through each statement, print quads
	struct func_list *fl = funcs;
	while (fl = fl->next) {
		printf("%s:",fl->id);
		print_quads(fl->bb);
		putchar('\n');
	}
}



/* quads.c by William Ho  */
#include "quads.h"
#include "symtable.h"
#include "statements.h"
#include "expressions.h"
#include "declarations.h"
#include "y.tab.h"
#include <stdlib.h>
#include <stdio.h>

int func_counter = 1;
int block_counter = 1;
int tmp_counter = 1;
struct block *first_bb = 0;
struct block *cur_bb = 0;
struct block *newest_bb = 0;

void stmt_list_to_quads(struct stmt_node *stmt) {
	if (!stmt)
		return;

	if (!cur_bb)
		first_bb = cur_bb = newest_bb = new_block();

	do {
		stmt_to_quad(stmt);
	}
	while (stmt = stmt->next);
	new_quad(Q_RETURN,0,0,0);
}

struct quad *stmt_to_quad(struct stmt_node *stmt) {
	struct for_node *for_node = (struct for_node *)stmt;
	struct if_node *if_node = (struct if_node *)stmt;
	struct while_node *while_node = (struct while_node *)stmt;

	switch(stmt->nodetype) {
	case ';': // expression
		expr_to_node(stmt->expr);
		break;
	case FOR:
		break;
	case IF:
		gen_if(stmt);
		break;
	case WHILE:
		break;
	case RETURN:
		break;
	case BREAK:
		break;
	case CONTINUE:
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

	// DEBUG!!!!!
	new_quad(Q_CMP,0,node,new_const_node_q(0));
	new_quad(Q_BRNE,0,(struct generic_node *)bt,(struct generic_node *)bf);
	return;

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
	struct generic_node *dest, *src1, *src2;
	struct symbol *sym = ((struct sym_node *)expr)->sym;
	struct array_access_node *arrnode = (struct array_access_node *)expr;
	
	switch(expr->nodetype) {
	case E_ASGN:
		src1 = expr_to_node(((struct asgn_node *)expr)->rval);
		dest = expr_to_node(((struct asgn_node *)expr)->lval);

		if (!dest->nodetype == N_VAR)
			new_quad(Q_STORE,0,src1,dest);

		//if (src1->nodetype == N_CONST)
		new_quad(Q_MOV,dest,src1,0); // vs load
		return src1;
	case E_UNARY:
		return unary_to_node(expr);
		break;
	case E_BINARY:
		return binary_to_node(expr);
		break;
	case STRING:
		break;
	case NUMBER:
		return new_const_node_q(((struct const_node *)expr)->val);
	case IDENT:
		if (sym->id[0] != '%' && sym->scope->scope_type != S_FILE)
			rename_sym(sym);
		return (struct generic_node *)sym;
	case E_ARRAY_ACCESS:
		// note: not actually used; array access converted to ptr arith
		break;
	case E_FUNC_CALL:
		break;
	case E_FUNC_ARG:
		break;
	}
}

struct symbol *rename_sym(struct symbol *sym) {
	char *tmp_name;

	//TODO: when generating target code, change this to indicate that it is a local variable
	return sym; //debug
		
	tmp_name = malloc(16);
	if (sym->type->nodetype == N_ARR)
		tmp_counter += get_size_of_arr(sym->type)/4;
	sprintf(tmp_name,"%%T%d",tmp_counter++);
	sym->id = tmp_name;
	return sym;
}

void new_func() {
	func_counter++;
	block_counter = 1;
	tmp_counter = 1;
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
		new_quad(Q_ADD,src,src,new_const_node_q(1));
		break;
	case E_PREDEC:
		new_quad(Q_SUB,src,src,new_const_node_q(1));
		break;
	case E_POSTINC:
		break;
	case E_POSTDEC:
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
	int type1, type2, type_tmp;

	type1 = (src1->nodetype==N_CONST || src1->nodetype==N_INT) ? 
			N_INT : ((struct symbol *)src1)->type->nodetype;
	type2 = (src2->nodetype==N_CONST || src2->nodetype==N_INT) ? 
			N_INT : ((struct symbol *)src2)->type->nodetype;

	// normal arithmetic
	if (type1 == N_INT && type2 == N_INT)
	{
		new_quad(opcode,dest,src1,src2);
		/*new_quad(Q_ADD,dest,src1,src2);*/
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

		if (type1 == N_ARR && ((struct arr_node *) // if array of arrays
			((struct symbol *)src1)->type)->base->nodetype == N_ARR)
		{
			yyerror("multidimensional arrays not implemented");
			exit(-1);
			tmp2 = new_tmp_node();
			new_quad(Q_LEA,tmp2,src1,0);
			new_quad(Q_MUL,tmp,src2,new_const_node_q(
				get_size_of_arr((struct arr_node *)((struct arr_node *)
				((struct symbol *)src1)->type)->base)));  // ugh
		}
		else
			new_quad(Q_MUL,tmp,src2,new_const_node_q(4)); // sizeof ptr

		new_quad(opcode,dest,src1,tmp);
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
		/*stmt = new_if(e->left,new_stmt_list(e->right),0);*/
		/*gen_if(stmt);*/
		new_quad(Q_LOGAND,dest,src1,src2);
		break;
	case LOGOR:
		new_quad(Q_LOGOR,dest,src1,src2);
		break;

	// Not implemented
	case ',': 
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

void print_quads() {
	struct block *bb = first_bb;
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

void emit(struct quad *q) {
	struct symbol *r, *s1, *s2;
	r = (struct symbol *)q->result;
	s1 = (struct symbol *)q->src1;
	s2 = (struct symbol *)q->src2;

	if (r)
		printf("%s = ",r->id);
	if (q)
		printf("%d ",q->opcode);
	if (s1)
		printf("%s",s1->id);
	if (s2)
		printf(",%s",s2->id);
	putchar('\n');
}


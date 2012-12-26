/* quads.c by William Ho  */
#include "quads.h"
#include "symtable.h"
#include "statements.h"
#include "expressions.h"
#include "declarations.h"
#include "y.tab.h"
#include <stdlib.h>
#include <stdio.h>

int func_counter = 0;
int tmp_counter = 0;

void stmt_list_to_quads(struct stmt_node *stmt) {
	if (!stmt)
		return;

	do {
		stmt_to_quad(stmt);
	}
	while (stmt = stmt->next);
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

struct generic_node *expr_to_node(struct expr_node *expr) {
	struct generic_node *dest, *src1, *src2;
	struct symbol *sym = ((struct sym_node *)expr)->sym;
	switch(expr->nodetype) {
	case E_ASGN:
		src1 = expr_to_node(((struct asgn_node *)expr)->rval);
		dest = expr_to_node(((struct asgn_node *)expr)->lval);
		new_quad(Q_ASGN,dest,src1,0);
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
		if (sym->id[0] != '%')
			rename_sym(sym);
		
		return (struct generic_node *)sym;
	case E_ARRAY_ACCESS:
		break;
	case E_FUNC_CALL:
		break;
	case E_FUNC_ARG:
		break;
	}
}

struct symbol *rename_sym(struct symbol *sym) {
	char *tmp_name = malloc(7); // %T00000\0
	sprintf(tmp_name,"%%T{%d}",tmp_counter++);
	sym->id = tmp_name;
	return sym;
}

struct generic_node *new_block() {
	
}

struct generic_node *new_tmp_node() {
	struct symbol *node = malloc(sizeof(struct symbol));
	char *tmp_name = malloc(7); // %T00000\0
	sprintf(tmp_name,"%%T{%d}",tmp_counter++);
	node->id = tmp_name;
	return (struct generic_node *)node;
}

struct generic_node *new_const_node_q(int val) {
	struct symbol *node = malloc(sizeof(struct symbol));
	char *const_name = malloc(10); 
	sprintf(const_name,"%d",val);
	node->id = const_name;
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
		new_quad(Q_ADDRESSOF,dest,src,0);
		break;
	case '!':
		new_quad(Q_LOGNOT,dest,src,0);
		break;
	case '*':
		new_quad(Q_DEREF,dest,src,0);
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
		new_quad(Q_ADD,dest,src1,src2);
		break;
	case '-':
		new_quad(Q_SUB,dest,src1,src2);
		break;
	case SHL: // not implemented
		new_quad(Q_SHL,dest,src1,src2);
		break;
	case SHR: // not implemented
		new_quad(Q_SHR,dest,src1,src2);
		break;
	case '>':
		new_quad(Q_GT,dest,src1,src2);
		break;
	case '<':
		new_quad(Q_LT,dest,src1,src2);
		break;
	case GTEQ:
		new_quad(Q_GTEQ,dest,src1,src2);
		break;
	case LTEQ:
		new_quad(Q_LTEQ,dest,src1,src2);
		break;
	case EQEQ:
		new_quad(Q_EQEQ,dest,src1,src2);
		break;
	case NOTEQ:
		new_quad(Q_NOTEQ,dest,src1,src2);
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

	// Short circuit operators
	case LOGAND:
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
	emit(q);
	return q;
}

void emit(struct quad *q) {
	struct symbol *r, *s1, *s2;
	r = (struct symbol *)q->result;
	s1 = (struct symbol *)q->src1;
	s2 = (struct symbol *)q->src2;

	printf("%s = ",r->id);
	if (q->opcode)
		printf("%d ",q->opcode);
	if (s1)
		printf("%s",s1->id);
	if (s2)
		printf(" %s",s2->id);
	putchar('\n');
}


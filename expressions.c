/* expressions.c by William Ho */
#include "expressions.h"
#include "declarations.h"
#include "symtable.h"
#include "y.tab.h"
#include <stdlib.h>
#include <stdio.h>

struct expr_node *new_string_node(char *ptr) {
	struct string_node *node = malloc(sizeof(struct string_node));
	node->nodetype = STRING;
	node->str = ptr;
	return (struct expr_node *) node;
}

struct expr_node *new_sym_node(struct symbol *sym) {
	struct sym_node *node = malloc(sizeof(struct sym_node));
	node->nodetype = IDENT;
	node->sym = sym;
	return (struct expr_node *) node;
}

// for PLUSEQ, MINUSEQ, TIMESEQ, etc
struct expr_node *new_asgn(int type, struct expr_node *lval, 
	struct expr_node *rval) 
{
	struct expr_node *node = new_binary_node(type,lval,rval); 
	return new_binary_node('=',lval,node); 
}

struct expr_node *new_func_call_node(
	struct expr_node *func, struct expr_node *arg) 
{
	struct func_call_node *node = malloc(sizeof(struct func_call_node));
	node->nodetype = E_FUNC_CALL;
	node->func = func;
	node->first_arg = (struct func_arg_node *)arg;
	return (struct expr_node *) node;
}

struct expr_node *new_func_arg_list(struct expr_node *first_arg) {
	struct func_arg_node *node = malloc(sizeof(struct func_arg_node));
	node->nodetype = E_FUNC_ARG;
	node->val = first_arg;
	node->next = 0;
	node->last = node;
	return (struct expr_node *) node;
}

struct expr_node *add_func_arg(struct expr_node *to, struct expr_node *arg) {
	struct func_arg_node *node = malloc(sizeof(struct func_arg_node));
	struct func_arg_node *to_casted = (struct func_arg_node *) to;
	node->nodetype = E_FUNC_ARG;
	node->val = arg;
	node->next = 0;

	to_casted->last->next = node;
	to_casted->last = node;
	return (struct expr_node *) node;
}

struct expr_node *new_const_node(int val) {
	struct const_node *node = malloc(sizeof(struct const_node));
	node->nodetype = NUMBER;
	node->val = val;
	return (struct expr_node *) node;
}

struct expr_node *new_unary_node(int type, struct expr_node *n) {
	struct unary_node *node = malloc(sizeof(struct unary_node));
	node->nodetype = E_UNARY;
	node->type = type;
	node->child = n;
	return (struct expr_node *) node;
}

struct expr_node *new_binary_node(int type, struct expr_node *l,
	struct expr_node *r) 
{
	struct binary_node *node = malloc(sizeof(struct binary_node));
	node->nodetype = E_BINARY;
	node->type = type;
	node->left = l;
	node->right = r;
	return (struct expr_node *) node;
}

struct expr_node *new_asgn_node(struct expr_node *lval, 
	struct expr_node *rval) 
{
	struct asgn_node *node = malloc(sizeof(struct asgn_node));
	node->nodetype = E_ASGN;
	node->lval = lval;
	node->rval = rval;
	return (struct expr_node *) node;
}

struct expr_node *new_array_access_node(struct expr_node *array, struct expr_node *offset) {
	struct array_access_node *node = malloc(sizeof(struct array_access_node));
	node->nodetype = E_ARRAY_ACCESS;
	node->array = array;
	node->offset = offset;
	return (struct expr_node *) node;
}

#define EXPR_SPACING(d) for(i=0;i<d;i++) putchar(' ')
void print_expr(struct expr_node *node, int depth) {
	struct unary_node *unode = (struct unary_node *)node;
	struct binary_node *bnode = (struct binary_node *)node;
	struct asgn_node *anode = (struct asgn_node *)node;
	struct array_access_node *arrnode = (struct array_access_node *)node;
	struct string_node *strnode = (struct string_node *)node;
	struct const_node *cnode = (struct const_node *)node;
	struct sym_node *symnode = (struct sym_node *)node;
	struct func_call_node *fnode = (struct func_call_node *)node;
	struct func_arg_node *fanode = (struct func_arg_node *)node;
	int i,j = 0;
	char c;
	EXPR_SPACING(depth);
	depth++;

	switch(node->nodetype) {
	case E_ASGN:
		printf("ASSIGNMENT\n");
		print_expr(anode->lval,depth);
		print_expr(anode->rval,depth);
		break;
	case E_UNARY:
		printf("UNARY OP '%c' (%d)\n",unode->type,unode->type);
		print_expr(unode->child,depth);
		break;
	case E_BINARY:
		printf("BINARY OP '%c' (%d)\n",bnode->type,bnode->type);
		print_expr(bnode->left,depth);
		print_expr(bnode->right,depth);
		break;
	case STRING:
		printf("STRING \"");
		print_escaped_string(strnode->str);
		printf("\"\n");
		break;
	case NUMBER:
		printf("CONSTANT %d\n",cnode->val);
		break;
	case IDENT:
		if (!symnode->sym) {
			yyerror("invalid symbol\n");
			return;
		}
		printf("SYMBOL %s (declared @%s:%d)\n",symnode->sym->id,
			symnode->sym->file,symnode->sym->line);
		break;
	case E_ARRAY_ACCESS:
		printf("ARRAY ACCESS\n");
		EXPR_SPACING(depth);
		printf("array\n");
		print_expr(arrnode->array,depth+1);
		EXPR_SPACING(depth);
		printf("offset\n");
		print_expr(arrnode->offset,depth+1);
		break;
	case E_FUNC_CALL:
		printf("FUNCTION CALL\n");
		EXPR_SPACING(depth+1);
		printf("function\n");
		print_expr(fnode->func,depth+2);
		EXPR_SPACING(depth+1);
		printf("arguments\n");
		print_expr((struct expr_node *)fnode->first_arg,depth+2);
		break;
	case E_FUNC_ARG:
		do {
			printf("FUNCTION ARGUMENT\n");
			print_expr(fanode->val,depth);
			if (fanode->next)
				EXPR_SPACING(depth);
		}
		while (fanode = fanode->next);
		break;
	}
}


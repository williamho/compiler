/* statements.c by William Ho */
#include "expressions.h"
#include "statements.h"
#include "declarations.h"
#include "y.tab.h"
#include <stdio.h>
#include <stdlib.h>

struct stmt_node *new_jump_stmt(int type) {
	struct stmt_node *node = new_stmt_list(0);
	node->nodetype = type;
	return node;
}

struct stmt_node *new_stmt_list(struct expr_node *e) {
	struct stmt_node *node = malloc(sizeof(struct stmt_node));
	node->nodetype = ';';
	node->next = 0;
	node->last = node;
	node->expr = e;
	return node;
}

void add_stmt_list(struct stmt_node *to, struct stmt_node *s) {
	to->last->next = s;
	to->last = s;
}

struct stmt_node *new_if(struct expr_node *ifnode, struct stmt_node *thennode,
	struct stmt_node *elsenode)
{
	struct if_node *node = malloc(sizeof(struct if_node));
	node->next = 0;
	node->last = (struct stmt_node *)node;
	node->nodetype = IF;
	node->check = ifnode;
	node->then = thennode;
	node->otherwise = elsenode;
	return (struct stmt_node *)node;
}

struct stmt_node *new_for(struct stmt_node *init, struct stmt_node *cond,
	struct expr_node *incr, struct stmt_node *body) 
{
	struct for_node *node = malloc(sizeof(struct for_node));
	node->next = 0;
	node->last = (struct stmt_node *)node;
	node->nodetype = FOR;
	node->init = init;
	node->cond = cond;
	node->incr = incr;
	node->body = body;
	return (struct stmt_node *)node;
}

struct stmt_node *new_while(struct expr_node *check, struct stmt_node *body) {
	struct while_node *node = malloc(sizeof(struct while_node));
	node->next = 0;
	node->last = (struct stmt_node *)node;
	node->nodetype = WHILE;
	node->check = check;
	node->body = body;
	return (struct stmt_node *)node;
}

#define STMT_SPACING(d) for(i=0;i<d;i++) putchar('-')
void print_stmts(struct stmt_node *node, int depth) {
	struct for_node *for_node = (struct for_node *)node;
	struct if_node *if_node = (struct if_node *)node;
	struct while_node *while_node = (struct while_node *)node;
	int i;

	if (!node) 
		return;

	if (node->nodetype != ';')
		STMT_SPACING(depth);
	switch(node->nodetype) {
	case ';': // expression
		print_expr(node->expr,depth);
		break;
	case FOR:
		printf("FOR LOOP\n");
		STMT_SPACING(depth+1);
		printf("INIT\n");
		print_stmts(for_node->init,depth+1);
		STMT_SPACING(depth+1);
		printf("COND\n");
		print_stmts(for_node->cond,depth+1);
		STMT_SPACING(depth+1);
		printf("BODY\n");
		print_stmts(for_node->body,depth+1);
		STMT_SPACING(depth+1);
		printf("INCR\n");
		print_expr(for_node->incr,depth+1);
		break;
	case IF:
		printf("IF\n");
		STMT_SPACING(depth+1);
		printf("CHECK\n");
		print_expr(if_node->check,depth+1);
		STMT_SPACING(depth+1);
		printf("THEN\n");
		print_stmts(if_node->then,depth+2);
		if (if_node->otherwise) {
			STMT_SPACING(depth+2);
			printf("ELSE\n");
			print_stmts(if_node->otherwise,depth+2);
		}
		break;
	case WHILE:
		printf("WHILE\n");
		STMT_SPACING(depth+1);
		printf("CHECK\n");
		print_expr(while_node->check,depth+1);
		STMT_SPACING(depth+1);
		printf("BODY\n");
		print_stmts(while_node->body,depth+1);
		break;
	}
	print_stmts(node->next,depth);
}


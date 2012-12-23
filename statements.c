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

void print_stmts(struct stmt_node *node) {
	struct for_node *for_node = (struct for_node *)node;
	struct if_node *if_node = (struct if_node *)node;
	struct while_node *while_node = (struct while_node *)node;

	if (!node) 
		return;

	switch(node->nodetype) {
	case ';':
		print_expr(node->expr);
		break;
	case FOR:
		printf("FOR LOOP\n");
		printf("INIT\n");
		print_stmts(for_node->init);
		printf("COND\n");
		print_stmts(for_node->cond);
		printf("BODY\n");
		print_stmts(for_node->body);
		printf("INCR\n");
		print_expr(for_node->incr);
		break;
	case IF:
		printf("IF\n");
		printf("CHECK\n");
		print_expr(if_node->check);
		printf("THEN\n");
		print_stmts(if_node->then);
		if (if_node->otherwise) {
			printf("ELSE\n");
			print_stmts(if_node->otherwise);
		}
		break;
	case WHILE:
		printf("WHILE\n");
		printf("CHECK\n");
		print_expr(while_node->check);
		printf("BODY\n");
		print_stmts(while_node->body);
		break;
	}
	print_stmts(node->next);
}


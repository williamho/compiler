/* statements.h by William Ho  */
#ifndef STATEMENTS_H
#define STATEMENTS_H

struct expr_node;
#define COMMON_NODE_ATTRIBUTES \
	int nodetype

struct for_node {
	COMMON_NODE_ATTRIBUTES;
	struct stmt_node *next;
	struct stmt_node *last;
	struct stmt_node *init;
	struct stmt_node *cond;
	struct stmt_node *body;
	struct expr_node *incr;
};

struct stmt_node {
	COMMON_NODE_ATTRIBUTES;
	struct stmt_node *next;
	struct stmt_node *last;
	struct expr_node *expr;
};

struct if_node {
	COMMON_NODE_ATTRIBUTES;
	struct stmt_node *next;
	struct stmt_node *last;
	struct expr_node *check;
	struct stmt_node *then;
	struct stmt_node *otherwise;
};

struct while_node {
	COMMON_NODE_ATTRIBUTES;
	struct stmt_node *next;
	struct stmt_node *last;
	struct expr_node *check;
	struct stmt_node *body;
};

struct stmt_node *new_if(struct expr_node *ifnode, struct stmt_node *thennode,
	struct stmt_node *elsenode);
void add_stmt_list(struct stmt_node *to, struct stmt_node *s);
struct stmt_node *new_stmt_list(struct expr_node *e);
struct stmt_node *new_for(struct stmt_node *init, struct stmt_node *cond,
	struct expr_node *incr, struct stmt_node *body);
struct stmt_node *new_while(struct expr_node *check, struct stmt_node *body);
struct stmt_node *new_jump_stmt(int type);
#endif


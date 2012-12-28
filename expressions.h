/* expressions.h by William Ho  */
#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#define COMMON_NODE_ATTRIBUTES \
	int nodetype

enum expr_types {
	E_FUNC_CALL, E_FUNC_ARG, E_ARRAY_ACCESS,
	E_POSTINC, E_PREINC, E_POSTDEC, E_PREDEC,
	E_UNARY, E_BINARY, E_ASGN
};

struct expr_node {
	COMMON_NODE_ATTRIBUTES;
};

struct generic_node;

struct ident_node {
	COMMON_NODE_ATTRIBUTES;
	struct generic_node *node;
};

struct const_node {
	COMMON_NODE_ATTRIBUTES;
	int val;
};

struct string_node {
	COMMON_NODE_ATTRIBUTES;
	char *str;
};

struct symbol;
struct sym_node {
	COMMON_NODE_ATTRIBUTES;
	struct symbol *sym;
};

struct asgn_node {
	COMMON_NODE_ATTRIBUTES;
	struct expr_node *lval;
	struct expr_node *rval;
};

struct unary_node {
	COMMON_NODE_ATTRIBUTES;
	int type;
	struct expr_node *child;
};

struct binary_node { // e.g., plus, minus, times, etc
	COMMON_NODE_ATTRIBUTES;
	int type;
	struct expr_node *left;
	struct expr_node *right;
};

struct func_call_node {
	COMMON_NODE_ATTRIBUTES;
	struct expr_node *func; // ident_node
	struct func_arg_node *first_arg;
};

struct func_arg_node {
	COMMON_NODE_ATTRIBUTES;
	struct expr_node *val;
	struct func_arg_node *next;
	struct func_arg_node *prev;
	struct func_arg_node *last;
};

struct array_access_node {
	COMMON_NODE_ATTRIBUTES;
	struct expr_node *array; // ident_node
	struct expr_node *offset;
};

struct expr_node *new_string_node(char *ptr);
struct expr_node *new_sym_node(struct symbol *sym);
struct expr_node *new_func_call_node(
	struct expr_node *func, struct expr_node *arg) ;
struct expr_node *new_func_arg_list(struct expr_node *first_arg);
struct expr_node *add_func_arg(struct expr_node *to, 
	struct expr_node *arg);
struct expr_node *new_const_node(int val);
struct expr_node *new_unary_node(int type, struct expr_node *n);
struct expr_node *new_binary_node(int type, struct expr_node *l,
	struct expr_node *r);
struct expr_node *new_array_access_node(struct expr_node *array, 
	struct expr_node *offset);
struct expr_node *new_asgn(int type, struct expr_node *lval, 
	struct expr_node *rval);
struct expr_node *new_asgn_node(struct expr_node *lval, 
	struct expr_node *rval);
#endif

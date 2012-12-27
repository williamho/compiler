/* quads.h by William Ho  */
#ifndef QUADS_H
#define QUADS_H

enum quad_opcodes {
	Q_MOV=1, Q_LOAD, Q_LEA, Q_STORE,
	Q_JMP, 
	
	Q_ADD, Q_SUB, Q_MUL, Q_DIV, Q_MOD, 
	Q_GT, Q_LT, Q_GTEQ, Q_LTEQ, Q_EQEQ, Q_NOTEQ,
	Q_AND, Q_XOR, Q_OR, Q_LOGAND, Q_LOGOR, 
	Q_NOT, Q_LOGNOT, Q_SHL, Q_SHR,

	Q_FUNC_CALL, Q_FUNC_ARC, 
};

struct quad {
	int opcode;
	struct generic_node *result, *src1, *src2;
	struct quad *next;
};

#define COMMON_NODE_ATTRIBUTES \
	int nodetype
struct block {
	COMMON_NODE_ATTRIBUTES;
	struct generic_node *type;
	char *id;
	struct quad *first, *last;
	struct block *next;
};

struct generic_node;
struct symbol;
struct expr_node;
struct stmt_node;

struct symbol *rename_sym(struct symbol *sym);
struct quad *new_quad(int opcode, struct generic_node *r, 
	struct generic_node *s1,struct generic_node *s2);
struct generic_node *binary_to_node(struct expr_node *expr);
struct generic_node *unary_to_node(struct expr_node *expr);
struct generic_node *expr_to_node(struct expr_node *expr);
struct quad *stmt_to_quad(struct stmt_node *stmt);
struct generic_node *new_const_node_q(int val);
void emit(struct quad *q);
struct generic_node *gen_if(struct stmt_node *stmt);
void print_bb(struct block *bb);
void link_bb(struct block *bb1, struct block *bb2);

void new_func();
struct block *new_block();
struct generic_node *new_tmp_node();
#endif


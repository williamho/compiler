/* quads.h by William Ho  */
#ifndef QUADS_H
#define QUADS_H

enum quad_opcodes {
	Q_ASGN, Q_ADD, Q_SUB, Q_MUL, Q_DIV, Q_MOD, 
	Q_GT, Q_LT, Q_GTEQ, Q_LTEQ, Q_EQEQ, Q_NOTEQ,
	Q_AND, Q_XOR, Q_OR, Q_LOGAND, Q_LOGOR, 
	Q_DEREF, Q_NOT, Q_LOGNOT, Q_SHL, Q_SHR, Q_ADDRESSOF,
};

struct quad {
	int opcode;
	struct generic_node *result, *src1, *src2;
};

struct generic_node;
struct symbol;
struct expr_node;
struct stmt_node;

struct quad *new_quad(int opcode, struct generic_node *r, 
	struct generic_node *s1,struct generic_node *s2);
struct generic_node *binary_to_node(struct expr_node *expr);
struct generic_node *unary_to_node(struct expr_node *expr);
struct generic_node *new_tmp_node();
struct generic_node *expr_to_node(struct expr_node *expr);
struct quad *stmt_to_quad(struct stmt_node *stmt);
struct generic_node *new_const_node_q(int val);
void emit(struct quad *q);
#endif


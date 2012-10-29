/* symtable.h by William Ho */
#ifndef SYMTABLE_H
#define SYMTABLE_H

#define TABLE_LENGTH 128

enum scope_types {
	S_FILE, S_BLOCK, S_FUNC, S_PROTO, 
	/* pseudo-scope: */ S_STRUCT
};

enum node_types {
	/* Number types*/
	N_CHAR, N_SHORT, N_INT, N_LONG, N_LONGLONG, N_FLOAT, N_DOUBLE, N_LONGDOUBLE,
	
	/* Misc types */
	N_PTR, N_ARR, N_STRUCT, N_UNION, N_FUNC, N_TYPEDEF
};

struct symtable {
	int scope_type;
	struct generic_node *s[TABLE_LENGTH];
	struct symtable *prev; // symbol table one level up
};

#define COMMON_NODE_ATTRIBUTES \
	char nodetype; \
	char *id;     \
	char *file;   \
	int line;     \
	char scope_type;    \
	struct generic_node *chain

struct generic_node {
	COMMON_NODE_ATTRIBUTES;
};
	
struct typedef_node {
	COMMON_NODE_ATTRIBUTES;
	struct generic_node *type;
};
	
struct ptr_node {
	COMMON_NODE_ATTRIBUTES;
	struct generic_node *to;
};

struct arr_node {
	COMMON_NODE_ATTRIBUTES;
	struct generic_node *base;
	int size;
};

struct num_node {
	COMMON_NODE_ATTRIBUTES;
	int is_unsigned; 
	unsigned long long ival; 
	long double rval;
};

struct struct_node {
	COMMON_NODE_ATTRIBUTES;
	struct symtable members; // mini-symtable to keep track of struct members
};

struct func_node {
	COMMON_NODE_ATTRIBUTES;
	struct generic_node *returns;
	struct symtable args; // mini-symtable to keep track of function arguments
};

struct symtable *new_symtable(int stype);
int remove_symtable();
unsigned long hash(unsigned char *str);
struct generic_node *new_sym(char *sname, int stype, struct symtable *table);
struct generic_node *get_sym(char *sname);

/*
long long get_sym_p(struct symbol *sym);
int set_sym(char *sname, long long sval);
int set_sym_p(struct symbol *sym, long long sval);
*/
#endif

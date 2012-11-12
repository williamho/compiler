/* symtable.h by William Ho */
#ifndef SYMTABLE_H
#define SYMTABLE_H

#define TABLE_LENGTH 128

enum scope_types {
	S_FILE, S_BLOCK, S_FUNC, S_PROTO, 
	/* pseudo-scope: */ S_STRUCT
};

enum node_types {
	N_VOID, N_CHAR, N_SHORT, N_INT, N_LONG, N_LONGLONG, 
	N_UCHAR, N_USHORT, N_UINT, N_ULONG, N_ULONGLONG,
	N_FLOAT, N_DOUBLE, N_LONGDOUBLE,
	N_BOOL, N_CFLOAT, N_CDOUBLE, N_CLONGDOUBLE,
	N_STRUCT, N_UNION, N_ENUM, N_TYPEDEF, 
	
	N_PTR, N_ARR, N_FUNC, N_TYPENAME, N_IDENT
};

struct symtable {
	int scope_type;
	char *file;
	int line;
	struct symbol *s[TABLE_LENGTH];
	struct symtable *prev; // symbol table one level up
};

#define COMMON_NODE_ATTRIBUTES \
	char nodetype

#define COMMON_SYMBOL_ATTRIBUTES \
	COMMON_NODE_ATTRIBUTES; \
	struct generic_node *type; \
	char *id; \
	char *file; \
	int line; \
	struct symtable *scope; \
	struct symbol *chain
	
struct generic_node {
	COMMON_NODE_ATTRIBUTES;
};
	
struct symbol {
	COMMON_SYMBOL_ATTRIBUTES;
	
	char storage;
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
struct generic_node *new_node(int ntype);
struct symbol *new_sym(char *sname);
int add_sym(struct symbol *sym, struct symtable *table);
void free_sym(struct symbol *sym);
struct symbol *get_sym(char *sname);
struct generic_node *new_arr_node(int size);
struct generic_node *new_ptr_node();

/*
long long get_sym_p(struct symbol *sym);
int set_sym(char *sname, long long sval);
int set_sym_p(struct symbol *sym, long long sval);
*/
#endif

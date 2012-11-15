/* symtable.h by William Ho */
#ifndef SYMTABLE_H
#define SYMTABLE_H

#define TABLE_LENGTH 128

enum scope_types {
	S_FILE, S_BLOCK, S_FUNC, S_PROTO, 
	/* pseudo-scope: */ S_STRUCT
};

enum namespaces {
	NS_OTHER, NS_STRUCT_TAG, NS_LABEL, NS_STRUCT_MEM
};

enum node_types {
	N_VOID, N_CHAR, N_SHORT, N_INT, N_LONG, N_LONGLONG, 
	N_UCHAR, N_USHORT, N_UINT, N_ULONG, N_ULONGLONG,
	N_FLOAT, N_DOUBLE, N_LONGDOUBLE,
	N_BOOL, N_CFLOAT, N_CDOUBLE, N_CLONGDOUBLE,
	N_STRUCT, N_UNION, N_ENUM, N_TYPEDEF, 
	
	N_PTR, N_ARR, N_FUNC, N_TYPENAME, N_VAR, N_STRUCT_MEM
};

// symtable_map maps file names to file-scope symbol tables
struct symtable_map {
	struct symtable *st[TABLE_LENGTH];
};

struct symtable {
	int scope_type;
	char *file;
	int line;
	struct symbol *s[TABLE_LENGTH];
	struct symtable *prev; // symbol table one level up
	struct symtable *chain; // if file scope, next symbol table with same hashed file name
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
	struct symbol *chain; \
	char namespace
	
struct generic_node {
	COMMON_NODE_ATTRIBUTES;
};
	
struct symbol {
	COMMON_SYMBOL_ATTRIBUTES;
	char storage;
};

struct struct_member {
	COMMON_SYMBOL_ATTRIBUTES;
	int offset;
	int bit_offset;
	int size;
};

struct struct_tag {
	COMMON_SYMBOL_ATTRIBUTES;
	struct symtable *members; // mini-symtable to keep track of struct members
	char complete;
};

struct func {
	COMMON_SYMBOL_ATTRIBUTES;
	struct symtable *args; // mini-symtable to keep track of function arguments
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

struct symtable *new_file(char *fname);
struct symtable *new_symtable(int stype);
int remove_symtable();
unsigned long hash(unsigned char *str);
struct generic_node *new_node(int ntype);
struct symbol *new_sym(char *sname);
struct symbol *add_sym(struct symbol *sym, struct symtable *table);
void free_sym(struct symbol *sym);
struct symbol *get_sym(char *sname, char nspace, struct symtable *table);
struct generic_node *new_arr_node(int size);
struct generic_node *new_ptr_node();
struct struct_tag *new_struct(char *struct_name, char complete);

#endif

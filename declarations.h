/* declarations.h by William Ho */
#ifndef DECLARATIONS_H
#define DECLARATIONS_H

enum spec_type { 
	TS /* type specifier */, 
	SC /* storage class */,
	TQ /* type qualifier */
};

enum type_specs {
	TS_VOID, TS_CHAR, TS_SHORT, TS_INT, TS_LONG,
	TS_FLOAT, TS_DOUBLE, TS_BOOL, TS_COMPLEX, 
	TS_STRUCT, TS_ENUM, TS_TYPENAME, 
	TS_SIGNED, TS_UNSIGNED, 
	TS_COUNT
};

enum storage_classes {
	SC_AUTO, SC_TYPEDEF, SC_EXTERN, SC_STATIC, SC_REGISTER, SC_COUNT
};

enum type_quals {
	TQ_CONST, TQ_RESTRICT, TQ_VOLATILE, TQ_COUNT
};

/*
struct decl_spec {
	char type; // type spec or storage class
	char val; // for storage classes or scalar nodes
	struct generic_node *node; // for structs, enums, typedefs
	struct decl_spec *next; // next decl spec in the list
};*/

struct decl_spec {
	char type;
	char storage;
	char qualifier;
	struct generic_node *node; // for structs, enums, typedefs
	struct decl_spec *next; // next decl spec in the list
};

struct declarator {
	struct generic_node *top, *deepest;
	struct declarator *next; // Next declarator in list
};

struct declarator_list {
	struct declarator *leftmost, *rightmost;
};

struct generic_node;
struct decl_specs;
struct symtable;
struct decl_spec *new_spec(char which, char val);
struct declarator *new_declarator(struct generic_node *n);
void new_declarator_list(struct declarator_list *dl, struct declarator *d);
void add_declarator_list(struct declarator_list *to, struct declarator_list *from, struct declarator *d);
void new_declaration(struct decl_spec *d, struct declarator_list *dl);
void new_decl(struct decl_spec *d, struct declarator_list *dl, struct symtable *st);

void print_node_info(struct generic_node *node);
void print_node_info_r(struct generic_node *node);
char *check_decl_specs(struct decl_spec *spec);
int check_storage_classes(char *sc);
int check_type_specs(char *ts);
void reset_storage_flags(char *sc);
#endif

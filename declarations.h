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

struct decl_spec {
	char type; // type spec or storage class
	char val;
	struct decl_spec *next; // next decl spec in the list
};

struct declarator {
	struct generic_node *top, *deepest;
	struct declarator *next; // Next declarator in list
};
 
struct generic_node;
struct decl_spec *new_spec(char type, char val);
struct declarator *new_declarator(struct generic_node *n);
void print_node_info(struct generic_node *node);
int check_storage_classes(char *sc);
int check_type_specs(char *ts);
void reset_storage_flags(char *sc);
void print_decl_info(char *ts, char *sc, char *tq);
#endif

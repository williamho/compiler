/* symtable.c by William Ho */
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "file_info.h"

struct symtable global_symtable = {S_FILE};
struct symtable *cur_symtable = &global_symtable;

/** Install new symbol in specified symbol table */
struct generic_node *new_sym(char *sname, int stype, struct symtable *table) {
	unsigned long hashval = hash(sname);
	struct generic_node *cur_sym, **new_sym;
	struct symtable *tmp_table;
	
	if (cur_sym = table->s[hashval]) { // collision
		// Compare names of existing symbols with that hash value
		while (cur_sym && strcmp(sname, cur_sym->id))
			cur_sym = cur_sym->chain;
				
		if (cur_sym)
			return 0; // symbol exists, return null pointer
		
		cur_sym = table->s[hashval];
	}
	
	new_sym = table->s + hashval;
	switch(stype) {
	case N_CHAR:
	case N_SHORT: 
	case N_INT:
	case N_LONG: 
	case N_LONGLONG: 
	case N_FLOAT: 
	case N_DOUBLE: 
	case N_LONGDOUBLE:
		*new_sym = malloc(sizeof(struct num_node));
		break;
	case N_TYPEDEF:
		*new_sym = malloc(sizeof(struct typedef_node));
		break;
	case N_PTR: 
		*new_sym = malloc(sizeof(struct ptr_node));
		break;
	case N_ARR:
		*new_sym = malloc(sizeof(struct arr_node));
		break;
	case N_FUNC:
		*new_sym = calloc(1,sizeof(struct func_node));
		(*new_sym)->scope_type = S_PROTO;
		break;
	case N_STRUCT:
	case N_UNION:
		*new_sym = calloc(1,sizeof(struct struct_node));
		(*new_sym)->scope_type = S_PROTO;
		break;
	}
	
	// Add symbol to symtable
	(*new_sym)->nodetype = stype;
	(*new_sym)->id = sname;
	(*new_sym)->chain = cur_sym;
	(*new_sym)->file = strdup(filename);
	(*new_sym)->line = line_num;
	(*new_sym)->scope_type = table->scope_type;
	
	return *new_sym;
}

/** Create new symbol table upon entering a new scope */
struct symtable *new_symtable(int stype) {
	struct symtable *old = cur_symtable;
	cur_symtable = calloc(1,sizeof(struct symtable));
	cur_symtable->prev = old;
	cur_symtable->scope_type = stype;
	
	return cur_symtable;
}

/** Remove current symbol table and return to nearest enclosing scope */
int remove_symtable() {
	struct symtable *old = cur_symtable;
	cur_symtable = old->prev;
	
	// Note: Memory leak. Freeing symbol table but not the symbols themselves.
	free(old);
	
	if (!cur_symtable)
		return -1; // Somehow outside of global scope
	return 0;
}

unsigned long hash(unsigned char *str) {
	unsigned long hash = 0;
	int c;

	while (c = *str++)
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash % TABLE_LENGTH;
}

/** Get pointer to symbol with identifier sname */
struct generic_node *get_sym(char *sname) {
	unsigned long hashval = hash(sname);
	struct generic_node *sym_ptr;
	struct symtable *table = cur_symtable;
	
	while (table) {
		if (sym_ptr = table->s[hashval])
			while (sym_ptr && strcmp(sname, sym_ptr->id))
				sym_ptr = sym_ptr->chain;
		
		if (sym_ptr)
			return sym_ptr;
		else
			table = table->prev;
	}
	
	return 0;
}

/*
CURRENTLY UNUSED

int set_sym(char *sname, long long sval) {
	struct symbol *sym = get_sym(sname);
	if (sym) {
		sym->ival = sval;
		return 0;
	}
	
	// later, modify to handle type mismatches
	return 1; //symbol doesn't exist
}

int set_sym_p(struct symbol *sym, long long sval) {
	if (sym) {
		sym->ival = sval;
		return 0;
	}
	return 1;
}

long long get_sym_p(struct symbol *sym) {
	if (sym) 
		return sym->ival;
	yyerror("symbol undefined");
}

*/
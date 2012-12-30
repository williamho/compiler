/* symtable.c by William Ho */
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "file_info.h"

struct symtable *cur_symtable;
struct symtable_map symtable_map;
extern int line_num;
extern char filename[MAX_STR_LEN];

struct symtable *new_file(char *fname) {
	unsigned long hashval = hash(fname);
	struct symtable *table, *new_table;
	
	if (table = symtable_map.st[hashval]) { // hashval collision
		while (table && strcmp(fname,table->file))
			table = table->chain;
		if (table) 
			return table;
	}
	
	table = symtable_map.st[hashval];
	new_table = new_symtable(S_FILE);
	new_table->chain = table;
	symtable_map.st[hashval] = new_table;
	return new_table;
}

struct generic_node *new_arr_node(int size) {
	struct arr_node *node = (struct arr_node *) new_node(N_ARR);
	node->base = 0;
	node->size = size;
	
	return (struct generic_node *) node;
}

struct generic_node *new_ptr_node() {
	struct ptr_node *node = (struct ptr_node *) new_node(N_PTR);
	node->to = 0;
	
	return (struct generic_node *) node;
}

struct generic_node *new_func_node() {
	struct func_node *node = (struct func_node *) new_node(N_FUNC);
	node->ret = 0;
	node->args = cur_symtable;
	
	return (struct generic_node *) node;
}

struct generic_node *new_node(int ntype) {
	struct generic_node *node;

	switch(ntype) {
	case N_TYPEDEF:
		node = malloc(sizeof(struct typedef_node));
		break;
	case N_PTR: 
		node = malloc(sizeof(struct ptr_node));
		break;
	case N_ARR:
		node = malloc(sizeof(struct arr_node));
		break;
	case N_FUNC:
		node = malloc(sizeof(struct func_node));
		break;
	case N_STRUCT:
	case N_UNION:
		node = malloc(sizeof(struct struct_tag));
		break;
	case N_ENUM: // Not implemented
		node = malloc(sizeof(struct generic_node)); 
		break;
	default:
		node = malloc(sizeof(struct generic_node));
		break;
	}
	node->nodetype = ntype;
	return node;
}

/** Add symbol to symbol table */
struct symbol *add_sym(struct symbol *sym, struct symtable *table) {	
	unsigned long hashval = hash(sym->id);
	struct symbol *new_sym, *cur_sym;
	char namespace, *file;
	int line;
	
	if (!table)
		table = cur_symtable;
	
	// Determmine the namespace based on the node type
	switch(sym->nodetype) {
	case N_VAR: 
		new_sym = malloc(sizeof(struct symbol)); 
		sym->namespace = NS_OTHER;
		break;
	case N_STRUCT:
		new_sym = calloc(1,sizeof(struct struct_tag)); 
		file = cur_symtable->file;
		line = cur_symtable->line;
		cur_symtable = cur_symtable->prev;
		sym->namespace = NS_STRUCT_TAG;
		break;
	case N_STRUCT_MEM:
		new_sym = calloc(1,sizeof(struct struct_member)); 
		sym->namespace = NS_STRUCT_MEM;
		break;
	case N_FUNC:
		new_sym = calloc(1,sizeof(struct func_node)); 
		sym->namespace = NS_OTHER;
		break;
	}
	
	if (cur_sym = table->s[hashval]) { // hashval collision
		// Compare names of existing symbols with that hash value
		while (cur_sym && !(sym->namespace == cur_sym->namespace && !strcmp(sym->id, cur_sym->id))) {
			cur_sym = cur_sym->chain;
		}
				
		if (cur_sym) {
			// If the symbol being added is a struct tag and has the same name as an incomplete struct tag
			if (sym->nodetype == N_STRUCT && !((struct struct_tag *)cur_sym)->complete) {
				// If the struct tag being added is also incomplete
				if (!((struct struct_tag *)sym)->complete) {
					free_sym(sym);
					return cur_sym;
				}
				
				// Replacing an incomplete struct tag with a complete one
				free(cur_sym->id);
				free(cur_sym->file);
				memcpy(cur_sym,sym,sizeof(struct struct_tag));
				((struct struct_tag *)cur_sym)->complete = 1;
				cur_sym->line = line;
				cur_sym->file = file;
				return cur_sym;
			}
			
			// Redeclaration
			yyerror("redefinition of '%s' previously declared at %s %d", sym->id, cur_sym->file, cur_sym->line);
			free_sym(sym);
			free(new_sym);
			return 0;
		}
		
		cur_sym = table->s[hashval];
	}
	
	// Change the file/line declaration info for structs to the beginning of the declaration
	if (sym->nodetype == N_STRUCT) {
		sym->line = line;
		sym->file = file;
	}
	sym->scope = table;
	sym->chain = cur_sym;
	memcpy(new_sym,sym,sizeof(struct symbol));
	table->s[hashval] = new_sym;

	if(new_sym->nodetype != N_FUNC && new_sym->scope->scope_type == S_FILE)
		new_global(new_sym);

	free(sym);
	return new_sym;
}

/** Return address of symbol with specified name and namespace in the specified symbol table */
struct symbol *get_sym(char *sname, char nspace, struct symtable *table) {
	unsigned long hashval = hash(sname);
	struct symbol *sym;
	char tableflag = 0;
	
	if (!table) {
		table = cur_symtable;
		tableflag = 1; // Check in upper symbol tables also
	}
	
	// If tableflag is set, check in current symbol table and all parent tables
	do {
		if (sym = table->s[hashval])
			while (sym && !(nspace == sym->namespace && !strcmp(sname, sym->id)))
				sym = sym->chain;
				
		if (sym) 
			return sym;
		else
			table = table->prev;
	} while (tableflag && table);
	return 0;
}

/** Create new symbol (not installed in any symbol table) */
struct symbol *new_sym(char *sname) {
	struct symbol *sym = malloc(sizeof(struct symbol));
	
	sym->nodetype = -1; // Don't know the type yet
	sym->id = sname;
	sym->file = strdup(filename);
	sym->line = line_num;
	sym->type = 0;
	sym->chain = 0;
	sym->scope = 0;
	
	return sym;
}

void free_sym(struct symbol *sym) {
	free(sym->file);
	free(sym->id);
	free(sym);
}

struct struct_tag *new_struct(char *struct_name, char complete) {
	struct struct_tag *st = (struct struct_tag *)new_sym(struct_name);
	st->nodetype = N_STRUCT;
	
	// If not given a name, don't add it to the symbol table
	// Also if a completed struct tag already exists, return 0
	if (struct_name && !(st = (struct struct_tag *)add_sym((struct symbol *)st,cur_symtable->prev))) {
		free(st);
		return 0;
	}
	
	st->members = cur_symtable;
	st->complete = complete;
	return st;
}

/** Create new symbol table upon entering a new scope */
struct symtable *new_symtable(int stype) {
	struct symtable *old = cur_symtable;
	cur_symtable = calloc(1,sizeof(struct symtable));
	if (stype == S_FILE)
		cur_symtable->prev = 0;
	else
		cur_symtable->prev = old;
	cur_symtable->scope_type = stype;
	cur_symtable->file = strdup(filename);
	cur_symtable->line = line_num;
	
	return cur_symtable;
}

/** Remove current symbol table and return to nearest enclosing scope */
int remove_symtable() {
	struct symtable *old = cur_symtable;
	
	if (cur_symtable->scope_type != S_FILE)
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


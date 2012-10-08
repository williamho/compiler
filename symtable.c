/* symtable.c by William Ho */
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "file_info.h"

struct symtable global_symtable = {S_FILE};
struct symtable *cur_symtable = &global_symtable;

struct symtable *new_symtable(int stype) {
	struct symtable *old = cur_symtable;
	cur_symtable = malloc(sizeof(struct symtable));
	cur_symtable->prev = old;
	cur_symtable->type = stype;
	
	return cur_symtable;
}

void remove_symtable() {
	struct symtable *old = cur_symtable;
	cur_symtable = old->prev;
	free(old);
}

unsigned long hash(unsigned char *str) {
	unsigned long hash = 0;
	int c;

	while (c = *str++)
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash % TABLE_LENGTH;
}

struct symbol *new_sym(char *sname) {
	struct symtable *table = cur_symtable; // may need to change later if the symbol is a label

	unsigned long hashval = hash(sname);
	struct symbol *sym_ptr;
	if (!table)
		table = cur_symtable;
	
	if (sym_ptr = table->s[hashval]) { // collision
		// Compare names of existing symbols with that hash value
		while (sym_ptr && strcmp(sname, sym_ptr->name))
			sym_ptr = sym_ptr->next;
				
		if (sym_ptr)
			return 0; // symbol exists, return null pointer
		
		sym_ptr = table->s[hashval];
	}
	
	// Add symbol to symtable
	table->s[hashval] = malloc(sizeof(struct symbol));
	table->s[hashval]->name = sname;
	table->s[hashval]->next = sym_ptr;
	table->s[hashval]->fname = strdup(filename);
	table->s[hashval]->line = line_num;
	return table->s[hashval];
}

/** Get pointer to symbol */

struct symbol *get_sym(char *sname) {
	unsigned long hashval = hash(sname);
	struct symbol *sym_ptr;
	struct symtable *table = cur_symtable;
	
	while (table) {
		if (sym_ptr = table->s[hashval])
			while (sym_ptr && strcmp(sname, sym_ptr->name))
				sym_ptr = sym_ptr->next;
		
		if (sym_ptr)
			return sym_ptr;
		else
			table = table->prev;
	}
	
	return 0;
}

int set_sym(char *sname, long long sval) {
	struct symbol *sym = get_sym(sname);
	if (sym) {
		sym->val = sval;
		return 0;
	}
	
	// later, modify to handle type mismatches
	return 1; //symbol doesn't exist
}
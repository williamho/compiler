/* symtable.h by William Ho */
#ifndef SYMTABLE_H
#define SYMTABLE_H

#define TABLE_LENGTH 256

struct symbol {
	char *name;
	long long val;
	struct symbol *next; // ptr to next sym in symtable with same hashval
	
	char *fname;
	int line;
};

enum scope_types {
	S_FILE, S_BLOCK, S_FUNC, S_PROTO
};

struct symtable {
	int type;
	struct symbol *s[TABLE_LENGTH];
	struct symtable *prev; // symbol table one level up
};

struct symtable *new_symtable(int stype);
int remove_symtable();
unsigned long hash(unsigned char *str);
struct symbol *new_sym(char *sname);
struct symbol *get_sym(char *sname);
long long get_sym_p(struct symbol *sym);
int set_sym(char *sname, long long sval);
int set_sym_p(struct symbol *sym, long long sval);
#endif

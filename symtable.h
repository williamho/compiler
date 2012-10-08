/* symtable.h by William Ho */
#ifndef SYMTABLE_H
#define SYMTABLE_H

#define TABLE_LENGTH 256

struct symbol {
	char *name;
	long long val;
	struct symbol *next;
	
	// the following are currently unused
	char *fname;
	int line;
};

enum symtable_types {
	S_FILE, S_BLOCK, S_FUNC, S_PROTO
};

struct symtable {
	int type;
	struct symbol *s[TABLE_LENGTH];
	struct symtable *prev; // symbol table one level up
};

struct symtable *new_symtable(int stype);
void remove_symtable();
unsigned long hash(unsigned char *str);
struct symbol *new_sym(char *sname);
struct symbol *get_sym(char *sname);
int set_sym(char *sname, long long sval);

#endif

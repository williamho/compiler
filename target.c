#include <stdio.h>
#include <stdlib.h>
#include "target.h"
#include "quads.h"
#include "globals.h"
#include "symtable.h"
#include "statements.h"
#include "expressions.h"
#include "declarations.h"
#include "y.tab.h"

struct string_lit *strings;
struct global *globals;
struct func_list *funcs;

print_target_code() {
	print_function_data();
	print_globals();
	print_strings();
	print_functions();
}

print_function_data() {
	struct func_list *fl = funcs;
	printf("\t.text\n");
	while (fl = fl->next) {
		printf("\t.globl\t%s\n",fl->id);
		printf("\t.type\t%s,@function\n",fl->id);
	}
	putchar('\n');
}

print_globals() {
	struct global *g = globals;
	struct symbol *sym;

	printf("\t.data\n");
	while(g = g->next) {
		sym = (struct symbol *)g->var;
		printf("\t.align 4\n");
		printf("\t.type %s,@object\n",sym->id);
		printf("\t.size %s,",sym->id);
		if (sym->type->nodetype == N_ARR)
			printf("%d",get_size_of_arr((struct arr_node *)(sym->type)));
		else
			printf("4");
		putchar('\n');
	}
	putchar('\n');

	return;
	// go through each statement, print quads
	struct func_list *fl = funcs;
	while (fl = fl->next) {
		printf("%s:",fl->id);
		print_quads(fl->bb);
		putchar('\n');
	}
}

print_strings() {
	struct string_lit *s = strings;
	printf("\t.section\t.rodata\n");
	while(s = s->next) {
		printf(".LC%d:\n",s->num);
		printf("\t.string\t\"%s\"\n",s->str);
	}
	putchar('\n');
}

print_functions() {
	struct func_list *fl = funcs;
	while (fl = fl->next) {
		printf("%s:\n",fl->id); // function name
		printf("\tpushl %%ebp\n");
		printf("\tmovl %%esp, %%ebp\n");
		printf("\t.subl %d, %%esp\n",fl->num_locals*4);
	}
	putchar('\n');
}


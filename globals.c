/* quads.c by William Ho  */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "globals.h"
#include "quads.h"
#include "symtable.h"
#include "statements.h"
#include "expressions.h"
#include "declarations.h"
#include "y.tab.h"
struct string_lit *strings;
struct global *globals;
int string_counter;

struct generic_node *new_string(char *str) {
	// add to list of strings
	struct string_lit *s = malloc(sizeof(struct string_lit));
	s->num = string_counter++;
	s->str = get_escaped_string(str);
	s->next = 0;
	s->last = s;
	strings->last->next = s;
	strings->last = s;

	// return a node
	struct symbol *sym = malloc(sizeof(struct symbol));
	char *const_name = malloc(10); 
	sprintf(const_name,".LC%d",s->num);
	sym->id = const_name;
	sym->nodetype = N_CONST;

	/*printf("string info: %s %s",sym->id,s->str);*/

	return (struct generic_node *)sym;
}

struct generic_node *new_global(struct generic_node *n) {
	struct global *g = malloc(sizeof(struct global));
	g->var = n;
	g->next = 0;
	g->last = g;
	globals->last->next = g;
	globals->last = g;

	return n;
}

char *get_escaped_string(char *p) {
	char *s = malloc(strlen(p)*4);

	char *read=p, *write=s;
	int i=0, j=0;
	char c;
	while (c = *read++) {
		if (isprint(c)) {
			*write++ = c;
		}
		else {
			write += sprintf(write,"\\%02d",(unsigned char)c);
		}
	}
	*write = '\0';
	return s;
}


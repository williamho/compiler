/* globals.h by William Ho  */
#ifndef GLOBALS_H
#define GLOBALS_H

struct string_lit {
	int num;
	char *str;
	struct string_lit *next, *last;
};

struct generic_node;
struct global {
	struct generic_node *var;
	struct global *next, *last;
};

struct generic_node *new_string(char *str);
struct generic_node *new_global(struct generic_node *n);
char *get_escaped_string(char *p);
#endif


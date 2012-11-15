#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "declarations.h"
#include "symtable.h"

/** Start a new declarator list */
void new_declarator_list(struct declarator_list *dl, struct declarator *d) {
	dl->rightmost = dl->leftmost = d;
	d->next = 0;
}

/** Add declarator "d" to existing declarator list "from", 
	store the result in "to" */
void add_declarator_list(struct declarator_list *to, 
	struct declarator_list *from, struct declarator *d) {
	to->leftmost = from->leftmost;
	to->rightmost = d;
	from->rightmost->next = d;
	to->rightmost->next = 0;
}

struct declarator *new_declarator(struct generic_node *n) {
	struct declarator *d = malloc(sizeof(struct declarator));
	d->top = d->deepest = n;
	return d;
}

/** Start a new decl_spec list */
struct decl_spec *new_spec(char which, char val) {
	struct decl_spec *spec = malloc(sizeof (struct decl_spec));
	spec->type = spec->storage = spec->qualifier = -1;
	switch (which) {
	case TS: spec->type = val; break;
	case SC: spec->storage = val; break;
	case TQ: spec->qualifier = val; break;
	}
	
	spec->next = 0;
	return spec;
}

/** Process a declarator list and install the symbols into a symbol table */
void new_declaration(struct decl_spec *d, struct declarator_list *dl) {
	char type_flags[TS_COUNT] = {0}, storage_flags[SC_COUNT] = {0}, qual_flags[TQ_COUNT] = {0};
	char which, storage, qualifiers, *ret;
	char ts, sc, tq;
	struct generic_node *node, *type_node;
	struct decl_spec *old_spec;
	struct declarator *dec, *dec_old;
	struct symbol *sym;
	
	// Go through the decl specs and check if it is a valid combination
	while(d) {
		if (d->type >= 0) {
			if (d->type == TS_STRUCT || d->type == TS_TYPENAME)
				node = d->node; 
			type_flags[d->type]++; 
		}
		if (d->storage >= 0)
			storage_flags[d->storage]++;
		if (d->qualifier >= 0)
			qual_flags[d->qualifier]++; 
		
		old_spec = d;
		d = d->next;
		free(old_spec);
	}
	ts = check_type_specs(type_flags);
	sc = check_storage_classes(storage_flags);
	tq = 0; // Ignore type qualifiers for now
	
	// If valid, complete the declaration and print the info
	if (ts >= 0 && sc>= 0) {
		if (ts == N_STRUCT || ts == N_TYPENAME)
			type_node = node;
		else
			type_node = new_node(ts);
			
		for (dec = dl->leftmost; dec != 0; dec = dec->next, free(dec_old)) {
			((struct ptr_node *)dec->top)->to = type_node;
			((struct symbol *)dec->deepest)->storage = sc;
			
			if (sym = add_sym((struct symbol *)dec->deepest,0))
				print_node_info_r((struct generic_node *)sym);
			dec_old = dec; // old declarator freed before next loop
		}
	}
}

/** Print information about a node and its linked nodes, recursively */
void print_node_info_r(struct generic_node *node) {
	struct ptr_node *n = (struct ptr_node *) node;
	int i, depth = 0;
	
	while (n->nodetype == N_ARR || n->nodetype == N_PTR || 
			n->nodetype == N_VAR || n->nodetype == N_STRUCT_MEM) {	
		for (i=0; i<depth; i++)
			printf("   ");
		depth++;
		print_node_info((struct generic_node *)n);
		putchar('\n');
		
		n = (struct ptr_node *)(n->to);
	}	
	
	for (i=0; i<depth; i++)
		printf("   ");		
	print_node_info((struct generic_node *)n);
	printf("\n\n");
}

/** Print into about a node */
void print_node_info(struct generic_node *node) {
	struct symbol *n = (struct symbol *)node;
	switch(node->nodetype) {
	case N_VAR:
	case N_STRUCT_MEM:
		printf("'%s' declared at %s:%d ",n->id,n->file,n->line);

		putchar('[');
		switch(n->scope->scope_type) {
		case S_FILE: printf("file"); break;
		case S_BLOCK: printf("block"); break;
		case S_FUNC: printf("function"); break;
		case S_PROTO: printf("prototype"); break;
		case S_STRUCT: printf("struct/union"); break;
		}
		
		printf(" scope starting at %s:%d] as\n",n->scope->file,n->scope->line);
		if (n->storage != SC_AUTO) {
			switch(n->storage)  {
			case SC_TYPEDEF: printf("typedef"); break;
			case SC_EXTERN: printf("extern"); break; 
			case SC_STATIC: printf("static"); break;
			case SC_REGISTER: printf("register"); break;
			break;
			}
			putchar(' ');
		}
		printf("variable of type");
		break;
	case N_ARR:	printf("array of %d",((struct arr_node *)node)->size);	break;
	case N_PTR: printf("pointer to");	break;
	case N_VOID: printf("void"); break;
	case N_CHAR: printf("char"); break;
	case N_UCHAR: printf("unsigned char"); break;
	case N_SHORT: printf("short"); break;
	case N_USHORT: printf("unsigned short"); break;
	case N_INT: printf("int"); break;
	case N_UINT: printf("unsigned int"); break; 
	case N_LONG: printf("long"); break;
	case N_ULONG: printf("unsigned long"); break;
	case N_LONGLONG: printf("long long"); break;
	case N_ULONGLONG: printf("unsigned long long"); break;
	case N_FLOAT: printf("float"); break;
	case N_CFLOAT: printf("complex float"); break;
	case N_DOUBLE: printf("double"); break;
	case N_CDOUBLE: printf("complex double"); break;
	case N_LONGDOUBLE: printf("long double"); break;
	case N_CLONGDOUBLE: printf("complex long double"); break;
	case N_BOOL: printf("bool"); break;
	case N_STRUCT: 
		printf("struct %s (defined at %s:%d)",n->id,n->file,n->line); 
		break;
	case N_UNION: printf("union"); break;
	case N_ENUM: printf("enum"); break;
	case N_TYPEDEF: printf("typedef"); break;
	case N_FUNC: printf("function"); break;

	default:
		printf("type %d",node->nodetype);
		break;
	}
	putchar(' ');
}

/** Check declaration specifiers (type, storage, qualifiers).
	If valid, return pointer to an array of three chars specifying the values 
	for each declaration specifier */
	/*
char *check_decl_specs(struct decl_spec *spec) {
	char type_flags[TS_COUNT] = {0}, storage_flags[SC_COUNT] = {0}, qual_flags[TQ_COUNT] = {0};
	char which, storage, qualifiers, *ret;
	struct generic_node *node;
	struct decl_spec *old_spec;
	
	while(spec) {
		switch(spec->which) {
		case TS: 
			switch (spec->type) {
			case TS_STRUCT: node = spec->node; break;
			case TS_FUNC: node = spec->node; break;
			case TS_TYPENAME: node = spec->node; break;
			}
			type_flags[spec->type]++; 
			break;
		case SC: storage_flags[spec->storage]++; break;
		case TQ: qual_flags[spec->qualifier]++; break;
		}
		
		old_spec = spec;
		spec = spec->next;
		free(old_spec);
	}
	
	ret = malloc(3); // 0: type, 1: storage, 2: qualifier
	ret[TS]= check_type_specs(type_flags);
	ret[SC] = check_storage_classes(storage_flags);
	ret[TQ] = 0; // Ignore type qualifiers for now
	
	return ret;
}*/

/** Check validity of storage classes. If valid, return which storage class. */
int check_storage_classes(char *sc) {
	int i, total = 0;
	for (i=0; i<SC_COUNT; i++)
		total += sc[i];
	if (total > 1) {
		yyerror("more than one storage class specified in declaration");
		return -1; 
	}
	if (total == 0) // No storage class specified
		return SC_AUTO;
	for (i=1; i<SC_COUNT && !sc[i]; i++);
		
	return i;
}
 
/** Check validity of type specifiers. If valid, return which type spec. */
int check_type_specs(char *ts) {
	int i, total = 0;
	for (i=0; i<TS_COUNT; i++) {
		total += ts[i];
		if ((i!=TS_LONG && ts[i]>1) || (i==TS_LONG && ts[i]>2)) {
			yyerror("duplicate type specifiers in declaration");
			return -1; // Too many of the same keyword, i.e. "void void"
		}
	}
	
	if (total == 0) {
		yyerror("no type specifiers in declaration");
		return -2;
	}
	
	/* Need to check the total so that things like 
		"long _Complex x;" aren't valid	*/
	if (total == ts[TS_VOID])
		return N_VOID;
		
	if (total == ts[TS_CHAR] ||
		total == 2 && total == ts[TS_SIGNED]+ts[TS_CHAR])
		return N_CHAR;
		
	if (total == 2 && total == ts[TS_UNSIGNED]+ts[TS_CHAR])
		return N_UCHAR;
	
	if (total == ts[TS_SHORT] ||
		total == 2 && total == ts[TS_SIGNED]+ts[TS_SHORT] ||
		total == 2 && total == ts[TS_SHORT]+ts[TS_INT] ||
		total == 3 && total == ts[TS_SIGNED]+ts[TS_SHORT]+ts[TS_INT])
		return N_SHORT;
		
	if (total == 2 && total == ts[TS_UNSIGNED]+ts[TS_SHORT] ||
		total == 3 && total == ts[TS_UNSIGNED]+ts[TS_SHORT]+ts[TS_INT])
		return N_USHORT;
	
	if (total == ts[TS_INT] ||
		total == ts[TS_SIGNED] ||
		total == 2 && total == ts[TS_SIGNED]+ts[TS_INT])
		return N_INT;
		
	if (total == ts[TS_UNSIGNED] ||
		total == 2 && total == ts[TS_UNSIGNED]+ts[TS_INT])
		return N_UINT;
	
	if (total == 1 && total == ts[TS_LONG] ||
		total == 2 && total == ts[TS_SIGNED]+ts[TS_LONG] && ts[TS_SIGNED] ||
		total == 2 && total == ts[TS_LONG]+ts[TS_INT] && ts[TS_INT] ||
		total == 3 && total == ts[TS_SIGNED]+ts[TS_LONG]+ts[TS_INT] && ts[TS_SIGNED] && ts[TS_INT])
		return N_LONG;
	
	if (total == 2 && total == ts[TS_UNSIGNED]+ts[TS_LONG] && ts[TS_UNSIGNED] ||
		total == 3 && total == ts[TS_UNSIGNED]+ts[TS_LONG]+ts[TS_INT] && ts[TS_UNSIGNED] && ts[TS_INT])
		return N_ULONG;
		
	if (total == 2 && total == ts[TS_LONG] ||
		total == 3 && total == ts[TS_SIGNED]+ts[TS_LONG] && ts[TS_SIGNED] ||
		total == 3 && total == ts[TS_LONG]+ts[TS_INT] && ts[TS_INT] ||
		total == 4 && total == ts[TS_SIGNED]+ts[TS_LONG]+ts[TS_INT] && ts[TS_SIGNED] && ts[TS_INT])
		return N_LONGLONG;
	
	if (total == 3 && total == ts[TS_UNSIGNED]+ts[TS_LONG] && ts[TS_UNSIGNED] ||
		total == 4 && total == ts[TS_UNSIGNED]+ts[TS_LONG]+ts[TS_INT] && ts[TS_UNSIGNED] && ts[TS_INT])
		return N_ULONGLONG;
		
	if (total == ts[TS_FLOAT])
		return N_FLOAT;
		
	if (total == ts[TS_DOUBLE])
		return N_DOUBLE;
		
	if (total == 2 && total == ts[TS_LONG]+ts[TS_DOUBLE])
		return N_LONGDOUBLE;
		
	if (total == ts[TS_BOOL])
		return N_BOOL;
		
	if (total == 2 && total == ts[TS_FLOAT]+ts[TS_COMPLEX])
		return N_CFLOAT;
		
	if (total == 2 && total == ts[TS_DOUBLE]+ts[TS_COMPLEX])
		return N_CDOUBLE;
		
	if (total == 3 && total == ts[TS_LONG]+ts[TS_DOUBLE]+ts[TS_COMPLEX] && ts[TS_LONG] == 1)
		return N_CLONGDOUBLE;
	
	if (total == ts[TS_STRUCT])
		return N_STRUCT;
	
	if (total == ts[TS_ENUM])
		return N_ENUM;
		
	if (total == ts[TS_TYPENAME]) {
		return 0; // actually, return the type
	}

	else {
		yyerror("invalid combination of type specifiers in declaration");
		return -3; 
	}
}

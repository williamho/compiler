#include <string.h>
#include <stdio.h>
#include "declarations.h"
#include "symtable.h"

void print_node_info_r(struct generic_node *node) {
	struct ptr_node *n = (struct ptr_node *) node;
	int i, depth = 0;
	
	while (n->nodetype == N_ARR || n->nodetype == N_PTR || n->nodetype == N_IDENT) {
		for (i=0; i<depth; i++)
			printf("  ");
		
		print_node_info((struct generic_node *)n);
		if (n->to == (struct generic_node *)n || n->to == 0)
			break;
		n = (struct ptr_node *)(n->to);
		
		depth++;
	}	

	for (i=0; i<depth; i++)
		printf("  ");
		
	//print_node_info((struct generic_node *)n);
}

void print_node_info(struct generic_node *node) {
	switch(node->nodetype) {
	case N_ARR:
		printf("array of %d\n",((struct arr_node *)node)->size);
		break;
	case N_PTR:
		printf("pointer to\n");
		break;
	case N_IDENT:
		printf("symbol '%s'\n",((struct symbol *)node)->id);
		break;
	default:
		printf("type %d\n",node->nodetype);
		break;
	}
}

void print_decl_info(char *ts, char *sc, char *tq) {
	if (tq[TQ_CONST]) printf("const ");
	if (tq[TQ_RESTRICT]) printf("restrict ");
	if (tq[TQ_VOLATILE]) printf("volatile ");

	if (sc[SC_TYPEDEF]) printf("typedef ");
	if (sc[SC_EXTERN]) printf("extern ");
	if (sc[SC_STATIC]) printf("static ");
	if (sc[SC_AUTO]) printf("auto ");
	if (sc[SC_REGISTER]) printf("register ");

	if (ts[TS_SIGNED]) printf("signed ");
	if (ts[TS_UNSIGNED]) printf("unsigned ");
	if (ts[TS_VOID]) printf("void ");
	if (ts[TS_CHAR]) printf("char ");
	if (ts[TS_SHORT]) printf("short ");
	if (ts[TS_LONG] == 1) printf("long ");
	if (ts[TS_LONG] == 2) printf("long long ");
	if (ts[TS_INT]) printf("int ");
	if (ts[TS_FLOAT]) printf("float ");
	if (ts[TS_DOUBLE]) printf("double ");
	if (ts[TS_BOOL]) printf("bool ");
	if (ts[TS_COMPLEX]) printf("complex ");
	if (ts[TS_STRUCT]) printf("struct ");
	if (ts[TS_ENUM]) printf("enum ");
	if (ts[TS_TYPENAME]) printf("typename ");
	putchar('\n');
}

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
	for (i=1; sc[i]; i++);
		
	return i;
}
 
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
		return N_LONGDOUBLE;
	
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

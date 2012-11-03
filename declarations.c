#include <string.h>
#include <stdio.h>
#include "declarations.h"

void print_decl_info(char *ts, char *sc) {
	if (sc[SC_TYPEDEF]) printf("typedef ");
	if (sc[SC_EXTERN]) printf("extern ");
	if (sc[SC_STATIC]) printf("static ");
	if (sc[SC_AUTO]) printf("auto ");
	if (sc[SC_REGISTER]) printf("register ");

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
	if (ts[TS_SIGNED]) printf("signed ");
	if (ts[TS_UNSIGNED]) printf("unsigned ");
	putchar('\n');
}

int check_storage_classes(char *sc) {
	int i, total = 0;
	for (i=0; i<SC_COUNT; i++)
		total += sc[i];
	if (total > 1) {
		yyerror("more than one storage class specified in declaration");
		return 1; 
	}
	return 0;
}
 
int check_type_specs(char *ts) {
	int i, total = 0;
	for (i=0; i<TS_COUNT; i++) {
		total += ts[i];
		if ((i!=TS_LONG && ts[i]>1) || (i==TS_LONG && ts[i]>2)) {
			yyerror("duplicate type specifiers in declaration");
			return 1; // Too many of the same keyword, i.e. "void void"
		}
	}
	
	if (total == 0) {
		yyerror("no type specifiers in declaration");
		return 2;
	}
	
	/* Need to check the total so that things like 
		"long _Complex x;" aren't valid	*/
	if (total == ts[TS_VOID] ||
	
		total == ts[TS_CHAR] ||
		total == 2 && total == ts[TS_SIGNED]+ts[TS_CHAR] ||
		
		total == 2 && total == ts[TS_UNSIGNED]+ts[TS_CHAR] ||
		
		total == ts[TS_SHORT] ||
		total == 2 && total == ts[TS_SIGNED]+ts[TS_SHORT] ||
		total == 2 && total == ts[TS_SHORT]+ts[TS_INT] ||
		total == 3 && total == ts[TS_SIGNED]+ts[TS_SHORT]+ts[TS_INT] ||
		
		total == 2 && total == ts[TS_UNSIGNED]+ts[TS_SHORT] ||
		total == 3 && total == ts[TS_UNSIGNED]+ts[TS_SHORT]+ts[TS_INT] ||
		
		total == ts[TS_INT] ||
		total == ts[TS_SIGNED] ||
		total == 2 && total == ts[TS_SIGNED]+ts[TS_INT] ||
		
		total == ts[TS_UNSIGNED] ||
		total == 2 && total == ts[TS_UNSIGNED]+ts[TS_INT] ||
		
			// Works for "long" and "long long"
		total <= 2 && total == ts[TS_LONG] ||
		total <= 3 && total == ts[TS_SIGNED]+ts[TS_LONG] && ts[TS_SIGNED] ||
		total <= 3 && total == ts[TS_LONG]+ts[TS_INT] && ts[TS_INT] ||
		total <= 4 && total == ts[TS_SIGNED]+ts[TS_LONG]+ts[TS_INT] && ts[TS_SIGNED] && ts[TS_INT] ||
		
		total <= 3 && total == ts[TS_UNSIGNED]+ts[TS_LONG] && ts[TS_UNSIGNED] ||
		total <= 3 && total == ts[TS_UNSIGNED]+ts[TS_LONG]+ts[TS_INT] && ts[TS_UNSIGNED] && ts[TS_INT] ||
		
		// Real types
		total == ts[TS_FLOAT] ||
		
		total == ts[TS_DOUBLE] ||
		
		total == 2 && total == ts[TS_LONG]+ts[TS_DOUBLE] ||
		
		total == ts[TS_BOOL] ||
		
		total == 2 && total == ts[TS_FLOAT]+ts[TS_COMPLEX] ||
		
		total == 2 && total == ts[TS_DOUBLE]+ts[TS_COMPLEX] ||
		
		total == 3 && total == ts[TS_LONG]+ts[TS_DOUBLE]+ts[TS_COMPLEX] && ts[TS_LONG] == 1 ||
		
		// Misc
		total == ts[TS_STRUCT] ||
		
		total == ts[TS_ENUM] ||
		
		total == ts[TS_TYPENAME]
		)
		return 0;
	else {
		yyerror("invalid combination of type specifiers in declaration");
		return 3; 
	}
}

#include "declarations.h"

int check_storage_classes(char *sc) {
	int i, total = 0;
	for (i=0; i<SC_COUNT; i++)
		total += sc[i];
	if (total > 1)
		return 1; // More than one storage class specified
	return 0;
}
 
int check_type_specs(char *ts) {
	int i, total = 0;
	for (i=0; i<TS_COUNT; i++) {
		total += ts[i];
		if ((i!=TS_LONG && ts[i]>1) || (i==TS_LONG && ts[i]>2))
			return 1; // Too many of the same keyword, i.e. "void void"
	}
	
	if (total == ts[TS_VOID] ||
	
		total == ts[TS_CHAR] ||
		
		total == ts[TS_SIGNED]+ts[TS_CHAR] ||
		total == ts[TS_UNSIGNED]+ts[TS_CHAR] ||
		
		total == ts[TS_SHORT] ||
		total == ts[TS_SIGNED]+ts[TS_SHORT] ||
		total == ts[TS_SHORT]+ts[TS_INT] ||
		total == ts[TS_SIGNED]+ts[TS_SHORT]+ts[TS_INT] ||
		
		total == ts[TS_UNSIGNED]+ts[TS_SHORT] ||
		total == ts[TS_UNSIGNED]+ts[TS_SHORT]+ts[TS_INT] ||
		
		total == ts[TS_INT] ||
		total == ts[TS_SIGNED] ||
		total == ts[TS_SIGNED]+ts[TS_INT] ||
		
		total == ts[TS_UNSIGNED] ||
		total == ts[TS_UNSIGNED]+ts[TS_INT] ||
		
			// Works for "long" and "long long"
		total == ts[TS_LONG] ||
		total == ts[TS_SIGNED]+ts[TS_LONG] ||
		total == ts[TS_LONG]+ts[TS_INT] ||
		total == ts[TS_SIGNED]+ts[TS_LONG]+ts[TS_INT] ||
		
		total == ts[TS_UNSIGNED]+ts[TS_LONG] ||
		total == ts[TS_UNSIGNED]+ts[TS_LONG]+ts[TS_INT] ||
		
		// Real types
		total == ts[TS_FLOAT] ||
		
		total == ts[TS_DOUBLE] ||
		
		total == ts[TS_LONG]+ts[TS_DOUBLE] ||
		
		total == ts[TS_LONG]+ts[TS_BOOL] ||
		
		total == ts[TS_FLOAT]+ts[TS_COMPLEX] ||
		
		total == ts[TS_DOUBLE]+ts[TS_COMPLEX] ||
		
		total == ts[TS_LONG]+ts[TS_DOUBLE]+ts[TS_COMPLEX] ||
		
		// Misc
		total == ts[TS_STRUCT] ||
		
		total == ts[TS_ENUM] ||
		
		total == ts[TS_TYPENAME]
		)
		return 0;
	else
		return 2; // Invalid combination of type specifiers
}
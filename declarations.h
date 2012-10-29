/* declarations.h by William Ho */
#ifndef DECLARATIONS_H
#define DECLARATIONS_H

enum type_specs {
	TS_VOID, TS_CHAR, TS_SHORT, TS_INT, TS_LONG,
	TS_FLOAT, TS_DOUBLE, TS_BOOL, TS_COMPLEX, 
	TS_STRUCT, TS_ENUM, TS_TYPENAME, 
	TS_SIGNED, TS_UNSIGNED, 
	TS_COUNT
};

enum storage_classes {
	SC_TYPEDEF, SC_EXTERN, SC_STATIC, SC_AUTO, SC_REGISTER, SC_COUNT
};
 
int check_storage_classes(char *sc);
int check_type_specs(char *ts);
#endif
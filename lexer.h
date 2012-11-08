/* lexer.h by William Ho */
#ifndef LEXER_H
#define LEXER_H

enum ntype {
	N_INT, N_LONG, N_LONGLONG, N_FLOAT, N_DOUBLE, N_LONGDOUBLE
};

struct num {
	int ntype;  /* 0: int,   1: long,   2: long long, 
				   3: float, 4: double, 5: long double */
	int is_unsigned;
	long long int ival;
	long double rval;
};

typedef union {
	char cval;
	char *sval;
	char *idval;
	struct num num;
} YYSTYPE;
extern YYSTYPE yylval;

enum tokens {
	TOKEOF=0,
	IDENT=257,
	CHARLIT,
	STRING,
	NUMBER,
	INDSEL,
	PLUSPLUS,
	MINUSMINUS,
	SHL,
	SHR,
	LTEQ,
	GTEQ,
	EQEQ,
	NOTEQ,
	LOGAND,
	LOGOR,
	ELLIPSIS,
	TIMESEQ,
	DIVEQ,
	MODEQ,
	PLUSEQ,
	MINUSEQ,
	SHLEQ,
	SHREQ,
	ANDEQ,
	OREQ,
	XOREQ,
	AUTO,
	BREAK,
	CASE,
	CHAR,
	CONST,
	CONTINUE,
	DEFAULT,
	DO,
	DOUBLE,
	ELSE,
	ENUM,
	EXTERN,
	FLOAT,
	FOR,
	GOTO,
	IF,
	INLINE,
	INT,
	LONG,
	REGISTER,
	RESTRICT,
	RETURN,
	SHORT,
	SIGNED,
	SIZEOF,
	STATIC,
	STRUCT,
	SWITCH,
	TYPEDEF,
	UNION,
	UNSIGNED,
	VOID,
	VOLATILE,
	WHILE,
	_BOOL,
	_COMPLEX,
	_IMAGINARY
};

#endif
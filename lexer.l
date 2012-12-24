/* lexer.l by William Ho */
%option noyywrap

%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "symtable.h"
#include "declarations.h"
#include "file_info.h"
#include "y.tab.h"
YYSTYPE yylval;

void yyerror(char *s, ...); 
void yywarn(char *s, ...); 
void yyprint(char *s, ...); 
void get_int_info();
void get_real_info();
void print_escaped_char(char c);

int str_len;
int line_num=1;
char str_buf[MAX_STR_LEN];
char filename[MAX_STR_LEN] = "<stdin>";
char *buf_ptr;
struct symtable *cur_symtable;
%}

NUM [0-9]
ALPHA [a-zA-Z_]
EXP [eE][+-]?([0-9]+)?
BEXP [pP][+-]?[0-9]+
HEX [0-9a-fA-F]
LONG (l|L|ll|LL)
INTSUFFIX ([uU]{LONG}?|{LONG}[uU]?)
FLOATSUFFIX [fFlL]

%x STR CHARA LINENUM FILENAME FLAGS
%%

# { BEGIN(LINENUM); }
<LINENUM>{
	[0-9]+ { line_num = atoi(yytext); BEGIN(FILENAME);  }
	.	  { }
}
<FILENAME>{
	\".*\" { 
		yytext[yyleng-1] = 0; 
		sscanf(yytext+1,"%s",filename); 
		
		// Create new symbol table with file scope
		cur_symtable = new_file(filename);
		//new_symtable(S_FILE);
		BEGIN(FLAGS); 
	}
	.	  { }
}
<FLAGS>.*\n { BEGIN(INITIAL); }

auto	   { return AUTO; }
break	  { return BREAK; }
case	   { return CASE; }
char	   { return CHAR; }
const	  { return CONST; }
continue   { return CONTINUE; }
default	{ return DEFAULT; }
do		 { return DO; }
double	 { return DOUBLE; }
else	   { return ELSE; }
enum	   { return ENUM; }
extern	 { return EXTERN; }
float	  { return FLOAT; }
for		{ return FOR; }
goto	   { return GOTO; }
if		 { return IF; }
inline	 { return INLINE; }
int		{ return INT; }
long	   { return LONG; }
register   { return REGISTER; }
restrict   { return RESTRICT; }
return	 { return RETURN; }
short	  { return SHORT; }
signed	 { return SIGNED; }
sizeof	 { return SIZEOF; }
static	 { return STATIC; }
struct	 { return STRUCT; }
switch	 { return SWITCH; }
typedef	{ return TYPEDEF; }
union	  { return UNION; }
unsigned   { return UNSIGNED; }
void	   { return VOID; }
volatile   { return VOLATILE; }
while	  { return WHILE; }
_Bool	  { return _BOOL; }
_Complex   { return _COMPLEX; }
_Imaginary { return _IMAGINARY; }

{ALPHA}[a-zA-Z0-9_]*  { 
	struct symbol *node;
	yylval.sval = strdup(yytext); 
	
	// TODO: turn typedefs on/off
	if ((node = get_sym(yylval.sval,NS_OTHER,0)) && node->storage == SC_TYPEDEF)
		return TYPEDEF_NAME;
			
	return IDENT; 
}

{NUM}+{EXP}{FLOATSUFFIX}?	  { get_real_info(); return NUMBER; }
{NUM}*\.{NUM}+{EXP}?{FLOATSUFFIX}? { get_real_info(); return NUMBER; }
{NUM}+\.{NUM}*{EXP}?{FLOATSUFFIX}? { get_real_info(); return NUMBER; }

0[xX]{HEX}*\.{HEX}+{BEXP}{FLOATSUFFIX}? { get_real_info(); return NUMBER; }
0[xX]{HEX}+\.?{BEXP}{FLOATSUFFIX}? { get_real_info(); return NUMBER; }

0[xX]{HEX}+{INTSUFFIX}? { sscanf(yytext+2, "%llx", &yylval.num.ival); get_int_info(); return NUMBER; }
0[0-7]*{INTSUFFIX}?	 { sscanf(yytext, "%llo", &yylval.num.ival); get_int_info(); return NUMBER; }
[1-9][0-9]*{INTSUFFIX}? { sscanf(yytext, "%lld", &yylval.num.ival); get_int_info(); return NUMBER; }

L?\'		         { BEGIN(CHARA); str_len=0; buf_ptr=str_buf; }
L?\"		         { BEGIN(STR); str_len=0; buf_ptr=str_buf; }

<CHARA>{
	\' { 
		BEGIN(INITIAL);
		yylval.cval = str_buf[0];
		
		if (str_len>1) {
			yytext[0] = str_buf[0];
			yywarn("multi-byte character constant truncated to first byte: %s",yytext); 
		}
		else if (str_len == 0) {
			yytext[0] = 0;
			yyerror("empty character constant"); 
		}
		return CHARLIT;
	}
}

<STR>{
	\" { 
		BEGIN(INITIAL);
		if (str_len>MAX_STR_LEN) {
			str_len = MAX_STR_LEN;
			yywarn("string length exceeds maximum length");
		}
		*buf_ptr++ = 0;
		yylval.sval = malloc(str_len+1);
		memcpy(yylval.sval,str_buf,str_len+1);
		return STRING;
	}
}
	
<CHARA,STR>{
	\\a	      { if (str_len++<MAX_STR_LEN) *buf_ptr++ = '\a'; }
	\\b	      { if (str_len++<MAX_STR_LEN) *buf_ptr++ = '\b'; }
	\\f	      { if (str_len++<MAX_STR_LEN) *buf_ptr++ = '\f'; }
	\\n	      { if (str_len++<MAX_STR_LEN) *buf_ptr++ = '\n'; }
	\\r		  { if (str_len++<MAX_STR_LEN) *buf_ptr++ = '\r'; }
	\\t	      { if (str_len++<MAX_STR_LEN) *buf_ptr++ = '\t'; }
	\\v		  { if (str_len++<MAX_STR_LEN) *buf_ptr++ = '\v'; }
	\\0		  { if (str_len++<MAX_STR_LEN) *buf_ptr++ = '\0'; }
	\\\\	     { if (str_len++<MAX_STR_LEN) *buf_ptr++ = '\\'; }
	\\\"	     { if (str_len++<MAX_STR_LEN) *buf_ptr++ = '\"'; }
	\\'		  { if (str_len++<MAX_STR_LEN) *buf_ptr++ = '\''; }
	\\\?	     { if (str_len++<MAX_STR_LEN) *buf_ptr++ = '?'; }
	\\[0-7]{1,3} { 
		// Octal escape sequence 
		int val;
		sscanf(yytext+1, "%o", &val);
		if (val > 0xff) 
			yywarn("octal escape sequence out of range: %s",yytext);
		*buf_ptr++ = val;
		str_len++;
	}
	\\x{HEX}+	{ 
		// Hex escape sequence
		int val;
		sscanf(yytext+2, "%x", &val);
		if (val > 0xff) 
			yywarn("hex escape sequence out of range: %s",yytext);
		*buf_ptr++ = val;
		str_len++;
	}
	\\x" "	   { yyerror("\\x used with no following hex digits"); }
	\n	       { yyerror("unexpected newline"); }
	\\.		  { yywarn("unknown escape sequence %s",yytext); if (str_len++<MAX_STR_LEN) *buf_ptr++ = yytext[1]; }
	.		    { strcpy(buf_ptr,yytext); buf_ptr += yyleng; str_len += yyleng; }
}

"->"		{ return(INDSEL); }
"++"		{ return(PLUSPLUS); }
"--"		{ return(MINUSMINUS); }
"<<"		{ return(SHL); }
">>"		{ return(SHR); }
"<="		{ return(LTEQ); }
">="		{ return(GTEQ); }
"=="		{ return(EQEQ); }
"!="		{ return(NOTEQ); }
"&&"		{ return(LOGAND); }
"||"		{ return(LOGOR); }
"..."	   { return(ELLIPSIS); }
"*="		{ return(TIMESEQ); }
"/="		{ return(DIVEQ); }
"%="		{ return(MODEQ); }
"+="		{ return(PLUSEQ); }
"-="		{ return(MINUSEQ); }
"<<="	   { return(SHLEQ); }
">>="	   { return(SHREQ); }
"&="		{ return(ANDEQ); }
"|="		{ return(OREQ); }
"^="		{ return(XOREQ); }

";"		 { return(';'); }
("{"|"<%")  { return('{'); }
("}"|"%>")  { return('}'); }
","		 { return(','); }
":"		 { return(':'); }
"="		 { return('='); }
"("		 { return('('); }
")"		 { return(')'); }
("["|"<:")  { return('['); }
("]"|":>")  { return(']'); }
"."		 { return('.'); }
"&"		 { return('&'); }
"!"		 { return('!'); }
"~"		 { return('~'); }
"-"		 { return('-'); }
"+"		 { return('+'); }
"*"		 { return('*'); }
"/"		 { return('/'); }
"%"		 { return('%'); }
"<"		 { return('<'); }
">"		 { return('>'); }
"^"		 { return('^'); }
"|"		 { return('|'); }
"?"		 { return('?'); }

\n		  { line_num++; }
[ \t\v\f] {}
.		   { yyerror("unexpected token"); }

%%
void get_int_info() {
	int i;
	yylval.num.is_unsigned = 0;
	yylval.num.ntype = N_INT;
	for (i=yyleng-1;; i--) {
		if (yytext[i] == 'u' || yytext[i] == 'U')
			yylval.num.is_unsigned = 1;
		else if (yytext[i] == 'l' || yytext[i] == 'L')
			yylval.num.ntype++;
		else
			break;
	}
}

void get_real_info() {
	int i = yyleng-1;
	
	sscanf(yytext,"%Lf",&yylval.num.rval);
	
	if (yytext[i] == 'f' || yytext[i] == 'F')
		yylval.num.ntype = N_FLOAT;
	else if (yytext[i] == 'l' || yytext[i] == 'L')
		yylval.num.ntype = N_LONGDOUBLE;
	else
		yylval.num.ntype = N_DOUBLE;
}

void print_escaped_string(char *p) {
	int i=0;
	char c;
	while(c = p[i++])
		print_escaped_char(c);
}

void print_escaped_char(char c) {
	switch(c) {
		case '\\': printf("\\\\"); break;
		case '\'': printf("\\\'"); break;
		case '\"': printf("\\\""); break;
		case '\0': printf("\\0"); break;
		case '\a': printf("\\a"); break;
		case '\b': printf("\\b"); break;
		case '\f': printf("\\f"); break;
		case '\n': printf("\\n"); break;
		case '\r': printf("\\r"); break;
		case '\t': printf("\\t"); break;
		case '\v': printf("\\v"); break;
		default: 
			if (isprint(c))
				putchar(c);
			else
				printf("\\%03o",(unsigned char)c); 
			break;
	}
}

void yyerror(char *s, ...) {
	va_list args;
	char buffer[MAX_STR_LEN];

	va_start(args,s);
	vsnprintf(buffer,MAX_STR_LEN-1,s,args);
	va_end(args);
	fprintf(stderr,"%s:%d: error: %s\n",filename, line_num, buffer);
}

void yywarn(char *s, ...) {
	va_list args;
	char buffer[MAX_STR_LEN];

	va_start(args,s);
	vsnprintf(buffer,MAX_STR_LEN-1,s,args);
	va_end(args);
	fprintf(stderr,"%s:%d: warning: %s\n",filename, line_num, buffer);
}

void yyprint(char *s, ...) {
	va_list args;
	char buffer[MAX_STR_LEN];

	va_start(args,s);
	vsnprintf(buffer,MAX_STR_LEN-1,s,args);
	va_end(args);
	fprintf(stderr,"%s:%d: %s\n",filename, line_num, buffer);
}

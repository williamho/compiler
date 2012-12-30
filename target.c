#include <stdio.h>
#include <string.h>
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
char show_target;

print_target_code() {
	print_globals();
	print_strings();
	print_functions();
}

print_globals() {
	struct global *g = globals;
	struct symbol *sym;

	while(g = g->next) {
		sym = (struct symbol *)g->var;

		printf("\t.comm %s,",sym->id);
		if (sym->type->nodetype == N_ARR)
			printf("%d",get_size_of_arr((struct arr_node *)(sym->type)));
		else
			printf("4");
		printf(",4");
		putchar('\n');
	}
	putchar('\n');
	return;
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
	int fnum = 0;
	while (fl = fl->next) {
		// Function header
		printf("\t.text\n");
		printf("\t.globl\t%s\n",fl->id);
		printf("\t.type\t%s, @function\n",fl->id);

		printf("%s:\n",fl->id); // function name
		printf("\tpushl %%ebp\n");
		printf("\tmovl %%esp, %%ebp\n");
		printf("\tsubl $%d, %%esp\n",(fl->num_locals+1)*4);

		print_function_body(fl->bb);

		// Function footer
		printf("\tleave\n\tret\n");
		printf(".LFE%d:\n",fnum);
		printf("\t.size %s, .-%s\n",fl->id,fl->id);
		fnum++;
	}
	putchar('\n');
}

print_function_body(struct block *bb) {
	struct quad *q;
	while (bb) {
		printf("%s:\n",bb->id);
		q = bb->first;

		// Go through the quads in the block
		while (q) {
			print_target_from_quad(q);
			q = q->next;
		}

		bb = bb->next; // Next basic block
	}
}

int argnum = 0;
print_target_from_quad(struct quad *q) {
	struct symbol *r, *s1, *s2;
	r = (struct symbol *)q->result;
	s1 = (struct symbol *)q->src1;
	s2 = (struct symbol *)q->src2;
	char *opname;

	if (show_target == 2) {
		printf("\t# ");
		emit(q);
	}
	switch(q->opcode) {
	case Q_LEA:
		printf("\tleal %s, %%edx\n",get_name(s1));
		printf("\tmovl %%edx, %s\n",get_name(r));
		break;
	case Q_LOAD:
		printf("\tmovl %s, %%eax\n",get_name(s1));
		printf("\tmovl (%%eax), %%edx\n");
		printf("\tmovl %%edx, %s\n",get_name(r));
		break;
	case Q_STORE:
		printf("\tmovl %s, %%eax\n",get_name(s2));
		printf("\tmovl %s, %%edx\n",get_name(s1));
		printf("\tmovl %%edx, (%%eax)\n");
		break;
	case Q_ARG_BEGIN:
		printf("\tsubl $%d, %%esp\n",(atoi(s1->id)+1)*4);
		argnum = 0;
		break;
	case Q_FUNC_ARG:
		printf("\tmovl %s, %%eax\n",get_name(s1));
		printf("\tmovl %%eax, %d(%%esp)\n",argnum*4);
		argnum++;
		break;
	case Q_FUNC_CALL:
		printf("\tcall %s\n",s1->id);
		if (r) 	// assignment
			printf("\tmovl %%eax, %s\n",get_name(r));
		break;
	case Q_MOV:
		printf("\tmovl %s, %%eax\n",get_name(s1));
		printf("\tmovl %%eax, %s\n",get_name(r));
		break;
	case Q_INC:
	case Q_DEC:
		printf("\t%sl %s\n",(q->opcode == Q_INC)?"inc":"dec",get_name(s1));
		break;
	case Q_ADD:
	case Q_SUB:
		printf("\tmovl %s, %%eax\n",get_name(s2));
		printf("\tmovl %s, %%edx\n",get_name(s1));
		printf("\t%sl %%eax, %%edx\n",(q->opcode == Q_ADD)?"add":"sub");
		printf("\tmovl %%edx, %s\n",get_name(r));
		break;
	case Q_MUL:
		printf("\tmovl %s, %%eax\n",get_name(s2));
		printf("\tmovl %s, %%edx\n",get_name(s1));
		printf("\timull %%edx\n",get_name(s1));
		printf("\tmovl %%eax, %s\n",get_name(r));
		break;
	case Q_DIV:
	case Q_MOD:
		printf("\tmovl $0, %%edx\n"); // most significant 4 bytes
		printf("\tmovl %s, %%eax\n",get_name(s1)); // least sig 4 bytes
		printf("\tmovl %s, %%ecx\n",get_name(s2)); // least sig 4 bytes
		printf("\tidivl %%ecx\n");
		printf("\tmovl %%%s, %s\n",	// edx contains the remainder
			(q->opcode == Q_DIV)?"eax":"edx",get_name(r));
		break;
	case Q_AND:
	case Q_OR:
	case Q_XOR:
		switch(q->opcode) {
		case Q_AND: opname="and"; break;
		case Q_OR: opname="or"; break;
		case Q_XOR: opname="xor"; break;
		}
		printf("\tmovl %s, %%eax\n",get_name(s1));
		printf("\t%sl %s, %%eax\n",opname,get_name(s2));
		printf("\tmovl %%eax, %s\n",get_name(r));
		break;
	case Q_NOT:
		printf("\tmovl %s, %%eax\n",get_name(s1));
		printf("\tnotl %%eax\n");
		printf("\tmovl %%eax, %s\n",get_name(r));
		break;
	case Q_LOGNOT:
		printf("\tmovl %s, %%eax\n",get_name(s1));
		printf("\ttestl %%eax, %%eax\n");
		printf("\tsete %%al\n");
		printf("\tmovzbl %%al, %%eax\n");
		printf("\tmovl %%eax, %s\n",get_name(r));
		break;
	case Q_SHL:
	case Q_SHR:
		opname = (q->opcode == Q_SHL)?"sal":"sar";
		printf("\tmovl %%ebx, %%edx\n");
		printf("\tmovl %s, %%ebx\n",get_name(s1));
		printf("\tmovl %s, %%ecx\n",get_name(s2));
		printf("\t%sl %%cl, %%ebx\n",opname);
		printf("\tmovl %%ebx, %s\n",get_name(r));
		printf("\tmovl %%edx, %%ebx\n");
		break;
	case Q_LOGOR: 
	case Q_LOGAND: // These aren't actually short circuit
		opname = (q->opcode == Q_LOGOR)?"or":"and";
		printf("\tmovl %s, %%edx\n",get_name(s1));
		printf("\tmovl %s, %%ecx\n",get_name(s2));
		printf("\ttestl %%edx, %%edx\n");
		printf("\tsetne %%al\n");
		printf("\txorl %%edx, %%edx\n");
		printf("\ttestl %%ecx, %%ecx\n");
		printf("\tsetne %%dl\n");
		printf("\t%sl %%edx, %%eax\n",opname);
		printf("\tandl $1, %%eax\n");
		printf("\tmovl %%eax, %s\n",get_name(r));
		break;
	case Q_CMP:
		printf("\tmovl %s, %%eax\n",get_name(s1));
		printf("\tcmpl %s, %%eax\n",get_name(s2));
		break;

#define JUMP_CODE(x) \
	printf("\t%s %s\n",x,s1->id); \
	printf("\tjmp %s\n",s2->id)

	case Q_BR: printf("\tjmp %s\n",s1->id); break;
	case Q_BREQ: JUMP_CODE("je"); break;
	case Q_BRNE: JUMP_CODE("jne"); break;
	case Q_BRGT: JUMP_CODE("jg"); break;
	case Q_BRGE: JUMP_CODE("jge"); break;
	case Q_BRLT: JUMP_CODE("jl"); break;
	case Q_BRLE: JUMP_CODE("jle"); break;
	case Q_RETURN:
		if (s1)
			printf("\tmovl %s, %%eax\n",get_name(s1));
		printf("\tleave\n\tret\n");
		break;
	default:
		break;
	}
	if (show_target == 2) 
		putchar('\n');
}

char *get_name(struct symbol *sym) {
	char *name;
	int tmpnum;
	if (sym->nodetype == N_CONST) {
		name = malloc(strlen(sym->id)+2);
		sprintf(name,"$%s",sym->id);
		return name;
	}
	if (sym->id[0] == '%') { // temp/local var
		sscanf(sym->id,"%%T%d",&tmpnum);
		name = malloc(10);
		sprintf(name,"-%d(%%ebp)",tmpnum*4); // get offset from ebp
		return name;
	}
	return sym->id;
}


a: bin/expressions.o bin/symtable.o bin/declarations.o bin/statements.o quads.o bin/lex.yy.o bin/y.tab.o 
	gcc -g -o a bin/declarations.o bin/symtable.o bin/expressions.o bin/statements.o quads.o bin/y.tab.o bin/lex.yy.o 

bin/quads.o: quads.c quads.h
	gcc -c quads.c -o bin/quads.o

bin/declarations.o: declarations.c declarations.h
	gcc -c declarations.c -o bin/declarations.o
	
bin/symtable.o: symtable.c symtable.h
	gcc -c symtable.c -o bin/symtable.o
	
bin/expressions.o: expressions.c expressions.h
	gcc -c expressions.c -o bin/expressions.o

bin/statements.o: statements.c statements.h
	gcc -c statements.c -o bin/statements.o

bin/y.tab.o: parser.y
	bison --report=state -d parser.y -y
	gcc -c y.tab.c -o bin/y.tab.o
	
bin/lex.yy.o: lexer.l
	flex lexer.l
	gcc -c lex.yy.c -o bin/lex.yy.o

clean:
	rm -rf bin/*.o y.tab.* *.output *.out

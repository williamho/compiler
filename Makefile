compile: bin/y.tab.o bin/lex.yy.o bin/expressions.o bin/symtable.o bin/declarations.o bin/statements.o bin/quads.o bin/globals.o bin/target.o
	gcc -g -o compile bin/declarations.o bin/symtable.o bin/expressions.o bin/statements.o bin/quads.o bin/y.tab.o bin/lex.yy.o bin/globals.o bin/target.o

bin/y.tab.o: parser.y
	bison --report=state -d parser.y -y
	gcc -c y.tab.c -o bin/y.tab.o

bin/target.o: target.c target.h
	gcc -c target.c -o bin/target.o

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

bin/globals.o: globals.c globals.h
	gcc -c globals.c -o bin/globals.o

bin/lex.yy.o: lexer.l
	flex lexer.l
	gcc -c lex.yy.c -o bin/lex.yy.o

clean:
	rm -rf bin/*.o y.tab.* *.output *.out lex.yy.* compile


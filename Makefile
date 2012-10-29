a: bin/y.tab.o bin/lex.yy.o bin/symtable.o bin/declarations.o
	gcc -g -o a bin/declarations.o bin/symtable.o bin/y.tab.o bin/lex.yy.o 

bin/declarations.o: declarations.c 
	gcc -c declarations.c -o bin/declarations.o
	
bin/symtable.o: symtable.c 
	gcc -c symtable.c -o bin/symtable.o
	
bin/y.tab.o: parser.y
	bison --report=state -d parser.y -y
	gcc -c y.tab.c -o bin/y.tab.o
	
bin/lex.yy.o: lexer.l
	flex lexer.l
	gcc -c lex.yy.c -o bin/lex.yy.o

clean:
	rm -rf bin/*.o y.tab.* *.output *.out

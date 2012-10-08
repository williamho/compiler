a: bin/y.tab.o bin/lex.yy.o bin/symtable.o
	gcc -g -o a bin/symtable.o bin/y.tab.o bin/lex.yy.o 

bin/symtable.o: symtable.c 
	gcc -c symtable.c -o bin/symtable.o
	
bin/y.tab.o: parser.y
	bison -d parser.y -y
	gcc -c y.tab.c -o bin/y.tab.o
	
bin/lex.yy.o: lexer.l
	flex lexer.l
	gcc -c lex.yy.c -o bin/lex.yy.o

clean:
	rm -f *.o

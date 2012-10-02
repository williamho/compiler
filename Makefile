a: bin/y.tab.o bin/lex.yy.o
	gcc -g -o a bin/y.tab.o bin/lex.yy.o

bin/y.tab.o: y.tab.c 
	gcc -c y.tab.c -o bin/y.tab.o

bin/lex.yy.o: lex.yy.c
	gcc -c lex.yy.c -o bin/lex.yy.o

y.tab.c: parser.y
	bison -d parser.y -y

lex.yy.c: lexer.l
	flex lexer.l

clean:
	rm -f *.o

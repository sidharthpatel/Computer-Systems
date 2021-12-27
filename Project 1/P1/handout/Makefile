CC=gcc-9

all: fp 

fp:	fp_program.o fp_functs.o lex.yy.o 
	${CC} -o fp fp_program.o fp_functs.o lex.yy.o -L /home/connorbaker/.linuxbrew/Cellar/flex/2.6.4_1/lib -lfl -lm

fp_functs.o:	fp_functs.c fp.h
	${CC} -g -Wall -std=gnu99 -c fp_functs.c	

fp_program.o:	fp_program.c fp_parse.h
	${CC} -c fp_program.c	

lex.yy.o:	lex.yy.c fp_parse.h
	${CC} -c lex.yy.c

lex.yy.c:	fp.l
	flex fp.l

clean:
	rm -rf lex.yy.c lex.yy.o fp_program.o fp_functs.o 

reallyclean:
	rm -rf fp lex.yy.c lex.yy.o fp_program.o fp_functs.o 

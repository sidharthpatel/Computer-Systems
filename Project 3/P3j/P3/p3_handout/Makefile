CFLAGS = -D_POSIX_C_SOURCE -Wall -std=c99
CC = gcc-4.9

all: shell 

shell: shell.o logging.o
	${CC} ${CFLAGS} -o shell shell.o logging.o

shell.o: shell.c shell.h
	${CC} ${CFLAGS} -g -c shell.c   

logging.o:logging.c logging.h
	${CC} -Wall -g -std=c99 -c logging.c     

clean:
	rm -rf shell.o logging.o shell 

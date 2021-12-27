# comments are always a great idea.

# we use this variable as $(CC) when building commands.
CC=gcc

# we keep using these flags in combination, so they're
# stuffed into a variable. Saves us from having to 
# re-type them repeatedly, and of course we can then
# change it in one place for all uses.
CFLAGS=-O0 -std=c99

FUNC_OS=mul_20.o add3.o max2.o sumUpTo.o collatzLength.o fact.o caller.o func1.o func2.o func3.o 

# see below for the rec5 target.
all: rec5

# the command 'make clean' should remove all generated files.
clean:
	-rm -rf *~ *# *.o rec5.s rec5.o rec5

# target rec5 builds the overall executable, rec5.
# depends on all the individual functions' object files.
rec5: rec5.o  $(FUNC_OS)
	$(CC) $(CFLAGS) -o rec5 rec5.o $(FUNC_OS)

# compiles rec5.c into rec5.s; needs no other files.
rec5.s: rec5.c
	$(CC) $(CFLAGS) -S rec5.c

# compiles rec5.s into rec5.o; needs no other files.
rec5.o: rec5.s
	$(CC) $(CFLAGS) -c rec5.s

# each of the remaining targets also compiles each
# .s file into the corresponding .o file, with no
# other dependencies on other files.

mul_20.o: mul_20.s
	$(CC) $(CFLAGS) -c mul_20.s

add3.o: add3.s
	$(CC) $(CFLAGS) -c add3.s

max2.o: max2.s
	$(CC) $(CFLAGS) -c max2.s

sumUpTo.o: sumUpTo.s
	$(CC) $(CFLAGS) -c sumUpTo.s

collatzLength.o: collatzLength.s
	$(CC) $(CFLAGS) -c collatzLength.s

fact.o: fact.s
	$(CC) $(CFLAGS) -c fact.s

caller.o: caller.s
	$(CC) $(CFLAGS) -c caller.s


# our functions from recitation 4 are also available!

func1.o: func1.s
	$(CC) $(CFLAGS) -c func1.s

func2.o: func2.s
	$(CC) $(CFLAGS) -c func2.s

func3.o: func3.s
	$(CC) $(CFLAGS) -c func3.s





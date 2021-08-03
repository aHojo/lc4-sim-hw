all: trace

trace: LC4.o loader.o trace.c
	clang -Wall -g LC4.o loader.o trace.c -o trace

LC4.o: LC4.c LC4.h
	clang -Wall -c LC4.c

loader.o: loader.c loader.h

	clang -Wall -c loader.c

clean:
	rm -rf *.o

clobber: clean
	rm -rf trace
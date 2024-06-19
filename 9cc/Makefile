CFLAGS=-std=c11 -g -static

all: clean foo 9cc test

foo:
	cc -c foo.c

9cc: 9cc.c codegen.c

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.0 *~ tmp* foo.o

.PHONY: test clean

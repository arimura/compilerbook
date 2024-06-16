CFLAGS=-std=c11 -g -static

all: 9cc test

9cc: 9cc.c codegen.c

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.0 *~ tmp*

.PHONY: test clean

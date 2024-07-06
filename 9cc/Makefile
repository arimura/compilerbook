CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

all: clean foo 9cc test

foo:
	cc -c foo.c

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./test.sh

debug-assembly:
	gcc -g -o tmp-debug tmp.s foo.o
	gdb tmp-debug

clean:
	rm -f 9cc *.0 *~ tmp* foo.o

.PHONY: test clean

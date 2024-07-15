#include <stdio.h>

int foo(int x, int y)
{
    printf("foo: %d\n", x + y);
}

int qc_print(int x)
{
    printf("pc_print: %d\n", x);
}

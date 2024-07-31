#include <stdio.h>
#include <stdlib.h>

int foo(int x, int y)
{
    printf("foo: %d\n", x + y);
}

int qc_print(int x)
{
    printf("qc_print: %d\n", x);
}

int qc_print_str(char *s)
{
    printf("qc_print_str: %s\n", s);
}

void alloc4(void **p, int h, int i, int j, int k)
{
    void *a = malloc(4 * sizeof(int));

    if (a == NULL)
    {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1);
    }

    int *b = a;
    b[0] = h;
    b[1] = i;
    b[2] = j;
    b[3] = k;

    *p = a;
}

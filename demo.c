#include <stdio.h>

int main(int argc, char const *argv[])
{
    int *y;
    int x;
    y = &x;
    *y = 10;
    printf("x = %d\n", x);
    return 0;
}

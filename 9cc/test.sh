#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    # cc -o tmp tmp.s
    cc -c -o tmp.o tmp.s
    cc -o tmp tmp.o foo.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 "int main(){return 0;}"
assert 42 "int main(){return 42;}"
assert 21 "int main(){return 5+20-4;}"
assert 41 "int main(){ return 12 + 34 - 5 ;}"
assert 42 'int main(){return 6*7;}'
assert 47 'int main(){return 5+6*7;}'
assert 15 'int main(){return 5*(9-6);}'
assert 4 'int main(){return (3+5)/2;}'
assert 10 "int main(){return -10+20;}"
assert 1 "int main(){return 1<2;}"
assert 0 "int main(){return 1 < 1;}"
assert 1 "int main(){return 2>1;}"
assert 0 "int main(){return 1>1;}"
assert 1 "int main(){return 1<=2;}"
assert 1 "int main(){return 1<=1;}"
assert 1 "int main(){return 2>=1;}"
assert 1 "int main(){return 1>=1;}"
assert 1 "int main(){return 1==1;}"
assert 0 "int main(){return 1==2;}"
assert 1 "int main(){return 2!=1;}"
assert 0 "int main(){return 2!=2;}"
assert 1 "int main(){return (3+2)==(10-5) ;}"
assert 1 "int main(){int a; a=1; return 1;}"
assert 1 "int main(){int a;a=1; return a;}"
assert 6 "int main(){int a; int b; a=2; b=4; return a+b;}"
assert 1 "int main(){return 1;}"
assert 10 "int main(){return 10; return 1;}"
assert 23 "int main(){int a; a=(10+13); return a;}"
assert 23 "int main(){int axw1_3; axw1_3=(10+13); return axw1_3;}"
assert 3 "int main(){int hoge; hoge=1; if (1) { hoge=3; } return hoge; }"
assert 1 "int main(){int hoge; hoge=1; if (0) { hoge=3; } return hoge; }"
assert 3 "int main(){int hoge; hoge=1; if (0) { hoge=2; } else { hoge=3; } return hoge; }"
assert 2 "int main(){int a; while (0) {a=1;} return 2;}"
assert 2 "int main(){int a;a=0; while (a != 2) {a = a + 1;} return a;}"
assert 14 "int main(){int a; int b;a=10; for(b=0;b < 4; b = b + 1) {a = a + 1;} return a;}"
assert 14 "int main(){int a; int b; a=10; b=0; for(;b < 4; b = b + 1) {a = a + 1;} return a;}"
assert 1 "int main(){{1;} return 1;}"
assert 2 "int main(){int a;{a=2;} return a;}"
assert 2 "int main(){{}return 2;}"
assert 4 "int main(){int a; int b;a=0;b=0;while(a != 2){a = a + 1; b = b + 2;} return b;}"
assert 1 "int main(){foo(1, 2);return 1;}"
assert 1 "int hoge(){1;} int main(){return 1;}"
assert 2 "int hoge(){return 2;} int main(){return hoge();}"
assert 2 "int hoge(int x){return x;} int main(){return hoge(2);}"
assert 5 "int hoge(int x, int y){return x + y;} int main(){return hoge(2, 3);}"
assert 1 "int main(){qc_print(10);return 1;}"
assert 1 "int fib(int i, int s){ if(i == 0){return 1;}  return fib(i-1,0);} int main(){return fib(1,0);}"
assert 0 "int rec(int x){if(x == 0){return x;} return rec(x-1);} int main(){return rec(5);}"
assert 0 "int rec(int x, int y){if(x == 0){return x;} return rec(x - 1, y * 2);} int main(){return rec(1, 2);}"
assert 8 "int fib(int p, int n, int i){if(i == 0){return n;} return fib(n, p + n, i -1) ;} int main(){return fib(1,1,4);}"
assert 2 "int main(){int v;int p;v = 2;p = &v; return *p;}"
assert 1 "int main(){int i; i = 1; return i;}"
assert 3 "int main(){ int x; int *y; y = &x; *y = 3; return x;}"
assert 3 "int main(){ int x; int *y; y = &x; x = 3; return *y;}"
assert 3 "int main(){ int x; int *y; int **z; y = &x; z = &y; **z = 3; return x;}"
assert 1 "int main(){ int *p; int *q; alloc4(&p, 1,2,3,4); q = p; return *q; }"
assert 2 "int main(){ int *p; int *q; alloc4(&p, 1,2,3,4); q = p + 1; return *q; }"
assert 3 "int main(){ int *p; int *q; alloc4(&p, 1,2,3,4); q = p + 2; return *q; }"
assert 4 "int main(){ int t; return sizeof(t);}"
assert 4 "int main(){ return sizeof(100);}"
assert 8 "int main(){ int *t; return sizeof(t);}"
assert 3 "int main(){ int i[2]; return 3; }"
assert 3 "int main(){ int a[2]; *a = 3 ; return 3; }"
assert 3 "int main(){ int a[2]; *a = 3 ; return *a; }"
# assert 3 "int main(){ int a[2]; *(a + 1) = 3 ; return *(a + 1); }"
# TO DO: handle "int *a[2]"
echo OK
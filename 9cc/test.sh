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

assert 0 "main(){return 0;}"
assert 42 "main(){return 42;}"
assert 21 "main(){return 5+20-4;}"
assert 41 "main(){ return 12 + 34 - 5 ;}"
assert 42 'main(){return 6*7;}'
assert 47 'main(){return 5+6*7;}'
assert 15 'main(){return 5*(9-6);}'
assert 4 'main(){return (3+5)/2;}'
assert 10 "main(){return -10+20;}"
assert 1 "main(){return 1<2;}"
assert 0 "main(){return 1 < 1;}"
assert 1 "main(){return 2>1;}"
assert 0 "main(){return 1>1;}"
assert 1 "main(){return 1<=2;}"
assert 1 "main(){return 1<=1;}"
assert 1 "main(){return 2>=1;}"
assert 1 "main(){return 1>=1;}"
assert 1 "main(){return 1==1;}"
assert 0 "main(){return 1==2;}"
assert 1 "main(){return 2!=1;}"
assert 0 "main(){return 2!=2;}"
assert 1 "main(){return (3+2)==(10-5) ;}"
assert 1 "main(){a=1; return 1;}"
assert 1 "main(){a=1; return a;}"
assert 6 "main(){a=2; b=4; return a+b;}"
assert 1 "main(){return 1;}"
assert 10 "main(){return 10; return 1;}"
assert 23 "main(){a=(10+13); return a;}"
assert 23 "main(){axw1_3=(10+13); return axw1_3;}"
assert 3 "main(){ hoge=1; if (1) { hoge=3; } return hoge; }"
assert 1 "main(){ hoge=1; if (0) { hoge=3; } return hoge; }"
assert 3 "main(){ hoge=1; if (0) { hoge=2; } else { hoge=3; } return hoge; }"
assert 2 "main(){while (0) {a=1;} return 2;}"
assert 2 "main(){a=0; while (a != 2) {a = a + 1;} return a;}"
assert 14 "main(){a=10; for(b=0;b < 4; b = b + 1) {a = a + 1;} return a;}"
assert 14 "main(){a=10; b=0; for(;b < 4; b = b + 1) {a = a + 1;} return a;}"
assert 1 "main(){{1;} return 1;}"
assert 2 "main(){{a=2;} return a;}"
assert 2 "main(){{}return 2;}"
assert 4 "main(){a=0;b=0;while(a != 2){a = a + 1; b = b + 2;} return b;}"
assert 1 "main(){foo(1, 2);return 1;}"
assert 1 "hoge(){1;} main(){return 1;}"
assert 2 "hoge(){return 2;} main(){return hoge();}"
assert 2 "hoge(x){return x;} main(){return hoge(2);}"
assert 5 "hoge(x, y){return x + y;} main(){return hoge(2, 3);}"
assert 1 "main(){qc_print(10);return 1;}"
assert 1 "fib(i,s){ if(i == 0){return 1;}  return fib(i-1,0);} main(){return fib(1,0);}"
assert 0 "rec(x){if(x == 0){return x;} return rec(x-1);} main(){return rec(5);}"
assert 0 "rec(x,y){if(x == 0){return x;} return rec(x - 1, y * 2);} main(){return rec(1, 2);}"
assert 8 "fib(p, n, i){if(i == 0){return n;} return fib(n, p + n, i -1) ;} main(){return fib(1,1,4);}"
assert 2 "main(){v = 2;p = &v; return *p;}"
assert 1 "main(){int i; i = 1; return i;}"
echo OK
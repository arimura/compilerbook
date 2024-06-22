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

assert 0 "main(){0;}"
# assert 0 "0;"
# assert 42 "42;"
# assert 21 "5+20-4;"
# assert 41 " 12 + 34 - 5 ;"
# assert 42 '6*7;'
# assert 47 '5+6*7;'
# assert 15 '5*(9-6);'
# assert 4 '(3+5)/2;'
# assert 10 "-10+20;"
# assert 1 "1<2;"
# assert 0 "1 < 1;"
# assert 1 "2>1;"
# assert 0 "1>1;"
# assert 1 "1<=2;"
# assert 1 "1<=1;"
# assert 1 "2>=1;"
# assert 1 "1>=1;"
# assert 1 "1==1;"
# assert 0 "1==2;"
# assert 1 "2!=1;"
# assert 0 "2!=2;"
# assert 1 "(3+2)==(10-5) ;"
# assert 1 "a=1; 1;"
# assert 1 "a=1; a;"
# assert 6 "a=2; b=4; a+b;"
# assert 1 "return 1;"
# assert 10 "return 10; return 1;"
# assert 23 "a=(10+13); return a;"
# assert 23 "axw1_3=(10+13); return axw1_3;"
# assert 3 "hoge=1; if (1) hoge=3; hoge;"
# assert 1 "hoge=1; if (0) hoge=3; hoge;"
# assert 3 "hoge=1; if (0) hoge=2; else hoge=3; hoge;"
# assert 2 "while (0) a=1; 2;"
# assert 2 "a=0; while (a != 2) a = a + 1; a;"
# assert 14 "a=10; for(b=0;b < 4; b = b + 1) a = a + 1; a;" 
# assert 14 "a=10; b=0; for(;b < 4; b = b + 1) a = a + 1; a;" 
# assert 1 "{1;}1;"
# assert 2 "{a=2;} a;"
# assert 2 "{}2;"
# assert 4 "a=0;b=0;while(a != 2){a = a + 1; b = b + 2;} b;"
# assert 1  "foo(1, 2);1;"
# assert 1 "hoge(){1;};1"
echo OK
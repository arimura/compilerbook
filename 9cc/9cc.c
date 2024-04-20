#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    chat *str;
}

Token *token;

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfpring(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char op) {
    if(token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    token = token->next;
    return true;
}

void expect(char op){
    if(token->kind != TK_RESERVED || token->str[0] != op)
        error("'%c'ではありません", op);
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM)
        error("数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof(){
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}




int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません");
        return 1;
    }

    char *p = argv[1];

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main: \n");
    printf("    mov rax, %ld\n", strtol(p, &p, 10));

    while (*p) {
        if (*p == '+') {
            p++;
            printf("    add rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        if (*p == '-') {
            p++;
            printf("    sub rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        fprintf(stderr, "予期しない文字です: '%c'\n", *p);
        return 1;
    } 

    printf("    ret\n");
    return 0;
}

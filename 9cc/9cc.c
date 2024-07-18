#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>
#include <unistd.h>
#include "9cc.h"
#include "codegen.h"
#include "parser.h"

char *user_input;
Token *token;
Node *code[100];

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

const char *getTokenKindName(TokenKind kind)
{
    switch (kind)
    {
    case TK_RESERVED:
        return "TK_RESERVED";
    case TK_NUM:
        return "TK_NUM";
    case TK_EOF:
        return "TK_EOF";
    case TK_INDENT:
        return "TK_INDENT";
    case TK_RETURN:
        return "TK_RETUREN";
    case TK_IF:
        return "TK_IF";
    case TK_ELSE:
        return "TK_ELSE";
    case TK_WHILE:
        return "TK_WHILE";
    case TK_FOR:
        return "TK_FOR";
    default:
        return "Unknown";
    }
}

void printToken(const Token *token)
{
    if (token == NULL)
    {
        return;
    }
    fprintf(stderr, "Token:\n");
    fprintf(stderr, "  Kind: %s\n", getTokenKindName(token->kind));
    fprintf(stderr, "  Value: %d\n", token->val);
    fprintf(stderr, "  String: %s\n", token->str ? token->str : "NULL");
    fprintf(stderr, "  Length: %d\n", token->len);
    printToken(token->next);
}

const char *getNodeKindName(NodeKind kind)
{
    switch (kind)
    {
    case ND_ADD:
        return "Add";
    case ND_SUB:
        return "Subtract";
    case ND_MUL:
        return "Multiply";
    case ND_DIV:
        return "Divide";
    case ND_NUM:
        return "Number";
    case ND_UNARY:
        return "Unary";
    case ND_LESS_THAN:
        return "LessThan";
    case ND_EQUAL_LESS_THAN:
        return "EqualLessThan";
    case ND_EQ:
        return "Equal";
    case ND_NE:
        return "NotEqual";
    case ND_ASSIGN:
        return "Assign";
    case ND_LVAR:
        return "LocalVariable";
    case ND_RETURN:
        return "Return";
    case ND_IF:
        return "If";
    case ND_ELSE:
        return "Else";
    case ND_WHILE:
        return "While";
    case ND_FOR:
        return "For";
    case ND_FUNCALL:
        return "Funcall";
    default:
        return "Unknown";
    }
}

void printNode(const Node *node, int depth)
{
    if (node == NULL)
    {
        return;
    }
    for (int i = 0; i < depth; i++)
    {
        fprintf(stderr, "  ");
    }

    if (node->kind == ND_NUM)
    {
        fprintf(stderr, "%s (%d)\n", getNodeKindName(node->kind), node->val);
    }
    else
    {
        fprintf(stderr, "%s\n", getNodeKindName(node->kind));
        printNode(node->lhs, depth + 1);
        printNode(node->rhs, depth + 1);
    }
}

void printCode()
{
    for (int i = 0; code[i]; i++)
    {
        printNode(code[i], 0);
    }
}

/*
 * Tokenizer
 */
Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

int is_alnum(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

bool is_ident1(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           (c == '_');
}

bool is_ident2(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6]))
        {
            cur = new_token(TK_RETURN, cur, p);
            cur->len = 6;
            p = p + 6;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2]))
        {
            cur = new_token(TK_IF, cur, p);
            cur->len = 2;
            p = p + 2;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4]))
        {
            cur = new_token(TK_ELSE, cur, p);
            cur->len = 4;
            p = p + 4;
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5]))
        {
            cur = new_token(TK_WHILE, cur, p);
            cur->len = 5;
            p = p + 5;
            continue;
        }

        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3]))
        {
            cur = new_token(TK_FOR, cur, p);
            cur->len = 3;
            p = p + 3;
            continue;
        }

        if (strncmp(p, "int", 3) == 0 && !is_alnum(p[3]))
        {
            cur = new_token(TK_TYPE, cur, p);
            cur->len = 3;
            p = p + 3;
            continue;
        }

        if (strncmp(p, "sizeof", 6) == 0 && !is_alnum(p[6]))
        {
            cur = new_token(TK_SIZEOF, cur, p);
            cur->len = 6;
            p = p + 6;
            continue;
        }

        if (is_ident1(*p))
        {
            char *cnt = p;
            do
            {
                cnt++;
            } while (is_ident2(*cnt));
            cur = new_token(TK_INDENT, cur, p);
            cur->len = cnt - p;
            p = cnt;
            continue;
        }

        if (startswith(p, "<=") || startswith(p, ">=") || startswith(p, "==") || startswith(p, "!="))
        {
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 2;
            p = p + 2;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == ')' || *p == '(' || *p == '>' || *p == '<' || *p == '=' || *p == ';' || *p == '{' || *p == '}' || *p == ',' || *p == '&' || *p == '[' || *p == ']')
        {
            cur = new_token(TK_RESERVED, cur, p++);
            cur->len = 1;
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        fprintf(stderr, "%s\n", p);
        error("トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("引数の個数が正しくありません");
        return 1;
    }
    // no buffering
    setvbuf(stdout, NULL, _IONBF, 0);

    user_input = argv[1];
    token = tokenize(user_input);
    // printToken(token);
    program();
    // printCode();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");

    for (int i = 0; code[i]; i++)
    {
        gen(code[i]);
    }

    return 0;
}

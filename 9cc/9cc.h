#pragma once

typedef enum
{
    TK_RESERVED,
    TK_INDENT,
    TK_NUM,
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_EOF,
    TK_WHILE,
    TK_FOR,
    TK_TYPE, //int, etc
} TokenKind;

typedef struct Token Token;

struct Token
{
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

typedef enum
{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_UNARY,
    ND_LESS_THAN,
    ND_EQUAL_LESS_THAN,
    ND_EQ,
    ND_NE,
    ND_ASSIGN,
    ND_LVAR,
    ND_RETURN,
    ND_IF,
    ND_ELSE,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,
    ND_FUNCALL,
    ND_FUNC,
    ND_FUNC_ARG,
    ND_ADDR,
    ND_DEREF,
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind; 
    Node *lhs;
    Node *rhs;
    int val;
    int offset;

    //"if" or "for" statement
    Node *cond;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;

    //For block body, statements and func body
    Node *body;
    Node *next;

    //For func call and func declaretion
    char *funcname;
    int funcname_len;
    char *argname;
    Node *args;
    int argname_len;
};

typedef struct LVar LVar;
struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
};

typedef struct Scope Scope;
struct Scope {
    Scope *next;
    LVar *locals;
};

void enter_scope();
void leave_scope();

void error(char *fmt, ...);
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
    TK_SIZEOF,
    TK_STRING_LITERAL,
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

typedef struct Type Type;
struct Type {
    enum {INT, PTR, ARRAY, CHAR} ty;
    struct Type *ptr_to;
    size_t array_size;
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
    ND_GVAR,
    ND_GVAR_DECL,
    ND_STR_LITERAL,
} NodeKind;

typedef struct Node Node;

struct Node
{
    NodeKind kind; 
    Node *lhs;
    Node *rhs;
    int val;
    int offset;
    Type *type;

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

    //global variable
    char *gvarname;
    int gvarname_len;

    //string literal
    char *strliteral;
    int strliteral_len;
};

typedef struct LVar LVar;
struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
    Type *type;
};

typedef struct GVar GVar;
struct GVar {
    GVar *next;
    char *name;
    int len;
    Type *type;
};

typedef struct Scope Scope;
struct Scope {
    Scope *next;
    LVar *locals;
};

void enter_scope();
void leave_scope();

void error(char *fmt, ...);

extern Token *token;
extern Node *data[100];
extern Node *data_string_literal[100];
extern Node *text[100];

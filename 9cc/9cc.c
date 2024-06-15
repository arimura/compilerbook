#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>
#include <unistd.h>

typedef enum
{
    TK_RESERVED,
    TK_INDENT,
    TK_NUM,
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_EOF,
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

char *user_input;
Token *token;

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

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
};

typedef struct LVar LVar;
struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
};
LVar *locals;

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}

Node *code[100];

/*
* Print Util
*/
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

void printLocals(){
    for(LVar *var = locals; var; var = var->next){
        fprintf(stderr, "name: %s, offset: %d\n", var->name, var->offset);
    }
}

void printCode(){
    for(int i = 0; code[i]; i++){
        printNode(code[i], 0);
    }
}

/*
* Tokenizer
*/
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

bool consume(char *op)
{
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

Token *consume_ident(){
    if (token->kind != TK_INDENT)
        return NULL;
    Token *t = token;
    token = token->next;
    return t;
}

bool consume_kind(TokenKind kind){
    if(token->kind != kind){
        return false;
    }
    token = token->next;
    return true;
}

void expect(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
        error("'%c'ではありません", op);
    token = token->next;
}

int expect_number()
{
    if (token->kind != TK_NUM)
        error("数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

bool is_ident1(char c){
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           (c == '_');
}

bool is_ident2(char c) {
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

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])){
           cur = new_token(TK_RETURN, cur, p);
           cur->len = 6;
           p = p + 6;
           continue; 
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])){
            cur = new_token(TK_IF, cur, p);
            cur->len = 2;
            p = p + 2;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])){
            cur = new_token(TK_ELSE, cur, p);
            cur->len = 4;
            p = p + 4;
            continue;
        }

        if (is_ident1(*p)){
            char *cnt = p;
            do {
                cnt++;
            } while (is_ident2(*cnt));
            cur = new_token(TK_INDENT, cur, p);
            cur->len = cnt - p;
            p = cnt; 
            continue;
        }

        if (startswith(p, "<=") 
        || startswith(p, ">=") 
        || startswith(p, "==")
        || startswith(p, "!=")) 
        {
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 2;
            p = p+2;
            continue;
        }

        if (*p == '+' 
        || *p == '-'
        || *p == '*' 
        || *p == '/' 
        || *p == ')' 
        || *p == '(' 
        || *p == '>' 
        || *p == '<'
        || *p == '='
        || *p == ';'
        )
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


/*
* Parser
*/
Node *expr();
Node *add();
Node *mul();
Node *primary();
Node *unary();
Node *relational();
Node *equality();
Node *assign();


Node *expr()
{
    return assign();
}

Node *stmt(){
    Node *node;
    
    if(consume_kind(TK_IF)){
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect('(');
        node->cond = expr();
        expect(')');
        node->then = stmt();
    }else if(consume_kind(TK_RETURN)){
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(';');
    }else{
        node = expr();
        expect(';');
    } 
    
    return node;
}

void program(){
    int i = 0;
    while(!at_eof()){
        code[i++] = stmt();
    }
    code[i] = NULL;
}

Node *assign(){
    Node *node = equality();
    if(consume("="))
        node = new_node(ND_ASSIGN, node, assign());
    return node;
}

Node *equality(){
    Node *node = relational();

    for (;;){
        if(consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if(consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    } 
}


Node *relational()
{
    Node *node = add();

    for (;;)
    {
        if (consume("<"))
            node = new_node(ND_LESS_THAN, node, add());
        else if (consume(">"))
            node = new_node(ND_LESS_THAN, add(), node);
        else if (consume("<="))
            node = new_node(ND_EQUAL_LESS_THAN, node, add());
        else if (consume(">="))
            node = new_node(ND_EQUAL_LESS_THAN, add(), node);
        else
            return node;
    }
}

Node *add()
{
    Node *node = mul();

    for (;;)
    {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary()
{
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}


Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(')');
        return node;
    }

    Token *tok = consume_ident();
    if(tok){
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if(lvar) {
            node->offset = lvar->offset;
        }else{
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            //To be refactored?
            if(locals){
                lvar->offset = locals->offset + 8;
            }else{
                lvar->offset = 8;
            }
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    }

    return new_node_num(expect_number());
}

void gen_lval(Node *node){
    if(node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");

    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

static int count(void) {
    static int i = 1;
    return i++;
}

void gen(Node *node)
{
    switch(node->kind){
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_IF:
        int c = count();
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je  .Lend%d\n", c);
        gen(node->then);
        printf(".Lend%d:\n",c);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_LESS_THAN:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_EQUAL_LESS_THAN:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("引数の個数が正しくありません");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(user_input);
    printToken(token);
    program();
    // printLocals();
    // printCode();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    //prologue
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    for(int i = 0; code[i]; i++){
        gen(code[i]);
        printf("    pop rax\n");
    }

    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}

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
    char *str;
    int len;
};

const char* getTokenKindName(TokenKind kind) {
    switch (kind) {
        case TK_RESERVED:    return "TK_RESERVED";
        case TK_NUM:   return "TK_NUM";
        case TK_EOF: return "TK_EOF";
        // Add more cases as necessary
        default:               return "Unknown";
    }
}

void printToken(const Token *token) {
    if (token == NULL) {
        return;
    }
    fprintf(stderr, "Token:\n");
    fprintf(stderr, "  Kind: %s\n", getTokenKindName(token->kind));
    fprintf(stderr, "  Value: %d\n", token->val);
    fprintf(stderr, "  String: %s\n", token->str ? token->str : "NULL");
    printToken(token->next);
}

char *user_input;
Token *token;

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_UNARY,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};

const char* getNodeKindName(NodeKind kind) {
    switch (kind) {
        case ND_ADD: return "Add";
        case ND_SUB: return "Subtract";
        case ND_MUL: return "Multiply";
        case ND_DIV: return "Divide";
        case ND_NUM: return "Number";
        case ND_UNARY: return "Unary";
        default:     return "Unknown";
    }
}

void printNode(const Node *node, int depth) {
    if (node == NULL) {
        return;
    }
    for (int i = 0; i < depth; i++) {
        fprintf(stderr, "  ");
    }

    if (node->kind == ND_NUM) {
        fprintf(stderr, "%s (%d)\n", getNodeKindName(node->kind), node->val);
    } else {
        fprintf(stderr, "%s\n", getNodeKindName(node->kind));
        printNode(node->lhs, depth + 1);
        printNode(node->rhs, depth + 1);
    }
}

Node *expr();
Node *mul();
Node *primary();
Node *unary();

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val){
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

bool consume(char *op) {
    if(token->kind != TK_RESERVED ||
       strlen(op) != token->len ||
       memcmp(token->str, op, token->len))
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

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p) {
        if(isspace(*p)){
            p++;
            continue;
        }

        if( *p == '+' 
         || *p == '-' 
         || *p == '*' 
         || *p == '/' 
         || *p == ')' 
         || *p == '(') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if(isdigit(*p)) {
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


Node *primary(){
    if (consume("(")) {
        Node *node = expr();
        expect(')');
        return node;
    }

    return new_node_num(expect_number());
}


Node *mul(){
    Node *node = unary();

    for (;;){
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *expr(){
    Node *node = mul();

    for(;;){
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if(consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *unary(){
    if (consume("+"))
        return primary();
    if(consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("    push %d\n", node->val);
        return;
    }
    
    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind){
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
    }

    printf(" push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(user_input);
    // printToken(token);

    Node *node = expr();
    printNode(node, 0);

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}

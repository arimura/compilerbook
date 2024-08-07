#include <stdio.h>
#include "codegen.h"
#include <string.h>
#include <stdlib.h>

char *get_name(char *org, int len)
{
    char *name = malloc((len + 1) * sizeof(char));
    strncpy(name, org, len);
    name[len] = '\0';
    return name;
}

char *str_literal_name(Node *n){
    char *b = malloc(256);
    if(n->kind != ND_STR_LITERAL){
        error("Not string literal node");
    }

    sprintf(b,".LC%d", n->offset);

    return b;
}

// Generates the assembly code to push the address of a local variable onto the stack.
void gen_lval_address(Node *node)
{
    if (node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");

    printf("# gen_lval\n");
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
    printf("# gen_lval end\n");
}

static int count(void)
{
    static int i = 1;
    return i++;
}

static Type *op_result_type(Node *n1, Node *n2)
{
    Type *t1 = n1->type;
    Type *t2 = n2->type;

    if (!t1)
    {
        error("No type for n1");
    }
    if (!t2)
    {
        error("No typ for n2");
    }

    if (t1->ty == INT)
    {
        // INTの場合は他方の型を優先
        return t2;
    }
    else if (t1->ty == PTR || t1->ty == ARRAY)
    {
        // PTR/ARRAYの場合はINTのみ許可
        if (t2->ty != INT)
        {
            error("ptr and no int arithmetic");
        }
        return t1;
    }
    error("Unexptected type arithmetic");
}

static Type *int_type()
{
    Type *t = calloc(1, sizeof(Type));
    t->ty = INT;
    return t;
}

void gen_address(Node *node)
{
    switch (node->kind)
    {
    case ND_LVAR:
        gen_lval_address(node);
        return;
    case ND_DEREF:
        gen(node->lhs);
        return;
    case ND_GVAR:
        printf("# gen gvar address\n");
        printf("    lea rax, [rip + %s]\n", get_name(node->gvarname, node->gvarname_len));
        printf("    push rax\n");
        printf("# gen gvar address end\n");
        return;
    }
    error("Not supported on gen_address. node kind: %d", node->kind);
}

void gen(Node *node)
{
    // fprintf(stderr, "gen: %d\n", node->kind);
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval_address(node);
        if (node->type && node->type->ty == ARRAY)
        {
            printf("# skip lvar value for ARRAY\n");
            return;
        }

        printf("# lvar value\n");
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        printf("# lvar value end\n");
        return;
    case ND_GVAR_DECL:
        printf("# gvar declare\n");
        printf("%s:\n", get_name(node->gvarname, node->gvarname_len));
        // lvarではintを8にしているので合わせる
        printf("    .zero %d\n", node->type->ty == ARRAY ? 8 * (int)node->type->array_size : 8);
        printf("# gvar declare end\n");
        return;
    case ND_GVAR:
        gen_address(node);
        if (node->type && node->type->ty == ARRAY)
        {
            printf("# skip gvar value for ARRAY\n");
            return;
        }
        printf("# gvar\n");
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        printf("# gvar end\n");
        return;
    case ND_STR_LITERAL:
        printf("# str literal\n");
        // printf("    mov rax, OFFSET FLAT:%s\n", str_literal_name(node));
        printf("    lea rax, [rip + %s]\n", str_literal_name(node));
        printf("    push rax\n");
        printf("# str literal end\n");
        return;
    case ND_ASSIGN:
        gen_address(node->lhs);
        gen(node->rhs);

        printf("# assign\n");
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        printf("# assign end\n");
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
        printf("    je  .Lelse%d\n", c);
        gen(node->then);
        printf("    jmp  .Lend%d\n", c);
        printf(".Lelse%d:\n", c);
        if (node->els)
        {
            gen(node->els);
        }
        printf(".Lend%d:\n", c);
        return;
    case ND_WHILE:
        int cw = count();
        printf(".Lbegin%d:\n", cw);
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", cw);
        gen(node->then);
        printf("    jmp .Lbegin%d\n", cw);
        printf(".Lend%d:\n", cw);
        return;
    case ND_FOR:
        int cf = count();
        if (node->init)
        {
            gen(node->init);
        }
        printf(".Lbegin%d:\n", cf);
        if (node->cond)
        {
            gen(node->cond);
        }
        else
        {
            printf("    push 1\n");
        }
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", cf);
        gen(node->then);
        if (node->inc)
        {
            gen(node->inc);
        }
        printf("    jmp .Lbegin%d\n", cf);
        printf(".Lend%d:\n", cf);
        return;
    case ND_BLOCK:
        Node *n = node->body;
        while (n)
        {
            gen(n);
            printf("    pop rax\n");
            n = n->next;
        }
        return;
    case ND_FUNCALL:
        Node *a = node->args;
        // push args
        int ac = 0;
        while (a && ac < 6)
        {
            gen(a);
            a = a->next;
            ac++;
        }

        int i = 0;
        // System V ABIでは6つのregister以上の引数を利用する場合はrspを16の倍数にする必要がある。
        // 今はregisterのみ利用
        const char *r[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
        while (i < ac)
        {
            printf("    pop %s\n", r[ac - i - 1]);
            i++;
        }

        char *name = malloc((node->funcname_len + 1) * sizeof(char));
        strncpy(name, node->funcname, node->funcname_len);
        name[node->funcname_len] = '\0';
        printf("    call %s\n", name);
        printf("    push rax\n");
        return;
    case ND_FUNC:
        char *f = malloc((node->funcname_len + 1) * sizeof(char));
        strncpy(f, node->funcname, node->funcname_len);
        f[node->funcname_len] = '\0';
        
        if(strcmp(f, "main") == 0)
        {
            printf(".globl main\n");
        }
        printf("%s:\n", f);
        printf("# prologue\n");
        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        printf("    sub rsp, 208\n");
        printf("# prologue end\n");

        Node *fa = node->args;
        const char *fr[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
        int fi = 0;
        while (fa && fi < 6)
        {
            gen_lval_address(fa);
            printf("    pop rax\n");
            printf("    mov [rax], %s\n", fr[fi]);
            fa = fa->next;
            fi++;
        }

        gen(node->body);

        printf("# epilogue\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        printf("# epilogue end\n");

        return;
    case ND_ADDR:
        gen_lval_address(node->lhs);
        return;
    case ND_DEREF:
        gen(node->lhs);
        node->type = node->lhs->type;

        printf("# deref\n");
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        printf("# deref\n");
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
        node->type = op_result_type(node->lhs, node->rhs);
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        node->type = op_result_type(node->lhs, node->rhs);
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        node->type = int_type();
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        node->type = int_type();
        break;
    case ND_LESS_THAN:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        node->type = int_type();
        break;
    case ND_EQUAL_LESS_THAN:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        node->type = int_type();
        break;
    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        node->type = int_type();
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        node->type = int_type();
        break;
    }

    printf("    push rax\n");
}

void gen_string_literal(Node *node)
{
    if(node->kind != ND_STR_LITERAL){
        error("Not string literal node");
    }

    printf("%s:\n", str_literal_name(node));
    printf("    .string \"%.*s\"\n", node->strliteral_len, node->strliteral);
}

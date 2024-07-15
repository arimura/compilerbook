#include <stdio.h>
#include "codegen.h"
#include <string.h>
#include <stdlib.h>

// Generates the assembly code to push the address of a local variable onto the stack.
void gen_lval(Node *node)
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

void gen_address(Node *node)
{
    switch(node->kind){
        case ND_LVAR:
            gen_lval(node);
            return;
        case ND_DEREF:
            gen_address(node->lhs);
            printf("    pop rax\n");
            printf("    push [rax]\n");
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
        gen_lval(node);
        printf("# lvar value\n");
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        printf("# lvar value end\n");
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
            gen_lval(fa);
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
        gen_lval(node->lhs);
        return;
    case ND_DEREF:
        gen(node->lhs);
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

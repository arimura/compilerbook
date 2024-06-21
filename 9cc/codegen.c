#include <stdio.h>
#include "codegen.h"
#include <string.h>
#include <stdlib.h>

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
        printf("    je  .Lelse%d\n", c);
        gen(node->then);
        printf("    jmp  .Lend%d\n", c);
        printf(".Lelse%d:\n",c);
        if(node->els){
            gen(node->els);
        }
        printf(".Lend%d:\n",c);
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
        if(node->init){
            gen(node->init);
        }
        printf(".Lbegin%d:\n", cf);
        if(node->cond){
            gen(node->cond);
        }else{
            printf("    push 1\n");
        }
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", cf);
        gen(node->then);
        if(node->inc){
            gen(node->inc);
        }
        printf("    jmp .Lbegin%d\n", cf);
        printf(".Lend%d:\n", cf);
        return;
    case ND_BLOCK:
        Node *n = node->body;
        while(n){
            gen(n);
            printf("    pop rax\n");
            n = n->next;
        }
        return;
    case ND_FUNCALL:
        Node *a = node->args;
        int i = 0;
        const char *r[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
        while(a && i < 6){
            gen(a);
            printf("    pop %s\n", r[i]);
            a = a->next;
            i++;
        }
        //TODO: System V ABIに合わせるために、ここでrspを16の倍数にする。
        //ただし引数でregiserだけを使っている（=stackを使わない）なら常にrspは16の倍数になっている？

        char *name = malloc((node->funcname_len + 1) * sizeof(char));
        strncpy(name, node->funcname, node->funcname_len);
        name[node->funcname_len] = '\0';
        printf("    call %s\n", "foo");
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

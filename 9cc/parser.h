#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "9cc.h"

void init_lvar(void);
void destroy_lvar(void);
LVar *find_lvar(Token *tok);
void enter_scope(void);
void leave_scope(void);
Node *expr(void);
Node *declare_lvar(Token *tok);
bool is_lvar_decl(void);
Node *stmt(void);
Node *lvar(Token *tok);
Node *declare(void);
void program(void);
Node *assign(void);
Node *equality(void);
Node *relational(void);
Node *add(void);
Node *mul(void);
Node *unary(void);
Node *primary(void);

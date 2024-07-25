#include "parser.h"
Scope *scope = &(Scope){};
LVar *current_lvar;
GVar *global_var;

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
    node->type = calloc(1, sizeof(Type));
    node->type->ty = INT;
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

Token *consume_ident()
{
    if (token->kind != TK_INDENT)
        return NULL;
    Token *t = token;
    token = token->next;
    return t;
}

Token *consume_type()
{
    if (token->kind != TK_TYPE)
        return NULL;
    Token *t = token;
    token = token->next;
    return t;
}

bool consume_kind(TokenKind kind)
{
    if (token->kind != kind)
    {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
    {
        error("'%c'ではありません。現在のトークンは%dです。", op, token->kind);
    }
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

void init_lvar()
{
    current_lvar = NULL;
}

void destroy_lvar()
{
    current_lvar = NULL;
}

LVar *find_lvar(Token *tok)
{
    for (LVar *var = current_lvar; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}

GVar *find_gvar(Token *tok)
{
    for (GVar *var = global_var; var; var = var->next)
    {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
        {
            return var;
        }
    }
    return NULL;
}

void enter_scope()
{
    Scope *s = calloc(1, sizeof(Scope));
    s->next = scope;
    s->locals = calloc(1, sizeof(LVar));
    scope = s;
}

void leave_scope()
{
    scope = scope->next;
}

Node *expr()
{
    return assign();
}

Node *declare_lvar()
{
    if (!consume_kind(TK_TYPE))
    {
        return NULL;
    }

    // 変数名の左側の型情報（"int *i[3]"の"int *"の部分）
    //  現時点ではintのみ
    Type *base = calloc(1, sizeof(Type));
    base->ty = INT;
    Type *head = calloc(1, sizeof(Type));
    Type *c = head;
    while (consume("*"))
    {
        Type *n = calloc(1, sizeof(Type));
        n->ty = PTR;
        c->ptr_to = n;
        c = n;
    }
    c->ptr_to = base;

    // 変数名のtoken
    Token *i = consume_ident();
    if (!i || i->kind != TK_INDENT)
    {
        error("Not indent token\n");
    }

    // 変数名の右側の型情報（"int i[3]"の"[3]"の部分）
    if (consume("["))
    {
        int num = expect_number();
        Type *a = calloc(1, sizeof(Type));
        a->ty = ARRAY;
        a->ptr_to = c;
        a->array_size = num;
        head->ptr_to = a;
        expect(']');
    }

    // 変数ノードの生成とLVar型を管理用データ構造に登録
    Node *n = calloc(1, sizeof(Node));
    n->kind = ND_LVAR;
    LVar *l = find_lvar(i);
    if (l)
    {
        error("lvar already declared\n");
    }

    l = calloc(1, sizeof(LVar));
    l->next = current_lvar;
    l->name = i->str;
    l->len = i->len;
    l->type = head->ptr_to;
    int current_offset = current_lvar ? current_lvar->offset : 0;
    int offset;
    if (l->type->ty == INT)
    {
        offset = 8;
    }
    else if (l->type->ty == PTR)
    {
        offset = 8;
    }
    else if (l->type->ty == ARRAY)
    {
        offset = l->type->array_size * 8;
    }
    else
    {
        error("Unsupported type for lvar");
    }
    l->offset = current_offset + offset;
    n->offset = l->offset;
    current_lvar = l;
    return n;
}

Node *declare_func()
{
    Node *node = calloc(1, sizeof(Node));
    // 関数宣言
    if (!consume_kind(TK_TYPE))
    {
        error("関数宣言の型がありません");
    }

    Token *t = consume_ident();
    if (!t)
    {
        error("関数名がありません");
    }

    enter_scope();
    init_lvar();
    expect('(');
    node->kind = ND_FUNC;
    node->funcname = t->str;
    node->funcname_len = t->len;
    Node head = {};
    Node *cur = &head;

    Node *a;
    while (a = declare_lvar())
    {
        cur->next = a;
        cur = a;
        consume(",");
    }
    expect(')');
    node->args = head.next;
    node->body = stmt();
    destroy_lvar();
    leave_scope();
    return node;
}

Node *declare_gvar()
{
    if (!consume_kind(TK_TYPE))
    {
        return NULL;
    }

    // 変数名の左側の型情報（"int *i[3]"の"int *"の部分）
    //  現時点ではintのみ
    Type *base = calloc(1, sizeof(Type));
    base->ty = INT;
    Type *head = calloc(1, sizeof(Type));
    Type *c = head;
    while (consume("*"))
    {
        Type *n = calloc(1, sizeof(Type));
        n->ty = PTR;
        c->ptr_to = n;
        c = n;
    }
    c->ptr_to = base;

    // 変数名のtoken
    Token *i = consume_ident();
    if (!i || i->kind != TK_INDENT)
    {
        error("Not indent token\n");
    }

    // 変数名の右側の型情報（"int i[3]"の"[3]"の部分）
    if (consume("["))
    {
        int num = expect_number();
        Type *a = calloc(1, sizeof(Type));
        a->ty = ARRAY;
        a->ptr_to = c;
        a->array_size = num;
        head->ptr_to = a;
        expect(']');
    }

    Node *n = calloc(1, sizeof(Node));
    n->kind = ND_GVAR_DECL;
    GVar *g = find_gvar(i);
    if (g)
    {
        error("gvar already declared\n");
    }

    g = calloc(1, sizeof(GVar));
    g->next = global_var;
    g->name = i->str;
    g->len = i->len;
    g->type = head->ptr_to;
    global_var = g;

    n->type = g->type;
    n->gvarname = g->name;
    n->gvarname_len = g->len;
    return n;
}

bool is_lvar_decl()
{
    Token *org = token;
    while (consume("*"))
    {
    }
    bool r = consume_kind(TK_TYPE);
    token = org;
    return r;
}

bool is_func_decl()
{
    Token *org = token;
    Token *t = consume_type();
    if (!t)
    {
        token = org;
        return false;
    }

    t = consume_ident();
    if (!t)
    {
        token = org;
        return false;
    }

    if (!consume("("))
    {
        token = org;
        return false;
    }

    token = org;
    return true;
}

Node *stmt()
{
    Node *node;

    if (consume_kind(TK_IF))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect('(');
        node->cond = expr();
        expect(')');
        node->then = stmt();
        if (consume_kind(TK_ELSE))
        {
            node->els = stmt();
        }
    }
    else if (consume_kind(TK_WHILE))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect('(');
        node->cond = expr();
        expect(')');
        node->then = stmt();
    }
    else if (consume_kind(TK_FOR))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect('(');
        if (!consume(";"))
        {
            node->init = expr();
            expect(';');
        }
        if (!consume(";"))
        {
            node->cond = expr();
            expect(';');
        }
        if (!consume(")"))
        {
            node->inc = expr();
            expect(')');
        }
        node->then = stmt();
    }
    else if (consume_kind(TK_RETURN))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(';');
    }
    else if (consume("{"))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        Node head = {};
        Node *cur = &head;

        while (!consume("}"))
        {
            cur->next = stmt();
            cur = cur->next;
        }
        node->body = head.next;
    }
    else
    {
        // local var declaratoin or expression
        // TODO: arrayにも対応する
        // e.g. int i[2];
        // Type *t = lvar_type_declare();
        // if (t)
        Node *l = declare_lvar();
        if (l)
        {
            node = l;
            expect(';');
        }
        else
        {
            node = expr();
            expect(';');
        }
    }

    return node;
}

Node *lvar(Token *tok)
{
    if (tok->kind != TK_INDENT)
    {
        return NULL;
    }

    Node *ret;
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    LVar *lvar = find_lvar(tok);
    if (lvar)
    {
        node->offset = lvar->offset;
        node->type = lvar->type;
    }
    else
    {
        return NULL;
    }

    // 配列添字
    if (lvar->type->ty == ARRAY && consume("["))
    {
        int s = expect_number();
        Node *d = calloc(1, sizeof(Node));
        d->kind = ND_DEREF;
        d->lhs = new_node(ND_ADD, node, new_node_num(s));
        ret = d;
        expect(']');
    }
    else
    {
        ret = node;
    }

    return ret;
}

Node *gvar(Token *tok)
{
    if (tok->kind != TK_INDENT)
    {
        return NULL;
    }

    Node *ret;
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_GVAR;
    GVar *gvar = find_gvar(tok);
    if (gvar)
    {
        node->type = gvar->type;
        node->gvarname = gvar->name;
        node->gvarname_len = gvar->len;
    }
    else
    {
        return NULL;
    }

    // 配列添字
    if (gvar->type->ty == ARRAY && consume("["))
    {
        int s = expect_number();
        Node *d = calloc(1, sizeof(Node));
        d->kind = ND_DEREF;
        d->lhs = new_node(ND_ADD, node, new_node_num(s));
        ret = d;
        expect(']');
    }
    else
    {
        ret = node;
    }

    return ret;
}

Node *var(Token *t)
{
    Node *v = lvar(t);
    if (v)
    {
        return v;
    }

    v = gvar(t);
    if (v)
    {
        return v;
    }

    error("Undefined var");
}

Node *declare()
{
    if (is_func_decl())
    {
        return declare_func();
    }
    else
    {
        Node *g = declare_gvar();
        expect(';');
        return g;
    }
}

void program()
{
    int i = 0;
    while (!at_eof())
    {
        code[i++] = declare();
    }
    code[i] = NULL;
}

Node *assign()
{
    Node *node = equality();
    if (consume("="))
        node = new_node(ND_ASSIGN, node, assign());
    return node;
}

Node *equality()
{
    Node *node = relational();

    for (;;)
    {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
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
        {
            Node *l = node;
            Node *r = mul();

            // 複雑なパターンは一旦無視
            //  pointer arithmetic
            if (l && l->type && l->type->ty == PTR)
            {
                if (r && r->type && r->type->ty == PTR)
                {
                    error("poitner + pointer");
                }
                node = new_node(ND_ADD, l, new_node(ND_MUL, r, new_node_num(4)));
            }
            else if (r && r->type && r->type->ty == PTR)
            {
                if (l && l->type && l->type->ty == PTR)
                {
                    error("poitner + pointer");
                }
                node = new_node(ND_ADD, new_node(ND_MUL, l, new_node_num(4)), r);
            }
            else
            {
                node = new_node(ND_ADD, l, r);
            }
        }
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
    if (consume_kind(TK_SIZEOF))
    {
        // only support sizeof(1) or sizeof(var)
        // Not support sizefo(1 + 1)
        expect('(');
        Node *n = unary();
        expect(')');
        if (!n->type || n->type->ty == INT)
        {
            return new_node_num(4);
        }
        // pointer
        return new_node_num(8);
    }

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

    if (consume("&"))
    {
        Node *n = calloc(1, sizeof(Node));
        n->kind = ND_ADDR;
        n->lhs = unary();
        return n;
    }

    if (consume("*"))
    {
        Node *n = calloc(1, sizeof(Node));
        n->kind = ND_DEREF;
        n->lhs = unary();
        return n;
    }

    Token *tok = consume_ident();
    if (tok)
    {

        // func call
        if (consume("("))
        {
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_FUNCALL;
            node->funcname = tok->str;
            node->funcname_len = tok->len;
            Node head = {};
            Node *cur = &head;

            while (!consume(")"))
            {
                cur->next = expr();
                cur = cur->next;
                consume(",");
            }
            node->args = head.next;
            return node;
        }

        return var(tok);
    }

    return new_node_num(expect_number());
}

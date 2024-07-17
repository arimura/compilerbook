#include "parser.h"
Scope *scope = &(Scope){};
LVar *current_lvar;

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

Node *declare_lvar(Token *tok, Type *type)
{
    if (tok->kind != TK_INDENT)
    {
        error("Not indent token\n");
    }

    Node *n = calloc(1, sizeof(Node));
    n->kind = ND_LVAR;
    LVar *l = find_lvar(tok);
    if (l)
    {
        error("lvar already declared\n");
    }
    else
    {
        l = calloc(1, sizeof(LVar));
        l->next = current_lvar;
        l->name = tok->str;
        l->len = tok->len;
        l->offset = current_lvar ? current_lvar->offset + 8 : 8;
        l->type = type;
        n->offset = l->offset;
        current_lvar = l;
        return n;
    }
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

Type *lvar_type_declare()
{
    if (!consume_kind(TK_TYPE))
    {
        return NULL;
    }
    // 現時点ではintのみ
    Type *l = calloc(1, sizeof(Type));
    l->ty = INT;

    Type *head = calloc(1, sizeof(Type));
    Type *c = head;
    while (consume("*"))
    {
        Type *n = calloc(1, sizeof(Type));
        n->ty = PTR;
        c->ptr_to = n;
        c = n;
    }
    c->ptr_to = l;
    return head->ptr_to;
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
        Type *t = lvar_type_declare();
        if (t)
        {
            Token *i = consume_ident();
            node = declare_lvar(i, t);
            node->type = t;
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
        error("変数が見つかりません");
    }
    return node;
}

Node *declare()
{
    Node *node = calloc(1, sizeof(Node));
    // 関数宣言
    if (!consume_kind(TK_TYPE))
    {
        error("関数の戻り値の型がありません");
    }

    Token *t = consume_ident();
    if (t)
    {
        enter_scope();
        init_lvar();
        expect('(');
        node->kind = ND_FUNC;
        node->funcname = t->str;
        node->funcname_len = t->len;
        Node head = {};
        Node *cur = &head;

        Type *type;
        while (type = lvar_type_declare())
        {
            Node *n = declare_lvar(consume_ident(), type);
            cur->next = n;
            cur = n;
            consume(",");
        }
        expect(')');
        node->args = head.next;
        node->body = stmt();
        destroy_lvar();
        leave_scope();
        return node;
    }
    else
    {
        error("トップレベルでは宣言が必要です");
    }
    // 今は関数外のstatementも許可
    return stmt();
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

        return lvar(tok);
    }

    return new_node_num(expect_number());
}

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
expr    = equality
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul     = unary ("*" unary | "/" unary)*
unary   = ("+" | "-")? primary
primary = num | "(" expr ")"
**/

// 抽象構文木のノードの種類
typedef enum {
    ND_EQV, // ==
    ND_NEQ, // !=
    ND_LES, // <   // > 左右ひっくり返す
    ND_LEQ, // <=   // >= 左右ひっくり返して
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // 整数
} NodeKind;


typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;  // 左辺
    Node *rhs;  // 右辺
    int  val;   // Nodeが数値の場合のみ使用する。
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = (Node*)calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}


Node *new_node_num(int val) {
    Node *node = (Node*)calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->lhs = NULL;
    node->rhs = NULL;
    node->val = val;
    return node;
}

// function prototype
Node *mul();
Node *primary(); 
Node *equality();
Node *relational();
Node *add(); 


typedef enum {
    TK_RESERVED = 0,  // 記号
    TK_NUM,           // 整数トークン
    TK_EOF,           // 入力の終り
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;     // トークンの型
    Token *next;        // 次の入力トークン
    int val;            // kindが TK_NUMの場合、その数値
    char *str;          // トークン文字列
    int  len;           // トークンの長さ
};

Token *token;  // 現在のtoken;
char *user_input; // 入力プログラム

/**
 * エラーを報告するための関数
 */
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int pos = (int)(loc - user_input);
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");  // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

/**
 * 次のトークンが期待している記号の時には、トークンを読みすすめて、真を返す。
 * それ以外は、偽を返す。
 */
bool consume(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len || 
        memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

/**
 * 次のトークンが期待している記号のときには、トークンを1つ読み進める。
 * それ以外の場合にはエラーを報告する。
 */
void expect(char *op) {
  if (token->kind != TK_RESERVED ||
        strlen(op) != token->len || 
        memcmp(token->str, op, token->len)) {
    error_at(token->str, "'%s'ではありません", op);
  }
  token = token->next;
}

/**
 * 次のトークンが期待している数値の時には、トークンを読みすすめて、その数値を返す。
 * それ以外は、failする。
 */
int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "数ではありません。");
    }
    int val = token->val;
    token = token->next;
    return val;
}

/**
 * 次のトークンが終端の場合は真を返す。 
 */
bool at_eof() {
    return token->kind == TK_EOF;
}

/**
 * 新しいトークンを作成して、curに繋げる。
 */
Token* new_token(TokenKind kind, Token *cur, char* str, int len) {
    Token *tok = (Token*)calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

/**
 * 入力文字列をトークナイズして返す。
 */
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // 空白はskip
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (!memcmp(p, "==", 2) ||
            !memcmp(p, "!=", 2) ||
            !memcmp(p, ">=", 2) ||
            !memcmp(p, "<=", 2)) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if (*p == '+' || *p == '-' ||
            *p == '*' || *p == '/' ||
            *p == '>' || *p == '<' ||
            *p == '(' || *p == ')') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if(isdigit(*p)) {
            char *org_p = p;
            int val = (int)strtol(p, &p, 10);
            int len  = (int)(p - org_p);
            cur = new_token(TK_NUM, cur, org_p, len);
            cur->val = val;
            continue;
        }

        error_at(p, "トークナイズ出来ません。");
    }
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

Node *expr() {
    Node *node = equality();
    return node;
}

Node *equality() {
    Node *node = relational();
    for (;;) {
        if (consume("==")) {
            node = new_node(ND_EQV, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NEQ, node, relational());
        } else {
            break;
        }
    }
    return node;
}

Node *relational() {
    Node *node = add();
    for (;;) {
        if (consume(">=")) {
            node = new_node(ND_LEQ, add(), node);
        } else if (consume("<=")) {
            node = new_node(ND_LEQ, node, add());
        } else if (consume(">")) {
            node = new_node(ND_LES, add(), node);
        } else if (consume("<")) {
            node = new_node(ND_LES, node, add());
        } else {
            break;
        }
    }
    return node;
}

Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            break;
        }
    }
    return node;
}

Node *unary() {
    if (consume("+")) {
        return primary();
    }
    if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}

Node *primary() {
    // 次のトークンが"("なら、"(" expr ")" であるはず
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    //そうでないなら数値
    return new_node_num(expect_number());
}

Node *mul() {
    Node *node = unary();

    for(;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        }
        return node;
    }
    return NULL;
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

    switch(node->kind) {
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
    case ND_EQV:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NEQ:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LES:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LEQ:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }
    printf("    push rax\n");
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "引数の数が正しくありません。\n");
        return 1;
    }

    // 入力の保存
    user_input = argv[1];
    // トークナイズする。
    token = tokenize(user_input);
    Node *node  = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}

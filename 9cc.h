#ifndef _9CC_H_
#define _9CC_H_

// トークンの種類
typedef enum {
    TK_RESERVED = 0,  // 記号
    TK_IDENT,         // 識別子
    TK_NUM,           // 整数トークン
    TK_EOF,           // 入力の終り
} TokenKind;

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
    ND_ASSIGN, // =
    ND_LVAR,   // ローカル変数
    ND_NUM, // 整数
} NodeKind;

typedef struct LVar LVar;

struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
};

typedef struct Node Node;
typedef struct Token Token;

struct Node {
    NodeKind kind;
    Node *lhs;  // 左辺
    Node *rhs;  // 右辺
    int  val;   // Nodeが数値の場合のみ使用する。
    int offset;    // kindがND_LVARの場合のみ使う
};



// function prototype
Token *tokenize(char *p);
Node *program();
void gen(Node *node);
#endif

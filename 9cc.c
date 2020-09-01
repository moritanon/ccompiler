#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
};

Token *token;  // 現在のtoken;

/**
 * エラーを報告するための関数
 */
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

/**
 * 次のトークンが期待している記号の時には、トークンを読みすすめて、真を返す。
 * それ以外は、偽を返す。
 */
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

/**
 * 次のトークンが期待している数値の時には、トークンを読みすすめて、その数値を返す。
 * それ以外は、failする。
 */
int expect_number() {
    if (token->kind != TK_NUM) {
        error("数ではありません。");
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
Token* new_token(TokenKind kind, Token *cur, char* str) {
    Token *tok = (Token*)calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
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

        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if(isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = (int)strtol(p, &p, 10);
            continue;
        }

        error("トークナイズ出来ません。");
    }
    new_token(TK_EOF, cur, p);
    return head.next;
}


int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "引数の数が正しくありません。\n");
        return 1;
    }

    // トークナイズする。
    token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 式の最初は数でなければならないので、それをチェックして最初のmov命令発行
    printf("    mov rax, %d\n", expect_number());

    // `+ <数値>` または、`- <数値>` というトークンの並びを消費しつつ
    // アセンブリを出力
    while(!at_eof()) {
        if (consume('+')) {
            printf("    add rax, %d\n", expect_number());
            continue;
        }
        consume('-');
        printf("    sub rax, %d\n", expect_number());
    }
    printf("    ret\n");
    return 0;
}

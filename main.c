#include "9cc.h"

#include <stdio.h>

/**
expr    = equality
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul     = unary ("*" unary | "/" unary)*
unary   = ("+" | "-")? primary
primary = num | "(" expr ")"
**/
extern Token *token;
extern char *user_input;

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

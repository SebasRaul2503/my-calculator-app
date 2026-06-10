#include "lexer.h"

void lexer_init(Lexer *lx, const char *src) {
    lx->src = src;
    lx->pos = 0;
}

Token lexer_next(Lexer *lx) {
    (void)lx;
    Token t = {0};
    t.type = TOK_END;
    return t;
}

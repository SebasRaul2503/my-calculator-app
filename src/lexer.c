#include "lexer.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void lexer_init(Lexer *lx, const char *src) {
    lx->src = src;
    lx->pos = 0;
}

static void skip_spaces(Lexer *lx) {
    while (lx->src[lx->pos] == ' ' || lx->src[lx->pos] == '\t')
        lx->pos++;
}

Token lexer_next(Lexer *lx) {
    Token t;
    t.number = 0;
    t.ident[0] = '\0';

    skip_spaces(lx);
    char c = lx->src[lx->pos];

    if (c == '\0') { t.type = TOK_END; return t; }

    if (isdigit((unsigned char)c) || c == '.') {
        char *end;
        t.number = strtod(lx->src + lx->pos, &end);
        if (end == lx->src + lx->pos) { t.type = TOK_ERROR; return t; }
        lx->pos = (size_t)(end - lx->src);
        t.type = TOK_NUMBER;
        return t;
    }

    if (isalpha((unsigned char)c)) {
        size_t start = lx->pos;
        while (isalpha((unsigned char)lx->src[lx->pos])) lx->pos++;
        size_t len = lx->pos - start;
        if (len >= sizeof(t.ident)) { t.type = TOK_ERROR; return t; }
        memcpy(t.ident, lx->src + start, len);
        t.ident[len] = '\0';
        t.type = TOK_IDENT;
        return t;
    }

    lx->pos++;
    switch (c) {
        case '+': t.type = TOK_PLUS;    return t;
        case '-': t.type = TOK_MINUS;   return t;
        case '*': t.type = TOK_STAR;    return t;
        case '/': t.type = TOK_SLASH;   return t;
        case '%': t.type = TOK_PERCENT; return t;
        case '^': t.type = TOK_CARET;   return t;
        case '(': t.type = TOK_LPAREN;  return t;
        case ')': t.type = TOK_RPAREN;  return t;
        default:  t.type = TOK_ERROR;   return t;
    }
}

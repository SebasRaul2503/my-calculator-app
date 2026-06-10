#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum {
    TOK_NUMBER,
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_PERCENT, TOK_CARET,
    TOK_LPAREN, TOK_RPAREN,
    TOK_IDENT,
    TOK_END,
    TOK_ERROR
} TokenType;

typedef struct {
    TokenType type;
    double number;      // válido si type == TOK_NUMBER
    char ident[16];     // válido si type == TOK_IDENT
} Token;

typedef struct {
    const char *src;
    size_t pos;
} Lexer;

void lexer_init(Lexer *lx, const char *src);
Token lexer_next(Lexer *lx);

#endif

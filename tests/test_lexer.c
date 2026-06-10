#include "lexer.h"
#include "test_util.h"
#include <string.h>

int main(void) {
    Lexer lx;

    // Números y operadores
    lexer_init(&lx, "12+3.5");
    Token t = lexer_next(&lx);
    CHECK(t.type == TOK_NUMBER); CHECK_DBL(t.number, 12);
    t = lexer_next(&lx); CHECK(t.type == TOK_PLUS);
    t = lexer_next(&lx); CHECK(t.type == TOK_NUMBER); CHECK_DBL(t.number, 3.5);
    t = lexer_next(&lx); CHECK(t.type == TOK_END);

    // Todos los operadores y paréntesis, con espacios
    lexer_init(&lx, " - * / % ^ ( ) ");
    CHECK(lexer_next(&lx).type == TOK_MINUS);
    CHECK(lexer_next(&lx).type == TOK_STAR);
    CHECK(lexer_next(&lx).type == TOK_SLASH);
    CHECK(lexer_next(&lx).type == TOK_PERCENT);
    CHECK(lexer_next(&lx).type == TOK_CARET);
    CHECK(lexer_next(&lx).type == TOK_LPAREN);
    CHECK(lexer_next(&lx).type == TOK_RPAREN);
    CHECK(lexer_next(&lx).type == TOK_END);

    // Identificador
    lexer_init(&lx, "sin(pi)");
    t = lexer_next(&lx); CHECK(t.type == TOK_IDENT); CHECK(strcmp(t.ident, "sin") == 0);
    CHECK(lexer_next(&lx).type == TOK_LPAREN);
    t = lexer_next(&lx); CHECK(t.type == TOK_IDENT); CHECK(strcmp(t.ident, "pi") == 0);
    CHECK(lexer_next(&lx).type == TOK_RPAREN);

    // Carácter inválido
    lexer_init(&lx, "2 & 3");
    CHECK(lexer_next(&lx).type == TOK_NUMBER);
    CHECK(lexer_next(&lx).type == TOK_ERROR);

    TEST_REPORT();
}

#include "parser.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif

static double parse_expr(Parser *p);
static double parse_term(Parser *p);
static double parse_unary(Parser *p);
static double parse_power(Parser *p);
static double parse_primary(Parser *p);

static void advance(Parser *p) { p->cur = lexer_next(&p->lx); }

void parser_init(Parser *p, const char *expr, CalcAngleMode mode) {
    lexer_init(&p->lx, expr);
    p->mode = mode;
    p->status = CALC_OK;
    advance(p);
}

double parser_parse_expr(Parser *p) {
    return parse_expr(p);
}

static double parse_expr(Parser *p) {
    if (p->status != CALC_OK) return 0;
    double v = parse_term(p);
    while (p->status == CALC_OK &&
           (p->cur.type == TOK_PLUS || p->cur.type == TOK_MINUS)) {
        TokenType op = p->cur.type;
        advance(p);
        double r = parse_term(p);
        if (op == TOK_PLUS) v += r; else v -= r;
    }
    return v;
}

static double parse_term(Parser *p) {
    if (p->status != CALC_OK) return 0;
    double v = parse_unary(p);
    while (p->status == CALC_OK &&
           (p->cur.type == TOK_STAR || p->cur.type == TOK_SLASH ||
            p->cur.type == TOK_PERCENT)) {
        TokenType op = p->cur.type;
        advance(p);
        double r = parse_unary(p);
        if (p->status != CALC_OK) return 0;
        if (op == TOK_STAR) {
            v *= r;
        } else if (op == TOK_SLASH) {
            if (r == 0.0) { p->status = CALC_ERR_DIV_ZERO; return 0; }
            v /= r;
        } else {
            if (r == 0.0) { p->status = CALC_ERR_DIV_ZERO; return 0; }
            v = fmod(v, r);
        }
    }
    return v;
}

static double parse_unary(Parser *p) {
    if (p->status != CALC_OK) return 0;
    if (p->cur.type == TOK_MINUS) { advance(p); return -parse_unary(p); }
    if (p->cur.type == TOK_PLUS)  { advance(p); return  parse_unary(p); }
    return parse_power(p);
}

static double parse_power(Parser *p) {
    if (p->status != CALC_OK) return 0;
    double base = parse_primary(p);
    if (p->status == CALC_OK && p->cur.type == TOK_CARET) {
        advance(p);
        double exp = parse_unary(p);   // recursión → asociativa a la derecha
        if (p->status != CALC_OK) return 0;
        base = pow(base, exp);
    }
    return base;
}

static double parse_primary(Parser *p) {
    if (p->status != CALC_OK) return 0;
    Token t = p->cur;

    if (t.type == TOK_NUMBER) {
        advance(p);
        return t.number;
    }
    if (t.type == TOK_LPAREN) {
        advance(p);
        double v = parse_expr(p);
        if (p->status != CALC_OK) return 0;
        if (p->cur.type != TOK_RPAREN) { p->status = CALC_ERR_PAREN; return 0; }
        advance(p);
        return v;
    }
    // TOK_IDENT (constantes/funciones) se implementa en la Task 4.
    p->status = CALC_ERR_SYNTAX;
    return 0;
}

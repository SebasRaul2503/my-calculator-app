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
static double apply_func(Parser *p, const char *name, double arg);

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

static double to_rad(Parser *p, double x) {
    return (p->mode == CALC_MODE_DEG) ? x * M_PI / 180.0 : x;
}
static double from_rad(Parser *p, double x) {
    return (p->mode == CALC_MODE_DEG) ? x * 180.0 / M_PI : x;
}

static double apply_func(Parser *p, const char *name, double arg) {
    if (strcmp(name, "sin") == 0) return sin(to_rad(p, arg));
    if (strcmp(name, "cos") == 0) return cos(to_rad(p, arg));
    if (strcmp(name, "tan") == 0) return tan(to_rad(p, arg));
    if (strcmp(name, "asin") == 0) {
        if (arg < -1.0 || arg > 1.0) { p->status = CALC_ERR_DOMAIN; return 0; }
        return from_rad(p, asin(arg));
    }
    if (strcmp(name, "acos") == 0) {
        if (arg < -1.0 || arg > 1.0) { p->status = CALC_ERR_DOMAIN; return 0; }
        return from_rad(p, acos(arg));
    }
    if (strcmp(name, "atan") == 0) return from_rad(p, atan(arg));
    if (strcmp(name, "ln") == 0) {
        if (arg <= 0.0) { p->status = CALC_ERR_DOMAIN; return 0; }
        return log(arg);
    }
    if (strcmp(name, "log") == 0) {
        if (arg <= 0.0) { p->status = CALC_ERR_DOMAIN; return 0; }
        return log10(arg);
    }
    if (strcmp(name, "sqrt") == 0) {
        if (arg < 0.0) { p->status = CALC_ERR_DOMAIN; return 0; }
        return sqrt(arg);
    }
    if (strcmp(name, "exp") == 0) return exp(arg);
    if (strcmp(name, "abs") == 0) return fabs(arg);
    p->status = CALC_ERR_UNKNOWN_FUNC;
    return 0;
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
    if (t.type == TOK_IDENT) {
        advance(p);
        if (strcmp(t.ident, "pi") == 0) return M_PI;
        if (strcmp(t.ident, "e") == 0)  return M_E;
        // función: requiere '(' expr ')'
        if (p->cur.type != TOK_LPAREN) { p->status = CALC_ERR_UNKNOWN_FUNC; return 0; }
        advance(p);
        double arg = parse_expr(p);
        if (p->status != CALC_OK) return 0;
        if (p->cur.type != TOK_RPAREN) { p->status = CALC_ERR_PAREN; return 0; }
        advance(p);
        return apply_func(p, t.ident, arg);
    }
    p->status = CALC_ERR_SYNTAX;
    return 0;
}

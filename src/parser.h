#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "evaluator.h"

typedef struct {
    Lexer lx;
    Token cur;
    CalcAngleMode mode;
    CalcStatus status;
} Parser;

void parser_init(Parser *p, const char *expr, CalcAngleMode mode);
double parser_parse_expr(Parser *p);

#endif

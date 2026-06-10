#include "evaluator.h"
#include "parser.h"
#include <math.h>

CalcStatus calc_eval(const char *expr, CalcAngleMode mode, double *out_result) {
    Parser p;
    parser_init(&p, expr, mode);
    double v = parser_parse_expr(&p);

    if (p.status != CALC_OK) return p.status;
    if (p.cur.type != TOK_END) return CALC_ERR_SYNTAX;  // tokens sobrantes o error léxico
    if (!isfinite(v)) return CALC_ERR_OVERFLOW;

    *out_result = v;
    return CALC_OK;
}

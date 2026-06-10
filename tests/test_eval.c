#include "evaluator.h"
#include "test_util.h"

static double ev(const char *s) {
    double r = 0;
    CalcStatus st = calc_eval(s, CALC_MODE_DEG, &r);
    CHECK(st == CALC_OK);
    return r;
}

static CalcStatus err(const char *s) {
    double r = 0;
    return calc_eval(s, CALC_MODE_DEG, &r);
}

int main(void) {
    // Aritmética y precedencia
    CHECK_DBL(ev("2+3*4"), 14);
    CHECK_DBL(ev("(2+3)*4"), 20);
    CHECK_DBL(ev("10-2-3"), 5);        // izquierda-asociativo
    CHECK_DBL(ev("8/2/2"), 2);
    CHECK_DBL(ev("7%3"), 1);
    CHECK_DBL(ev("2.5*2"), 5);

    // Unario y potencia
    CHECK_DBL(ev("-2^2"), -4);         // ^ liga más fuerte que el menos unario
    CHECK_DBL(ev("2^3^2"), 512);       // ^ asociativa a la derecha
    CHECK_DBL(ev("2^-2"), 0.25);       // exponente unario
    CHECK_DBL(ev("--3"), 3);

    // Errores
    CHECK(err("1/0") == CALC_ERR_DIV_ZERO);
    CHECK(err("5%0") == CALC_ERR_DIV_ZERO);
    CHECK(err("(2+3") == CALC_ERR_PAREN);
    CHECK(err("2+") == CALC_ERR_SYNTAX);
    CHECK(err("") == CALC_ERR_SYNTAX);
    CHECK(err("2 3") == CALC_ERR_SYNTAX);   // tokens sobrantes
    CHECK(err("2 & 3") == CALC_ERR_SYNTAX);

    TEST_REPORT();
}

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

    // Constantes
    CHECK_DBL(ev("pi"), 3.14159265358979323846);
    CHECK_DBL(ev("e"), 2.71828182845904523536);

    // Funciones en modo DEG (ev usa CALC_MODE_DEG)
    CHECK_DBL(ev("sin(90)"), 1);
    CHECK_DBL(ev("cos(0)"), 1);
    CHECK_DBL(ev("sqrt(9)"), 3);
    CHECK_DBL(ev("abs(-5)"), 5);
    CHECK_DBL(ev("exp(0)"), 1);

    // Funciones en modo RAD
    double rr = 0;
    CHECK(calc_eval("sin(pi/2)", CALC_MODE_RAD, &rr) == CALC_OK); CHECK_DBL(rr, 1);
    CHECK(calc_eval("ln(e)",     CALC_MODE_RAD, &rr) == CALC_OK); CHECK_DBL(rr, 1);
    CHECK(calc_eval("log(1000)", CALC_MODE_RAD, &rr) == CALC_OK); CHECK_DBL(rr, 3);

    // Errores de dominio / función desconocida
    CHECK(err("sqrt(-1)") == CALC_ERR_DOMAIN);
    CHECK(err("ln(0)")    == CALC_ERR_DOMAIN);
    CHECK(err("asin(2)")  == CALC_ERR_DOMAIN);
    CHECK(err("foo(1)")   == CALC_ERR_UNKNOWN_FUNC);
    CHECK(err("bar")      == CALC_ERR_UNKNOWN_FUNC);

    // Casos de borde adicionales
    CHECK(err("1e308*1e308") == CALC_ERR_OVERFLOW);  // resultado no finito
    CHECK(err("   ") == CALC_ERR_SYNTAX);             // solo espacios
    CHECK(err("2+3)") == CALC_ERR_SYNTAX);            // paréntesis de cierre extra
    // out_result no se modifica en error
    double sentinel = 42.0;
    CHECK(calc_eval("1/0", CALC_MODE_DEG, &sentinel) == CALC_ERR_DIV_ZERO);
    CHECK_DBL(sentinel, 42.0);

    TEST_REPORT();
}

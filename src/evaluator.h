#ifndef EVALUATOR_H
#define EVALUATOR_H

typedef enum {
    CALC_OK,
    CALC_ERR_SYNTAX,        // expresión malformada / tokens sobrantes
    CALC_ERR_DIV_ZERO,      // división o módulo por cero
    CALC_ERR_PAREN,         // paréntesis desbalanceados
    CALC_ERR_UNKNOWN_FUNC,  // función o nombre desconocido
    CALC_ERR_DOMAIN,        // dominio inválido (sqrt(-1), ln(0), asin(2)...)
    CALC_ERR_OVERFLOW       // resultado no finito (inf/nan)
} CalcStatus;

typedef enum { CALC_MODE_DEG, CALC_MODE_RAD } CalcAngleMode;

// Evalúa `expr`. En éxito escribe en *out_result y devuelve CALC_OK.
// En error devuelve el código y no modifica *out_result.
CalcStatus calc_eval(const char *expr, CalcAngleMode mode, double *out_result);

#endif

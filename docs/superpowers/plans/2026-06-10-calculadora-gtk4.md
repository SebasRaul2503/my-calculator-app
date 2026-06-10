# Plan de implementación: Calculadora científica en C + GTK4

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Construir una calculadora científica de escritorio en C con GTK4, con motor de evaluación de expresiones propio (descenso recursivo), historial y memoria, distribuible vía un script `.sh`.

**Architecture:** Separación estricta entre un **motor** (lógica pura en C, sin GTK: lexer → parser de descenso recursivo → evaluador, más módulos de memoria e historial) compilado como librería estática, y una **GUI GTK4** que enlaza contra esa librería. El motor se testea de forma aislada con `meson test`.

**Tech Stack:** C11, GTK4, Meson + Ninja, librería `m` (math). Tests propios con macros de aserción (sin framework externo).

---

## Notas de implementación (refinamientos sobre el spec)

Estas decisiones afinan el spec aprobado; ninguna cambia el alcance:

1. **Gramática de potencia/unario.** Para respetar la convención matemática
   (`-2^2 = -4`, exponenciación liga más fuerte que el menos unario, y `^` es
   asociativa a la derecha), la gramática implementada es:
   ```
   expr    → term  (('+' | '-') term)*
   term    → unary (('*' | '/' | '%') unary)*
   unary   → ('+' | '-') unary | power
   power   → primary ('^' unary)?        # recursión → asociativa a la derecha
   primary → NUMBER | CONST | FUNC '(' expr ')' | '(' expr ')'
   ```
2. **GUI construida programáticamente** (no `.ui`/GResource). Construir ~35
   botones con un array data-driven en C es más robusto y mantenible que un XML,
   mantiene toda la GUI en un archivo testeable manualmente y elimina la
   dependencia de `gnome.compile_resources`. El binario sigue siendo
   autocontenido.
3. **Icono temático estándar.** El `.desktop` usa `Icon=accessories-calculator`
   (nombre del tema freedesktop) para no enviar un asset binario; se puede
   sustituir por un icono propio más adelante.
4. **Indicador de memoria** = "memoria en uso" (`has_value`), que es más claro
   que "valor ≠ 0": tras MC se apaga; tras M+/M- se enciende aunque el total sea 0.

---

## Estructura de archivos

```
my-calculator-app/
├── meson.build              # build raíz (crece por tarea)
├── build.sh                 # compila + instala
├── src/
│   ├── lexer.h / lexer.c           # tokenizador
│   ├── evaluator.h / evaluator.c   # API pública calc_eval + códigos de error
│   ├── parser.h / parser.c         # descenso recursivo (interno)
│   ├── memory_reg.h / memory_reg.c # registro de memoria
│   ├── history.h / history.c       # historial en memoria
│   ├── calc_window.h / calc_window.c # ventana y widgets GTK4
│   └── main.c                       # GtkApplication
├── tests/
│   ├── meson.build
│   ├── test_util.h
│   ├── test_lexer.c
│   ├── test_eval.c
│   ├── test_memory.c
│   └── test_history.c
└── data/
    └── io.github.sebastian.Calc.desktop
```

---

## Task 1: Scaffold del proyecto y build mínimo

Establece la estructura, el build con Meson y un lexer stub que compila, para
tener un proyecto configurable desde el inicio.

**Files:**
- Create: `meson.build`
- Create: `tests/meson.build`
- Create: `tests/test_util.h`
- Create: `src/lexer.h`
- Create: `src/lexer.c` (stub)
- Create: `.gitignore`

- [ ] **Step 1: Crear `.gitignore`**

```gitignore
/build/
```

- [ ] **Step 2: Crear `meson.build` raíz**

```meson
project('mycalc', 'c',
  version: '0.1.0',
  default_options: ['c_std=c11', 'warning_level=2'])

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required: false)

engine_inc = include_directories('src')

engine_lib = static_library('calcengine',
  ['src/lexer.c'],
  include_directories: engine_inc,
  dependencies: m_dep)

subdir('tests')
```

- [ ] **Step 3: Crear `tests/meson.build` vacío** (placeholder; se llena en tareas siguientes)

```meson
# Tests registrados en tareas posteriores
```

- [ ] **Step 4: Crear `tests/test_util.h`**

```c
#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stdio.h>
#include <math.h>

static int test_failures = 0;

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        test_failures++; \
    } \
} while (0)

#define CHECK_DBL(a, b) do { \
    double _a = (a), _b = (b); \
    if (fabs(_a - _b) > 1e-9) { \
        fprintf(stderr, "FAIL %s:%d: %g != %g\n", __FILE__, __LINE__, _a, _b); \
        test_failures++; \
    } \
} while (0)

#define TEST_REPORT() return test_failures ? 1 : 0

#endif
```

- [ ] **Step 5: Crear `src/lexer.h`** (interfaz final)

```c
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
```

- [ ] **Step 6: Crear `src/lexer.c`** (stub que compila)

```c
#include "lexer.h"

void lexer_init(Lexer *lx, const char *src) {
    lx->src = src;
    lx->pos = 0;
}

Token lexer_next(Lexer *lx) {
    (void)lx;
    Token t = {0};
    t.type = TOK_END;
    return t;
}
```

- [ ] **Step 7: Configurar y compilar**

Run: `meson setup build && meson compile -C build`
Expected: configura y compila sin errores (genera `build/`).

- [ ] **Step 8: Commit**

```bash
git add .gitignore meson.build tests/meson.build tests/test_util.h src/lexer.h src/lexer.c
git commit -m "build: scaffold meson + lexer stub"
```

---

## Task 2: Lexer (tokenizador)

**Files:**
- Modify: `src/lexer.c` (implementación completa)
- Create: `tests/test_lexer.c`
- Modify: `tests/meson.build`

- [ ] **Step 1: Escribir el test que falla — `tests/test_lexer.c`**

```c
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
```

- [ ] **Step 2: Registrar el test en `tests/meson.build`**

```meson
test_lexer = executable('test_lexer', 'test_lexer.c',
  link_with: engine_lib,
  include_directories: engine_inc,
  dependencies: m_dep)
test('lexer', test_lexer)
```

- [ ] **Step 3: Ejecutar el test y verificar que falla**

Run: `meson compile -C build && meson test -C build lexer`
Expected: FAIL (el stub devuelve siempre TOK_END).

- [ ] **Step 4: Implementar `src/lexer.c` completo**

```c
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
```

- [ ] **Step 5: Ejecutar el test y verificar que pasa**

Run: `meson compile -C build && meson test -C build lexer`
Expected: PASS.

- [ ] **Step 6: Commit**

```bash
git add src/lexer.c tests/test_lexer.c tests/meson.build
git commit -m "feat: tokenizador (lexer)"
```

---

## Task 3: Evaluador — aritmética, paréntesis, unario, potencia y errores

Implementa el descenso recursivo para `+ - * / %`, paréntesis, menos unario y
potencia `^`, junto con los errores que producen estas reglas (división por
cero, paréntesis desbalanceados, sintaxis, overflow).

**Files:**
- Create: `src/evaluator.h`
- Create: `src/parser.h`
- Create: `src/parser.c`
- Create: `src/evaluator.c`
- Create: `tests/test_eval.c`
- Modify: `meson.build` (añadir fuentes al engine_lib)
- Modify: `tests/meson.build`

- [ ] **Step 1: Crear `src/evaluator.h`** (API pública del motor)

```c
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
```

- [ ] **Step 2: Crear `src/parser.h`** (interfaz interna)

```c
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
```

- [ ] **Step 3: Escribir los tests que fallan — `tests/test_eval.c`**

```c
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
```

- [ ] **Step 4: Añadir las fuentes del motor a `meson.build`**

Reemplaza la lista de fuentes de `engine_lib` por:

```meson
engine_lib = static_library('calcengine',
  ['src/lexer.c', 'src/parser.c', 'src/evaluator.c'],
  include_directories: engine_inc,
  dependencies: m_dep)
```

- [ ] **Step 5: Registrar el test en `tests/meson.build`** (añadir al final)

```meson
test_eval = executable('test_eval', 'test_eval.c',
  link_with: engine_lib,
  include_directories: engine_inc,
  dependencies: m_dep)
test('eval', test_eval)
```

- [ ] **Step 6: Crear `src/parser.c`** (descenso recursivo; funciones/constantes en la Task 4)

```c
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
```

- [ ] **Step 7: Crear `src/evaluator.c`** (orquesta y mapea errores)

```c
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
```

- [ ] **Step 8: Ejecutar y verificar que falla, luego que pasa**

Run: `meson compile -C build && meson test -C build eval`
Expected: primero FAIL si se ejecuta antes de los pasos 6-7; tras implementarlos, PASS.

- [ ] **Step 9: Commit**

```bash
git add src/evaluator.h src/parser.h src/parser.c src/evaluator.c tests/test_eval.c meson.build tests/meson.build
git commit -m "feat: evaluador (aritmética, paréntesis, unario, potencia, errores)"
```

---

## Task 4: Funciones científicas, constantes y modo ángulo

Añade `pi`, `e` y las funciones (`sin cos tan asin acos atan log ln sqrt exp abs`)
con respeto al modo DEG/RAD y los errores de dominio / función desconocida.

**Files:**
- Modify: `src/parser.c` (`parse_primary` + helpers)
- Modify: `tests/test_eval.c` (añadir casos)

- [ ] **Step 1: Añadir los tests que fallan a `tests/test_eval.c`**

Añade una segunda función de evaluación con modo y nuevos asserts dentro de `main`
(antes de `TEST_REPORT()`):

```c
    // (añadir helper arriba, junto a ev/err)
    // --- en main, antes de TEST_REPORT(): ---

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
    double r = 0;
    CHECK(calc_eval("sin(pi/2)", CALC_MODE_RAD, &r) == CALC_OK); CHECK_DBL(r, 1);
    CHECK(calc_eval("ln(e)",     CALC_MODE_RAD, &r) == CALC_OK); CHECK_DBL(r, 1);
    CHECK(calc_eval("log(1000)", CALC_MODE_RAD, &r) == CALC_OK); CHECK_DBL(r, 3);

    // Errores de dominio / función desconocida
    CHECK(err("sqrt(-1)") == CALC_ERR_DOMAIN);
    CHECK(err("ln(0)")    == CALC_ERR_DOMAIN);
    CHECK(err("asin(2)")  == CALC_ERR_DOMAIN);
    CHECK(err("foo(1)")   == CALC_ERR_UNKNOWN_FUNC);
    CHECK(err("bar")      == CALC_ERR_UNKNOWN_FUNC);
```

- [ ] **Step 2: Ejecutar y verificar que falla**

Run: `meson compile -C build && meson test -C build eval`
Expected: FAIL (constantes/funciones aún no implementadas → SYNTAX).

- [ ] **Step 3: Implementar constantes y funciones en `src/parser.c`**

Añade estos helpers justo antes de `parse_primary`:

```c
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
```

Reemplaza el comentario `// TOK_IDENT ...` en `parse_primary` por:

```c
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
```

Añade la declaración adelantada de `apply_func` junto a las otras (arriba del archivo):

```c
static double apply_func(Parser *p, const char *name, double arg);
```

- [ ] **Step 4: Ejecutar y verificar que pasa**

Run: `meson compile -C build && meson test -C build eval`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/parser.c tests/test_eval.c
git commit -m "feat: funciones científicas, constantes y modo DEG/RAD"
```

---

## Task 5: Registro de memoria

**Files:**
- Create: `src/memory_reg.h`
- Create: `src/memory_reg.c`
- Create: `tests/test_memory.c`
- Modify: `meson.build` (añadir fuente)
- Modify: `tests/meson.build`

- [ ] **Step 1: Crear `src/memory_reg.h`**

```c
#ifndef MEMORY_REG_H
#define MEMORY_REG_H

#include <stdbool.h>

typedef struct {
    double value;
    bool has_value;
} MemoryReg;

void memreg_init(MemoryReg *m);
void memreg_clear(MemoryReg *m);            // MC
double memreg_recall(const MemoryReg *m);   // MR
void memreg_add(MemoryReg *m, double x);    // M+
void memreg_sub(MemoryReg *m, double x);    // M-
bool memreg_has_value(const MemoryReg *m);

#endif
```

- [ ] **Step 2: Escribir el test que falla — `tests/test_memory.c`**

```c
#include "memory_reg.h"
#include "test_util.h"

int main(void) {
    MemoryReg m;
    memreg_init(&m);
    CHECK(!memreg_has_value(&m));
    CHECK_DBL(memreg_recall(&m), 0);

    memreg_add(&m, 5);
    CHECK(memreg_has_value(&m));
    CHECK_DBL(memreg_recall(&m), 5);

    memreg_sub(&m, 2);
    CHECK_DBL(memreg_recall(&m), 3);

    memreg_clear(&m);
    CHECK(!memreg_has_value(&m));
    CHECK_DBL(memreg_recall(&m), 0);

    // tras operar a cero, sigue "en uso"
    memreg_add(&m, 4);
    memreg_sub(&m, 4);
    CHECK(memreg_has_value(&m));
    CHECK_DBL(memreg_recall(&m), 0);

    TEST_REPORT();
}
```

- [ ] **Step 3: Añadir fuente y test al build**

En `meson.build`, actualiza la lista de fuentes de `engine_lib`:

```meson
engine_lib = static_library('calcengine',
  ['src/lexer.c', 'src/parser.c', 'src/evaluator.c', 'src/memory_reg.c'],
  include_directories: engine_inc,
  dependencies: m_dep)
```

En `tests/meson.build` (al final):

```meson
test_memory = executable('test_memory', 'test_memory.c',
  link_with: engine_lib,
  include_directories: engine_inc,
  dependencies: m_dep)
test('memory', test_memory)
```

- [ ] **Step 4: Ejecutar y verificar que falla**

Run: `meson compile -C build && meson test -C build memory`
Expected: FAIL (link error: símbolos no definidos).

- [ ] **Step 5: Implementar `src/memory_reg.c`**

```c
#include "memory_reg.h"

void memreg_init(MemoryReg *m)  { m->value = 0.0; m->has_value = false; }
void memreg_clear(MemoryReg *m) { m->value = 0.0; m->has_value = false; }
double memreg_recall(const MemoryReg *m) { return m->value; }
void memreg_add(MemoryReg *m, double x) { m->value += x; m->has_value = true; }
void memreg_sub(MemoryReg *m, double x) { m->value -= x; m->has_value = true; }
bool memreg_has_value(const MemoryReg *m) { return m->has_value; }
```

- [ ] **Step 6: Ejecutar y verificar que pasa**

Run: `meson compile -C build && meson test -C build memory`
Expected: PASS.

- [ ] **Step 7: Commit**

```bash
git add src/memory_reg.h src/memory_reg.c tests/test_memory.c meson.build tests/meson.build
git commit -m "feat: registro de memoria (MC/MR/M+/M-)"
```

---

## Task 6: Historial

**Files:**
- Create: `src/history.h`
- Create: `src/history.c`
- Create: `tests/test_history.c`
- Modify: `meson.build`
- Modify: `tests/meson.build`

- [ ] **Step 1: Crear `src/history.h`**

```c
#ifndef HISTORY_H
#define HISTORY_H

#include <stddef.h>

#define HISTORY_MAX 100
#define HISTORY_EXPR_LEN 256

typedef struct {
    char expr[HISTORY_EXPR_LEN];
    double result;
} HistoryEntry;

typedef struct {
    HistoryEntry entries[HISTORY_MAX];
    size_t count;   // entradas válidas; index 0 = más antigua
} History;

void history_init(History *h);
void history_add(History *h, const char *expr, double result);
size_t history_count(const History *h);
const HistoryEntry *history_get(const History *h, size_t index);
void history_clear(History *h);

#endif
```

- [ ] **Step 2: Escribir el test que falla — `tests/test_history.c`**

```c
#include "history.h"
#include "test_util.h"
#include <string.h>
#include <stdio.h>

int main(void) {
    History h;
    history_init(&h);
    CHECK(history_count(&h) == 0);
    CHECK(history_get(&h, 0) == NULL);

    history_add(&h, "2+2", 4);
    history_add(&h, "3*3", 9);
    CHECK(history_count(&h) == 2);
    CHECK(strcmp(history_get(&h, 0)->expr, "2+2") == 0);
    CHECK_DBL(history_get(&h, 0)->result, 4);
    CHECK(strcmp(history_get(&h, 1)->expr, "3*3") == 0);
    CHECK_DBL(history_get(&h, 1)->result, 9);

    history_clear(&h);
    CHECK(history_count(&h) == 0);

    // límite: al superar HISTORY_MAX se descarta la más antigua
    for (int i = 0; i < HISTORY_MAX + 5; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", i);
        history_add(&h, buf, i);
    }
    CHECK(history_count(&h) == HISTORY_MAX);
    // la más antigua ahora debe ser "5" (se descartaron 0..4)
    CHECK(strcmp(history_get(&h, 0)->expr, "5") == 0);

    TEST_REPORT();
}
```

- [ ] **Step 3: Añadir fuente y test al build**

En `meson.build`, lista final de `engine_lib`:

```meson
engine_lib = static_library('calcengine',
  ['src/lexer.c', 'src/parser.c', 'src/evaluator.c',
   'src/memory_reg.c', 'src/history.c'],
  include_directories: engine_inc,
  dependencies: m_dep)
```

En `tests/meson.build` (al final):

```meson
test_history = executable('test_history', 'test_history.c',
  link_with: engine_lib,
  include_directories: engine_inc,
  dependencies: m_dep)
test('history', test_history)
```

- [ ] **Step 4: Ejecutar y verificar que falla**

Run: `meson compile -C build && meson test -C build history`
Expected: FAIL (link error).

- [ ] **Step 5: Implementar `src/history.c`**

```c
#include "history.h"
#include <string.h>

void history_init(History *h) { h->count = 0; }

void history_clear(History *h) { h->count = 0; }

size_t history_count(const History *h) { return h->count; }

const HistoryEntry *history_get(const History *h, size_t index) {
    if (index >= h->count) return NULL;
    return &h->entries[index];
}

void history_add(History *h, const char *expr, double result) {
    if (h->count == HISTORY_MAX) {
        for (size_t i = 1; i < HISTORY_MAX; i++)
            h->entries[i - 1] = h->entries[i];
        h->count--;
    }
    HistoryEntry *e = &h->entries[h->count];
    strncpy(e->expr, expr, HISTORY_EXPR_LEN - 1);
    e->expr[HISTORY_EXPR_LEN - 1] = '\0';
    e->result = result;
    h->count++;
}
```

- [ ] **Step 6: Ejecutar y verificar que pasa**

Run: `meson compile -C build && meson test -C build history`
Expected: PASS.

- [ ] **Step 7: Ejecutar TODOS los tests del motor**

Run: `meson test -C build`
Expected: 4/4 PASS (lexer, eval, memory, history).

- [ ] **Step 8: Commit**

```bash
git add src/history.h src/history.c tests/test_history.c meson.build tests/meson.build
git commit -m "feat: historial de cálculos"
```

---

## Task 7: GUI GTK4

Construye la ventana, la barra de expresión editable, la grilla de botones, el
indicador de memoria, el toggle DEG/RAD y el panel de historial. Esta tarea se
valida **manualmente** ejecutando la app.

**Files:**
- Create: `src/calc_window.h`
- Create: `src/calc_window.c`
- Create: `src/main.c`
- Modify: `meson.build` (añadir ejecutable GTK)

- [ ] **Step 1: Crear `src/calc_window.h`**

```c
#ifndef CALC_WINDOW_H
#define CALC_WINDOW_H

#include <gtk/gtk.h>

GtkWidget *calc_window_new(GtkApplication *app);

#endif
```

- [ ] **Step 2: Crear `src/calc_window.c`**

```c
#include "calc_window.h"
#include "evaluator.h"
#include "memory_reg.h"
#include "history.h"
#include <stdio.h>
#include <string.h>

typedef enum {
    BTN_INSERT, BTN_EQUALS, BTN_CLEAR, BTN_BACK,
    BTN_MC, BTN_MR, BTN_MPLUS, BTN_MMINUS
} BtnKind;

typedef struct { const char *label; const char *insert; BtnKind kind; int row; int col; } Btn;

static const Btn BUTTONS[] = {
    {"sin","sin(",BTN_INSERT,0,0}, {"cos","cos(",BTN_INSERT,0,1}, {"tan","tan(",BTN_INSERT,0,2}, {"^","^",BTN_INSERT,0,3}, {"sqrt","sqrt(",BTN_INSERT,0,4},
    {"asin","asin(",BTN_INSERT,1,0}, {"acos","acos(",BTN_INSERT,1,1}, {"atan","atan(",BTN_INSERT,1,2}, {"ln","ln(",BTN_INSERT,1,3}, {"log","log(",BTN_INSERT,1,4},
    {"pi","pi",BTN_INSERT,2,0}, {"e","e",BTN_INSERT,2,1}, {"exp","exp(",BTN_INSERT,2,2}, {"abs","abs(",BTN_INSERT,2,3}, {"%","%",BTN_INSERT,2,4},
    {"(","(",BTN_INSERT,3,0}, {")",")",BTN_INSERT,3,1}, {"MC",NULL,BTN_MC,3,2}, {"MR",NULL,BTN_MR,3,3}, {"M+",NULL,BTN_MPLUS,3,4},
    {"7","7",BTN_INSERT,4,0}, {"8","8",BTN_INSERT,4,1}, {"9","9",BTN_INSERT,4,2}, {"/","/",BTN_INSERT,4,3}, {"M-",NULL,BTN_MMINUS,4,4},
    {"4","4",BTN_INSERT,5,0}, {"5","5",BTN_INSERT,5,1}, {"6","6",BTN_INSERT,5,2}, {"*","*",BTN_INSERT,5,3}, {"C",NULL,BTN_CLEAR,5,4},
    {"1","1",BTN_INSERT,6,0}, {"2","2",BTN_INSERT,6,1}, {"3","3",BTN_INSERT,6,2}, {"-","-",BTN_INSERT,6,3}, {"⌫",NULL,BTN_BACK,6,4},
    {"0","0",BTN_INSERT,7,0}, {".",".",BTN_INSERT,7,1}, {"+","+",BTN_INSERT,7,2}, {"=",NULL,BTN_EQUALS,7,3},
};

typedef struct {
    GtkWidget *window;
    GtkWidget *entry;
    GtkWidget *result_label;
    GtkWidget *mem_label;
    GtkWidget *deg_toggle;
    GtkWidget *history_list;
    GtkWidget *history_revealer;
    MemoryReg mem;
    History hist;
    double last_result;
    gboolean has_last_result;
} CalcApp;

static void insert_text(CalcApp *c, const char *text) {
    int pos = gtk_editable_get_position(GTK_EDITABLE(c->entry));
    gtk_editable_insert_text(GTK_EDITABLE(c->entry), text, -1, &pos);
    gtk_editable_set_position(GTK_EDITABLE(c->entry), pos);
    gtk_widget_grab_focus(c->entry);
    gtk_editable_set_position(GTK_EDITABLE(c->entry), pos);
}

static void update_mem_label(CalcApp *c) {
    gtk_label_set_text(GTK_LABEL(c->mem_label),
                       memreg_has_value(&c->mem) ? "M" : "");
}

static const char *err_message(CalcStatus s) {
    switch (s) {
        case CALC_ERR_DIV_ZERO:     return "Error: división por cero";
        case CALC_ERR_PAREN:        return "Error: paréntesis";
        case CALC_ERR_UNKNOWN_FUNC: return "Error: función desconocida";
        case CALC_ERR_DOMAIN:       return "Error: dominio";
        case CALC_ERR_OVERFLOW:     return "Error: resultado no finito";
        default:                    return "Error de sintaxis";
    }
}

static void history_add_row(CalcApp *c, const char *expr, double result) {
    char text[320];
    snprintf(text, sizeof(text), "%s = %g", expr, result);
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *lbl = gtk_label_new(text);
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), lbl);
    g_object_set_data_full(G_OBJECT(row), "expr", g_strdup(expr), g_free);
    gtk_list_box_insert(GTK_LIST_BOX(c->history_list), row, 0);
}

static void do_evaluate(CalcApp *c) {
    const char *expr = gtk_editable_get_text(GTK_EDITABLE(c->entry));
    if (expr == NULL || expr[0] == '\0') return;
    CalcAngleMode mode =
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->deg_toggle))
            ? CALC_MODE_RAD : CALC_MODE_DEG;
    double result;
    CalcStatus s = calc_eval(expr, mode, &result);
    if (s != CALC_OK) {
        gtk_label_set_text(GTK_LABEL(c->result_label), err_message(s));
        return;
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "%g", result);
    gtk_label_set_text(GTK_LABEL(c->result_label), buf);
    c->last_result = result;
    c->has_last_result = TRUE;
    history_add(&c->hist, expr, result);
    history_add_row(c, expr, result);
}

static void on_button_clicked(GtkButton *btn, gpointer user_data) {
    CalcApp *c = user_data;
    BtnKind kind = (BtnKind)GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "kind"));
    const char *insert = g_object_get_data(G_OBJECT(btn), "insert");
    switch (kind) {
        case BTN_INSERT: insert_text(c, insert); break;
        case BTN_EQUALS: do_evaluate(c); break;
        case BTN_CLEAR:
            gtk_editable_set_text(GTK_EDITABLE(c->entry), "");
            gtk_label_set_text(GTK_LABEL(c->result_label), "");
            break;
        case BTN_BACK: {
            int pos = gtk_editable_get_position(GTK_EDITABLE(c->entry));
            if (pos > 0)
                gtk_editable_delete_text(GTK_EDITABLE(c->entry), pos - 1, pos);
            break;
        }
        case BTN_MC: memreg_clear(&c->mem); update_mem_label(c); break;
        case BTN_MR:
            if (memreg_has_value(&c->mem)) {
                char buf[64];
                snprintf(buf, sizeof(buf), "%g", memreg_recall(&c->mem));
                insert_text(c, buf);
            }
            break;
        case BTN_MPLUS:
            if (c->has_last_result) { memreg_add(&c->mem, c->last_result); update_mem_label(c); }
            break;
        case BTN_MMINUS:
            if (c->has_last_result) { memreg_sub(&c->mem, c->last_result); update_mem_label(c); }
            break;
    }
}

static void on_entry_activate(GtkEntry *entry, gpointer user_data) {
    (void)entry;
    do_evaluate(user_data);
}

static void on_deg_toggled(GtkToggleButton *t, gpointer user_data) {
    (void)user_data;
    gtk_button_set_label(GTK_BUTTON(t),
        gtk_toggle_button_get_active(t) ? "RAD" : "DEG");
}

static void on_hist_toggled(GtkToggleButton *t, gpointer user_data) {
    CalcApp *c = user_data;
    gtk_revealer_set_reveal_child(GTK_REVEALER(c->history_revealer),
                                  gtk_toggle_button_get_active(t));
}

static void on_history_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    (void)box;
    CalcApp *c = user_data;
    const char *expr = g_object_get_data(G_OBJECT(row), "expr");
    if (expr) {
        gtk_editable_set_text(GTK_EDITABLE(c->entry), expr);
        gtk_editable_set_position(GTK_EDITABLE(c->entry), -1);
        gtk_widget_grab_focus(c->entry);
    }
}

GtkWidget *calc_window_new(GtkApplication *app) {
    CalcApp *c = g_new0(CalcApp, 1);
    memreg_init(&c->mem);
    history_init(&c->hist);
    c->has_last_result = FALSE;

    c->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(c->window), "Calc");
    gtk_window_set_default_size(GTK_WINDOW(c->window), 440, 600);
    g_object_set_data_full(G_OBJECT(c->window), "calcapp", c, g_free);

    GtkWidget *outer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_top(outer, 8);
    gtk_widget_set_margin_bottom(outer, 8);
    gtk_widget_set_margin_start(outer, 8);
    gtk_widget_set_margin_end(outer, 8);
    gtk_window_set_child(GTK_WINDOW(c->window), outer);

    GtkWidget *col = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_hexpand(col, TRUE);
    gtk_box_append(GTK_BOX(outer), col);

    // Barra superior: entrada + toggle de historial
    GtkWidget *topbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    c->entry = gtk_entry_new();
    gtk_widget_set_hexpand(c->entry, TRUE);
    g_signal_connect(c->entry, "activate", G_CALLBACK(on_entry_activate), c);
    gtk_box_append(GTK_BOX(topbar), c->entry);
    GtkWidget *hist_toggle = gtk_toggle_button_new_with_label("☰");
    g_signal_connect(hist_toggle, "toggled", G_CALLBACK(on_hist_toggled), c);
    gtk_box_append(GTK_BOX(topbar), hist_toggle);
    gtk_box_append(GTK_BOX(col), topbar);

    // Resultado + indicador de memoria
    GtkWidget *resbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    c->mem_label = gtk_label_new("");
    c->result_label = gtk_label_new("");
    gtk_widget_set_halign(c->result_label, GTK_ALIGN_END);
    gtk_widget_set_hexpand(c->result_label, TRUE);
    gtk_box_append(GTK_BOX(resbar), c->mem_label);
    gtk_box_append(GTK_BOX(resbar), c->result_label);
    gtk_box_append(GTK_BOX(col), resbar);

    // Toggle DEG/RAD (inactivo = DEG)
    c->deg_toggle = gtk_toggle_button_new_with_label("DEG");
    gtk_widget_set_halign(c->deg_toggle, GTK_ALIGN_START);
    g_signal_connect(c->deg_toggle, "toggled", G_CALLBACK(on_deg_toggled), NULL);
    gtk_box_append(GTK_BOX(col), c->deg_toggle);

    // Grilla de botones
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 4);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 4);
    gtk_widget_set_vexpand(grid, TRUE);
    for (gsize i = 0; i < G_N_ELEMENTS(BUTTONS); i++) {
        const Btn *b = &BUTTONS[i];
        GtkWidget *btn = gtk_button_new_with_label(b->label);
        gtk_widget_set_hexpand(btn, TRUE);
        gtk_widget_set_vexpand(btn, TRUE);
        g_object_set_data(G_OBJECT(btn), "kind", GINT_TO_POINTER(b->kind));
        g_object_set_data(G_OBJECT(btn), "insert", (gpointer)b->insert);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_button_clicked), c);
        gtk_grid_attach(GTK_GRID(grid), btn, b->col, b->row, 1, 1);
    }
    gtk_box_append(GTK_BOX(col), grid);

    // Panel de historial (oculto por defecto)
    c->history_revealer = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(c->history_revealer),
                                     GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT);
    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scrolled, 200, -1);
    c->history_list = gtk_list_box_new();
    g_signal_connect(c->history_list, "row-activated",
                     G_CALLBACK(on_history_row_activated), c);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), c->history_list);
    gtk_revealer_set_child(GTK_REVEALER(c->history_revealer), scrolled);
    gtk_box_append(GTK_BOX(outer), c->history_revealer);

    return c->window;
}
```

- [ ] **Step 3: Crear `src/main.c`**

```c
#include <gtk/gtk.h>
#include "calc_window.h"

static void on_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    GtkWidget *win = calc_window_new(app);
    gtk_window_present(GTK_WINDOW(win));
}

int main(int argc, char **argv) {
    GtkApplication *app =
        gtk_application_new("io.github.sebastian.Calc", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
```

- [ ] **Step 4: Añadir el ejecutable GTK a `meson.build`** (al final del archivo)

```meson
gtk_dep = dependency('gtk4')

executable('mycalc',
  ['src/main.c', 'src/calc_window.c'],
  include_directories: engine_inc,
  link_with: engine_lib,
  dependencies: [gtk_dep, m_dep],
  install: true)
```

- [ ] **Step 5: Compilar**

Run: `meson setup --reconfigure build && meson compile -C build`
Expected: compila `build/mycalc` sin errores.

- [ ] **Step 6: Verificación manual**

Run: `./build/mycalc`
Comprobar manualmente:
- Escribir `2+3*4`, Enter → resultado `14`.
- Botones: pulsar `sin` `9` `0` `)` `=` en modo DEG → `1`.
- Activar el toggle a `RAD`, escribir `sin(pi/2)`, `=` → `1`.
- `1`/`0`/`=` (división por cero) → muestra "Error: división por cero" sin cerrar.
- `M+` tras un resultado enciende la "M"; `MR` inserta el valor; `MC` apaga la "M".
- `⌫` borra el último carácter; `C` limpia todo.
- Toggle `☰` muestra el panel de historial; clic en una entrada la reinserta.
Cerrar la ventana sin crashes.

- [ ] **Step 7: Commit**

```bash
git add src/calc_window.h src/calc_window.c src/main.c meson.build
git commit -m "feat: interfaz GTK4 (entrada, botones, memoria, historial)"
```

---

## Task 8: Empaquetado — build.sh e integración de escritorio

**Files:**
- Create: `build.sh`
- Create: `data/io.github.sebastian.Calc.desktop`
- Modify: `meson.build` (instalar el `.desktop`)

- [ ] **Step 1: Crear `data/io.github.sebastian.Calc.desktop`**

```ini
[Desktop Entry]
Type=Application
Name=Calc
Comment=Calculadora científica
Exec=mycalc
Icon=accessories-calculator
Terminal=false
Categories=Utility;Calculator;
```

- [ ] **Step 2: Instalar el `.desktop` desde `meson.build`** (añadir al final)

```meson
install_data('data/io.github.sebastian.Calc.desktop',
  install_dir: get_option('datadir') / 'applications')
```

- [ ] **Step 3: Crear `build.sh`**

```bash
#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
INSTALL=0

for arg in "$@"; do
    case "$arg" in
        --install) INSTALL=1 ;;
        -h|--help) echo "Uso: ./build.sh [--install]"; exit 0 ;;
        *) echo "Opción desconocida: $arg" >&2; exit 1 ;;
    esac
done

missing=0
for tool in meson ninja pkg-config cc; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "Falta la herramienta: $tool" >&2
        missing=1
    fi
done
if ! pkg-config --exists gtk4; then
    echo "Falta la librería de desarrollo gtk4 (pkg-config gtk4)" >&2
    missing=1
fi
if [ "$missing" -ne 0 ]; then
    echo "Instala las dependencias faltantes y vuelve a intentar." >&2
    echo "Arch:   sudo pacman -S --needed base-devel meson ninja gtk4 pkgconf" >&2
    echo "Fedora: sudo dnf install meson ninja-build gtk4-devel gcc pkgconf" >&2
    echo "Debian: sudo apt install meson ninja-build libgtk-4-dev build-essential pkg-config" >&2
    exit 1
fi

if [ ! -d "$BUILD_DIR" ]; then
    meson setup "$BUILD_DIR"
fi
meson compile -C "$BUILD_DIR"

if [ "$INSTALL" -eq 1 ]; then
    sudo meson install -C "$BUILD_DIR"
    echo "Instalado. Ejecuta: mycalc"
else
    echo "Compilado. Binario en $BUILD_DIR/mycalc (instala con: ./build.sh --install)"
fi
```

- [ ] **Step 4: Hacer ejecutable el script**

Run: `chmod +x build.sh`

- [ ] **Step 5: Verificar el flujo completo desde limpio**

Run: `rm -rf build && ./build.sh`
Expected: comprueba dependencias, configura, compila y reporta `build/mycalc`.

- [ ] **Step 6: Verificar la instalación en un prefijo de prueba (sin sudo)**

Run: `meson setup --reconfigure --prefix "$PWD/_install_test" build && meson install -C build`
Expected: instala `mycalc` en `_install_test/bin/` y el `.desktop` en `_install_test/share/applications/`. Luego limpiar: `rm -rf _install_test`.

- [ ] **Step 7: Verificar la suite completa una última vez**

Run: `meson test -C build`
Expected: 4/4 PASS.

- [ ] **Step 8: Commit**

```bash
git add build.sh data/io.github.sebastian.Calc.desktop meson.build
git commit -m "build: script build.sh e integración de escritorio (.desktop)"
```

---

## Verificación final del plan contra el spec

- **C + GTK4, multi-distro, Wayland/X11** → Tasks 7-8 (GtkApplication, `.desktop`, `dependency('gtk4')`).
- **Calculadora completa (científica + historial + memoria)** → Tasks 4 (funciones), 6 (historial), 5 (memoria).
- **Expresión editable + botones** → Task 7 (GtkEntry editable + grilla de botones que insertan en el cursor + Enter evalúa).
- **Motor de descenso recursivo (enfoque A), sin dependencias** → Tasks 2-4.
- **Modo DEG/RAD** → Task 4 (motor) + Task 7 (toggle).
- **Manejo de errores sin crashear** → Tasks 3-4 (códigos de error) + Task 7 (`err_message`).
- **Memoria M+/M-/MR/MC + indicador** → Task 5 + Task 7.
- **Testing del motor sin GUI con meson test** → Tasks 2-6 (test_lexer, test_eval, test_memory, test_history).
- **Build con Meson + script `.sh`, sin repos oficiales** → Tasks 1 y 8.

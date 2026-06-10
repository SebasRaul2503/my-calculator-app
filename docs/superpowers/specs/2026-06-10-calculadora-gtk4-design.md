# Diseño: Calculadora científica en C + GTK4

- **Fecha:** 2026-06-10
- **Autor:** Sebastian Castillo
- **Estado:** Aprobado para planificación

## Objetivo

Una aplicación de calculadora **científica** de escritorio, escrita en **C** con
**GTK4**, pensada para máximo rendimiento y control de memoria. Debe funcionar en
Arch Linux (distro del autor) y en cualquier otra distro, bajo Wayland o X11, e
integrarse con cualquier entorno gráfico (GNOME, KDE, etc.). Se distribuye desde
el repositorio público propio mediante un script `.sh`, sin releases a repos
oficiales (AUR, Flatpak, etc.).

## Alcance

Calculadora **completa**:

- Operaciones básicas: `+`, `-`, `*`, `/`, `%` (módulo/porcentaje), signo.
- Científicas: funciones trigonométricas, logaritmos, potencias, raíces.
- **Historial** de cálculos recientes.
- **Memoria** (M+, M-, MR, MC).
- **Entrada por expresión editable + botones**: el usuario escribe/edita la
  expresión completa (p. ej. `2+3*sin(45)`) y/o usa los botones.

Fuera de alcance (YAGNI): conversión de unidades, modo programador (hex/bin),
graficado, releases a repos oficiales, Flatpak, PKGBUILD.

## Convenciones del proyecto

- **Nombre de la app:** Calc (provisional — ajustable).
- **Binario:** `mycalc`.
- **Application ID (GTK):** `io.github.sebastian.Calc`.
- **Lenguaje:** C11.
- **Estilo:** separación estricta entre motor (lógica pura) y GUI.

## Arquitectura y módulos

Separación estricta entre el **motor** (lógica pura, sin ninguna dependencia de
GTK) y la **GUI**, para que el motor sea testeable de forma aislada.

```
my-calculator-app/
├── src/
│   ├── main.c            # arranque GTK, GtkApplication
│   ├── calc_window.c/.h  # ventana, botones, layout, eventos
│   ├── lexer.c/.h        # tokenizador
│   ├── parser.c/.h       # descenso recursivo → evaluación
│   ├── evaluator.c/.h    # API pública del motor
│   ├── history.c/.h      # lista de cálculos recientes
│   └── memory_reg.c/.h   # registro de memoria (M+, M-, MR, MC)
├── tests/                # tests del motor, sin GUI
├── data/                 # .desktop, icono, UI (.ui) como GResource
├── meson.build
├── build.sh              # compila + instala
└── docs/
```

El motor (`lexer` + `parser` + `evaluator`) no contiene ni una línea de GTK. La
GUI llama al motor y recibe un resultado o un código de error.

## Motor de cálculo (parser de descenso recursivo)

### Pipeline

1. **Lexer** (`lexer.c`): convierte la cadena en una secuencia de tokens:
   números, operadores, paréntesis, nombres de función, constantes.
2. **Parser de descenso recursivo** (`parser.c`): consume tokens y evalúa
   respetando precedencia y asociatividad.
3. **Evaluator** (`evaluator.c`): API pública que orquesta lexer + parser y
   expone el resultado/errores a la GUI.

### Gramática (de menor a mayor precedencia)

```
expr    → term  (('+' | '-') term)*
term    → factor (('*' | '/' | '%') factor)*
factor  → unary  ('^' unary)*           # potencia, asociativa a la derecha
unary   → ('-' | '+') unary | primary
primary → NUMBER
        | CONST                          # pi, e
        | FUNC '(' expr ')'              # sin(expr), etc.
        | '(' expr ')'
```

### Funciones y constantes soportadas

- **Funciones:** `sin cos tan asin acos atan log ln sqrt exp abs`.
- **Potencia:** operador `^`. **Raíz cuadrada:** `sqrt`.
- **Constantes:** `pi`, `e`.

### Modo ángulo

Toggle **DEG/RAD**. Las funciones trigonométricas (`sin/cos/tan/asin/acos/atan`)
respetan el modo activo. Por defecto: **DEG**.

### Tipo numérico

`double` (IEEE 754 de doble precisión).

### Manejo de errores (sin crashear nunca)

La API devuelve un código de estado; la GUI muestra un mensaje y nunca aborta.

```c
typedef enum {
    CALC_OK,
    CALC_ERR_SYNTAX,        // expresión malformada
    CALC_ERR_DIV_ZERO,      // división o módulo por cero
    CALC_ERR_PAREN,         // paréntesis desbalanceados
    CALC_ERR_UNKNOWN_FUNC,  // función o constante desconocida
    CALC_ERR_DOMAIN,        // dominio inválido (p. ej. sqrt(-1), ln(0))
    CALC_ERR_OVERFLOW       // resultado no finito (inf/nan)
} CalcStatus;

typedef enum { CALC_MODE_DEG, CALC_MODE_RAD } CalcAngleMode;

// Evalúa `expr`. En éxito escribe en *out_result y devuelve CALC_OK.
// En error devuelve el código correspondiente y no toca *out_result.
CalcStatus calc_eval(const char *expr, CalcAngleMode mode, double *out_result);
```

Reglas de memoria: el parser usa la pila de C (descenso recursivo), sin estado
global mutable. Cualquier asignación dinámica en el lexer se libera antes de
retornar; objetivo: cero fugas (verificable con valgrind/ASan).

## Registro de memoria

Módulo `memory_reg` con un único valor `double` acumulado y operaciones:

- **MC** — borra el registro (a 0).
- **MR** — recupera el valor (lo inserta en la barra de expresión).
- **M+** — suma el resultado actual al registro.
- **M-** — resta el resultado actual al registro.

Indicador visual (p. ej. una "M") cuando el registro es distinto de cero.

## Historial

Módulo `history`: lista de entradas `{ expresión, resultado }` de cálculos
recientes (en memoria, durante la sesión). La GUI muestra un panel lateral
conmutable; al hacer clic en una entrada, su expresión se reinserta en la barra.
Tamaño máximo acotado (p. ej. últimas 100 entradas) para no crecer sin límite.

## Interfaz (GTK4)

- **Barra de expresión editable** (`GtkEntry`/`GtkText`) en la parte superior:
  el usuario escribe y edita la expresión completa con el teclado.
- **Etiqueta de resultado** debajo de la barra.
- **Grilla de botones**: dígitos, `.`, operadores (`+ - * / % ^`), paréntesis,
  funciones científicas, constantes (`pi`, `e`), toggle **DEG/RAD**, memoria
  (M+, M-, MR, MC), `=`, `C` (limpiar todo), `⌫` (borrar último).
- **Teclado totalmente funcional:** Enter evalúa; los botones insertan su texto
  en la barra en la posición del cursor.
- **Panel de historial** lateral conmutable.
- La UI se declara en un archivo `.ui` (GtkBuilder) compilado como **GResource**,
  de modo que el binario resultante es autocontenido (no busca archivos `.ui` en
  disco en tiempo de ejecución).
- **Wayland y X11**: gestionados automáticamente por GTK4. Integración de
  escritorio vía archivo `.desktop` + icono instalados en rutas estándar.

## Build y distribución

- **Meson** + **Ninja** como sistema de build (estándar en proyectos GTK).
- **`build.sh`** en la raíz del repo:
  - Comprueba dependencias presentes: `gtk4` (pkg-config `gtk4`), `meson`,
    `ninja`, compilador C. Si falta algo, lo informa con instrucciones.
  - Configura y compila en `build/`.
  - Con la opción `--install`, ejecuta `meson install` (instala binario `mycalc`,
    `.desktop` e icono en el prefijo estándar, por defecto `/usr/local`).
  - Pensado para: clonar el repo público y ejecutar `./build.sh` (compilar) o
    `./build.sh --install` (compilar e instalar).
- Sin Flatpak ni PKGBUILD.

## Testing

- Tests del **motor** en `tests/`, integrados con `meson test`. Sin GUI.
- Cobertura mínima de casos:
  - Precedencia y asociatividad (`2+3*4`, `2^3^2`, `-2^2`).
  - Paréntesis (anidados, desbalanceados → `CALC_ERR_PAREN`).
  - Funciones y constantes (`sin(pi/2)` en RAD, `sin(90)` en DEG, `ln(e)`).
  - Modo DEG vs RAD.
  - Todos los errores: división por cero, sintaxis inválida, función
    desconocida, dominio inválido, overflow/no finito.
- La GUI se valida manualmente ejecutando la app (lanzar, teclear, botones,
  historial, memoria).

## Decisiones tomadas y razones

- **C + GTK4**: rendimiento y control de memoria; GTK es una librería en C,
  nativa de Wayland, presente en casi todas las distros, integrable con cualquier
  entorno sin atar a GNOME.
- **Parser de descenso recursivo propio (enfoque A)**: sin dependencias extra,
  control total de memoria/errores, módulo aislado y testeable. Se descartó
  shunting-yard (menos legible para funciones) y librerías externas (dependencia
  + menos control).
- **Script `.sh` + Meson**: distribución simple desde el repo propio, sin
  empaquetado para repos oficiales.

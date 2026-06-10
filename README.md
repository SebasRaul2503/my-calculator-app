# Calc — Calculadora científica en C + GTK4

Calculadora científica de escritorio para Linux, escrita en **C (C11)** con **GTK4**.
Pensada para máximo rendimiento y control de memoria, funciona en cualquier
distribución y bajo **Wayland o X11**, integrándose con cualquier entorno gráfico
(GNOME, KDE, etc.) sin atarse a ninguno.

El motor de cálculo es **C puro, sin dependencias de GTK**, construido como una
librería estática independiente y cubierto por tests; la interfaz GTK4 enlaza
contra él.

## Características

- **Entrada por expresión editable + botones**: escribe y edita la expresión
  completa con el teclado (p. ej. `2+3*sin(45)`) y/o usa la grilla de botones.
- **Científica**: funciones trigonométricas, inversas, logaritmos, exponencial,
  raíz, potencia y valor absoluto.
- **Modo de ángulo DEG/RAD** conmutable (las trigonométricas lo respetan).
- **Memoria**: `M+`, `M-`, `MR`, `MC`, con indicador "M" cuando está en uso.
- **Historial** de cálculos: panel lateral conmutable; clic en una entrada la
  reinserta en la barra de expresión.
- **Manejo de errores robusto**: división por cero, paréntesis desbalanceados,
  función desconocida, dominio inválido y desbordamiento se muestran como un
  mensaje, nunca cierran la app.
- **Teclado totalmente funcional**: Enter evalúa; los botones insertan texto en
  la posición del cursor.

## Dependencias

- `gcc` (o cualquier compilador C11)
- `meson` y `ninja`
- `gtk4` (librerías de desarrollo)
- `pkg-config`

Instalación de dependencias según distribución:

```bash
# Arch
sudo pacman -S --needed base-devel meson ninja gtk4 pkgconf

# Fedora
sudo dnf install meson ninja-build gtk4-devel gcc pkgconf

# Debian / Ubuntu
sudo apt install meson ninja-build libgtk-4-dev build-essential pkg-config
```

## Compilar e instalar

El repositorio incluye un script `build.sh` que comprueba dependencias, configura
y compila:

```bash
./build.sh            # compila → binario en build/mycalc
./build.sh --install  # compila e instala (pide sudo) → ejecutable: mycalc
./build.sh --help     # ayuda
```

`--install` instala el binario `mycalc`, el archivo `.desktop` y registra la
entrada de menú en las rutas estándar (por defecto `/usr/local`).

### Alternativa: Meson directo

```bash
meson setup build
meson compile -C build
./build/mycalc                 # ejecutar sin instalar
sudo meson install -C build    # instalar
```

## Uso

Escribe una expresión en la barra superior y pulsa **Enter** (o el botón `=`).

### Sintaxis de expresiones

- **Operadores**: `+`  `-`  `*`  `/`  `%` (módulo)  `^` (potencia, asociativa a la
  derecha)
- **Paréntesis**: `( )`
- **Constantes**: `pi`, `e`
- **Funciones**: `sin` `cos` `tan` `asin` `acos` `atan` `ln` `log` `sqrt` `exp`
  `abs`
  - `log` es base 10; `ln` es logaritmo natural.
  - Las trigonométricas usan el modo **DEG/RAD** activo; las inversas devuelven el
    resultado en ese mismo modo.

Ejemplos:

| Expresión        | Modo | Resultado |
|------------------|------|-----------|
| `2+3*4`          | —    | `14`      |
| `(2+3)*4`        | —    | `20`      |
| `-2^2`           | —    | `-4`      |
| `2^3^2`          | —    | `512`     |
| `sin(90)`        | DEG  | `1`       |
| `sin(pi/2)`      | RAD  | `1`       |
| `ln(e)`          | —    | `1`       |
| `sqrt(9)`        | —    | `3`       |

### Memoria

- `M+` / `M-`: suma/resta el último resultado al registro de memoria.
- `MR`: inserta el valor en memoria.
- `MC`: borra la memoria.

### Historial

Pulsa el botón `☰` para mostrar/ocultar el panel de historial. Haz clic en
cualquier entrada para reinsertar su expresión.

## Notas y limitaciones

- `%` es el operador **módulo** (resto), no porcentaje.
- **No hay multiplicación implícita**: `2pi` o `2(3)` dan error de sintaxis; usa
  `2*pi` y `2*(3)`.
- `tan(90)` en grados devuelve un número muy grande en lugar de error, por la
  precisión finita de coma flotante (90° no es exactamente π/2).
- Los resultados se muestran con hasta 10 cifras significativas (`%.10g`).

## Arquitectura

Separación estricta entre motor (lógica pura) e interfaz:

```
src/
├── lexer.{c,h}        # tokenizador
├── parser.{c,h}       # parser de descenso recursivo (interno)
├── evaluator.{c,h}    # API pública: calc_eval() + códigos de error
├── memory_reg.{c,h}   # registro de memoria (M+/M-/MR/MC)
├── history.{c,h}      # historial FIFO acotado
├── calc_window.{c,h}  # ventana y widgets GTK4
└── main.c             # GtkApplication
```

El motor (`lexer` + `parser` + `evaluator` + `memory_reg` + `history`) se compila
como la librería estática `calcengine`, sin ninguna dependencia de GTK, y se
testea de forma aislada. La GUI enlaza contra esa librería.

El parser implementa esta gramática (de menor a mayor precedencia):

```
expr    → term  (('+' | '-') term)*
term    → unary (('*' | '/' | '%') unary)*
unary   → ('+' | '-') unary | power
power   → primary ('^' unary)?        # asociativa a la derecha
primary → número | constante | función '(' expr ')' | '(' expr ')'
```

## Tests

El motor está cubierto por tests que corren sin GUI:

```bash
meson test -C build
```

Cubren precedencia, asociatividad, paréntesis, funciones, constantes, modo
DEG/RAD, memoria, historial y todos los códigos de error.

## Documentación de diseño

- Especificación: [`docs/superpowers/specs/2026-06-10-calculadora-gtk4-design.md`](docs/superpowers/specs/2026-06-10-calculadora-gtk4-design.md)
- Plan de implementación: [`docs/superpowers/plans/2026-06-10-calculadora-gtk4.md`](docs/superpowers/plans/2026-06-10-calculadora-gtk4.md)

## Licencia

Sin licencia definida todavía. Si vas a publicarlo, considera añadir un archivo
`LICENSE` (p. ej. MIT o GPL-3.0).

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

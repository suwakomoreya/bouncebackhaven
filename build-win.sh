#!/usr/bin/env bash
# Build bouncebackhaven for Windows with MSYS2 MinGW-w64 g++ + raylib.
#
# Run this from an "MSYS2 UCRT64" terminal (not CMD):
#   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib
#   cd /c/path/to/bouncebackhaven
#   ./build-win.sh
#
# Env overrides:
#   NAME=mygame        output exe name (default: bouncebackhaven)
#   SRC_DIR=src        source folder
#   BUILD_DIR=build    output folder
#   RAYLIB_DIR=C:/raylib   use a downloaded raylib instead of the MSYS2 package

set -euo pipefail

NAME="${NAME:-bouncebackhaven}"
SRC_DIR="${SRC_DIR:-src}"
BUILD_DIR="${BUILD_DIR:-build}"

cd "$(dirname "$0")"

if ! command -v g++ >/dev/null 2>&1; then
    echo "error: g++ not found." >&2
    echo "Open the MSYS2 UCRT64 terminal and run:" >&2
    echo "  pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib" >&2
    exit 1
fi

# Collect sources (MSYS2 bash expands these correctly, unlike CMD).
shopt -s nullglob
SOURCES=("$SRC_DIR"/*.cpp "$SRC_DIR"/*.cc "$SRC_DIR"/*.cxx)
shopt -u nullglob

if [ ${#SOURCES[@]} -eq 0 ]; then
    echo "error: no C++ sources found in $SRC_DIR/" >&2
    exit 1
fi

mkdir -p "$BUILD_DIR"

CXXFLAGS=(-std=c++17 -O2 -Wall -Wextra)
LDFLAGS=(-lopengl32 -lgdi32 -lwinmm)
RAYLIB_NOTE="MSYS2 raylib ($(pkg-config --modversion raylib 2>/dev/null || echo unknown))"

if [ -n "${RAYLIB_DIR:-}" ] && [ -f "$RAYLIB_DIR/lib/libraylib.a" ]; then
    # Downloaded raylib release (raylib-6.2_win64_mingw-w64.zip).
    CXXFLAGS+=(-I"$RAYLIB_DIR/include")
    LDFLAGS=("$RAYLIB_DIR/lib/libraylib.a" "${LDFLAGS[@]}")
    RAYLIB_NOTE="raylib at $RAYLIB_DIR"
elif pkg-config --exists raylib 2>/dev/null; then
    # MSYS2 package. It ships a shared libraylib.dll, so link glfw3 too.
    read -r -a PC_FLAGS <<< "$(pkg-config --cflags --libs raylib)"
    CXXFLAGS+=("${PC_FLAGS[@]}")
    LDFLAGS=(-lraylib -lglfw3 "${LDFLAGS[@]}")
else
    LDFLAGS=(-lraylib -lglfw3 "${LDFLAGS[@]}")
fi

echo "[build] ${SOURCES[*]}"
g++ "${CXXFLAGS[@]}" "${SOURCES[@]}" -o "$BUILD_DIR/$NAME.exe" "${LDFLAGS[@]}"

# Copy runtime DLLs next to the exe so it also runs by double-clicking.
if command -v ldd >/dev/null 2>&1 && [ -n "${MINGW_PREFIX:-}" ]; then
    ldd "$BUILD_DIR/$NAME.exe" 2>/dev/null \
        | awk -v p="$MINGW_PREFIX/bin/" 'index($0,p) { print $3 }' \
        | while read -r dll; do
            [ -f "$dll" ] && cp -f "$dll" "$BUILD_DIR/"
        done
fi

echo "[ok]    $BUILD_DIR/$NAME.exe  ($RAYLIB_NOTE)"
echo "        run: ./$BUILD_DIR/$NAME.exe"

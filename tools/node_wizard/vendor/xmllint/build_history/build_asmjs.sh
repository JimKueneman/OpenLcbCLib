#!/bin/bash
# Build patched libxml2 xmllint as standalone asm.js
set -e

source ~/emsdk/emsdk_env.sh 2>/dev/null

LIBXML2_DIR=/tmp/libxml2
OUT_DIR=/tmp/xmllint_build

mkdir -p "$OUT_DIR"

# Collect all .o files from the libxml2 build
OBJ_FILES=$(ls "$LIBXML2_DIR"/.libs/libxml2_la-*.o)

# Compile xmllint.c (the CLI tool)
echo "=== Compiling xmllint.c ==="
emcc -c \
    -I"$LIBXML2_DIR"/include \
    -I"$LIBXML2_DIR" \
    -DHAVE_CONFIG_H \
    -O2 \
    "$LIBXML2_DIR"/xmllint.c \
    -o "$OUT_DIR"/xmllint_main.o

# Link everything into asm.js
echo "=== Linking to asm.js ==="
emcc \
    -O2 \
    -s WASM=0 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="createXmllintModule" \
    -s EXIT_RUNTIME=1 \
    -s INVOKE_RUN=0 \
    -s FORCE_FILESYSTEM=1 \
    -s ENVIRONMENT='web,node' \
    --memory-init-file 0 \
    "$OUT_DIR"/xmllint_main.o \
    $OBJ_FILES \
    -o "$OUT_DIR"/xmllint_raw.js

echo "=== Build complete: $OUT_DIR/xmllint_raw.js ==="
ls -lh "$OUT_DIR"/xmllint_raw.js

#!/usr/bin/env bash
# Build the OpenLcbCLib WASM artifact.
# Output: wasm/dist/openlcb-core.{mjs,wasm}
set -euo pipefail

cd "$(dirname "$0")"

if ! command -v emcmake >/dev/null 2>&1; then
    echo "error: emcmake not found on PATH. Install Emscripten (brew install emscripten)." >&2
    exit 1
fi

rm -rf build
emcmake cmake -S . -B build
cmake --build build -j

mkdir -p dist
rm -f dist/openlcb-core.js
cp build/openlcb-core.mjs build/openlcb-core.wasm dist/

# Generate the JS constants module from the C headers (single source of truth).
node generate_defines.mjs

echo ""
echo "Built:"
ls -lh dist/openlcb-core.*

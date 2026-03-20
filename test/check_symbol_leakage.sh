#!/bin/bash
# =============================================================================
# check_symbol_leakage.sh — Verify disabled modules export no code symbols
#
# Usage: check_symbol_leakage.sh <build_dir> <library_file> <prefix1> [prefix2] ...
#
# Scans the static library for defined text symbols (T) matching the given
# prefixes.  If any are found, the module was not properly #ifdef'd out
# or a DI violation pulled it in.
#
# On macOS, nm prefixes C symbols with underscore, so we match both
# "T _Prefix" and "T Prefix".
#
# Exit code: 0 = clean, 1 = leaked symbols found
# =============================================================================

set -euo pipefail

if [ $# -lt 3 ]; then
    echo "Usage: $0 <build_dir> <library_file> <prefix1> [prefix2] ..."
    exit 1
fi

BUILD_DIR="$1"
LIBRARY="$2"
shift 2

LEAKED=0

for PREFIX in "$@"; do

    # Look for defined text symbols (T or t) matching the prefix
    # macOS nm adds leading underscore to C symbols
    MATCHES=$(nm "$BUILD_DIR/$LIBRARY" 2>/dev/null \
        | grep -E ' [Tt] _?'"$PREFIX" \
        || true)

    if [ -n "$MATCHES" ]; then
        echo "FAIL: Symbol leakage detected for prefix '$PREFIX' in $LIBRARY:"
        echo "$MATCHES"
        echo ""
        LEAKED=1
    fi

done

if [ $LEAKED -eq 0 ]; then
    echo "PASS: No symbol leakage detected in $LIBRARY"
fi

exit $LEAKED

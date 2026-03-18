#!/bin/bash
#
# OlcbChecker Integration Test Runner
#
# Builds the ComplianceTestNode and runs each protocol mode in separate passes
# (node process restarted between modes, bridge stays up for all runs).
#
# Usage:
#   ./run_olcbchecker.sh                                 # core tests only
#   ./run_olcbchecker.sh -m all                          # run all protocol modes
#   ./run_olcbchecker.sh -m core,trains                  # core + train tests
#   ./run_olcbchecker.sh -m trains -s check_tr090_controller  # one test in train mode
#   ./run_olcbchecker.sh -r -w                           # enable reboot + write tests
#   ./run_olcbchecker.sh -v --skip-build                 # debug with last build
#

set -e

# ============================================================================
# Configuration
# ============================================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
CHECKER_DIR="/Users/jimkueneman/Documents/OlcbChecker"
BRIDGE_PORT=12021
NODE_ID="05.07.01.01.00.33"

# ComplianceTestNode paths
COMPLIANCE_BUILD_DIR="/tmp/compliance_build"
COMPLIANCE_XCODE_PROJECT="$REPO_ROOT/test/compliance_node/ComplianceTestNode.xcodeproj"

# ============================================================================
# Mode registry
#
# Each mode maps to a ComplianceTestNode flag, a run_tests.py section name,
# and a human-readable label.  To add a new protocol mode, add one line here
# and it works everywhere (CLI, --single, --mode all).
# ============================================================================

#          mode name                 node flag                    section name                label
MODES=(
    "core                           --basic                      core                        Core Compliance"
    "broadcast-time-consumer        --broadcast-time-consumer    broadcast_time_consumer     Broadcast Time Consumer"
    "broadcast-time-producer        --broadcast-time-producer    broadcast_time_producer     Broadcast Time Producer"
    "trains                         --train                      trains                      Train Protocol"
)

# ============================================================================
# Parse arguments
# ============================================================================

VERBOSE=""
SKIP_BUILD=false
AUTO_REBOOT=false
FORCE_WRITES=false
MODE_LIST="core"
SINGLE_SCRIPT=""

ARGS=("$@")
i=0
while [ $i -lt ${#ARGS[@]} ]; do
    arg="${ARGS[$i]}"
    case "$arg" in
        --verbose|-v)
            VERBOSE="--verbose"
            ;;
        --skip-build)
            SKIP_BUILD=true
            ;;
        --auto-reboot|-r)
            AUTO_REBOOT=true
            ;;
        --force-writes|-w)
            FORCE_WRITES=true
            ;;
        --mode|-m)
            i=$((i + 1))
            MODE_LIST="${ARGS[$i]}"
            ;;
        --single|-s)
            i=$((i + 1))
            SINGLE_SCRIPT="${ARGS[$i]}"
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "  -m, --mode MODES       Comma-separated list of protocol modes to run (default: core)"
            echo "                         Available: core, broadcast-time-consumer, broadcast-time-producer, trains, all"
            echo "  -s, --single SCRIPT    Run a single check or control script (use -m to set the node mode)"
            echo "  -r, --auto-reboot      Pass --auto-reboot to OlcbChecker (programmatic restart)"
            echo "  -w, --force-writes     Enable tests that write to config memory (0xFD)"
            echo "  -v, --verbose          Show GridConnect traffic in bridge"
            echo "  --skip-build           Skip xcodebuild step"
            echo "  --help, -h             Show this help"
            exit 0
            ;;
        *)
            echo "Unknown option: $arg (use --help for usage)"
            exit 1
            ;;
    esac
    i=$((i + 1))
done

# ============================================================================
# Resolve mode list
# ============================================================================

# Expand "all" to every registered mode
if [ "$MODE_LIST" = "all" ]; then
    MODE_LIST=""
    for entry in "${MODES[@]}"; do
        read -r name _ <<< "$entry"
        [ -n "$MODE_LIST" ] && MODE_LIST="$MODE_LIST,"
        MODE_LIST="$MODE_LIST$name"
    done
fi

# Split comma-separated modes into an array
IFS=',' read -ra SELECTED_MODES <<< "$MODE_LIST"

# Look up a mode's fields by name.  Sets: MODE_NODE_FLAG, MODE_SECTION, MODE_LABEL
lookup_mode() {
    local target="$1"
    for entry in "${MODES[@]}"; do
        read -r name node_flag section label_rest <<< "$entry"
        if [ "$name" = "$target" ]; then
            MODE_NODE_FLAG="$node_flag"
            MODE_SECTION="$section"
            MODE_LABEL="$label_rest"
            return 0
        fi
    done
    echo "ERROR: Unknown mode '$target'"
    echo "  Available: core, broadcast-time-consumer, broadcast-time-producer, trains, all"
    exit 1
}

# Validate all selected modes up front
for mode in "${SELECTED_MODES[@]}"; do
    lookup_mode "$mode"
done

# ============================================================================
# Cleanup handler
# ============================================================================

NODE_PID=""
BRIDGE_PID=""

cleanup() {
    echo ""
    echo "=== Cleanup ==="
    if [ -n "$NODE_PID" ]; then
        kill "$NODE_PID" 2>/dev/null || true
        wait "$NODE_PID" 2>/dev/null || true
    fi
    if [ -n "$BRIDGE_PID" ]; then
        kill "$BRIDGE_PID" 2>/dev/null || true
        wait "$BRIDGE_PID" 2>/dev/null || true
    fi
}

trap cleanup EXIT

# ============================================================================
# Build
# ============================================================================

echo "=== ComplianceTestNode ==="

if [ "$SKIP_BUILD" = false ]; then
    echo "=== Build ComplianceTestNode ==="
    xcodebuild -project "$COMPLIANCE_XCODE_PROJECT" \
               -scheme ComplianceTestNode \
               -configuration Debug \
               -derivedDataPath "$COMPLIANCE_BUILD_DIR" \
               clean build 2>&1 | tail -30
    echo "  Build complete."
fi

BINARY="$COMPLIANCE_BUILD_DIR/Build/Products/Debug/ComplianceTestNode"
if [ ! -f "$BINARY" ]; then
    echo "ERROR: Binary not found at $BINARY"
    echo "  Run without --skip-build first."
    exit 1
fi

# ============================================================================
# Bridge (stays up for all runs)
# ============================================================================

echo "=== Start bridge server ==="
python3 "$SCRIPT_DIR/bridge_server.py" --port "$BRIDGE_PORT" $VERBOSE &
BRIDGE_PID=$!
sleep 1
if ! kill -0 "$BRIDGE_PID" 2>/dev/null; then
    echo "ERROR: Bridge server failed to start"
    exit 1
fi

# ============================================================================
# Build OlcbChecker flags
# ============================================================================

CHECKER_FLAGS="-i"
if [ "$AUTO_REBOOT" = true ]; then
    CHECKER_FLAGS="$CHECKER_FLAGS --auto-reboot"
fi
if [ "$FORCE_WRITES" = true ]; then
    CHECKER_FLAGS="$CHECKER_FLAGS -w"
fi

# ============================================================================
# Generic test runner: start node, run tests, kill node
# ============================================================================

total=0

run_protocol_test() {
    local node_flag="$1"
    local test_section="$2"
    local label="$3"

    echo ""
    echo "=========================================="
    echo "=== $label ==="
    echo "=========================================="

    if [ -n "$VERBOSE" ]; then
        "$BINARY" --node-id "$NODE_ID" $node_flag </dev/null &
    else
        "$BINARY" --node-id "$NODE_ID" $node_flag </dev/null >/dev/null 2>&1 &
    fi
    NODE_PID=$!
    sleep 3

    if ! kill -0 "$NODE_PID" 2>/dev/null; then
        echo "ERROR: ComplianceTestNode failed to start for $label"
        total=$((total + 1))
        NODE_PID=""
        return
    fi

    echo "  ComplianceTestNode running (PID $NODE_PID) mode=$node_flag"

    local rc=0
    RUN_SECTIONS="$test_section" \
    SINGLE_SCRIPT="$SINGLE_SCRIPT" \
    python3 "$SCRIPT_DIR/run_tests.py" \
        -a "127.0.0.1:$BRIDGE_PORT" \
        -t "$NODE_ID" \
        $CHECKER_FLAGS \
        || rc=$?
    total=$((total + rc))

    kill "$NODE_PID" 2>/dev/null || true
    wait "$NODE_PID" 2>/dev/null || true
    NODE_PID=""
    sleep 1
}

# ============================================================================
# Test runs
# ============================================================================

if [ -n "$SINGLE_SCRIPT" ]; then
    # Single-script mode: use the first selected mode
    lookup_mode "${SELECTED_MODES[0]}"
    run_protocol_test "$MODE_NODE_FLAG" "single" "Single: $SINGLE_SCRIPT"
else
    # Run each selected mode in order
    for mode in "${SELECTED_MODES[@]}"; do
        lookup_mode "$mode"
        run_protocol_test "$MODE_NODE_FLAG" "$MODE_SECTION" "$MODE_LABEL"
    done
fi

# ============================================================================
# Summary
# ============================================================================

echo ""
echo "============================================"
if [ $total -eq 0 ]; then
    echo "  ALL OLCBCHECKER TESTS PASSED"
else
    echo "  SOME OLCBCHECKER TESTS FAILED (total failures: $total)"
fi
echo "============================================"

exit $total

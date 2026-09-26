#!/bin/bash
#
# OlcbChecker Integration Test Runner
#
# Builds the ComplianceTestNode and runs each protocol mode in separate passes
# (node process restarted between modes, bridge stays up for all runs).
#
# Two node binaries are built as needed: a base build (stream off) for most
# modes, and a stream build (OPENLCB_COMPILE_STREAM on) for stream and trains-stream.
#
# Usage:
#   ./run_olcbchecker.sh                                 # core, stream, trains-stream
#   ./run_olcbchecker.sh -m all                          # run all protocol modes
#   ./run_olcbchecker.sh -m core,trains                  # core + train tests
#   ./run_olcbchecker.sh -m trains -s check_tr090_controller  # one test in train mode
#   ./run_olcbchecker.sh -r                              # enable reboot tests
#   ./run_olcbchecker.sh --no-writes                     # skip config memory write tests
#

set -e

# ============================================================================
# Configuration
# ============================================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
CHECKER_DIR="/Users/jimkueneman/Documents/OlcbCheckerClone"
BRIDGE_PORT=12021
NODE_ID="05.07.01.01.00.33"
PYTHON="${PYTHON:-/opt/homebrew/bin/python3.13}"

# ComplianceTestNode paths
COMPLIANCE_BUILD_DIR="/tmp/compliance_build"
COMPLIANCE_XCODE_PROJECT="$REPO_ROOT/test/compliance_node/ComplianceTestNode.xcodeproj"

# Both builds share COMPLIANCE_BUILD_DIR; each finished binary is copied out
# so the second build does not overwrite the first.
BASE_BINARY="/tmp/compliance_node_base"
STREAM_BINARY="/tmp/compliance_node_stream"

# Concurrent streams for the stream mode build (2 so the concurrent stream
# checks exercise two open streams instead of passing on a rejection)
STREAM_MODE_COUNT=2

# ============================================================================
# Mode registry
#
# Each mode maps to a ComplianceTestNode flag, a run_tests.py section name,
# a build (base or stream), and a human-readable label.  To add a new protocol
# mode, add one line here and it works everywhere (CLI, --single, --mode all).
# ============================================================================

#          mode name                 node flag                    section name                build     label
MODES=(
    "core                           --basic                      core                        base      Core Compliance"
    "stream                         --basic                      stream                      stream    Stream Enabled"
    "trains-stream                  --train                      fdi_stream                  stream    Train FDI"
    "broadcast-time-consumer        --broadcast-time-consumer    broadcast_time_consumer     base      Broadcast Time Consumer"
    "broadcast-time-producer        --broadcast-time-producer    broadcast_time_producer     base      Broadcast Time Producer"
    "trains                         --train                      trains                      base      Train Protocol"
    "dcc-detector                   --dcc-detector               dcc_detector                base      DCC Detector Protocol"
)

# ============================================================================
# Parse arguments
# ============================================================================

VERBOSE=""
AUTO_REBOOT=false
FORCE_WRITES=true
ENABLE_STREAM=false
STREAM_COUNT=1
MODE_LIST="core,stream,trains-stream"
SINGLE_SCRIPT=""

ARGS=("$@")
i=0
while [ $i -lt ${#ARGS[@]} ]; do
    arg="${ARGS[$i]}"
    case "$arg" in
        --verbose|-v)
            VERBOSE="--verbose"
            ;;
        --auto-reboot|-r)
            AUTO_REBOOT=true
            ;;
        --force-writes|-w)
            FORCE_WRITES=true
            ;;
        --no-writes)
            FORCE_WRITES=false
            ;;
        --stream)
            ENABLE_STREAM=true
            # Check if next arg is a number (stream count)
            if [ $((i + 1)) -lt ${#ARGS[@]} ]; then
                next="${ARGS[$((i + 1))]}"
                if [[ "$next" =~ ^[0-9]+$ ]]; then
                    STREAM_COUNT="$next"
                    i=$((i + 1))
                fi
            fi
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
            echo "  -m, --mode MODES       Comma-separated list of protocol modes to run (default: core,stream,trains-stream)"
            echo "                         Available: core, stream, trains-stream, broadcast-time-consumer, broadcast-time-producer, trains, dcc-detector, all"
            echo "                         stream = Datagram, Memory Config, CDI, and Stream checks on a stream-enabled build"
            echo "                         trains-stream = FDI checks on a stream-enabled train node"
            echo "  -s, --single SCRIPT    Run a single check or control script (use -m to set the node mode)"
            echo "  -r, --auto-reboot      Pass --auto-reboot to OlcbChecker (programmatic restart)"
            echo "  -w, --force-writes     Enable tests that write to config memory (0xFD) (default: on)"
            echo "  --no-writes            Skip tests that write to config memory"
            echo "  --stream [N]           Build every mode with stream transport, N concurrent streams (default: 1)"
            echo "  -v, --verbose          Show GridConnect traffic in bridge"
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

# Look up a mode's fields by name.  Sets: MODE_NODE_FLAG, MODE_SECTION, MODE_BUILD, MODE_LABEL
lookup_mode() {
    local target="$1"
    for entry in "${MODES[@]}"; do
        read -r name node_flag section build label_rest <<< "$entry"
        if [ "$name" = "$target" ]; then
            MODE_NODE_FLAG="$node_flag"
            MODE_SECTION="$section"
            MODE_BUILD="$build"
            MODE_LABEL="$label_rest"
            return 0
        fi
    done
    echo "ERROR: Unknown mode '$target'"
    echo "  Available: core, stream, trains-stream, broadcast-time-consumer, broadcast-time-producer, trains, dcc-detector, all"
    exit 1
}

# Validate all selected modes up front
for mode in "${SELECTED_MODES[@]}"; do
    lookup_mode "$mode"
done

# Single-script mode only runs the first selected mode
if [ -n "$SINGLE_SCRIPT" ]; then
    RUN_MODES=("${SELECTED_MODES[0]}")
else
    RUN_MODES=("${SELECTED_MODES[@]}")
fi

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

# Build the node with the given preprocessor defines and copy it to out_binary
build_node() {
    local defines="$1"
    local out_binary="$2"
    local built="$COMPLIANCE_BUILD_DIR/Build/Products/Debug/ComplianceTestNode"

    xcodebuild -project "$COMPLIANCE_XCODE_PROJECT" \
               -scheme ComplianceTestNode \
               -configuration Debug \
               -derivedDataPath "$COMPLIANCE_BUILD_DIR" \
               GCC_PREPROCESSOR_DEFINITIONS='$(inherited) '"$defines" \
               clean build 2>&1 | tail -30

    if [ ! -f "$built" ]; then
        echo "ERROR: Build failed - binary not found at $built"
        exit 1
    fi
    cp "$built" "$out_binary"
    echo "  Build complete."
}

# The stream mode uses the --stream count when given, otherwise STREAM_MODE_COUNT
if [ "$ENABLE_STREAM" = true ]; then
    STREAM_PASS_COUNT="$STREAM_COUNT"
else
    STREAM_PASS_COUNT="$STREAM_MODE_COUNT"
fi
STREAM_DEFINE="OPENLCB_COMPILE_STREAM=1 USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS=$STREAM_PASS_COUNT"

# Build only the binaries the selected modes need
NEED_BASE=false
NEED_STREAM=false
for mode in "${RUN_MODES[@]}"; do
    lookup_mode "$mode"
    if [ "$MODE_BUILD" = "stream" ]; then
        NEED_STREAM=true
    else
        NEED_BASE=true
    fi
done

# --stream builds every mode with stream on, so one build serves both
if [ "$ENABLE_STREAM" = true ]; then
    STREAM_BINARY="$BASE_BINARY"
    if [ "$NEED_STREAM" = true ]; then
        NEED_BASE=true
        NEED_STREAM=false
    fi
fi

if [ "$NEED_BASE" = true ]; then
    echo "=== Build ComplianceTestNode (base) ==="
    if [ "$ENABLE_STREAM" = true ]; then
        echo "  Stream support: ON (max concurrent: $STREAM_COUNT)"
        build_node "$STREAM_DEFINE" "$BASE_BINARY"
    else
        echo "  Stream support: OFF"
        build_node "" "$BASE_BINARY"
    fi
fi

if [ "$NEED_STREAM" = true ]; then
    echo "=== Build ComplianceTestNode (stream) ==="
    echo "  Stream support: ON (max concurrent: $STREAM_PASS_COUNT)"
    build_node "$STREAM_DEFINE" "$STREAM_BINARY"
fi

# ============================================================================
# Bridge (stays up for all runs)
# ============================================================================

echo "=== Start bridge server ==="
$PYTHON "$SCRIPT_DIR/bridge_server.py" --port "$BRIDGE_PORT" $VERBOSE &
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
    local build="$4"

    local binary="$BASE_BINARY"
    if [ "$build" = "stream" ]; then
        binary="$STREAM_BINARY"
    fi

    if [ "$build" = "stream" ] || [ "$ENABLE_STREAM" = true ]; then
        label="$label (stream x$STREAM_PASS_COUNT)"
    fi

    echo ""
    echo "=========================================="
    echo "=== $label ==="
    echo "=========================================="

    NODE_LOG="/tmp/compliance_node.log"
    "$binary" --node-id "$NODE_ID" $node_flag </dev/null >"$NODE_LOG" 2>&1 &
    NODE_PID=$!
    sleep 3

    if ! kill -0 "$NODE_PID" 2>/dev/null; then
        echo "ERROR: ComplianceTestNode failed to start for $label"
        total=$((total + 1))
        NODE_PID=""
        return
    fi

    echo "  ComplianceTestNode running (PID $NODE_PID) mode=$label"

    local rc=0
    RUN_SECTIONS="$test_section" \
    SINGLE_SCRIPT="$SINGLE_SCRIPT" \
    $PYTHON "$SCRIPT_DIR/run_tests.py" \
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
    lookup_mode "${RUN_MODES[0]}"
    run_protocol_test "$MODE_NODE_FLAG" "single" "Single: $SINGLE_SCRIPT" "$MODE_BUILD"
else
    # Run each selected mode in order
    for mode in "${RUN_MODES[@]}"; do
        lookup_mode "$mode"
        run_protocol_test "$MODE_NODE_FLAG" "$MODE_SECTION" "$MODE_LABEL" "$MODE_BUILD"
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

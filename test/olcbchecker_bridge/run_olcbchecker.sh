#!/bin/bash
#
# OlcbChecker Integration Test Runner
#
# Builds the ComplianceTestNode and runs each protocol mode in separate passes
# (node process restarted between modes, bridge stays up for all runs).
#
# Usage:
#   ./run_olcbchecker.sh                             # core tests only
#   ./run_olcbchecker.sh --broadcast-time-consumer   # include broadcast time consumer tests
#   ./run_olcbchecker.sh --broadcast-time-producer   # include broadcast time producer tests
#   ./run_olcbchecker.sh --trains                    # include train protocol tests
#   ./run_olcbchecker.sh --all                       # run all protocol modes
#   ./run_olcbchecker.sh --auto-reboot               # enable programmatic restart in OlcbChecker
#   ./run_olcbchecker.sh --verbose                   # show GridConnect traffic
#   ./run_olcbchecker.sh --skip-build                # reuse last build
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
COMPLIANCE_BUILD_DIR="$REPO_ROOT/build/compliance"
COMPLIANCE_XCODE_PROJECT="$REPO_ROOT/test/compliance_node/ComplianceTestNode.xcodeproj"

# ============================================================================
# Parse arguments
# ============================================================================

VERBOSE=""
SKIP_BUILD=false
AUTO_REBOOT=false
BROADCAST_TIME_CONSUMER=false
BROADCAST_TIME_PRODUCER=false
TRAINS=false

for arg in "$@"; do
    case "$arg" in
        --verbose|-v)
            VERBOSE="--verbose"
            ;;
        --skip-build)
            SKIP_BUILD=true
            ;;
        --auto-reboot)
            AUTO_REBOOT=true
            ;;
        --broadcast-time-consumer)
            BROADCAST_TIME_CONSUMER=true
            ;;
        --broadcast-time-producer)
            BROADCAST_TIME_PRODUCER=true
            ;;
        --broadcast-time)
            # Shorthand for consumer tests
            BROADCAST_TIME_CONSUMER=true
            ;;
        --trains)
            TRAINS=true
            ;;
        --all)
            BROADCAST_TIME_CONSUMER=true
            BROADCAST_TIME_PRODUCER=true
            TRAINS=true
            ;;
        --no-broadcast-time)
            BROADCAST_TIME_CONSUMER=false
            BROADCAST_TIME_PRODUCER=false
            ;;
        --no-trains)
            TRAINS=false
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "  --verbose, -v          Show GridConnect traffic in bridge"
            echo "  --skip-build           Skip xcodebuild step"
            echo "  --auto-reboot          Pass --auto-reboot to OlcbChecker (programmatic restart)"
            echo "  --broadcast-time-consumer  Include Broadcast Time consumer tests"
            echo "  --broadcast-time-producer  Include Broadcast Time producer tests"
            echo "  --broadcast-time       Alias for --broadcast-time-consumer"
            echo "  --trains               Include Train Control/Search/FDI tests"
            echo "  --all                  Run all protocol modes"
            echo "  --no-broadcast-time    Skip all Broadcast Time tests"
            echo "  --no-trains            Skip Train protocol tests (default)"
            echo "  --help, -h             Show this help"
            exit 0
            ;;
    esac
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

# ============================================================================
# Generic test runner: start node, run tests, kill node
# ============================================================================

total=0

run_protocol_test() {
    local node_flag="$1"      # e.g., "--train" or "--basic"
    local test_section="$2"   # e.g., "trains" or "core"
    local label="$3"          # e.g., "Train Protocol"

    echo ""
    echo "=========================================="
    echo "=== $label ==="
    echo "=========================================="

    "$BINARY" --node-id "$NODE_ID" $node_flag </dev/null &
    NODE_PID=$!
    sleep 3

    if ! kill -0 "$NODE_PID" 2>/dev/null; then
        echo "ERROR: ComplianceTestNode failed to start for $label"
        total=$((total + 1))
        NODE_PID=""
        return
    fi

    echo "  ComplianceTestNode running (PID $NODE_PID) mode=$node_flag"

    RUN_SECTIONS="$test_section" \
    python3 "$SCRIPT_DIR/run_tests.py" \
        -a "127.0.0.1:$BRIDGE_PORT" \
        -t "$NODE_ID" \
        $CHECKER_FLAGS
    total=$((total + $?))

    kill "$NODE_PID" 2>/dev/null || true
    wait "$NODE_PID" 2>/dev/null || true
    NODE_PID=""
    sleep 1
}

# ============================================================================
# Test runs
# ============================================================================

# Core compliance (always)
run_protocol_test "--basic" "core" "Core Compliance"

# Broadcast time consumer
[ "$BROADCAST_TIME_CONSUMER" = true ] && \
    run_protocol_test "--broadcast-time-consumer" "broadcast_time_consumer" "Broadcast Time Consumer"

# Broadcast time producer
[ "$BROADCAST_TIME_PRODUCER" = true ] && \
    run_protocol_test "--broadcast-time-producer" "broadcast_time_producer" "Broadcast Time Producer"

# Train protocol
[ "$TRAINS" = true ] && \
    run_protocol_test "--train" "trains" "Train Protocol"

# ---- Add new protocol runs here ----
# run_protocol_test "--traction-proxy" "traction_proxy" "Traction Proxy"

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

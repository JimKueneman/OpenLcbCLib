#!/bin/bash
#
# release_update_wizard.sh
#
# One-shot pre-release workflow for OpenLcbCLib.  Runs the five steps that
# must all succeed before pushing a library change:
#
#   1. Unit tests          (cmake + make under test/)
#   2. Sync main library   (tools/update_applications/update_applications.sh)
#   3. Sync bootloader     (bootloader/tools/update_bootloaders/update_bootloaders.sh)
#   4. Compliance tests    (test/olcbchecker_bridge/run_olcbchecker.sh)
#   5. Doxygen docs        (documentation/Doxyfile)
#
# Order matters: unit tests gate the syncs so broken code never reaches the
# application folders, compliance runs against the synced state because that
# is what ships, and docs are only generated once everything passes.
#
# Hard-fails on the first failing step.
# Hard-fails on missing external tools (cmake, make, python3, doxygen).
#
# Usage:
#   ./release_update_wizard.sh                # full run
#   ./release_update_wizard.sh --skip-tests   # skip step 1
#   ./release_update_wizard.sh --skip-sync    # skip steps 2-3
#   ./release_update_wizard.sh --skip-compliance  # skip step 4
#   ./release_update_wizard.sh --skip-docs    # skip step 5
#   ./release_update_wizard.sh -v             # pass --verbose to compliance tests

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

SKIP_TESTS=0
SKIP_SYNC=0
SKIP_COMPLIANCE=0
SKIP_DOCS=0
VERBOSE=0

for arg in "$@"; do
    case "$arg" in
        --skip-tests)      SKIP_TESTS=1 ;;
        --skip-sync)       SKIP_SYNC=1 ;;
        --skip-compliance) SKIP_COMPLIANCE=1 ;;
        --skip-docs)       SKIP_DOCS=1 ;;
        -v|--verbose)      VERBOSE=1 ;;
        -h|--help)
            sed -n '3,27p' "$0"
            exit 0
            ;;
        *)
            echo "Unknown option: $arg"
            echo "Use --help for usage."
            exit 2
            ;;
    esac
done

# ----------------------------------------------------------------------------
# Output helpers
# ----------------------------------------------------------------------------
if [ -t 1 ]; then
    C_BOLD=$'\033[1m'
    C_GREEN=$'\033[32m'
    C_RED=$'\033[31m'
    C_YELLOW=$'\033[33m'
    C_RESET=$'\033[0m'
else
    C_BOLD=''; C_GREEN=''; C_RED=''; C_YELLOW=''; C_RESET=''
fi

section() {
    echo
    echo "${C_BOLD}=== $1 ===${C_RESET}"
}

ok() {
    echo "${C_GREEN}OK${C_RESET} $1"
}

fail() {
    echo "${C_RED}FAIL${C_RESET} $1" >&2
    exit 1
}

require_tool() {
    local tool="$1"
    if ! command -v "$tool" >/dev/null 2>&1; then
        fail "Required tool not found on PATH: $tool"
    fi
}

# ----------------------------------------------------------------------------
# Prerequisite check
# ----------------------------------------------------------------------------
section "Prerequisites"

if [ "$SKIP_TESTS" -eq 0 ]; then
    require_tool cmake
    require_tool make
fi

if [ "$SKIP_COMPLIANCE" -eq 0 ]; then
    require_tool python3
fi

if [ "$SKIP_DOCS" -eq 0 ]; then
    # Resolve doxygen: DOXYGEN env var > PATH > known Doxygen.app bundle.
    if [ -z "${DOXYGEN:-}" ]; then
        if command -v doxygen >/dev/null 2>&1; then
            DOXYGEN="$(command -v doxygen)"
        elif [ -x "${HOME}/Documents/Doxygen/Doxygen.app/Contents/Resources/doxygen" ]; then
            DOXYGEN="${HOME}/Documents/Doxygen/Doxygen.app/Contents/Resources/doxygen"
        else
            fail "doxygen not found. Set DOXYGEN=<path> or install doxygen on PATH."
        fi
    else
        [ -x "${DOXYGEN}" ] || fail "DOXYGEN=${DOXYGEN} is not an executable"
    fi
    echo "  doxygen: ${DOXYGEN}"
fi

ok "All required tools present"

# ----------------------------------------------------------------------------
# 1. Unit tests
# ----------------------------------------------------------------------------
if [ "$SKIP_TESTS" -eq 0 ]; then
    section "1/5  Unit tests"

    TEST_DIR="${REPO_ROOT}/test"
    BUILD_DIR="${TEST_DIR}/build"

    rm -rf "${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"

    (
        cd "${BUILD_DIR}"
        cmake .. \
            || fail "cmake configuration failed in ${BUILD_DIR}"
        make \
            || fail "unit tests failed"
    )

    ok "Unit tests passed"
else
    echo "${C_YELLOW}Skipping unit tests (--skip-tests)${C_RESET}"
fi

# ----------------------------------------------------------------------------
# 2. Sync main library into application folders
# ----------------------------------------------------------------------------
if [ "$SKIP_SYNC" -eq 0 ]; then
    section "2/5  Sync main library to applications"

    APP_SYNC="${REPO_ROOT}/tools/update_applications/update_applications.sh"

    [ -x "${APP_SYNC}" ] || fail "Not found or not executable: ${APP_SYNC}"

    (
        cd "$(dirname "${APP_SYNC}")"
        ./update_applications.sh \
            || fail "update_applications.sh exited non-zero"
    )

    ok "Application sync complete"

    # ----------------------------------------------------------------------------
    # 3. Sync bootloader library into bootloader application folders
    # ----------------------------------------------------------------------------
    section "3/5  Sync bootloader library to bootloader applications"

    BOOT_SYNC="${REPO_ROOT}/bootloader/tools/update_bootloaders/update_bootloaders.sh"

    [ -x "${BOOT_SYNC}" ] || fail "Not found or not executable: ${BOOT_SYNC}"

    (
        cd "$(dirname "${BOOT_SYNC}")"
        ./update_bootloaders.sh \
            || fail "update_bootloaders.sh exited non-zero"
    )

    ok "Bootloader sync complete"
else
    echo "${C_YELLOW}Skipping library sync (--skip-sync)${C_RESET}"
fi

# ----------------------------------------------------------------------------
# 4. Compliance tests (OlcbChecker)
# ----------------------------------------------------------------------------
if [ "$SKIP_COMPLIANCE" -eq 0 ]; then
    section "4/5  Compliance tests"

    COMPLIANCE="${REPO_ROOT}/test/olcbchecker_bridge/run_olcbchecker.sh"

    [ -x "${COMPLIANCE}" ] || fail "Not found or not executable: ${COMPLIANCE}"

    (
        cd "$(dirname "${COMPLIANCE}")"
        VERBOSE_FLAG=""
        [ "$VERBOSE" -eq 1 ] && VERBOSE_FLAG="-v"
        ./run_olcbchecker.sh --auto-reboot --force-writes --mode all --stream 2 $VERBOSE_FLAG \
            || fail "Compliance tests failed"
    )

    ok "Compliance tests passed"
else
    echo "${C_YELLOW}Skipping compliance tests (--skip-compliance)${C_RESET}"
fi

# ----------------------------------------------------------------------------
# 5. Doxygen documentation
# ----------------------------------------------------------------------------
if [ "$SKIP_DOCS" -eq 0 ]; then
    section "5/5  Doxygen documentation"

    DOC_DIR="${REPO_ROOT}/documentation"

    [ -f "${DOC_DIR}/Doxyfile" ] || fail "Doxyfile not found at ${DOC_DIR}/Doxyfile"

    (
        cd "${DOC_DIR}"
        "${DOXYGEN}" Doxyfile \
            || fail "doxygen generation failed"
    )

    ok "Doxygen documentation generated"
else
    echo "${C_YELLOW}Skipping doxygen docs (--skip-docs)${C_RESET}"
fi

# ----------------------------------------------------------------------------
# Summary
# ----------------------------------------------------------------------------
section "Release update wizard complete"
echo "${C_GREEN}All selected steps passed.${C_RESET}"

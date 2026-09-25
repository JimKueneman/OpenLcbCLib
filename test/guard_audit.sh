#!/bin/bash
# =============================================================================
# guard_audit.sh - Verify OPENLCB_COMPILE_* feature flag guards across configs
#
# Usage: test/guard_audit.sh            (run from anywhere)
#
# For each configuration under test/user_config/, plus a set synthesized from
# "typical" with one feature group removed, this script:
#   1. compiles every library source with -Wall -Werror,
#   2. archives the objects and checks that no undefined library symbol lacks
#      a definition in the archive (an unguarded call into a module that was
#      compiled out), and
#   3. lists text symbols from modules whose flag is off (leaks).
# It then syntax-checks the per-config *_compile_Test.cxx files.
#
# Exit code: 0 = every config compiled and had no unresolved symbols,
#            1 = otherwise. Leaks are reported but do not fail the run.
#
# Work files go under ${TMPDIR:-/tmp}/openlcb_guard_audit.
# =============================================================================

set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${TMPDIR:-/tmp}/openlcb_guard_audit"
rm -rf "$OUT"; mkdir -p "$OUT"

GTEST_INC=""
for d in "$ROOT"/test/build/_deps/googletest-src/googletest/include; do [ -d "$d" ] && GTEST_INC="$d"; done

CFLAGS="-std=gnu99 -Wall -Werror -Wno-error=#warnings -Wno-attributes -D__psv__= -D_SYS_COMMON_H -DSYS_TASKS_PRIORITY=int -D_GNU_SOURCE -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS"
SRC=$(find "$ROOT/src/openlcb" "$ROOT/src/drivers/canbus" "$ROOT/src/drivers/tcp_ip" "$ROOT/src/utilities" -name '*.c' | sort)
ALLFLAGS="CAN TCP EVENTS DATAGRAMS MEMORY_CONFIGURATION FIRMWARE BROADCAST_TIME TRAIN TRAIN_SEARCH STREAM DCC_DETECTOR DCC_CV"
STATUS=0

# ---- configurations -------------------------------------------------------
CFGS=""
for d in "$ROOT"/test/user_config/*/; do n=$(basename "$d"); cp -r "$d" "$OUT/cfg_$n"; CFGS="$CFGS $n"; done

synth() {
    local name=$1; shift
    cp -r "$ROOT/test/user_config/typical" "$OUT/cfg_$name"
    for f in "$@"; do
        sed -i '' -E "/^[[:space:]]*#define OPENLCB_COMPILE_$f\$/d" "$OUT/cfg_$name/openlcb_user_config.h"
    done
    CFGS="$CFGS $name"
}
synth typical_no_STREAM STREAM
synth typical_no_FIRMWARE FIRMWARE
synth typical_no_BROADCAST_TIME BROADCAST_TIME
synth typical_no_DCC_DETECTOR DCC_DETECTOR
synth typical_no_DCC_CV DCC_CV
synth typical_no_TRAIN_SEARCH TRAIN_SEARCH
synth typical_no_TRAIN TRAIN TRAIN_SEARCH
synth typical_no_EVENTS EVENTS BROADCAST_TIME DCC_DETECTOR TRAIN TRAIN_SEARCH
synth typical_no_MEMCFG MEMORY_CONFIGURATION FIRMWARE DCC_CV
synth typical_no_DATAGRAMS DATAGRAMS MEMORY_CONFIGURATION FIRMWARE DCC_CV
synth typical_tcp_only CAN
sed -i '' -E 's|^[[:space:]]*// #define OPENLCB_COMPILE_TCP|#define OPENLCB_COMPILE_TCP|' "$OUT/cfg_typical_tcp_only/openlcb_user_config.h"

# ---- which public prefixes belong to which flag ---------------------------
leak_prefixes() {
    case $1 in
        EVENTS) echo "ProtocolEventTransport_";;
        DATAGRAMS) echo "ProtocolDatagramHandler_";;
        MEMORY_CONFIGURATION) echo "ProtocolConfigMemReadHandler_ ProtocolConfigMemWriteHandler_ ProtocolConfigMemOperationsHandler_ ProtocolConfigMemStreamHandler_";;
        STREAM) echo "ProtocolStreamHandler_ ProtocolConfigMemStreamHandler_";;
        BROADCAST_TIME) echo "ProtocolBroadcastTimeHandler_ OpenLcbApplicationBroadcastTime_";;
        TRAIN) echo "ProtocolTrainHandler_ OpenLcbApplicationTrain_ ProtocolTrainSearchHandler_";;
        TRAIN_SEARCH) echo "ProtocolTrainSearchHandler_";;
        DCC_DETECTOR) echo "OpenLcbApplicationDccDetector_";;
        DCC_CV) echo "OpenLcbApplicationDccCv_";;
        FIRMWARE) echo "ProtocolConfigMemStreamHandler_handle_read_stream_space_firmware ProtocolConfigMemStreamHandler_handle_write_stream_space_firmware";;
        TCP) echo "Tcp";;
        CAN) echo "Can AliasMappingListener_ InternalNodeAliasTable_";;
    esac
}

# ---- main loop ------------------------------------------------------------
printf "%-26s %8s %6s %10s\n" CONFIG COMPILE LEAKS UNRESOLVED
for n in $CFGS; do
    cfg="$OUT/cfg_$n"; obj="$OUT/obj_$n"; mkdir -p "$obj"; : > "$obj/compile.log"; fails=0
    for f in $SRC; do
        cc $CFLAGS -I "$cfg" -I "$ROOT/templates/canbus" -I "$ROOT/src" -I "$ROOT/templates/tcp_ip" -c "$f" -o "$obj/$(basename "$f" .c).o" >> "$obj/compile.log" 2>&1 \
            || { fails=$((fails+1)); echo "FAIL: $f" >> "$obj/compile.log"; }
    done
    rm -f "$obj/lib.a"; ar rcs "$obj/lib.a" "$obj"/*.o 2>/dev/null
    nm -g "$obj/lib.a" 2>/dev/null > "$obj/nm.txt"
    awk 'NF==3 && $2 ~ /^[A-Za-z]$/ && $2 != "U" {print $3}' "$obj/nm.txt" | sort -u > "$obj/defined.txt"
    awk 'NF==2 && $1=="U" {print $2}' "$obj/nm.txt" | sort -u > "$obj/undefined.txt"
    comm -23 "$obj/undefined.txt" "$obj/defined.txt" | grep -E '^_[A-Z]' > "$obj/unresolved.txt" || true

    defined_flags=$(grep -oE "^[[:space:]]*#define OPENLCB_COMPILE_[A-Z_]+" "$cfg/openlcb_user_config.h" | sed -E 's/.*OPENLCB_COMPILE_//')
    # BOOTLOADER implies DATAGRAMS + MEMORY_CONFIGURATION + FIRMWARE
    echo "$defined_flags" | grep -qx BOOTLOADER && defined_flags="$(printf "%s\nDATAGRAMS\nMEMORY_CONFIGURATION\nFIRMWARE" "$defined_flags")"
    : > "$obj/leaks.txt"
    for fl in $ALLFLAGS; do
        echo "$defined_flags" | grep -qx "$fl" && continue
        for p in $(leak_prefixes "$fl"); do
            awk -v p="_$p" -v fl="$fl" 'NF==3 && $2 ~ /^[Tt]$/ && index($3,p)==1 {print fl" off but present: "$3}' "$obj/nm.txt" >> "$obj/leaks.txt"
        done
    done

    leaks=$(wc -l < "$obj/leaks.txt" | tr -d ' '); unres=$(wc -l < "$obj/unresolved.txt" | tr -d ' ')
    printf "%-26s %8s %6s %10s\n" "$n" "$fails" "$leaks" "$unres"
    [ "$fails" -ne 0 ] && STATUS=1
    [ "$unres" -ne 0 ] && STATUS=1
done

echo
echo "Details: $OUT/obj_<config>/{compile.log,leaks.txt,unresolved.txt}"

# ---- per-config compile tests ---------------------------------------------
echo
if [ -z "$GTEST_INC" ]; then
    echo "Per-config compile tests skipped: build test/ first so googletest headers exist."
else
    echo "Per-config compile tests (-fsyntax-only, C++14):"
    for t in minimal events_only datagrams_only memory_config broadcast_time train; do
        f="$ROOT/src/openlcb/openlcb_${t}_compile_Test.cxx"
        if c++ -std=c++14 -fsyntax-only -Wall -Werror -Wno-error=#warnings -Wno-attributes -DGTEST -D__psv__= -D_SYS_COMMON_H -DSYS_TASKS_PRIORITY=int -D_GNU_SOURCE \
                -I "$ROOT/test/user_config/$t" -I "$ROOT/templates/canbus" -I "$ROOT/src" -I "$ROOT/templates/tcp_ip" -I "$GTEST_INC" "$f" > "$OUT/compile_test_$t.log" 2>&1; then
            echo "  PASS $t"
        else
            echo "  FAIL $t"; grep -m3 "error" "$OUT/compile_test_$t.log"; STATUS=1
        fi
    done
fi

exit $STATUS

#!/bin/bash

# update_applications.sh
#
# Copies the library source files from src/ into each application's local
# openlcb_c_lib/ folder. Run from this directory:
#
#   cd tools/update_applications
#   ./update_applications.sh
#
# Each IDE project gets its own copy of the library files so it can be built
# standalone without depending on the repo structure.
#
# Only .c and .h files are copied. Test files (*_Test.cxx) are excluded.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC="${SCRIPT_DIR}/../../src"

# Verify source exists
if [ ! -d "${SRC}/openlcb" ]; then
    echo "ERROR: Cannot find library source at ${SRC}/openlcb"
    echo "       Run this script from tools/update_applications/"
    exit 1
fi

# Helper: copy .c and .h files from a source module to a destination directory.
# Creates the destination directory if it does not exist.
copy_module() {
    local src_dir="$1"
    local dst_dir="$2"

    if [ ! -d "${src_dir}" ]; then
        return
    fi

    mkdir -p "${dst_dir}"
    cp "${src_dir}"/*.c "${dst_dir}/" 2>/dev/null
    cp "${src_dir}"/*.h "${dst_dir}/" 2>/dev/null

    # Remove test files that got copied
    rm -f "${dst_dir}"/*_Test.cxx 2>/dev/null
}

# Helper: update one application target.
# $1 = display name
# $2 = path to the app's openlcb_c_lib root directory
# $3 = optional flag: "with_pc_utils" to also copy utilities_pc
update_app() {
    local app_name="$1"
    local lib_root="$2"
    local extras="$3"

    echo "Updating: ${app_name}"
    copy_module "${SRC}/openlcb"            "${lib_root}/openlcb"
    copy_module "${SRC}/drivers/canbus"     "${lib_root}/drivers/canbus"
    copy_module "${SRC}/drivers/tcp_ip"     "${lib_root}/drivers/tcp_ip"
    copy_module "${SRC}/utilities"          "${lib_root}/utilities"

    if [ "${extras}" = "with_pc_utils" ]; then
        # PC utilities go into the same utilities/ directory
        copy_module "${SRC}/utilities_pc"   "${lib_root}/utilities"
    fi
}

# =========================================================================
# Application targets — add new applications here
# =========================================================================

update_app "arduino/esp32/BasicNode" \
    "${SCRIPT_DIR}/../../applications/arduino/esp32/BasicNode/src/openlcb_c_lib"

update_app "arduino/rpi_pico/BasicNode" \
    "${SCRIPT_DIR}/../../applications/arduino/rpi_pico/BasicNode/src/openlcb_c_lib"

update_app "platformio/esp32/BasicNode" \
    "${SCRIPT_DIR}/../../applications/platformio/esp32/BasicNode/src/openlcb_c_lib"

update_app "platformio/esp32/BasicNode_WiFi" \
    "${SCRIPT_DIR}/../../applications/platformio/esp32/BasicNode_WiFi/src/openlcb_c_lib"

update_app "platformio/osx/BasicNode" \
    "${SCRIPT_DIR}/../../applications/platformio/osx/BasicNode/src/openlcb_c_lib" \
    "with_pc_utils"

update_app "xcode/BasicNode" \
    "${SCRIPT_DIR}/../../applications/xcode/BasicNode/openlcb_c_lib" \
    "with_pc_utils"

update_app "ti_theia/mspm03507_launchpad/BasicNode" \
    "${SCRIPT_DIR}/../../applications/ti_theia/mspm03507_launchpad/BasicNode/openlcb_c_lib"

update_app "dspic/BasicNode.X" \
    "${SCRIPT_DIR}/../../applications/dspic/BasicNode.X/src/openlcb_c_lib"

update_app "stm32_cubeide/stm32f407_discovery/BasicNode" \
    "${SCRIPT_DIR}/../../applications/stm32_cubeide/stm32f407_discovery/BasicNode/Core/Src/openlcb_c_lib"

update_app "stm32_cubeide/stm32f407_discovery/BasicNodeBootloader" \
    "${SCRIPT_DIR}/../../applications/stm32_cubeide/stm32f407_discovery/BasicNodeBootloader/Core/Src/openlcb_c_lib"

# =========================================================================
# External project targets
# =========================================================================

update_app "NmraLccProjects/RPiPico" \
    "${SCRIPT_DIR}/../../../NmraLccProjects/RPiPico/src"

update_app "LCC Projects/TurnoutBoss2_Firmware/TurnoutBossCommon" \
    "${SCRIPT_DIR}/../../../LCC Projects/TurnoutBoss2_Firmware/TurnoutBossCommon"

echo "Done."

#!/bin/bash

# update_bootloaders.sh
#
# Copies the bootloader library source files from bootloader/src/ into each
# application demo's local src/ folder. Run from this directory:
#
#   cd bootloader/tools/update_bootloaders
#   ./update_bootloaders.sh
#
# This mirrors how the main library's tools/update_applications/update_applications.sh
# works — each IDE project gets its own copy of the library files so it can be
# built standalone without depending on the repo structure.
#
# Only .c and .h files are copied. Test files (*_Test.cxx) and CMakeLists.txt
# are excluded since they are not part of the deployable library.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BOOT_SRC="${SCRIPT_DIR}/../../src"
BOOT_APPS="${SCRIPT_DIR}/../../applications"

# Verify source exists
if [ ! -d "${BOOT_SRC}/openlcb" ]; then
    echo "ERROR: Cannot find bootloader source at ${BOOT_SRC}/openlcb"
    echo "       Run this script from bootloader/tools/update_bootloaders/"
    exit 1
fi

# Helper function: copy .c and .h files from src module to app destination
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

# Helper function: update one application target
update_app() {
    local app_name="$1"
    local app_src="$2"

    echo "Updating: ${app_name}"
    copy_module "${BOOT_SRC}/openlcb"          "${app_src}/openlcb"
    copy_module "${BOOT_SRC}/crc"              "${app_src}/crc"
    copy_module "${BOOT_SRC}/drivers/canbus"   "${app_src}/drivers/canbus"
    copy_module "${BOOT_SRC}/drivers/tcp_ip"   "${app_src}/drivers/tcp_ip"
}

# =========================================================================
# Application targets — add new applications here
# =========================================================================

update_app "dspic/BasicNodeBootloader.X" \
    "${BOOT_APPS}/dspic/BasicNodeBootloader.X/openlcb_c_lib_bootloader"

update_app "stm32_cubeide/stm32f407_discovery/BasicNodeBootloader" \
    "${BOOT_APPS}/stm32_cubeide/stm32f407_discovery/BasicNodeBootloader/Core/Src/openlcb_c_lib_bootloader"

update_app "ti_theia/mspm03507_launchpad/BasicNodeBootloader" \
    "${BOOT_APPS}/ti_theia/mspm03507_launchpad/BasicNodeBootloader/openlcb_c_lib_bootloader"

update_app "esp32/BasicNodeBootloader" \
    "${BOOT_APPS}/esp32/BasicNodeBootloader/src/openlcb_c_lib_bootloader"

echo "Done."

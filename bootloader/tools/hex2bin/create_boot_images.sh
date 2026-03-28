#!/bin/bash
#
# create_boot_images.sh
#
# Generates bootloader-ready .bin images for all demo platforms.
# Output is placed next to each input .hex file.
#
# Usage:
#   cd bootloader/tools/hex2bin
#   ./create_boot_images.sh
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
HEX2BIN="${SCRIPT_DIR}/hex2bin.py"
APPS_DIR="${SCRIPT_DIR}/../../applications"

# -----------------------------------------------------------------------
# Platform table
#
# Platform         | Config     | Input .hex (relative to APPS_DIR)
# -----------------+------------+----------------------------------------------
# STM32 F407       | Debug      | stm32_cubeide/stm32f407_discovery/BasicNode/Debug/BasicNode.hex
# STM32 F407       | Release    | stm32_cubeide/stm32f407_discovery/BasicNode/Release/BasicNode.hex
# dsPIC 33EP512    | Production | dspic/BasicNode.X/dist/default/production/BasicNode.X.production.hex
# MSPM0 G3507      | Debug      | ti_theia/mspm03507_launchpad/BasicNode/Debug/BasicNode.out (ELF -> hex)
# -----------------------------------------------------------------------

# TI CCS hex output drops zero-filled regions, which corrupts the app_header.
# Use tiarmobjcopy to generate a correct Intel HEX from the ELF instead.
TIARMOBJCOPY="/Applications/TI/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmobjcopy"

elf_to_hex() {
    local elf_path="$1"
    local hex_path="$2"

    if [ ! -f "${elf_path}" ]; then
        echo "  SKIP  ${elf_path} (not found)"
        return 1
    fi

    if [ ! -x "${TIARMOBJCOPY}" ]; then
        echo "  ERROR tiarmobjcopy not found at ${TIARMOBJCOPY}"
        return 1
    fi

    echo "  HEX   ${elf_path} -> ${hex_path}"
    "${TIARMOBJCOPY}" -O ihex "${elf_path}" "${hex_path}"
}

convert() {
    local arch="$1"
    local start="$2"
    local header_offset="$3"
    local hex_path="$4"
    local bin_path="$5"

    if [ ! -f "${hex_path}" ]; then
        echo "  SKIP  ${hex_path} (not found)"
        return
    fi

    echo "  BUILD ${bin_path}"
    python3 "${HEX2BIN}" \
        --arch "${arch}" \
        --start "${start}" \
        --app-header-offset "${header_offset}" \
        --trim \
        "${hex_path}" \
        "${bin_path}"
}

echo "=== Creating boot images ==="
echo ""

# -- STM32 Debug --
HEX="${APPS_DIR}/stm32_cubeide/stm32f407_discovery/BasicNode/Debug/BasicNode.hex"
BIN="$(dirname "${HEX}")/BasicNode.stm32.debug.boot.bin"
convert flat 0x08008000 0x200 "${HEX}" "${BIN}"

# -- STM32 Release --
HEX="${APPS_DIR}/stm32_cubeide/stm32f407_discovery/BasicNode/Release/BasicNode.hex"
BIN="$(dirname "${HEX}")/BasicNode.stm32.release.boot.bin"
convert flat 0x08008000 0x200 "${HEX}" "${BIN}"

# -- dsPIC Production --
# App flash ends at PC 0x54FFC (0x55800 minus 2 reserved pages minus 4).
# Without --end the config words at 0x557F0+ inflate the binary to 667KB.
HEX="${APPS_DIR}/dspic/BasicNode.X/dist/default/production/BasicNode.X.production.hex"
BIN="$(dirname "${HEX}")/BasicNode.dspic.production.boot.bin"
convert_dspic() {
    if [ ! -f "${HEX}" ]; then
        echo "  SKIP  ${HEX} (not found)"
        return
    fi
    echo "  BUILD ${BIN}"
    python3 "${HEX2BIN}" \
        --arch dspic \
        --start 0x4000 \
        --end 0x54FFC \
        --app-header-offset 8 \
        --trim \
        "${HEX}" \
        "${BIN}"
}
convert_dspic

# -- MSPM0 Debug --
ELF="${APPS_DIR}/ti_theia/mspm03507_launchpad/BasicNode/Debug/BasicNode.out"
HEX="${APPS_DIR}/ti_theia/mspm03507_launchpad/BasicNode/Debug/BasicNode.objcopy.hex"
BIN="$(dirname "${ELF}")/BasicNode.mspm0.debug.boot.bin"
if elf_to_hex "${ELF}" "${HEX}"; then
    convert flat 0x00003C00 0xC0 "${HEX}" "${BIN}"
fi

echo ""
echo "=== Done ==="

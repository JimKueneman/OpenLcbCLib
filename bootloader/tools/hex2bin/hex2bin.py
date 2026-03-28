#!/usr/bin/env python3
"""
hex2bin.py -- Convert Intel HEX to a flat binary for bootloading.

Overview
--------
This script reads an Intel HEX file (.hex) produced by a compiler toolchain
and outputs a flat binary (.bin) suitable for uploading to an OpenLCB
standalone bootloader.  It optionally computes and patches triple CRC-16-IBM
checksums into the bootloader_app_header_t struct embedded in the binary.

The output .bin contains only the application region (no skip bytes needed).
The bootloader's firmware-space upload expects binary bytes starting at offset
zero, which maps to the first byte of the application flash region.

Architecture Support
--------------------
Two architecture modes control how addresses are translated between the Intel
HEX file, the binary image, and the target's flash hardware.

  --arch flat  (default)
      For Von Neumann targets (ARM, ESP32, RISC-V, etc.) where the program
      counter address equals the byte address.  One binary byte per flash
      byte.  --start and --end are byte addresses used directly.

  --arch dspic
      For dsPIC/PIC24 Harvard-architecture targets where each instruction is
      24 bits (3 data bytes) but occupies 4 bytes in the Intel HEX file
      (3 data + 1 phantom byte at 0x00).  The hex-file byte address is
      twice the PC address (PC * 2 = hex address).

      --start and --end accept PC addresses; the script multiplies by 2
      internally.

      PHANTOM BYTE HANDLING:
      The 4th byte of each instruction in the hex file is the "phantom byte".
      For programmed instructions the hex file always encodes this as 0x00.
      However, regions not present in the hex file (gaps, erased flash) are
      filled with --fill (default 0xFF), which puts 0xFF in the phantom
      position.  The dsPIC hardware ignores the phantom byte on write and
      always reads it back as 0x00.  Therefore, before computing CRCs for
      --arch dspic, this script forces every phantom byte (index 3, 7, 11,
      ...) to 0x00 so the checksum matches what the on-target CRC sees.

      The output .bin is NOT modified -- it retains the original fill bytes.
      Only the CRC computation uses the phantom-corrected view.  This is
      because the binary format matches what the upload tool sends to
      write_flash_bytes(), which extracts only the 3 data bytes per
      instruction and discards the phantom byte anyway.

Checksum Patching
-----------------
When --app-header-offset is provided (and --no-checksum is not), the script
computes triple CRC-16-IBM checksums and patches the bootloader_app_header_t
struct in the output binary.

The bootloader_app_header_t layout (36 bytes, little-endian):

    Offset  Size  Field
    ------  ----  -----
      0       4   app_size          Total image size in bytes
      4      16   checksum_pre[4]   Triple CRC of pre-header region + zero pad
     20      16   checksum_post[4]  Triple CRC of post-header region + zero pad

Each checksum array holds 4 uint32_t values:
    [0] = CRC-16-IBM over all bytes in the region
    [1] = CRC-16-IBM over odd-indexed bytes (1-based: bytes 1, 3, 5, ...)
    [2] = CRC-16-IBM over even-indexed bytes (1-based: bytes 2, 4, 6, ...)
    [3] = 0 (reserved, always zero)

The pre-header region is binary[0 .. header_offset).
The post-header region is binary[header_offset + 36 .. end).

The header_offset argument (--app-header-offset) is the byte offset from the
start of the binary, NOT a flash address.  It must point to a 36-byte region
of all zeros (the unpatched header from app_header.c).

Verification Flow
-----------------
On the target, the bootloader's finalize_flash() reads the header from flash,
recomputes the CRCs over the same pre/post regions, and compares them against
the stored values.  If they match, the image is valid.

For dsPIC targets, the on-target CRC reads 4 bytes per instruction (3 data +
phantom 0x00 from FLASH_ReadWord24).  This script must produce CRCs that
match that view, hence the phantom byte correction described above.

For flat targets, flash is memory-mapped and CRC is a straight memcpy+CRC
over the binary bytes.

Trim Mode
---------
--trim strips trailing fill bytes (default 0xFF) from the end of the binary,
rounding up to an 8-byte boundary for flash write alignment.  Trimming
happens BEFORE checksum computation, so app_size reflects the trimmed size.

Usage Examples
--------------
    # STM32 with checksum patching
    python3 hex2bin.py --arch flat --start 0x08008000 \\
        --app-header-offset 0x200 app.hex app.bin

    # MSPM0 with checksum patching
    python3 hex2bin.py --arch flat --start 0x00003C00 \\
        --app-header-offset 0xC0 app.hex app.bin

    # dsPIC with checksum patching (addresses are PC, not hex)
    python3 hex2bin.py --arch dspic --start 0x4000 --end 0x547FC \\
        --app-header-offset 8 --trim app.hex app.bin

    # Without checksum (legacy/debug)
    python3 hex2bin.py --arch dspic --start 0x4000 --no-checksum app.hex app.bin
"""

import argparse
import struct
import sys


# =============================================================================
# CRC-16-IBM (ANSI) -- ported from bootloader/src/crc/bootloader_crc.c
#
# Algorithm: CRC-16/ARC (also known as CRC-16-IBM, CRC-16-ANSI, CRC-16/LHA)
#   Width:      16 bits
#   Polynomial: 0x8005 (normal) / 0xA001 (reflected)
#   Init:       0x0000
#   RefIn:      True   (input bytes processed LSB-first)
#   RefOut:     True   (output CRC reflected)
#   XorOut:     0x0000
#   Check:      0xBB3D (CRC of ASCII "123456789")
#
# Implementation uses nibble lookup tables (64 bytes total) for speed on
# resource-constrained targets.  Each byte is processed as two 4-bit nibbles.
# =============================================================================

# High nibble table: pre-computed CRC contribution for bits 4-7 of the XOR.
_CRC16_IBM_HI = (
    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
)

# Low nibble table: pre-computed CRC contribution for bits 0-3 of the XOR.
_CRC16_IBM_LO = (
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
)


def _crc16_ibm_add(state, byte):
    """Append one byte to a running CRC-16-IBM state.

    Args:
        state: Current 16-bit CRC state.
        byte:  Next input byte (0-255).

    Returns:
        Updated 16-bit CRC state.
    """
    state ^= byte
    state = (state >> 8) ^ _CRC16_IBM_LO[state & 0x0F] ^ \
            _CRC16_IBM_HI[(state >> 4) & 0x0F]
    return state & 0xFFFF


def crc16_ibm(data):
    """Compute CRC-16-IBM over a bytes-like object.

    Args:
        data: Input bytes.

    Returns:
        16-bit CRC value (uint16).
    """
    state = 0x0000
    for b in data:
        state = _crc16_ibm_add(state, b)
    return state


def crc3_crc16_ibm(data):
    """Compute triple CRC-16-IBM (all, odd, even).

    Matches BootloaderCrc_crc3_crc16_ibm() in the C library.
    Three independent CRC-16-IBM accumulators run in parallel:
      - all:  every byte
      - odd:  bytes at 1-based odd positions (1, 3, 5, ...)
      - even: bytes at 1-based even positions (2, 4, 6, ...)

    The odd/even split is 1-based to match the C implementation where
    the byte counter starts at 1 (incremented before the odd/even check).

    Each CRC accumulator starts at 0x0000 and processes its subset of
    bytes independently.  The three CRCs together provide stronger error
    detection than a single CRC-16 alone: a bit error that happens to
    leave the all-bytes CRC unchanged is extremely unlikely to also
    leave both the odd and even CRCs unchanged.

    Args:
        data: Input bytes (the region to checksum).

    Returns:
        Tuple of (crc_all, crc_odd, crc_even), each a uint16.
    """
    state_all = 0x0000
    state_odd = 0x0000
    state_even = 0x0000

    for byte_index, b in enumerate(data, 1):
        state_all = _crc16_ibm_add(state_all, b)
        if byte_index & 1:
            state_odd = _crc16_ibm_add(state_odd, b)
        else:
            state_even = _crc16_ibm_add(state_even, b)

    return (state_all, state_odd, state_even)


# Size of bootloader_app_header_t in bytes:
#   uint32_t app_size              =  4
#   uint32_t checksum_pre[4]       = 16
#   uint32_t checksum_post[4]      = 16
#                            Total = 36
APP_HEADER_SIZE = 36


# =============================================================================
# Architecture definitions
#
# Each architecture specifies:
#   multiplier    -- factor to convert PC address to hex-file byte address.
#                    For flat targets this is 1 (PC = byte address).
#                    For dsPIC this is 2 (hex address = PC * 2).
#   default_start -- default --start if not provided on the command line.
#                    None means --start is required.
#   description   -- human-readable label for output messages.
# =============================================================================
ARCH_DEFS = {
    'dspic': {
        'multiplier': 2,
        'default_start': 0x8000,
        'description': 'dsPIC/PIC24 (PC x 2 = hex address)',
    },
    'flat': {
        'multiplier': 1,
        'default_start': None,   # must be provided by user
        'description': 'Byte-addressable (ARM, ESP32, RISC-V, etc.)',
    },
}


# =============================================================================
# Intel HEX parser
# =============================================================================

def parse_hex(path):
    """Parse an Intel HEX file and return its contents as a sparse dict.

    Reads the standard Intel HEX format (lines starting with ':') and
    extracts all data records.  Supports record types:
      00 -- Data (payload bytes at the current base + offset)
      01 -- End Of File (stops parsing)
      04 -- Extended Linear Address (sets upper 16 bits of base address)

    Record types 02 (Extended Segment Address), 03 (Start Segment Address),
    and 05 (Start Linear Address) are silently ignored since they are not
    needed for binary extraction.

    Each line's checksum is verified; a warning is printed for mismatches
    but parsing continues.

    Args:
        path: Path to the Intel HEX file.

    Returns:
        Dict mapping {hex_byte_address: byte_value} for every byte found
        in data records.  Addresses not present in the file are absent
        from the dict (they default to the fill byte during extraction).
    """
    data = {}
    base_address = 0

    with open(path, 'r') as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line or line[0] != ':':
                continue

            # Decode Intel HEX fields:
            #   :LLAAAATT[DD...]CC
            #   LL = byte count, AAAA = address, TT = record type,
            #   DD = data payload, CC = checksum (two's complement of sum)
            raw = bytes.fromhex(line[1:])
            byte_count = raw[0]
            address = (raw[1] << 8) | raw[2]
            record_type = raw[3]
            payload = raw[4:4 + byte_count]
            checksum = raw[4 + byte_count]

            # Verify line checksum: sum of all bytes (including checksum)
            # must be 0x00 modulo 256.
            total = sum(raw[:-1]) & 0xFF
            if (total + checksum) & 0xFF != 0:
                print(f"WARNING: bad checksum on line {line_num}",
                      file=sys.stderr)

            if record_type == 0x00:  # Data record
                full_addr = base_address + address
                for i, b in enumerate(payload):
                    data[full_addr + i] = b

            elif record_type == 0x01:  # End Of File
                break

            elif record_type == 0x04:  # Extended Linear Address
                # Payload is a 16-bit value that becomes bits 31:16 of the
                # base address for all subsequent data records.
                base_address = ((payload[0] << 8) | payload[1]) << 16

            # Ignore record types 02, 03, 05

    return data


# =============================================================================
# Binary extraction
# =============================================================================

def hex_to_bin(data, start_addr, end_addr, fill=0xFF):
    """Extract a contiguous byte range from the sparse hex data.

    Creates a flat byte array from start_addr (inclusive) to end_addr
    (exclusive).  Addresses present in the data dict get their recorded
    value; all others are filled with the fill byte (default 0xFF,
    matching erased flash).

    Args:
        data:       Sparse dict from parse_hex().
        start_addr: First hex-file byte address to include (inclusive).
        end_addr:   One past the last hex-file byte address (exclusive).
        fill:       Fill byte for gaps (default 0xFF).

    Returns:
        A bytes object of length (end_addr - start_addr).
    """
    size = end_addr - start_addr
    buf = bytearray([fill] * size)

    for addr, val in data.items():
        if start_addr <= addr < end_addr:
            buf[addr - start_addr] = val

    return bytes(buf)


# =============================================================================
# dsPIC phantom byte correction
# =============================================================================

def _dspic_fix_phantom_bytes(data):
    """Return a copy of data with every phantom byte forced to 0x00.

    On dsPIC/PIC24 targets, each instruction occupies 4 bytes in the binary
    image: 3 data bytes (the 24-bit instruction word) followed by 1 phantom
    byte.  The Intel HEX file produced by the XC16 toolchain always encodes
    the phantom byte as 0x00 for programmed instructions.

    However, when hex2bin fills gaps in the binary (regions not present in
    the hex file) with the fill byte (0xFF), the phantom position also gets
    0xFF.  On the actual hardware, the dsPIC flash stores only the 24-bit
    instruction; the phantom byte is not writable and always reads back as
    0x00.  The on-target CRC computation (compute_checksum in
    bootloader_drivers_openlcb.c) therefore always sees 0x00 at every
    phantom position.

    To make hex2bin's CRC match the on-target CRC, this function creates a
    corrected copy of the binary where every 4th byte (indices 3, 7, 11, ...)
    is forced to 0x00.

    IMPORTANT: This corrected copy is used ONLY for CRC computation.  The
    output .bin file retains the original fill bytes because the upload tool
    sends raw binary bytes to write_flash_bytes(), which extracts only the
    3 data bytes per instruction and discards byte[3] anyway.

    Args:
        data: The binary image as bytes or bytearray.

    Returns:
        A new bytes object with phantom bytes zeroed.
    """
    buf = bytearray(data)
    for i in range(3, len(buf), 4):
        buf[i] = 0x00
    return bytes(buf)


# =============================================================================
# Main
# =============================================================================

def main():
    arch_list = ', '.join(f"'{k}'" for k in ARCH_DEFS)

    parser = argparse.ArgumentParser(
        description='Convert Intel HEX to flat binary for bootloading.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=f'Supported architectures: {arch_list}')
    parser.add_argument('input', help='Input Intel HEX file (.hex)')
    parser.add_argument('output', help='Output binary file (.bin)')
    parser.add_argument('--arch', choices=ARCH_DEFS.keys(), default='flat',
                        help="Target architecture (default: 'flat')")
    parser.add_argument('--start', type=lambda x: int(x, 0), default=None,
                        help='Start PC address (arch-specific default '
                             'if omitted)')
    parser.add_argument('--end', type=lambda x: int(x, 0), default=None,
                        help='End PC address, exclusive (default: auto '
                             'from data)')
    parser.add_argument('--trim', action='store_true', default=False,
                        help='Strip trailing fill bytes from end of output '
                             '(rounded up to 8-byte boundary)')
    parser.add_argument('--fill', type=lambda x: int(x, 0), default=0xFF,
                        help='Fill byte for gaps (default: 0xFF)')
    parser.add_argument('--app-header-offset', type=lambda x: int(x, 0),
                        default=None,
                        help='Byte offset of bootloader_app_header_t from '
                             'start of binary (enables checksum patching)')
    parser.add_argument('--no-checksum', action='store_true', default=False,
                        help='Skip checksum computation; output binary as-is')
    args = parser.parse_args()

    # -- Validate arguments --------------------------------------------------

    if not args.no_checksum and args.app_header_offset is None:
        print("ERROR: --app-header-offset is required unless --no-checksum "
              "is given", file=sys.stderr)
        sys.exit(1)

    arch = ARCH_DEFS[args.arch]
    mult = arch['multiplier']

    # -- Resolve start address (PC -> hex-file byte address) -----------------

    if args.start is not None:
        start_pc = args.start
    elif arch['default_start'] is not None:
        start_pc = arch['default_start']
    else:
        print("ERROR: --start is required for --arch flat", file=sys.stderr)
        sys.exit(1)

    start_hex = start_pc * mult

    # -- Parse hex file ------------------------------------------------------

    data = parse_hex(args.input)

    if not data:
        print("ERROR: no data records found in hex file", file=sys.stderr)
        sys.exit(1)

    # -- Resolve end address (PC -> hex-file byte address) -------------------
    #
    # If --end is not given, auto-detect from the highest hex address that
    # is at or above start_hex, then round up to an 8-byte boundary.  The
    # 8-byte rounding ensures the binary length is aligned to the largest
    # common flash write granularity (dsPIC WriteDoubleWord24 = 8 image
    # bytes, STM32 doubleword = 8 bytes).

    if args.end is not None:
        end_hex = args.end * mult
    else:
        addrs_in_range = [a for a in data if a >= start_hex]
        if not addrs_in_range:
            print(f"ERROR: no data at or above start address "
                  f"0x{start_hex:X} (PC 0x{start_pc:X})", file=sys.stderr)
            sys.exit(1)
        end_hex = max(addrs_in_range) + 1
        end_hex = (end_hex + 7) & ~7

    # -- Extract the binary --------------------------------------------------

    buf = hex_to_bin(data, start_hex, end_hex, args.fill)

    # -- Trim trailing fill bytes --------------------------------------------
    #
    # Strips trailing bytes that equal the fill value, then rounds up the
    # length to an 8-byte boundary.  This reduces the image size (and
    # therefore the upload time) when the hex file includes large erased
    # regions (e.g. dsPIC config words far above the code).
    #
    # Trimming happens BEFORE checksum computation so that app_size in the
    # header reflects the actual image length the bootloader will verify.

    if args.trim:
        last_data = 0
        for i in range(len(buf) - 1, -1, -1):
            if buf[i] != args.fill:
                last_data = i + 1
                break
        # Round up to 8-byte boundary for flash write alignment.
        last_data = (last_data + 7) & ~7
        if last_data < len(buf):
            buf = buf[:last_data]

    # -- Checksum patching ---------------------------------------------------
    #
    # Computes triple CRC-16-IBM over the pre-header and post-header regions
    # of the binary, then writes the results into the app_header struct.
    #
    # For dsPIC targets, the CRC is computed over a phantom-corrected copy
    # of the binary (every 4th byte forced to 0x00) to match what the
    # on-target hardware reads back.  See _dspic_fix_phantom_bytes().
    #
    # For flat targets, the CRC is computed directly over the binary bytes
    # since flash is memory-mapped and reads back identically.

    patched = False
    if not args.no_checksum:
        hdr_off = args.app_header_offset
        buf = bytearray(buf)  # make mutable for header patching

        # Validate the header offset is within the binary.
        if hdr_off < 0 or hdr_off + APP_HEADER_SIZE > len(buf):
            print(f"ERROR: --app-header-offset 0x{hdr_off:X} is out of "
                  f"bounds (binary size = {len(buf)} bytes)", file=sys.stderr)
            sys.exit(1)

        # Safety check: the header region must be all zeros, indicating it
        # has not been patched yet.  A non-zero region means either the
        # offset is wrong or the binary was already patched by a previous
        # run.
        hdr_region = buf[hdr_off:hdr_off + APP_HEADER_SIZE]
        if any(b != 0x00 for b in hdr_region):
            print(f"ERROR: the {APP_HEADER_SIZE}-byte region at offset "
                  f"0x{hdr_off:X} is not all zeros -- wrong offset or "
                  f"already patched?", file=sys.stderr)
            sys.exit(1)

        # The total image size written into the header.  The bootloader's
        # finalize_flash() uses this to determine the extent of the post-
        # header CRC region.
        app_size = len(buf)

        # Choose the byte stream for CRC computation.
        #
        # dsPIC: use a phantom-corrected copy where every 4th byte is 0x00.
        #   The dsPIC hardware stores only 24-bit instructions; the phantom
        #   byte always reads back as 0x00 regardless of what was in the
        #   binary.  The on-target compute_checksum() feeds 4 bytes per
        #   instruction through CRC (3 data + phantom 0x00), so we must
        #   match that here.
        #
        # flat:  use the binary as-is.  Flash is memory-mapped; CRC of the
        #   binary bytes matches CRC of the flash contents exactly.
        if args.arch == 'dspic':
            crc_buf = _dspic_fix_phantom_bytes(buf)
        else:
            crc_buf = bytes(buf)

        # Pre-header region: binary[0 .. hdr_off)
        pre_data = crc_buf[:hdr_off]
        pre_crc = crc3_crc16_ibm(pre_data)

        # Post-header region: binary[hdr_off + 36 .. end)
        post_data = crc_buf[hdr_off + APP_HEADER_SIZE:]
        post_crc = crc3_crc16_ibm(post_data)

        # Pack the 36-byte header struct (little-endian):
        #   uint32_t app_size
        #   uint32_t checksum_pre[4]   (3 CRCs + 1 zero pad)
        #   uint32_t checksum_post[4]  (3 CRCs + 1 zero pad)
        header = struct.pack('<I IIII IIII',
                             app_size,
                             pre_crc[0], pre_crc[1], pre_crc[2], 0,
                             post_crc[0], post_crc[1], post_crc[2], 0)

        # Patch the header into the binary.
        buf[hdr_off:hdr_off + APP_HEADER_SIZE] = header
        buf = bytes(buf)
        patched = True

    # -- Write the output binary ---------------------------------------------

    with open(args.output, 'wb') as f:
        f.write(buf)

    # -- Summary output ------------------------------------------------------

    end_pc = end_hex // mult
    full_size = end_hex - start_hex
    print(f"Converted {args.input} -> {args.output}")
    print(f"  Architecture: {args.arch} ({arch['description']})")
    print(f"  PC range: 0x{start_pc:06X} .. 0x{end_pc:06X}")
    if mult > 1:
        print(f"  Hex address range: 0x{start_hex:06X} .. 0x{end_hex:06X}")
    print(f"  Output size: {len(buf)} bytes ({len(buf):#x})")
    if args.trim and len(buf) < full_size:
        print(f"  Trimmed from {full_size} bytes ({full_size:#x})")

    if patched:
        print(f"  App header offset: 0x{hdr_off:X} (byte {hdr_off})")
        print(f"  App size: {app_size} bytes ({app_size:#x})")
        if args.arch == 'dspic':
            print(f"  Phantom byte correction: applied (dsPIC mode)")
        print(f"  Pre-checksum:  [0x{pre_crc[0]:04X}, 0x{pre_crc[1]:04X}, "
              f"0x{pre_crc[2]:04X}, 0x0000]")
        print(f"  Post-checksum: [0x{post_crc[0]:04X}, 0x{post_crc[1]:04X}, "
              f"0x{post_crc[2]:04X}, 0x0000]")
    elif args.no_checksum:
        print(f"  Checksum: skipped (--no-checksum)")


if __name__ == '__main__':
    main()

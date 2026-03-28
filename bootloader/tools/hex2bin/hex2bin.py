#!/usr/bin/env python3
"""
hex2bin.py -- Convert Intel HEX to a flat binary for bootloading.

Supports two architectures:
  --arch dspic  : dsPIC/PIC24 -- PC addresses are half the hex file addresses.
                  --start and --end accept PC addresses; the script multiplies
                  by 2 internally.  (xc16-objcopy -O binary is broken for these
                  parts because VMA=PC but each instruction is 4 bytes, causing
                  section overlap.)
  --arch flat   : ARM, ESP32, RISC-V, etc. -- byte-addressable, PC = hex address.
                  --start and --end are used directly.  (Default)

Checksum patching (default when --app-header-offset is given):
  Computes triple CRC-16-IBM checksums and patches the bootloader_app_header_t
  struct in the output binary.  Use --no-checksum to skip patching.

Usage:
    python3 hex2bin.py [options] input.hex output.bin

Examples:
    # dsPIC33EP512MC506 -- addresses are what you see in the linker map
    python3 hex2bin.py --arch dspic --start 0x8000 --end 0x0547FC app.hex app.bin

    # STM32 with checksum patching
    python3 hex2bin.py --arch flat --start 0x08008000 --app-header-offset 0x200 app.hex app.bin

    # MSPM0 with checksum patching
    python3 hex2bin.py --arch flat --start 0x00003C00 --app-header-offset 0xC0 app.hex app.bin

    # dsPIC with checksum patching
    python3 hex2bin.py --arch dspic --start 0x4000 --app-header-offset 8 app.hex app.bin

    # Without checksum (legacy behavior)
    python3 hex2bin.py --arch dspic --start 0x4000 --no-checksum app.hex app.bin

The output .bin contains only the application region (no skip bytes needed).
"""

import argparse
import struct
import sys


# =========================================================================
# CRC-16-IBM (ANSI) -- ported from bootloader/src/crc/bootloader_crc.c
# =========================================================================

# Nibble lookup tables (64 bytes total).  Polynomial 0xA001 (reflected).
_CRC16_IBM_HI = (
    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
)

_CRC16_IBM_LO = (
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
)


def _crc16_ibm_add(state, byte):
    """Append one byte to the CRC-16-IBM state, return new state."""
    state ^= byte
    state = (state >> 8) ^ _CRC16_IBM_LO[state & 0x0F] ^ \
            _CRC16_IBM_HI[(state >> 4) & 0x0F]
    return state & 0xFFFF


def crc16_ibm(data):
    """Compute CRC-16-IBM over a bytes-like object.  Returns uint16."""
    state = 0x0000
    for b in data:
        state = _crc16_ibm_add(state, b)
    return state


def crc3_crc16_ibm(data):
    """Compute triple CRC-16-IBM (all, odd, even).

    Returns (crc_all, crc_odd, crc_even) matching
    BootloaderCrc_crc3_crc16_ibm() in the C library.

    Odd/even use 1-based indexing: byte 1 is odd, byte 2 is even, etc.
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


# Size of bootloader_app_header_t: uint32 + uint32[4] + uint32[4] = 36 bytes
APP_HEADER_SIZE = 36


# Architecture definitions: (pc_multiplier, default_start_pc, description)
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


def parse_hex(path):
    """Parse Intel HEX file, return dict of {hex_address: byte_value}."""
    data = {}
    base_address = 0

    with open(path, 'r') as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line or line[0] != ':':
                continue

            # Decode fields
            raw = bytes.fromhex(line[1:])
            byte_count = raw[0]
            address = (raw[1] << 8) | raw[2]
            record_type = raw[3]
            payload = raw[4:4 + byte_count]
            checksum = raw[4 + byte_count]

            # Verify checksum
            total = sum(raw[:-1]) & 0xFF
            if (total + checksum) & 0xFF != 0:
                print(f"WARNING: bad checksum on line {line_num}", file=sys.stderr)

            if record_type == 0x00:  # Data record
                full_addr = base_address + address
                for i, b in enumerate(payload):
                    data[full_addr + i] = b

            elif record_type == 0x01:  # EOF
                break

            elif record_type == 0x04:  # Extended Linear Address
                base_address = ((payload[0] << 8) | payload[1]) << 16

            # Ignore other record types (02, 03, 05)

    return data


def hex_to_bin(data, start_addr, end_addr, fill=0xFF):
    """Extract a contiguous region from the hex data as bytes."""
    size = end_addr - start_addr
    buf = bytearray([fill] * size)

    for addr, val in data.items():
        if start_addr <= addr < end_addr:
            buf[addr - start_addr] = val

    return bytes(buf)


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
                        help='Start PC address (arch-specific default if omitted)')
    parser.add_argument('--end', type=lambda x: int(x, 0), default=None,
                        help='End PC address, exclusive (default: auto from data)')
    parser.add_argument('--trim', action='store_true', default=False,
                        help='Strip trailing 0xFF bytes from end of output '
                             '(within --start/--end bounds)')
    parser.add_argument('--fill', type=lambda x: int(x, 0), default=0xFF,
                        help='Fill byte for gaps (default: 0xFF)')
    parser.add_argument('--app-header-offset', type=lambda x: int(x, 0),
                        default=None,
                        help='Byte offset of bootloader_app_header_t from '
                             'start of binary (enables checksum patching)')
    parser.add_argument('--no-checksum', action='store_true', default=False,
                        help='Skip checksum computation; output binary as-is')
    args = parser.parse_args()

    # Validate checksum arguments
    if not args.no_checksum and args.app_header_offset is None:
        print("ERROR: --app-header-offset is required unless --no-checksum "
              "is given", file=sys.stderr)
        sys.exit(1)

    arch = ARCH_DEFS[args.arch]
    mult = arch['multiplier']

    # Resolve start address (PC -> hex address)
    if args.start is not None:
        start_pc = args.start
    elif arch['default_start'] is not None:
        start_pc = arch['default_start']
    else:
        print("ERROR: --start is required for --arch flat", file=sys.stderr)
        sys.exit(1)

    start_hex = start_pc * mult

    # Parse hex file
    data = parse_hex(args.input)

    if not data:
        print("ERROR: no data records found in hex file", file=sys.stderr)
        sys.exit(1)

    # Resolve end address (PC -> hex address)
    if args.end is not None:
        end_hex = args.end * mult
    else:
        # Auto: highest address in data that is >= start, rounded up to 8-byte align
        addrs_in_range = [a for a in data if a >= start_hex]
        if not addrs_in_range:
            print(f"ERROR: no data at or above start address "
                  f"0x{start_hex:X} (PC 0x{start_pc:X})", file=sys.stderr)
            sys.exit(1)
        end_hex = max(addrs_in_range) + 1
        # Round up to 8-byte boundary (double-word write alignment)
        end_hex = (end_hex + 7) & ~7

    buf = hex_to_bin(data, start_hex, end_hex, args.fill)

    # --trim: strip trailing fill bytes, keep at least 1 byte,
    # round up to 8-byte alignment for flash write compatibility
    if args.trim:
        last_data = 0
        for i in range(len(buf) - 1, -1, -1):
            if buf[i] != args.fill:
                last_data = i + 1
                break
        # Round up to 8-byte boundary
        last_data = (last_data + 7) & ~7
        if last_data < len(buf):
            buf = buf[:last_data]

    # Checksum patching
    patched = False
    if not args.no_checksum:
        hdr_off = args.app_header_offset
        buf = bytearray(buf)  # make mutable

        # Validate offset
        if hdr_off < 0 or hdr_off + APP_HEADER_SIZE > len(buf):
            print(f"ERROR: --app-header-offset 0x{hdr_off:X} is out of "
                  f"bounds (binary size = {len(buf)} bytes)", file=sys.stderr)
            sys.exit(1)

        # Safety: the header region should be all zeros (not yet patched)
        hdr_region = buf[hdr_off:hdr_off + APP_HEADER_SIZE]
        if any(b != 0x00 for b in hdr_region):
            print(f"ERROR: the {APP_HEADER_SIZE}-byte region at offset "
                  f"0x{hdr_off:X} is not all zeros -- wrong offset or "
                  f"already patched?", file=sys.stderr)
            sys.exit(1)

        app_size = len(buf)

        # Pre-region: binary[0 .. hdr_off)
        pre_data = bytes(buf[:hdr_off])
        pre_crc = crc3_crc16_ibm(pre_data)

        # Post-region: binary[hdr_off + 36 .. end)
        post_data = bytes(buf[hdr_off + APP_HEADER_SIZE:])
        post_crc = crc3_crc16_ibm(post_data)

        # Pack header: app_size, checksum_pre[4], checksum_post[4]
        # Each CRC16 stored as uint32, 4th element is 0
        header = struct.pack('<I IIII IIII',
                             app_size,
                             pre_crc[0], pre_crc[1], pre_crc[2], 0,
                             post_crc[0], post_crc[1], post_crc[2], 0)

        buf[hdr_off:hdr_off + APP_HEADER_SIZE] = header
        buf = bytes(buf)
        patched = True

    with open(args.output, 'wb') as f:
        f.write(buf)

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
        print(f"  Pre-checksum:  [0x{pre_crc[0]:04X}, 0x{pre_crc[1]:04X}, "
              f"0x{pre_crc[2]:04X}, 0x0000]")
        print(f"  Post-checksum: [0x{post_crc[0]:04X}, 0x{post_crc[1]:04X}, "
              f"0x{post_crc[2]:04X}, 0x0000]")
    elif args.no_checksum:
        print(f"  Checksum: skipped (--no-checksum)")


if __name__ == '__main__':
    main()

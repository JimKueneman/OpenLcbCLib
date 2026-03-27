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

Usage:
    python3 hex2bin.py [options] input.hex output.bin

Examples:
    # dsPIC33EP512MC506 -- addresses are what you see in the linker map
    python3 hex2bin.py --arch dspic --start 0x8000 --end 0x0547FC app.hex app.bin

    # STM32 -- flat byte addressing
    python3 hex2bin.py --arch flat --start 0x08008000 app.hex app.bin

    # dsPIC with defaults (start=0x8000)
    python3 hex2bin.py --arch dspic app.hex app.bin

The output .bin contains only the application region (no skip bytes needed).
"""

import argparse
import sys


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
    args = parser.parse_args()

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


if __name__ == '__main__':
    main()

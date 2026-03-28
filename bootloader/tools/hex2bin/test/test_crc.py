#!/usr/bin/env python3
"""
Test vectors for the CRC-16-IBM implementation in hex2bin.py.

Run:  python3 -m pytest test/test_crc.py  (from the hex2bin directory)
  or: python3 test/test_crc.py            (standalone)
"""

import struct
import sys
import os

# Add parent directory so we can import hex2bin
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from hex2bin import crc16_ibm, crc3_crc16_ibm, APP_HEADER_SIZE


def test_crc16_ibm_standard_vector():
    """CRC-16-IBM test vector from the header: "123456789" -> 0xBB3D."""
    data = b"123456789"
    result = crc16_ibm(data)
    assert result == 0xBB3D, f"Expected 0xBB3D, got 0x{result:04X}"


def test_crc16_ibm_empty():
    """Empty input should return init value 0x0000."""
    assert crc16_ibm(b"") == 0x0000


def test_crc16_ibm_single_byte():
    """Single byte 0x01: known value for sanity check."""
    result = crc16_ibm(b"\x01")
    # Computed by hand: state = 0x0000 ^ 0x01 = 0x0001
    # low nibble 0x1 -> LO[1] = 0xC0C1, high nibble 0x0 -> HI[0] = 0x0000
    # result = (0x0001 >> 8) ^ 0xC0C1 ^ 0x0000 = 0x0000 ^ 0xC0C1 = 0xC0C1
    assert result == 0xC0C1, f"Expected 0xC0C1, got 0x{result:04X}"


def test_crc3_basic():
    """Triple CRC of a 4-byte sequence.  Verify all/odd/even separation."""
    data = b"\x01\x02\x03\x04"
    crc_all, crc_odd, crc_even = crc3_crc16_ibm(data)

    # crc_all should equal crc16_ibm of the full data
    assert crc_all == crc16_ibm(data), "crc_all must match single CRC"

    # crc_odd = CRC of bytes at 1-based odd positions: bytes[0], bytes[2]
    odd_bytes = bytes([data[0], data[2]])
    assert crc_odd == crc16_ibm(odd_bytes), "crc_odd must match CRC of odd-position bytes"

    # crc_even = CRC of bytes at 1-based even positions: bytes[1], bytes[3]
    even_bytes = bytes([data[1], data[3]])
    assert crc_even == crc16_ibm(even_bytes), "crc_even must match CRC of even-position bytes"


def test_crc3_standard_vector():
    """Triple CRC of "123456789"."""
    data = b"123456789"
    crc_all, crc_odd, crc_even = crc3_crc16_ibm(data)
    assert crc_all == 0xBB3D, f"crc_all expected 0xBB3D, got 0x{crc_all:04X}"

    # Odd bytes (1-based): '1','3','5','7','9'
    odd_data = b"13579"
    assert crc_odd == crc16_ibm(odd_data), "crc_odd mismatch for '123456789'"

    # Even bytes (1-based): '2','4','6','8'
    even_data = b"2468"
    assert crc_even == crc16_ibm(even_data), "crc_even mismatch for '123456789'"


def test_header_round_trip():
    """Build a fake binary, patch it, read back and verify checksums match."""
    # Create a 256-byte binary with known data and a zeroed header at offset 64
    hdr_off = 64
    pre_data = bytes(range(hdr_off))  # 0x00..0x3F
    post_data = bytes((i & 0xFF) for i in range(256 - hdr_off - APP_HEADER_SIZE))

    # Build the unpacked binary with zeroed header
    buf = bytearray(pre_data + b'\x00' * APP_HEADER_SIZE + post_data)
    app_size = len(buf)

    # Compute checksums
    pre_crc = crc3_crc16_ibm(pre_data)
    post_crc = crc3_crc16_ibm(post_data)

    # Pack and patch
    header = struct.pack('<I IIII IIII',
                         app_size,
                         pre_crc[0], pre_crc[1], pre_crc[2], 0,
                         post_crc[0], post_crc[1], post_crc[2], 0)
    buf[hdr_off:hdr_off + APP_HEADER_SIZE] = header

    # Read back and verify
    read_back = struct.unpack('<I IIII IIII', bytes(buf[hdr_off:hdr_off + APP_HEADER_SIZE]))
    assert read_back[0] == app_size, "app_size mismatch"
    assert read_back[1] == pre_crc[0], "pre crc_all mismatch"
    assert read_back[2] == pre_crc[1], "pre crc_odd mismatch"
    assert read_back[3] == pre_crc[2], "pre crc_even mismatch"
    assert read_back[4] == 0, "pre checksum[3] should be 0"
    assert read_back[5] == post_crc[0], "post crc_all mismatch"
    assert read_back[6] == post_crc[1], "post crc_odd mismatch"
    assert read_back[7] == post_crc[2], "post crc_even mismatch"
    assert read_back[8] == 0, "post checksum[3] should be 0"

    # Recompute from the patched binary and verify
    re_pre = crc3_crc16_ibm(bytes(buf[:hdr_off]))
    re_post = crc3_crc16_ibm(bytes(buf[hdr_off + APP_HEADER_SIZE:]))
    assert re_pre == pre_crc, "recomputed pre-checksum does not match"
    assert re_post == post_crc, "recomputed post-checksum does not match"


def test_app_header_size():
    """Verify APP_HEADER_SIZE matches the struct layout."""
    # uint32 + uint32[4] + uint32[4] = 4 + 16 + 16 = 36
    assert APP_HEADER_SIZE == 36


# =========================================================================
# dsPIC phantom-byte round-trip tests
#
# These tests simulate the full dsPIC checksum pipeline:
#   1. hex2bin writes 36 flat header bytes into the binary (same as flat)
#   2. write_flash_bytes stores 3 data bytes per instruction, discards byte[3]
#   3. _read_flash_image_bytes reads 4 bytes per instruction (3 data + 0x00)
#   4. compute_checksum CRCs 4 bytes per instruction (matching the binary)
#
# The key property: every uint32_t in the header has MSB = 0x00, so the
# phantom byte (always 0x00) lands where the struct already has 0x00.
# =========================================================================

def _simulate_dspic_write(binary_data):
    """Simulate write_flash_bytes: store 3 data bytes per instruction,
    discard the phantom (byte[3] of each 4-byte group).

    Returns a list of 24-bit instruction words (as uint32_t with bits[23:0]).
    """
    instructions = []
    for i in range(0, len(binary_data), 4):
        chunk = binary_data[i:i + 4]
        if len(chunk) < 3:
            break
        word = chunk[0] | (chunk[1] << 8) | (chunk[2] << 16)
        instructions.append(word)
    return instructions


def _simulate_dspic_read_image_bytes(instructions, count):
    """Simulate _read_flash_image_bytes: read 4 bytes per instruction
    (3 data bytes + phantom 0x00).

    This is the new reader added for checksum support.
    """
    result = bytearray()
    idx = 0
    while len(result) < count and idx < len(instructions):
        word = instructions[idx]
        result.append(word & 0xFF)
        result.append((word >> 8) & 0xFF)
        result.append((word >> 16) & 0xFF)
        result.append(0x00)  # phantom byte
        idx += 1
    return bytes(result[:count])


def _simulate_dspic_read_flash_bytes(instructions, count):
    """Simulate the existing read_flash_bytes: read 3 data bytes per
    instruction (no phantom).  Used for the blank-flash check.
    """
    result = bytearray()
    idx = 0
    while len(result) < count and idx < len(instructions):
        word = instructions[idx]
        result.append(word & 0xFF)
        result.append((word >> 8) & 0xFF)
        result.append((word >> 16) & 0xFF)
        idx += 1
    return bytes(result[:count])


def _simulate_dspic_compute_checksum(instructions, num_binary_bytes):
    """Simulate compute_checksum: CRC 4 bytes per instruction (3 data +
    phantom 0x00), matching the binary image format.
    """
    from hex2bin import _crc16_ibm_add

    state_all = 0x0000
    state_odd = 0x0000
    state_even = 0x0000
    byte_index = 0

    num_instructions = num_binary_bytes // 4
    for i in range(num_instructions):
        word = instructions[i]
        for shift in (0, 8, 16):
            b = (word >> shift) & 0xFF
            byte_index += 1
            state_all = _crc16_ibm_add(state_all, b)
            if byte_index & 1:
                state_odd = _crc16_ibm_add(state_odd, b)
            else:
                state_even = _crc16_ibm_add(state_even, b)

        # phantom byte (0x00)
        byte_index += 1
        state_all = _crc16_ibm_add(state_all, 0x00)
        if byte_index & 1:
            state_odd = _crc16_ibm_add(state_odd, 0x00)
        else:
            state_even = _crc16_ibm_add(state_even, 0x00)

    return (state_all, state_odd, state_even)


def test_dspic_phantom_byte_alignment():
    """Verify that every uint32_t in the header struct has byte[3] == 0x00,
    so the phantom byte lands on a naturally-zero position.
    """
    # Simulate realistic header values
    app_size = 0x0000A8F8   # typical dsPIC image size (< 16 MB)
    pre_crc = (0xBB3D, 0x1234, 0x5678)   # CRC-16 values (< 0x10000)
    post_crc = (0xABCD, 0x9876, 0x5432)

    header = struct.pack('<I IIII IIII',
                         app_size,
                         pre_crc[0], pre_crc[1], pre_crc[2], 0,
                         post_crc[0], post_crc[1], post_crc[2], 0)

    # Verify every 4th byte (phantom position) is 0x00
    for i in range(3, len(header), 4):
        assert header[i] == 0x00, (
            f"byte[{i}] = 0x{header[i]:02X}, expected 0x00 (phantom position)")


def test_dspic_header_write_read_round_trip():
    """Full round-trip: pack header -> write to flash -> read back with
    _read_flash_image_bytes -> verify struct matches original.
    """
    app_size = 0x0000A8F8
    pre_crc = (0xBB3D, 0x1234, 0x5678)
    post_crc = (0xABCD, 0x9876, 0x5432)

    # Pack the 36-byte header (same as hex2bin does for all platforms)
    header_bytes = struct.pack('<I IIII IIII',
                               app_size,
                               pre_crc[0], pre_crc[1], pre_crc[2], 0,
                               post_crc[0], post_crc[1], post_crc[2], 0)
    assert len(header_bytes) == 36

    # Write to simulated flash (discards phantom bytes)
    flash = _simulate_dspic_write(header_bytes)
    assert len(flash) == 9, f"Expected 9 instructions, got {len(flash)}"

    # Read back with _read_flash_image_bytes (4 bytes per instruction)
    read_back = _simulate_dspic_read_image_bytes(flash, 36)
    assert len(read_back) == 36

    # Unpack and verify every field matches
    fields = struct.unpack('<I IIII IIII', read_back)
    assert fields[0] == app_size, f"app_size: expected 0x{app_size:08X}, got 0x{fields[0]:08X}"
    assert fields[1] == pre_crc[0], f"pre[0]: expected 0x{pre_crc[0]:04X}, got 0x{fields[1]:04X}"
    assert fields[2] == pre_crc[1], f"pre[1]: expected 0x{pre_crc[1]:04X}, got 0x{fields[2]:04X}"
    assert fields[3] == pre_crc[2], f"pre[2]: expected 0x{pre_crc[2]:04X}, got 0x{fields[3]:04X}"
    assert fields[4] == 0, "pre[3] should be 0"
    assert fields[5] == post_crc[0], f"post[0]: expected 0x{post_crc[0]:04X}, got 0x{fields[5]:04X}"
    assert fields[6] == post_crc[1], f"post[1]: expected 0x{post_crc[1]:04X}, got 0x{fields[6]:04X}"
    assert fields[7] == post_crc[2], f"post[2]: expected 0x{post_crc[2]:04X}, got 0x{fields[7]:04X}"
    assert fields[8] == 0, "post[3] should be 0"


def test_dspic_full_checksum_pipeline():
    """End-to-end: build a dsPIC-style binary with phantom bytes,
    patch the header via hex2bin logic, simulate the on-target
    finalize_flash verification, confirm checksums match.
    """
    # Binary layout (dsPIC style, 4 bytes per instruction):
    #   [0..7]   = pre-region (2 instructions, e.g. reset GOTO)
    #   [8..43]  = app_header (9 instructions, 36 binary bytes)
    #   [44..]   = post-region (application code)
    hdr_off = 8

    # Build pre-region: 2 instructions with known data + phantom 0x00
    pre_binary = bytearray()
    pre_binary += bytes([0x04, 0x40, 0x00, 0x00])  # GOTO 0x4000 (low word)
    pre_binary += bytes([0x00, 0x00, 0x00, 0x00])  # GOTO high word + phantom

    # Build post-region: some application code (20 instructions = 80 bytes)
    post_binary = bytearray()
    for i in range(20):
        # Each instruction: 3 data bytes + phantom 0x00
        post_binary += bytes([
            (i * 3) & 0xFF,
            (i * 3 + 1) & 0xFF,
            (i * 3 + 2) & 0xFF,
            0x00
        ])

    # Assemble the full binary with zeroed header
    buf = bytearray(pre_binary + b'\x00' * APP_HEADER_SIZE + post_binary)
    app_size = len(buf)

    # ---- hex2bin side: compute and patch checksums ----
    pre_data = bytes(buf[:hdr_off])
    post_data = bytes(buf[hdr_off + APP_HEADER_SIZE:])

    pre_crc = crc3_crc16_ibm(pre_data)
    post_crc = crc3_crc16_ibm(post_data)

    header = struct.pack('<I IIII IIII',
                         app_size,
                         pre_crc[0], pre_crc[1], pre_crc[2], 0,
                         post_crc[0], post_crc[1], post_crc[2], 0)
    buf[hdr_off:hdr_off + APP_HEADER_SIZE] = header

    # ---- Simulate on-target: write entire binary to flash ----
    flash = _simulate_dspic_write(bytes(buf))
    total_instructions = len(buf) // 4
    assert len(flash) == total_instructions

    # ---- Simulate on-target: finalize_flash reads header ----
    # Header starts at instruction index hdr_off // 4 = 2
    hdr_instr_start = hdr_off // 4
    hdr_instructions = flash[hdr_instr_start:hdr_instr_start + 9]
    header_read_back = _simulate_dspic_read_image_bytes(hdr_instructions, 36)
    fields = struct.unpack('<I IIII IIII', header_read_back)

    read_app_size = fields[0]
    read_pre_crc = (fields[1], fields[2], fields[3])
    read_post_crc = (fields[5], fields[6], fields[7])

    assert read_app_size == app_size, (
        f"app_size: expected {app_size}, got {read_app_size}")

    # ---- Simulate on-target: compute_checksum over pre-region ----
    pre_instructions = flash[:hdr_instr_start]
    pre_size_binary = hdr_off  # 8 binary bytes
    computed_pre = _simulate_dspic_compute_checksum(pre_instructions, pre_size_binary)

    assert computed_pre[0] == read_pre_crc[0], (
        f"pre crc_all: expected 0x{read_pre_crc[0]:04X}, got 0x{computed_pre[0]:04X}")
    assert computed_pre[1] == read_pre_crc[1], (
        f"pre crc_odd: expected 0x{read_pre_crc[1]:04X}, got 0x{computed_pre[1]:04X}")
    assert computed_pre[2] == read_pre_crc[2], (
        f"pre crc_even: expected 0x{read_pre_crc[2]:04X}, got 0x{computed_pre[2]:04X}")

    # ---- Simulate on-target: compute_checksum over post-region ----
    post_instr_start = hdr_instr_start + 9
    post_instructions = flash[post_instr_start:]
    post_size_binary = read_app_size - hdr_off - APP_HEADER_SIZE
    computed_post = _simulate_dspic_compute_checksum(post_instructions, post_size_binary)

    assert computed_post[0] == read_post_crc[0], (
        f"post crc_all: expected 0x{read_post_crc[0]:04X}, got 0x{computed_post[0]:04X}")
    assert computed_post[1] == read_post_crc[1], (
        f"post crc_odd: expected 0x{read_post_crc[1]:04X}, got 0x{computed_post[1]:04X}")
    assert computed_post[2] == read_post_crc[2], (
        f"post crc_even: expected 0x{read_post_crc[2]:04X}, got 0x{computed_post[2]:04X}")


def test_dspic_blank_flash_check():
    """Verify the existing read_flash_bytes (3 bytes per instruction)
    still works for the blank-flash check in bootloader.c.

    Erased dsPIC flash reads as 0xFFFFFF per instruction (24 bits).
    read_flash_bytes reads 3 bytes per instruction, so reading 4 bytes
    for a uint32_t reads from 2 instructions: 0xFF 0xFF 0xFF | 0xFF ...
    giving first_word = 0xFFFFFFFF.
    """
    # Simulate erased flash: all instructions = 0x00FFFFFF (24-bit all-ones)
    erased_flash = [0x00FFFFFF] * 4

    # read_flash_bytes reads 3 data bytes per instruction
    first_word_bytes = _simulate_dspic_read_flash_bytes(erased_flash, 4)
    first_word = struct.unpack('<I', first_word_bytes)[0]

    assert first_word == 0xFFFFFFFF, (
        f"Expected 0xFFFFFFFF for erased flash, got 0x{first_word:08X}")


def test_dspic_programmed_flash_not_blank():
    """Verify that programmed flash (e.g. GOTO instruction) does NOT
    look like 0xFFFFFFFF when read with read_flash_bytes.
    """
    # GOTO 0x004000 as first instruction
    programmed_flash = [0x004000, 0x000000]

    first_word_bytes = _simulate_dspic_read_flash_bytes(programmed_flash, 4)
    first_word = struct.unpack('<I', first_word_bytes)[0]

    assert first_word != 0xFFFFFFFF, (
        f"Programmed flash should not look blank, got 0x{first_word:08X}")


# =========================================================================
# Flat architecture (STM32, MSPM0) full pipeline test
#
# On Von Neumann targets, binary bytes = flash bytes = data bytes.
# write/read/checksum are all identity operations on the byte stream.
# This test proves the same hex2bin patching logic works for flat targets
# and catches regressions where a change breaks one path but not the other.
# =========================================================================

def _simulate_flat_compute_checksum(flash_bytes):
    """Simulate compute_checksum on a flat (Von Neumann) target.
    CRC is computed directly over the byte stream -- no phantom bytes.
    """
    return crc3_crc16_ibm(flash_bytes)


def test_flat_full_checksum_pipeline():
    """End-to-end for flat architectures (STM32, MSPM0):
    build a binary, patch header, simulate on-target finalize_flash.
    """
    # Binary layout (STM32 style):
    #   [0..0x1FF]   = pre-region (vector table, 512 bytes)
    #   [0x200..0x223] = app_header (36 bytes)
    #   [0x224..]    = post-region (application code)
    hdr_off = 0x200

    # Build pre-region: 512 bytes of vector table data
    pre_binary = bytearray()
    for i in range(hdr_off):
        pre_binary.append(i & 0xFF)

    # Build post-region: 256 bytes of application code
    post_binary = bytearray()
    for i in range(256):
        post_binary.append((i * 7 + 3) & 0xFF)

    # Assemble the full binary with zeroed header
    buf = bytearray(pre_binary + b'\x00' * APP_HEADER_SIZE + post_binary)
    app_size = len(buf)

    # ---- hex2bin side: compute and patch checksums ----
    pre_data = bytes(buf[:hdr_off])
    post_data = bytes(buf[hdr_off + APP_HEADER_SIZE:])

    pre_crc = crc3_crc16_ibm(pre_data)
    post_crc = crc3_crc16_ibm(post_data)

    header = struct.pack('<I IIII IIII',
                         app_size,
                         pre_crc[0], pre_crc[1], pre_crc[2], 0,
                         post_crc[0], post_crc[1], post_crc[2], 0)
    buf[hdr_off:hdr_off + APP_HEADER_SIZE] = header

    # ---- Simulate on-target: flash = binary (Von Neumann, identity) ----
    flash = bytes(buf)

    # ---- Simulate on-target: finalize_flash reads header via memcpy ----
    header_read_back = flash[hdr_off:hdr_off + APP_HEADER_SIZE]
    fields = struct.unpack('<I IIII IIII', header_read_back)

    read_app_size = fields[0]
    read_pre_crc = (fields[1], fields[2], fields[3])
    read_post_crc = (fields[5], fields[6], fields[7])

    assert read_app_size == app_size, (
        f"app_size: expected {app_size}, got {read_app_size}")

    # ---- Simulate on-target: compute_checksum over pre-region ----
    computed_pre = _simulate_flat_compute_checksum(flash[:hdr_off])

    assert computed_pre[0] == read_pre_crc[0], (
        f"pre crc_all: expected 0x{read_pre_crc[0]:04X}, got 0x{computed_pre[0]:04X}")
    assert computed_pre[1] == read_pre_crc[1], (
        f"pre crc_odd: expected 0x{read_pre_crc[1]:04X}, got 0x{computed_pre[1]:04X}")
    assert computed_pre[2] == read_pre_crc[2], (
        f"pre crc_even: expected 0x{read_pre_crc[2]:04X}, got 0x{computed_pre[2]:04X}")

    # ---- Simulate on-target: compute_checksum over post-region ----
    post_start = hdr_off + APP_HEADER_SIZE
    post_size = read_app_size - hdr_off - APP_HEADER_SIZE
    computed_post = _simulate_flat_compute_checksum(
        flash[post_start:post_start + post_size])

    assert computed_post[0] == read_post_crc[0], (
        f"post crc_all: expected 0x{read_post_crc[0]:04X}, got 0x{computed_post[0]:04X}")
    assert computed_post[1] == read_post_crc[1], (
        f"post crc_odd: expected 0x{read_post_crc[1]:04X}, got 0x{computed_post[1]:04X}")
    assert computed_post[2] == read_post_crc[2], (
        f"post crc_even: expected 0x{read_post_crc[2]:04X}, got 0x{computed_post[2]:04X}")


def test_flat_blank_flash_check():
    """Verify erased flash reads as 0xFFFFFFFF on flat targets."""
    erased = b'\xFF' * 4
    first_word = struct.unpack('<I', erased)[0]
    assert first_word == 0xFFFFFFFF


# Standalone runner
if __name__ == '__main__':
    tests = [v for k, v in sorted(globals().items()) if k.startswith('test_')]
    passed = 0
    failed = 0
    for test_fn in tests:
        name = test_fn.__name__
        try:
            test_fn()
            print(f"  PASS  {name}")
            passed += 1
        except AssertionError as e:
            print(f"  FAIL  {name}: {e}")
            failed += 1

    print(f"\n{passed} passed, {failed} failed")
    sys.exit(1 if failed else 0)

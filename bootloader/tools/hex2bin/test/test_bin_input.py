#!/usr/bin/env python3
"""
Tests for the --input-format bin feature and --arch esp32 in hex2bin.py.

Verifies that raw binary input produces the same checksum patching as
the Intel HEX path, that the ESP32 XOR checksum + SHA256 fixup works
correctly, and that edge cases are handled.

Run:  python3 -m pytest test/test_bin_input.py  (from the hex2bin directory)
  or: python3 test/test_bin_input.py            (standalone)
"""

import hashlib
import os
import struct
import subprocess
import sys
import tempfile

# Add parent directory so we can import hex2bin
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from hex2bin import crc3_crc16_ibm, APP_HEADER_SIZE

HEX2BIN = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    'hex2bin.py')

# ESP-IDF image constants (must match hex2bin.py)
ESP_IMAGE_HEADER_MAGIC = 0xE9
ESP_CHECKSUM_SEED = 0xEF
ESP_IMAGE_HEADER_SIZE = 24
ESP_SEGMENT_HEADER_SIZE = 8
ESP_SHA256_SIZE = 32


def _build_test_binary(hdr_off, pre_fill, post_size):
    """Build a test binary with a zeroed header region.

    Args:
        hdr_off:   byte offset for the app_header
        pre_fill:  callable(index) returning a byte value for pre-region
        post_size: number of post-region bytes

    Returns:
        bytearray with zeroed header at hdr_off
    """
    pre = bytearray(pre_fill(i) for i in range(hdr_off))
    header = b'\x00' * APP_HEADER_SIZE
    post = bytearray((i * 7 + 3) & 0xFF for i in range(post_size))
    return bytearray(pre + header + post)


def _build_esp32_image(hdr_off, code_size, append_digest):
    """Build a minimal valid ESP-IDF firmware.bin image.

    Creates a single-segment image with the DROM segment containing
    the app_header region (zeroed) and code data.  Includes a valid
    XOR checksum byte and optionally a SHA256 hash.

    Args:
        hdr_off:        byte offset for the app_header (e.g., 0x120)
        code_size:      number of bytes of code data after the header
        append_digest:  True to append SHA256 hash

    Returns:
        bytearray -- a valid ESP-IDF image
    """
    # Segment data = everything from offset 0x20 to the end of code.
    # The segment starts at file offset 0x20 (after image header + segment header).
    # We need: segment data length = total_data_size
    # where total_data_size includes esp_app_desc_t (256 bytes) + app_header + code

    # The app_header is at hdr_off in the FILE.  In the segment data,
    # that is at offset (hdr_off - 0x20) since segment data starts at 0x20.
    seg_data_start = ESP_IMAGE_HEADER_SIZE + ESP_SEGMENT_HEADER_SIZE  # 0x20
    seg_data_len = (hdr_off - seg_data_start) + APP_HEADER_SIZE + code_size

    # Build segment data
    seg_data = bytearray(seg_data_len)

    # Simulate esp_app_desc_t magic at start of segment data
    struct.pack_into('<I', seg_data, 0, 0xABCD5432)  # ESP_APP_DESC_MAGIC

    # Fill code region with non-zero data (after app_header in segment)
    code_start_in_seg = (hdr_off - seg_data_start) + APP_HEADER_SIZE
    for i in range(code_size):
        seg_data[code_start_in_seg + i] = ((i * 13 + 7) & 0xFF)

    # App header region stays zeroed (already is from bytearray init)

    # Build image header (24 bytes)
    img_header = bytearray(ESP_IMAGE_HEADER_SIZE)
    img_header[0] = ESP_IMAGE_HEADER_MAGIC  # magic
    img_header[1] = 1                       # segment_count = 1
    img_header[2] = 0x02                    # SPI mode
    img_header[3] = 0x0F                    # SPI speed/size
    # entry_addr at offset 4 (4 bytes) -- leave as 0
    # append_digest flag at offset 0x17
    img_header[0x17] = 1 if append_digest else 0

    # Build segment header (8 bytes): load_addr + data_len
    seg_header = bytearray(ESP_SEGMENT_HEADER_SIZE)
    struct.pack_into('<II', seg_header, 0, 0x3F400020, seg_data_len)

    # Assemble: header + segment header + segment data
    image = bytearray(img_header + seg_header + seg_data)

    # Pad to 16-byte boundary (accounting for the checksum byte)
    # The checksum byte goes at the position such that total length
    # is a multiple of 16.
    unpadded_len = len(image)
    padded_len = ((unpadded_len + 1 + 15) & ~15)
    pad_count = padded_len - unpadded_len - 1
    image.extend(b'\x00' * pad_count)

    # Compute XOR checksum over segment data
    checksum = ESP_CHECKSUM_SEED
    for b in seg_data:
        checksum ^= b
    image.append(checksum & 0xFF)

    # Optionally append SHA256
    if append_digest:
        sha = hashlib.sha256(bytes(image)).digest()
        image.extend(sha)

    return image


def _verify_esp32_checksum(image_bytes):
    """Verify the ESP-IDF XOR checksum in an image.

    Returns True if valid, False if not.
    """
    if len(image_bytes) < ESP_IMAGE_HEADER_SIZE:
        return False

    has_sha256 = (image_bytes[0x17] == 1)
    if has_sha256:
        body = image_bytes[:-ESP_SHA256_SIZE]
    else:
        body = image_bytes

    segment_count = body[1]
    checksum = ESP_CHECKSUM_SEED
    offset = ESP_IMAGE_HEADER_SIZE

    for _ in range(segment_count):
        if offset + ESP_SEGMENT_HEADER_SIZE > len(body):
            return False
        seg_data_len = struct.unpack_from('<I', body, offset + 4)[0]
        offset += ESP_SEGMENT_HEADER_SIZE
        for i in range(offset, offset + seg_data_len):
            checksum ^= body[i]
        offset += seg_data_len

    return (body[-1] == (checksum & 0xFF))


def _verify_esp32_sha256(image_bytes):
    """Verify the trailing SHA256 hash.

    Returns True if valid or if no hash is appended.
    """
    if image_bytes[0x17] != 1:
        return True  # no hash to verify

    body = image_bytes[:-ESP_SHA256_SIZE]
    expected = hashlib.sha256(bytes(body)).digest()
    actual = image_bytes[-ESP_SHA256_SIZE:]
    return (bytes(actual) == expected)


def _run_hex2bin(args, input_bytes=None, input_path=None):
    """Run hex2bin.py as a subprocess.

    Either input_bytes (written to a temp file) or input_path must be
    provided.  Returns (returncode, stdout, stderr, output_bytes).
    """
    with tempfile.NamedTemporaryFile(suffix='.bin', delete=False) as out_f:
        out_path = out_f.name

    try:
        if input_bytes is not None:
            with tempfile.NamedTemporaryFile(suffix='.bin', delete=False) as in_f:
                in_f.write(input_bytes)
                in_path = in_f.name
        else:
            in_path = input_path

        cmd = [sys.executable, HEX2BIN] + args + [in_path, out_path]
        result = subprocess.run(cmd, capture_output=True, text=True)

        output_bytes = b''
        if os.path.exists(out_path):
            with open(out_path, 'rb') as f:
                output_bytes = f.read()

        return result.returncode, result.stdout, result.stderr, output_bytes

    finally:
        for p in [out_path]:
            if os.path.exists(p):
                os.unlink(p)
        if input_bytes is not None and os.path.exists(in_path):
            os.unlink(in_path)


# =========================================================================
# Basic binary input tests
# =========================================================================

def test_bin_input_with_checksum():
    """Binary input with checksum patching produces valid header."""
    hdr_off = 0x120
    buf = _build_test_binary(hdr_off, lambda i: i & 0xFF, 256)

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--app-header-offset', hex(hdr_off)],
        input_bytes=bytes(buf))

    assert rc == 0, f"hex2bin failed: {stderr}"
    assert len(out) == len(buf), f"Output size {len(out)} != input size {len(buf)}"

    # Read back the patched header
    fields = struct.unpack('<I IIII IIII',
                           out[hdr_off:hdr_off + APP_HEADER_SIZE])

    app_size = fields[0]
    pre_crc_stored = (fields[1], fields[2], fields[3])
    post_crc_stored = (fields[5], fields[6], fields[7])

    assert app_size == len(buf), f"app_size {app_size} != {len(buf)}"

    # Recompute and verify pre-checksum
    pre_crc = crc3_crc16_ibm(out[:hdr_off])
    assert pre_crc == pre_crc_stored, (
        f"pre CRC mismatch: computed {pre_crc} != stored {pre_crc_stored}")

    # Recompute and verify post-checksum
    post_crc = crc3_crc16_ibm(out[hdr_off + APP_HEADER_SIZE:])
    assert post_crc == post_crc_stored, (
        f"post CRC mismatch: computed {post_crc} != stored {post_crc_stored}")


def test_bin_input_no_checksum():
    """Binary input with --no-checksum passes through unchanged."""
    buf = bytes(range(256))

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--no-checksum'],
        input_bytes=buf)

    assert rc == 0, f"hex2bin failed: {stderr}"
    assert out == buf, "Output should be identical to input with --no-checksum"


def test_bin_input_preserves_non_header_bytes():
    """Checksum patching only modifies the header region."""
    hdr_off = 0x40
    buf = _build_test_binary(hdr_off, lambda i: (i * 3) & 0xFF, 128)
    original = bytes(buf)

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--app-header-offset', hex(hdr_off)],
        input_bytes=bytes(buf))

    assert rc == 0, f"hex2bin failed: {stderr}"

    # Pre-region must be unchanged
    assert out[:hdr_off] == original[:hdr_off], "Pre-region was modified"

    # Post-region must be unchanged
    post_start = hdr_off + APP_HEADER_SIZE
    assert out[post_start:] == original[post_start:], "Post-region was modified"


def test_bin_input_summary_shows_raw_binary():
    """Summary output should indicate raw binary format."""
    hdr_off = 0x20
    buf = _build_test_binary(hdr_off, lambda i: 0, 64)

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--app-header-offset', hex(hdr_off)],
        input_bytes=bytes(buf))

    assert rc == 0, f"hex2bin failed: {stderr}"
    assert 'raw binary' in stdout.lower(), (
        f"Expected 'raw binary' in output, got: {stdout}")


# =========================================================================
# Binary input matches hex input
# =========================================================================

def test_bin_produces_same_checksums_as_hex():
    """Given the same flat binary content, --input-format bin and
    --input-format hex (from a constructed hex file) produce identical
    checksum values.
    """
    hdr_off = 0x40
    buf = _build_test_binary(hdr_off, lambda i: (i * 11) & 0xFF, 200)

    # Run via binary input
    rc1, _, stderr1, out_bin = _run_hex2bin(
        ['--input-format', 'bin', '--app-header-offset', hex(hdr_off)],
        input_bytes=bytes(buf))
    assert rc1 == 0, f"bin path failed: {stderr1}"

    # Build an Intel HEX file from the same content
    hex_lines = []
    base_addr = 0x08008000  # arbitrary flash start
    for offset in range(0, len(buf), 16):
        chunk = buf[offset:offset + 16]
        addr = (base_addr + offset) & 0xFFFF

        # Extended linear address record if needed
        upper = ((base_addr + offset) >> 16) & 0xFFFF
        if offset == 0 or ((base_addr + offset) & 0xFFFF) < 16:
            ela = bytes([0x02, 0x00, 0x00, 0x04,
                         (upper >> 8) & 0xFF, upper & 0xFF])
            csum = (~sum(ela) + 1) & 0xFF
            hex_lines.append(':' + (ela + bytes([csum])).hex().upper())

        record = bytes([len(chunk),
                        (addr >> 8) & 0xFF, addr & 0xFF,
                        0x00]) + chunk
        csum = (~sum(record) + 1) & 0xFF
        hex_lines.append(':' + (record + bytes([csum])).hex().upper())

    hex_lines.append(':00000001FF')  # EOF
    hex_content = '\n'.join(hex_lines) + '\n'

    with tempfile.NamedTemporaryFile(suffix='.hex', mode='w',
                                     delete=False) as hf:
        hf.write(hex_content)
        hex_path = hf.name

    try:
        rc2, _, stderr2, out_hex = _run_hex2bin(
            ['--input-format', 'hex', '--arch', 'flat',
             '--start', hex(base_addr),
             '--end', hex(base_addr + len(buf)),
             '--app-header-offset', hex(hdr_off)],
            input_path=hex_path)
        assert rc2 == 0, f"hex path failed: {stderr2}"
    finally:
        os.unlink(hex_path)

    # Both outputs should have identical headers
    hdr_bin = out_bin[hdr_off:hdr_off + APP_HEADER_SIZE]
    hdr_hex = out_hex[hdr_off:hdr_off + APP_HEADER_SIZE]
    assert hdr_bin == hdr_hex, (
        f"Headers differ:\n  bin: {hdr_bin.hex()}\n  hex: {hdr_hex.hex()}")


# =========================================================================
# ESP32-style test (offset 0x120, --arch flat -- no image fixup)
# =========================================================================

def test_esp32_style_binary_flat():
    """Simulate an ESP32-like binary with header at offset 0x120 using
    --arch flat (no ESP-IDF image fixup).

    Verifies that the basic CRC patching works at the correct offset.
    """
    hdr_off = 0x120
    buf = bytearray(4096)  # 4KB

    # Simulate ESP-IDF image header at offset 0
    buf[0] = 0xE9  # ESP_IMAGE_HEADER_MAGIC
    buf[1] = 0x03  # segment count
    buf[2] = 0x02  # SPI mode
    buf[3] = 0x0F  # SPI speed/size

    # Fill code region with non-zero data
    for i in range(hdr_off + APP_HEADER_SIZE, len(buf)):
        buf[i] = (i * 13 + 7) & 0xFF

    # Header region must be all zeros
    assert all(b == 0 for b in buf[hdr_off:hdr_off + APP_HEADER_SIZE])

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--app-header-offset', hex(hdr_off)],
        input_bytes=bytes(buf))

    assert rc == 0, f"hex2bin failed: {stderr}"
    assert len(out) == len(buf)

    # Verify header was patched
    fields = struct.unpack('<I IIII IIII',
                           out[hdr_off:hdr_off + APP_HEADER_SIZE])
    app_size = fields[0]
    assert app_size == len(buf), f"app_size {app_size} != {len(buf)}"

    # Verify checksums
    pre_crc = crc3_crc16_ibm(out[:hdr_off])
    post_crc = crc3_crc16_ibm(out[hdr_off + APP_HEADER_SIZE:])

    assert pre_crc == (fields[1], fields[2], fields[3])
    assert post_crc == (fields[5], fields[6], fields[7])

    # Verify reserved fields are zero
    assert fields[4] == 0, "pre checksum[3] should be 0"
    assert fields[8] == 0, "post checksum[3] should be 0"


# =========================================================================
# Trim with binary input
# =========================================================================

def test_bin_input_with_trim():
    """--trim strips trailing 0xFF from binary input."""
    hdr_off = 0x20
    # 64 bytes of data + header + 64 bytes of data + 128 bytes of 0xFF
    buf = _build_test_binary(hdr_off, lambda i: (i + 1) & 0xFF, 64)
    buf.extend(b'\xFF' * 128)

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--app-header-offset', hex(hdr_off),
         '--trim'],
        input_bytes=bytes(buf))

    assert rc == 0, f"hex2bin failed: {stderr}"

    # Output should be shorter than input (trailing 0xFF stripped)
    assert len(out) < len(buf), (
        f"Expected trimmed output < {len(buf)}, got {len(out)}")

    # Output length should be 8-byte aligned
    assert len(out) % 8 == 0, f"Trimmed size {len(out)} not 8-byte aligned"

    # Verify the checksum reflects the trimmed size
    fields = struct.unpack('<I IIII IIII',
                           out[hdr_off:hdr_off + APP_HEADER_SIZE])
    assert fields[0] == len(out), (
        f"app_size {fields[0]} != trimmed size {len(out)}")


# =========================================================================
# Error handling
# =========================================================================

def test_bin_input_empty_file():
    """Empty binary input should fail with an error."""
    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--no-checksum'],
        input_bytes=b'')

    assert rc != 0, "Should fail on empty input"
    assert 'empty' in stderr.lower(), f"Expected 'empty' in error: {stderr}"


def test_bin_input_header_offset_out_of_bounds():
    """Header offset past end of binary should fail."""
    buf = bytes(64)  # 64 bytes of zeros

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--app-header-offset', '0x100'],
        input_bytes=buf)

    assert rc != 0, "Should fail when header offset exceeds binary size"
    assert 'out of bounds' in stderr.lower(), (
        f"Expected 'out of bounds' in error: {stderr}")


def test_bin_input_header_region_not_zeroed():
    """Non-zero header region should fail (already patched or wrong offset)."""
    hdr_off = 0x20
    buf = bytearray(256)
    for i in range(256):
        buf[i] = 0x42  # fill everything with non-zero

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--app-header-offset', hex(hdr_off)],
        input_bytes=bytes(buf))

    assert rc != 0, "Should fail when header region is not all zeros"
    assert 'not all zeros' in stderr.lower(), (
        f"Expected 'not all zeros' in error: {stderr}")


def test_bin_input_missing_header_offset():
    """Missing --app-header-offset without --no-checksum should fail."""
    buf = bytes(256)

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin'],
        input_bytes=buf)

    assert rc != 0, "Should fail without --app-header-offset"
    assert 'app-header-offset' in stderr.lower(), (
        f"Expected 'app-header-offset' in error: {stderr}")


# =========================================================================
# --arch esp32: XOR checksum fixup (no SHA256)
# =========================================================================

def test_esp32_arch_fixes_xor_checksum():
    """--arch esp32 recomputes the XOR checksum byte after CRC patching."""
    hdr_off = 0x120
    buf = _build_esp32_image(hdr_off, code_size=512, append_digest=False)

    # Verify the input image has a valid checksum before patching
    assert _verify_esp32_checksum(buf), "Input image checksum should be valid"

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--arch', 'esp32',
         '--app-header-offset', hex(hdr_off)],
        input_bytes=bytes(buf))

    assert rc == 0, f"hex2bin failed: {stderr}"

    # Output must have valid XOR checksum
    assert _verify_esp32_checksum(out), (
        "ESP-IDF XOR checksum is invalid after --arch esp32 patching")

    # Verify CRC header was actually patched (not all zeros)
    fields = struct.unpack('<I IIII IIII',
                           out[hdr_off:hdr_off + APP_HEADER_SIZE])
    # app_size is the segment data end, not the full file length
    # (excludes checksum padding to avoid circular dependency)
    assert fields[0] > 0, "app_size should be positive"
    assert fields[0] < len(out), (
        f"app_size {fields[0]} should be less than file size {len(out)}")
    assert fields[1] != 0, "pre CRC should not be zero"


def test_esp32_arch_fixes_xor_checksum_and_sha256():
    """--arch esp32 recomputes both XOR checksum and SHA256 hash."""
    hdr_off = 0x120
    buf = _build_esp32_image(hdr_off, code_size=512, append_digest=True)

    # Verify the input image is valid before patching
    assert _verify_esp32_checksum(buf), "Input image checksum should be valid"
    assert _verify_esp32_sha256(buf), "Input image SHA256 should be valid"

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--arch', 'esp32',
         '--app-header-offset', hex(hdr_off)],
        input_bytes=bytes(buf))

    assert rc == 0, f"hex2bin failed: {stderr}"

    # Both must be valid after patching
    assert _verify_esp32_checksum(out), (
        "ESP-IDF XOR checksum invalid after patching")
    assert _verify_esp32_sha256(out), (
        "ESP-IDF SHA256 hash invalid after patching")

    # Verify summary mentions SHA256
    assert 'sha256' in stdout.lower(), (
        f"Expected 'SHA256' in summary output, got: {stdout}")


def test_esp32_arch_preserves_non_header_bytes():
    """--arch esp32 only modifies the header, checksum byte, and hash.

    All other bytes (segment data outside the header) must be unchanged.
    """
    hdr_off = 0x120
    buf = _build_esp32_image(hdr_off, code_size=256, append_digest=True)
    original = bytes(buf)

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--arch', 'esp32',
         '--app-header-offset', hex(hdr_off)],
        input_bytes=bytes(buf))

    assert rc == 0, f"hex2bin failed: {stderr}"

    # Pre-header region must be unchanged (image header + segment header +
    # esp_app_desc_t region)
    assert out[:hdr_off] == original[:hdr_off], "Pre-header region was modified"

    # Post-header code region must be unchanged.
    # The post-header region starts after the app_header and extends to
    # the checksum byte.  The checksum byte and SHA256 are expected to change.
    post_start = hdr_off + APP_HEADER_SIZE
    # Find where the checksum padding starts: everything before the last
    # byte of the image body (excluding SHA256)
    body_end = len(out) - ESP_SHA256_SIZE - 1  # last data byte before checksum
    assert out[post_start:body_end] == original[post_start:body_end], (
        "Post-header code region was modified")


def test_esp32_arch_crc_matches_on_target_verification():
    """Full round-trip: build image, patch with --arch esp32, then simulate
    on-target finalize_flash verification.

    This is the definitive test that the ESP32 bootloader will accept
    the patched image.

    Key design point: app_size in the header is the end of segment data,
    NOT the full file length.  The trailing checksum byte and SHA256 hash
    are excluded from the CRC range because they depend on the CRC values
    (circular dependency).  The on-target bootloader uses app_size from
    the header to determine the CRC extent, so both sides agree.
    """
    hdr_off = 0x120
    buf = _build_esp32_image(hdr_off, code_size=2048, append_digest=True)

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--arch', 'esp32',
         '--app-header-offset', hex(hdr_off)],
        input_bytes=bytes(buf))
    assert rc == 0, f"hex2bin failed: {stderr}"

    # 1. ESP-IDF integrity checks pass
    assert _verify_esp32_checksum(out), "XOR checksum invalid"
    assert _verify_esp32_sha256(out), "SHA256 invalid"

    # 2. Read app header (simulating esp_partition_read)
    header_bytes = out[hdr_off:hdr_off + APP_HEADER_SIZE]
    fields = struct.unpack('<I IIII IIII', header_bytes)

    read_app_size = fields[0]
    read_pre_crc = (fields[1], fields[2], fields[3])
    read_post_crc = (fields[5], fields[6], fields[7])

    # 3. app_size should be the segment data end (< full file length)
    assert read_app_size > 0, "app_size should be positive"
    assert read_app_size < len(out), (
        f"app_size {read_app_size} should be less than file size {len(out)} "
        f"(excludes checksum padding + SHA256)")
    assert read_app_size > hdr_off + APP_HEADER_SIZE, (
        f"app_size {read_app_size} should extend past header")

    # 4. Verify CRCs against the partition data up to app_size
    # (what the on-target bootloader's compute_checksum sees)
    computed_pre = crc3_crc16_ibm(out[:hdr_off])
    assert computed_pre == read_pre_crc, (
        f"pre CRC mismatch: {computed_pre} != {read_pre_crc}")

    post_start = hdr_off + APP_HEADER_SIZE
    computed_post = crc3_crc16_ibm(out[post_start:read_app_size])
    assert computed_post == read_post_crc, (
        f"post CRC mismatch: {computed_post} != {read_post_crc}")


def test_esp32_arch_bad_magic_byte_fails():
    """--arch esp32 should fail if the image does not have ESP-IDF magic."""
    hdr_off = 0x120
    buf = _build_test_binary(hdr_off, lambda i: 0, 256)
    # buf[0] is 0x00, not 0xE9

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--arch', 'esp32',
         '--app-header-offset', hex(hdr_off)],
        input_bytes=bytes(buf))

    assert rc != 0, "Should fail with bad magic byte"
    assert 'magic' in stderr.lower(), (
        f"Expected 'magic' in error: {stderr}")


def test_esp32_arch_summary_shows_fixup():
    """Summary output should mention ESP-IDF image fixup."""
    hdr_off = 0x120
    buf = _build_esp32_image(hdr_off, code_size=128, append_digest=False)

    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--arch', 'esp32',
         '--app-header-offset', hex(hdr_off)],
        input_bytes=bytes(buf))

    assert rc == 0, f"hex2bin failed: {stderr}"
    assert 'esp-idf' in stdout.lower() or 'fixup' in stdout.lower() or \
           'xor' in stdout.lower(), (
        f"Expected ESP-IDF fixup info in summary, got: {stdout}")


# =========================================================================
# Simulate on-target finalize_flash verification (flat arch, no ESP fixup)
# =========================================================================

def test_bin_on_target_finalize_simulation():
    """Simulate the full on-target verification flow using --arch flat:
    1. hex2bin patches the binary
    2. Bootloader writes it to flash
    3. finalize_flash reads header via esp_partition_read
    4. compute_checksum computes CRC over pre and post regions
    5. Compare against stored values

    This test proves the hex2bin binary path produces checksums that
    a bootloader's finalize_flash will accept.
    """
    hdr_off = 0x120

    # Build a realistic-sized binary
    buf = bytearray(8192)
    # Simulate ESP-IDF image header
    buf[0] = 0xE9
    buf[1] = 0x02
    # Fill code region
    for i in range(hdr_off + APP_HEADER_SIZE, len(buf)):
        buf[i] = (i * 37 + 13) & 0xFF

    # Run hex2bin
    rc, stdout, stderr, out = _run_hex2bin(
        ['--input-format', 'bin', '--app-header-offset', hex(hdr_off)],
        input_bytes=bytes(buf))
    assert rc == 0, f"hex2bin failed: {stderr}"

    # --- Simulate on-target finalize_flash ---

    # 1. Read app header (simulating esp_partition_read)
    header_bytes = out[hdr_off:hdr_off + APP_HEADER_SIZE]
    fields = struct.unpack('<I IIII IIII', header_bytes)

    read_app_size = fields[0]
    read_pre_crc = (fields[1], fields[2], fields[3])
    read_post_crc = (fields[5], fields[6], fields[7])

    # 2. Validate app_size
    assert read_app_size == len(out), (
        f"app_size {read_app_size} != output size {len(out)}")

    # 3. Compute pre-checksum (simulating compute_checksum_helper)
    pre_size = hdr_off
    computed_pre = crc3_crc16_ibm(out[:pre_size])

    assert computed_pre[0] == read_pre_crc[0], (
        f"pre crc_all: 0x{computed_pre[0]:04X} != 0x{read_pre_crc[0]:04X}")
    assert computed_pre[1] == read_pre_crc[1], (
        f"pre crc_odd: 0x{computed_pre[1]:04X} != 0x{read_pre_crc[1]:04X}")
    assert computed_pre[2] == read_pre_crc[2], (
        f"pre crc_even: 0x{computed_pre[2]:04X} != 0x{read_pre_crc[2]:04X}")

    # 4. Compute post-checksum
    post_start = hdr_off + APP_HEADER_SIZE
    post_size = read_app_size - hdr_off - APP_HEADER_SIZE
    computed_post = crc3_crc16_ibm(out[post_start:post_start + post_size])

    assert computed_post[0] == read_post_crc[0], (
        f"post crc_all: 0x{computed_post[0]:04X} != 0x{read_post_crc[0]:04X}")
    assert computed_post[1] == read_post_crc[1], (
        f"post crc_odd: 0x{computed_post[1]:04X} != 0x{read_post_crc[1]:04X}")
    assert computed_post[2] == read_post_crc[2], (
        f"post crc_even: 0x{computed_post[2]:04X} != 0x{read_post_crc[2]:04X}")


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

# dsPIC Checksum Implementation Plan

## Goal

Enable checksum verification on the dsPIC bootloader so it matches the STM32
and MSPM0 implementations: same struct, same binary size, same hex2bin usage,
same offset math. The only dsPIC-specific piece is how flash is read back.

---

## Background: The Phantom Byte

dsPIC instructions are 24 bits (3 data bytes). In the Intel HEX file and the
binary image produced by hex2bin, each instruction is encoded as 4 bytes:

```
byte[0] = instruction[7:0]
byte[1] = instruction[15:8]
byte[2] = instruction[23:16]
byte[3] = 0x00              <-- phantom byte, always zero
```

When written to flash, `write_flash_bytes` extracts the 3 data bytes and
discards byte[3]. When read back, `FLASH_ReadWord24` returns the 3 data bytes
and the phantom reads as 0x00.

---

## Key Insight: uint32_t Alignment

The `bootloader_app_header_t` struct contains 9 `uint32_t` fields:

```c
typedef struct bootloader_app_header {
    uint32_t app_size;                                  //  4 bytes
    uint32_t checksum_pre[BOOTLOADER_CHECKSUM_COUNT];   // 16 bytes
    uint32_t checksum_post[BOOTLOADER_CHECKSUM_COUNT];  // 16 bytes
} bootloader_app_header_t;                              // 36 bytes total
```

Each uint32_t = 4 bytes = exactly 1 dsPIC instruction. The phantom byte
(byte[3]) lands on the MSB of each uint32_t in little-endian layout. Every
field in the struct has MSB = 0x00:

- `app_size`: dsPIC flash is < 16 MB, top byte is always 0x00.
- `checksum[0..2]`: CRC-16 values fit in 16 bits, stored as uint32_t with
  upper 16 bits zero.
- `checksum[3]`: reserved, explicitly 0x00000000.

Therefore the phantom byte always lands where the struct already has 0x00.
The round-trip through flash is lossless. No special encoding is needed.

**36 binary bytes = 9 instructions. Same size as STM32 and MSPM0.**

---

## What Needs to Change

### 1. dsPIC driver: new private flash read helper

Add `_read_flash_image_bytes()` to `bootloader_drivers_openlcb.c`. This reads
4 bytes per instruction (3 data + phantom 0x00), matching the binary image
format. Used only by `finalize_flash` to read the header.

The existing `read_flash_bytes` (3 data bytes per instruction) is unchanged
and remains in the DI struct for use by the blank-flash check in bootloader.c.

```c
static void _read_flash_image_bytes(uint32_t flash_addr, void *dest_ram,
                                    uint32_t size_bytes) {

    uint8_t  *dest_ptr   = (uint8_t *) dest_ram;
    uint32_t  pc_address = flash_addr / BINARY_TO_PC_DIVISOR;
    uint32_t  remaining  = size_bytes;

    while (remaining >= 4U) {

        uint32_t flash_word = FLASH_ReadWord24(pc_address);
        *dest_ptr++ = (uint8_t)(flash_word         & 0xFFU);
        *dest_ptr++ = (uint8_t)((flash_word >>  8) & 0xFFU);
        *dest_ptr++ = (uint8_t)((flash_word >> 16) & 0xFFU);
        *dest_ptr++ = 0x00U;   /* phantom byte */
        remaining  -= 4U;
        pc_address += 2U;

    }

}
```

### 2. dsPIC driver finalize_flash: use the new reader

Replace the `BootloaderDriversOpenlcb_read_flash_bytes` call with
`_read_flash_image_bytes`. All offset math continues to use
`sizeof(bootloader_app_header_t)` -- identical to STM32/MSPM0.

```c
uint16_t BootloaderDriversOpenlcb_finalize_flash(
        compute_checksum_func_t compute_checksum_helper) {

#ifndef NO_CHECKSUM

    bootloader_app_header_t header;
    uint32_t flash_min = APP_FLASH_START * BINARY_TO_PC_DIVISOR;
    uint32_t flash_max = APP_FLASH_END   * BINARY_TO_PC_DIVISOR;

    _read_flash_image_bytes(APP_HEADER_ADDRESS, &header, sizeof(header));

    /* ... rest identical to current code, using sizeof(header) for offsets ... */

#endif
}
```

### 3. bootloader.c: remove duplicate checksum logic

Remove `_check_application_checksum()` entirely. Replace the boot-time
verification with a call to `finalize_flash`:

```c
if (request == BOOTLOADER_NOT_REQUESTED) {

#ifndef NO_CHECKSUM
    uint16_t csum_result = openlcb_driver->finalize_flash(
            openlcb_driver->compute_checksum);
    bool csum_ok = (csum_result == 0);
    openlcb_driver->set_status_led(BOOTLOADER_LED_CSUM_ERROR, !csum_ok);
#else
    bool csum_ok = true;
    openlcb_driver->set_status_led(BOOTLOADER_LED_CSUM_ERROR, false);
#endif

    if (csum_ok) {
        /* ... existing blank-flash check and jump ... */
    }
}
```

This:
- Eliminates ~55 lines of duplicated verification logic.
- Keeps platform-specific flash reading in the driver where it belongs.
- Uses the same code path for boot-time and post-write verification.
- `finalize_flash` returns 0 on success, ERROR_PERMANENT on failure.

### 4. dsPIC application linker script: correct header reservation

The BasicNode.X linker script comment currently says:

```
** The header is 36 data bytes = 12 instructions = 24 PC addresses
```

Correct to:

```
** The header is 36 bytes = 9 instructions = 18 PC addresses,
** so it spans 0x4004-0x4015.
```

The `.app_header` section size is controlled by the `space(prog)` struct
placement -- the compiler allocates 12 instructions (36 data bytes / 3 per
instruction). This needs to change to 9 instructions (36 binary bytes / 4 per
instruction).

The struct in `app_header.c` must be laid out so the compiler generates
exactly 9 instruction words. Since XC16 with `space(prog)` packs 3 data bytes
per instruction, a naive 36-byte struct would occupy 12 instructions.

To get 9 instructions, define the application-side header as 27 bytes of
actual data (9 instructions x 3 data bytes). The simplest approach: use an
opaque byte array sized to produce exactly 9 instructions:

```c
/* 9 instructions x 3 data bytes = 27 bytes.
 * Phantom bytes are implicit in space(prog) encoding. */
typedef struct {
    uint8_t raw[27];
} app_header_image_t;

_Static_assert or negative-array-size check: sizeof == 27
```

The post-link tool (hex2bin) patches the 36 flat binary bytes at the correct
offset in the .bin file. The 9 phantom positions in those 36 bytes are all
0x00 (proven by the uint32_t alignment property above), so when write_flash_bytes
discards them, the correct 27 data bytes are stored.

Alternative: use `__attribute__((packed))` with a struct of uint8_t[3] groups.
The exact approach is an implementation detail -- the important thing is 9
instructions reserved, and hex2bin writes 36 flat bytes over them.

### 5. hex2bin.py: no changes

The existing hex2bin invocation works as-is:

```bash
python3 hex2bin.py --arch dspic --start 0x4000 --app-header-offset 8 app.hex app.bin
```

- Zeroes 36 bytes at offset 8 (correct: 9 instructions in the binary).
- Packs `struct.pack('<I IIII IIII', ...)` = 36 flat bytes.
- Patches them at offset 8.
- Pre-region: `buf[:8]` (2 instructions, 8 binary bytes).
- Post-region: `buf[44:]` (after 8 + 36 = 44).
- CRC computed over raw binary bytes, which include phantom 0x00 at every
  4th position -- this matches what `compute_checksum` does on-target.

### 6. Remove NO_CHECKSUM

Comment out `#define NO_CHECKSUM` in `bootloader_types.h` (line 218) after
all changes are tested end-to-end.

---

## What Does NOT Change

| Component | Why |
|-----------|-----|
| hex2bin.py | 36-byte flat header works for all architectures |
| bootloader_app_header_t | Same struct, same sizeof, all platforms |
| compute_checksum (dsPIC) | Already reads 4 bytes per instruction (3 data + phantom) |
| write_flash_bytes (dsPIC) | Already writes from binary format correctly |
| read_flash_bytes (dsPIC) | Stays 3-byte-per-instruction for blank-flash check |
| STM32/MSPM0 drivers | No changes needed |
| Offset calculations | sizeof(bootloader_app_header_t) = 36 everywhere |

---

## Address Map (dsPIC33EP512MC506)

```
PC addr    Binary addr    Binary offset    Content
(linker)   (PC x 2)       from app start
--------   -----------    -------------    ---------------------------
0x4000     0x8000         0                Reset vector GOTO (2 instrs, 8 bytes)
0x4004     0x8008         8                app_header start (9 instrs, 36 bytes)
0x4016     0x802C         44               Post-region start
...        ...            ...              Application code
0x547FC    0xA8FF8        ...              APP_FLASH_END
```

---

## Verification Flow (unified for all platforms)

```
Boot
  |
  +-- bootloader requested? --> yes --> bootloader mode
  |
  +-- finalize_flash(compute_checksum)
  |     |
  |     +-- read header from flash (driver-specific method)
  |     +-- validate app_size <= flash_size
  |     +-- compute pre-region CRC, compare to header.checksum_pre
  |     +-- compute post-region CRC, compare to header.checksum_post
  |     +-- return 0 (pass) or ERROR_PERMANENT (fail)
  |
  +-- csum_ok?
  |     NO  --> set CSUM_ERROR LED, stay in bootloader mode
  |     YES --> continue
  |
  +-- first word == 0xFFFFFFFF? (erased flash)
        YES --> stay in bootloader mode (no app)
        NO  --> cleanup_before_handoff() --> jump_to_application()
```

Post-write (Unfreeze datagram) uses the same `finalize_flash` call --
identical code path, no duplication.

---

## Testing Plan

1. **hex2bin unit tests**: verify the patched .bin for dsPIC has the correct
   36-byte header at offset 8, correct pre/post CRC regions.

2. **Round-trip test (Python)**: take a patched .bin, simulate the on-target
   CRC computation (reading 4 bytes per instruction including phantoms),
   confirm the CRCs match the header values.

3. **On-target**: flash a BasicNode image through the bootloader with
   NO_CHECKSUM still defined. Verify the image boots and runs.

4. **Enable checksum**: comment out NO_CHECKSUM, rebuild bootloader. Flash
   the same BasicNode image. Verify boot-time checksum passes and the app
   launches.

5. **Corrupt test**: flip a byte in the .bin before uploading. Verify the
   bootloader rejects the image (finalize_flash returns error on Unfreeze,
   boot-time verification fails on next power cycle).

---

## Files Modified (summary)

```
bootloader/src/openlcb/bootloader.c
    - Remove _check_application_checksum()
    - Bootloader_init calls finalize_flash at boot

bootloader/src/openlcb/bootloader_types.h
    - Comment out #define NO_CHECKSUM (last step)

bootloader/applications/dspic/BasicNodeBootloader.X/
  application_drivers/bootloader_drivers_openlcb.c
    - Add _read_flash_image_bytes() (private, 4 bytes per instruction)
    - finalize_flash uses _read_flash_image_bytes for header read

bootloader/applications/dspic/BasicNode.X/
  app_header.c
    - Resize to produce 9 instructions (27 data bytes)

  p33EP512MC506_BasicNode.gld
    - Correct comment: 9 instructions, 18 PC addresses
```

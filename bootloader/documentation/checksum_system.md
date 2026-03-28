# Checksum System

## Overview

The bootloader uses a triple-CRC scheme to verify firmware integrity. Before
jumping to the application, the bootloader reads a header embedded in flash,
recomputes the CRCs over the surrounding firmware bytes, and compares. If any
CRC mismatches the application is not launched and the bootloader stays active.

The same verification runs after a firmware write completes (Unfreeze command).
If the newly written image fails, the bootloader rejects the Unfreeze datagram
and does not reboot.

This design originates from the OpenMRN standalone bootloader
(`openmrn/src/openlcb/Bootloader.hxx`, `openmrn/src/utils/Crc.hxx`) and is
reimplemented here in plain C.

---

## Algorithm: CRC-16-IBM

| Property            | Value    |
|---------------------|----------|
| Name                | CRC-16-IBM (CRC-16-ANSI) |
| Polynomial (normal) | 0x8005   |
| Polynomial (reflected) | 0xA001 |
| Initial value       | 0x0000   |
| Final XOR           | 0x0000   |
| Test vector ("123456789") | 0xBB3D |

The implementation uses a nibble-at-a-time table lookup with two 16-entry
tables (64 bytes of ROM). This is a deliberate trade-off: half the ROM of a
full 256-entry table with acceptable speed for bootloader use.

```
Source: bootloader/src/crc/bootloader_crc.c
```

---

## Triple-CRC

Each region is verified by three parallel CRC computations stored in a
four-element array:

| Index | Covers              |
|-------|---------------------|
| 0     | All bytes in region |
| 1     | Odd-indexed bytes (1st, 3rd, 5th, ...) |
| 2     | Even-indexed bytes (2nd, 4th, 6th, ...) |
| 3     | Reserved (zero)     |

The odd/even split provides additional error detection beyond what a single
CRC-16 offers. This is compatible with the TI Tiva `ROM_Crc16Array3()` hardware
CRC function, which the OpenMRN bootloader uses on TI platforms.

The function `BootloaderCrc_crc3_crc16_ibm()` runs all three CRCs in a single
pass over the data.

---

## Application Header

The application binary contains a `bootloader_app_header_t` struct placed at a
fixed flash address by the linker script:

```c
typedef struct bootloader_app_header {
    uint32_t app_size;
    uint32_t checksum_pre[BOOTLOADER_CHECKSUM_COUNT];   // 4 x uint32_t
    uint32_t checksum_post[BOOTLOADER_CHECKSUM_COUNT];  // 4 x uint32_t
} bootloader_app_header_t;
```

`BOOTLOADER_CHECKSUM_COUNT` is 4. Each `uint32_t` slot holds a 16-bit CRC
value zero-extended to 32 bits.

The header splits flash into two regions that are checksummed independently:

```
flash_min                                                     flash_max
   |                                                              |
   v                                                              v
   +--------------------+------------------+----------------------+
   |   PRE region       |  app_header_t    |   POST region        |
   |  (code/data before |  (app_size,      |  (code/data after    |
   |   the header)      |   checksum_pre,  |   the header)        |
   |                    |   checksum_post) |                      |
   +--------------------+------------------+----------------------+
   |<--- pre_size ----->|                  |<--- post_size ------>|
```

- **pre_size** = app_header_address - flash_min
- **post_offset** = app_header_address + sizeof(bootloader_app_header_t)
- **post_size** = app_size - post_offset (may be zero)

The header itself is excluded from both CRC computations. This avoids a
chicken-and-egg problem: the CRCs are stored in the header, so the header
bytes cannot be included in the CRC input.

---

## How Checksums Are Created

A post-link tool (run after compilation) patches the checksums into the binary:

1. Locate `bootloader_app_header_t` at its linker-assigned address.
2. Compute triple-CRC over the PRE region (flash_min to app_header).
3. Store the result in `checksum_pre[]`.
4. Compute triple-CRC over the POST region (after header to app_size).
5. Store the result in `checksum_post[]`.
6. Write the patched header back into the binary.

The binary is now "sealed" and ready for upload.

---

## Verification: At Boot

`_check_application_checksum()` in `bootloader.c` runs during `Bootloader_init()`:

1. Read the header from flash via the `read_flash_bytes()` DI function.
2. Reject if `app_size > flash_size` (corrupted header).
3. Compute triple-CRC over the PRE region and compare to `checksum_pre[]`.
4. Compute triple-CRC over the POST region and compare to `checksum_post[]`.
5. Return true only if both regions match.

If verification fails the `BOOTLOADER_LED_CSUM_ERROR` LED flag is set.

---

## Verification: After Firmware Write

When the host sends a `CONFIG_MEM_UNFREEZE` datagram:

1. Bootloader calls `finalize_flash(compute_checksum)` through the DI struct.
2. The platform's finalize function re-reads the written flash and runs the
   same triple-CRC check.
3. If the check fails, a Datagram Rejected reply is sent with an error code.
4. If the check passes, Datagram OK is sent and a reboot is scheduled.

This ensures a corrupted transfer (CAN bit errors, truncated upload) is caught
before the bootloader attempts to jump to a broken image.

---

## Boot Decision Flow

```
Bootloader_init()
  |
  +--> Is bootloader explicitly requested? (button press, shared-RAM flag)
  |      YES --> stay in bootloader mode
  |      NO  --> continue
  |
  +--> Is NO_CHECKSUM defined?
  |      YES --> skip checksum (development only)
  |      NO  --> run _check_application_checksum()
  |               FAIL --> stay in bootloader mode, set CSUM_ERROR LED
  |               PASS --> continue
  |
  +--> Is first word at flash_min == 0xFFFFFFFF? (erased flash)
         YES --> stay in bootloader mode (no application present)
         NO  --> cleanup_before_handoff() --> jump to application
```

---

## Dependency Injection

The checksum computation is injected through the `bootloader_openlcb_driver_t`
struct:

```c
typedef void (*compute_checksum_func_t)(
    uint32_t flash_address,
    uint32_t size,
    uint32_t *checksum        // output: array of BOOTLOADER_CHECKSUM_COUNT
);
```

Flash reads also go through a DI function (`read_flash_bytes`). This is
required for Harvard architectures (e.g. dsPIC33) where flash is not
memory-mapped and must be read via special instructions (TBLRDL/TBLRDH).
All flash addresses are `uint32_t`, not pointers, to avoid silent truncation
on platforms with 16-bit pointers.

Platforms with hardware CRC units can provide their own implementation through
this function pointer.

---

## NO_CHECKSUM Bypass

Defined in `bootloader_types.h`:

```c
#define NO_CHECKSUM
```

When active, the bootloader skips all checksum verification at both boot and
post-write. This exists for early development before the post-link patching
tool is integrated. It should never be enabled in production firmware because
the bootloader will jump to any image in flash without verification.

---

## OpenMRN Correspondence

| OpenLcbCLib                          | OpenMRN                                      |
|--------------------------------------|----------------------------------------------|
| `bootloader_crc.c`                  | `src/utils/Crc.cxx` / `Crc.hxx`             |
| `BootloaderCrc_crc3_crc16_ibm()`    | `crc3_crc16_ibm()` (inline in Crc.hxx)      |
| `bootloader_app_header_t`           | `struct app_header` (Bootloader.hxx)         |
| `BOOTLOADER_CHECKSUM_COUNT` (4)     | `CHECKSUM_COUNT` (4)                         |
| `_check_application_checksum()`     | `check_application_checksum()` (ApplicationChecksum.hxx) |
| DI via `compute_checksum_func_t`    | HAL callback `checksum_data()` (bootloader_hal.h) |
| `NO_CHECKSUM` define                | Not present (OpenMRN always verifies)        |

---

## Source Files

```
bootloader/src/
  crc/
    bootloader_crc.c          CRC-16-IBM tables and triple-CRC function
    bootloader_crc.h          Public API
  openlcb/
    bootloader.c              Boot-time checksum verification
    bootloader_types.h        bootloader_app_header_t, BOOTLOADER_CHECKSUM_COUNT,
                              compute_checksum_func_t, NO_CHECKSUM
    bootloader_openlcb_statemachine.c   Post-write checksum (Unfreeze handler)
```

# dsPIC33 Bootloader TODO

The dsPIC implementation is the reference pattern. All three swimlanes are fully implemented
and working. The items below are shared issues that also apply to all other platforms.

## MEDIUM

### `NO_CHECKSUM` is globally enabled

**File:** Shared `bootloader_types.h:218`

`#define NO_CHECKSUM` is uncommented. All checksum validation is skipped -- both at boot and
after firmware write. Any corrupted image will be jumped to. Acceptable for development, must
be addressed before production.

## LOW

### Node ID is hardcoded

**File:** `BasicNodeBootloader.X/application_drivers/bootloader_drivers_openlcb.c`

Returns a fixed node ID for all boards. Two dsPIC boards on the same CAN bus will have
identical node IDs. TODO comment already present about reading from protected flash.

### `finalize_flash()` checksum validation is implemented but inactive

**File:** `BasicNodeBootloader.X/application_drivers/bootloader_drivers_openlcb.c`

The `#ifndef NO_CHECKSUM` branch now reads the app header via Harvard-safe
`BootloaderDriversOpenlcb_read_flash_bytes()`, recomputes both pre and post
triple-CRC checksums, and returns `ERROR_PERMANENT` on mismatch. Currently
inactive because `NO_CHECKSUM` is defined.

Blocked on: post-link checksum tool populating app_header in firmware images.

### BasicNode `app_header.c` and linker section added

**Files:**
- `BasicNode.X/src/application_callbacks/app_header.c` -- zero-initialized header struct with `space(prog)`
- `BasicNode.X/p33EP512MC506_BasicNode.gld` -- `.app_header` section at PC 0x4004

The app header struct is placed in program flash at the expected address. All
checksum fields are zero. A post-link tool must patch them before production
programming.

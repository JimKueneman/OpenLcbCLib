# TI MSPM0G3507 Launchpad Bootloader TODO

Issues found by swimlane analysis against the dsPIC reference implementation.

## FIXED

### Freeze callback does not disable interrupts before writing shared RAM

**Fixed in:** `BasicNode/application_callbacks/callbacks_config_mem.c`

Added `__disable_irq()` before shared RAM writes. Comment explains why no peripheral
teardown is needed (DL_SYSCTL_resetDevice resets all peripherals).

### Misleading comment about cleanup

**Fixed in:** `BasicNode/application_callbacks/callbacks_config_mem.c`

Removed the incorrect comment claiming the library would call cleanup. Replaced with
comments explaining each step of the drop-back sequence.

### `NO_CHECKSUM` commented out -- checksums now active

**Fixed in:** `bootloader_drivers_openlcb.h` and shared `bootloader_types.h`

`NO_CHECKSUM` is now commented out on both copies. Boot-time and post-write checksum
validation are active. The `finalize_flash()` triple-CRC logic in
`bootloader_drivers_openlcb.c` is no longer gated. Verified against source 2026-03-28.

### `finalize_flash()` checksum validation now active

**Fixed in:** `BasicNodeBootloader/application_drivers/bootloader_drivers_openlcb.c`

The `#ifndef NO_CHECKSUM` branch reads the app header, recomputes both pre and post
triple-CRC checksums, and returns `ERROR_PERMANENT` on mismatch. Now active since
`NO_CHECKSUM` is undefined. Verified against source 2026-03-28.

### BasicNode `app_header.c` and linker section added

**Fixed in:**
- `BasicNode/app_header.c` -- zero-initialized header struct with `_Static_assert` size guard
- `BasicNode/device_linker.cmd` -- `.app_header` section at 0x00003CC0

The app header struct is placed in flash at the expected address. The post-link
checksum tool patches the fields before programming. Verified against source 2026-03-28.

### `TI_MSPM0_BOOTLOADER_LESSONS_LEARNED.md` -- DELETED

Content was integrated into `How_To_Modify_For_Your_MSPM0.md`. The old file
has been removed.

## LOW

### Node ID is hardcoded

**File:** `BasicNodeBootloader/openlcb_c_lib_bootloader/drivers/bootloader_drivers_openlcb.c`

Returns `0x050101012200ULL` for all boards. Two MSPM0 boards on the same CAN bus will have
identical node IDs. TODO comment already present about reading from protected flash sector
at `NODEID_FLASH_ADDRESS`.

### No button debouncing

**File:** `BasicNodeBootloader/openlcb_c_lib_bootloader/drivers/bootloader_drivers_openlcb.c:116-122`

Single read of GPIO PB21 with no debounce logic. Electrical noise could theoretically
trigger false bootloader entry. Low risk since the button is only checked once at startup,
but worth noting for noisy environments.

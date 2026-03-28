# STM32F407 Discovery Bootloader TODO

Issues found by swimlane analysis against the dsPIC reference implementation.

## FIXED

### Freeze callback does not disable interrupts before writing shared RAM

**Fixed in:** `BasicNode/Core/Src/application_callbacks/callbacks_config_mem.c`

Added `__disable_irq()` before shared RAM writes. Comment explains why no peripheral
teardown is needed (NVIC_SystemReset resets all peripherals).

### Misleading comment about cleanup

**Fixed in:** `BasicNode/Core/Src/application_callbacks/callbacks_config_mem.c`

Removed the incorrect comment claiming the library would call cleanup. Replaced with
comments explaining each step of the drop-back sequence.

### `cleanup_before_handoff()` only clears NVIC register 0

**Fixed in:** `BasicNodeBootloader/Core/Src/application_drivers/bootloader_drivers_openlcb.c`

Now loops over all 3 ICER/ICPR registers to cover IRQ 0-81.

### No flash write read-back verification

**Fixed in:** `BasicNodeBootloader/Core/Src/application_drivers/bootloader_drivers_openlcb.c`

`write_flash_bytes()` now reads back each written word and returns ERROR_PERMANENT on
mismatch. Covers both the main write loop and the partial-word tail.

### Missing DSB/ISB after VTOR write in `jump_to_application()`

**Fixed in:** `BasicNodeBootloader/Core/Src/application_drivers/bootloader_drivers_openlcb.c`

Added `__DSB()` + `__ISB()` after writing `SCB->VTOR` to ensure the pipeline fetches
vectors from the new table before the jump.

### `NO_CHECKSUM` commented out -- checksums now active

**Fixed in:** `bootloader_drivers_openlcb.h` and shared `bootloader_types.h`

`NO_CHECKSUM` is now commented out on both copies. Boot-time and post-write checksum
validation are active. The `finalize_flash()` triple-CRC logic in
`bootloader_drivers_openlcb.c` is no longer gated. Verified against source 2026-03-28.

### `finalize_flash()` checksum validation now active

**Fixed in:** `BasicNodeBootloader/Core/Src/application_drivers/bootloader_drivers_openlcb.c`

The `#ifndef NO_CHECKSUM` branch reads the app header, recomputes both pre and post
triple-CRC checksums, and returns `ERROR_PERMANENT` on mismatch. The logic mirrors
`_check_application_checksum()` in the library's `bootloader.c`. Now active since
`NO_CHECKSUM` is undefined. Verified against source 2026-03-28.

### BasicNode `app_header.c` and linker section added

**Fixed in:**
- `BasicNode/Core/Src/app_header.c` -- zero-initialized header struct with `_Static_assert` size guard
- `BasicNode/STM32F407VGTX_APPLICATION.ld` -- `.app_header` section at 0x08008200

The app header struct is placed in flash at the expected address. The post-link
checksum tool patches the fields before programming. Verified against source 2026-03-28.

### STM32 lessons learned files -- DELETED

`STM32F407_BOOTLOADER_LESSONS_LEARNED.md` and `STM32_BOOTLOADER_LESSONS_LEARNED.md`
content was integrated into `How_To_Modify_For_Your_STM32.md`. The old files
have been removed.

## LOW

### Node ID is hardcoded

**File:** `BasicNodeBootloader/Core/Src/application_drivers/bootloader_drivers_openlcb.c`

Returns `0x050101012201ULL` for all boards. Two STM32 boards on the same CAN bus will
have identical node IDs. TODO comment already present about reading from protected flash
sector at `NODEID_FLASH_ADDRESS`.

### No button debouncing

**File:** `BasicNodeBootloader/Core/Src/application_drivers/bootloader_drivers_openlcb.c`

Single read of B1 (PA0) with no debounce logic. Low risk since the button is only
checked once at startup, but worth noting for noisy environments.

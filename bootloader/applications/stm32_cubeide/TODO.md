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

## OPEN -- Development Only

### `NO_CHECKSUM` is globally enabled

**File:** Shared `bootloader_types.h:218`

`#define NO_CHECKSUM` is uncommented. All checksum validation is skipped -- both at boot
and after firmware write. Any corrupted image will be jumped to. Acceptable for
development, must be addressed before production.

Blocked on: post-link checksum tool integration.

### `finalize_flash()` checksum validation is implemented but inactive

**File:** `BasicNodeBootloader/Core/Src/application_drivers/bootloader_drivers_openlcb.c`

The `#ifndef NO_CHECKSUM` branch now reads the app header, recomputes both
pre and post triple-CRC checksums, and returns `ERROR_PERMANENT` on mismatch.
The logic mirrors `_check_application_checksum()` in the library's `bootloader.c`.
Currently inactive because `NO_CHECKSUM` is defined.

Blocked on: post-link checksum tool populating app_header in firmware images.

### Node ID is hardcoded

**File:** `BasicNodeBootloader/Core/Src/application_drivers/bootloader_drivers_openlcb.c`

Returns `0x050101012201ULL` for all boards. Two STM32 boards on the same CAN bus will
have identical node IDs. TODO comment already present about reading from protected flash
sector at `NODEID_FLASH_ADDRESS`.

Blocked on: production flash programming workflow.

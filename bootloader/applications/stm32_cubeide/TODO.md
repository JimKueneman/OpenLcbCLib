# STM32F407 Discovery Bootloader TODO

Issues found by swimlane analysis against the dsPIC reference implementation.

## MEDIUM

### Freeze callback does not disable interrupts before writing shared RAM

**File:** `BasicNode/Core/Src/application_callbacks/callbacks_config_mem.c:67-80`

The freeze callback writes `bootloader_cached_alias` and `bootloader_request_flag` then
calls `HAL_NVIC_SystemReset()` without disabling interrupts first. If a CAN RX interrupt
fires between the alias write and the reset, it could corrupt state or cause the library
to process a stale message while shared RAM is half-written.

The dsPIC reference does `_GIE = 0` and disables CAN interrupts before touching shared
RAM (see `bootloader/applications/dspic/BasicNode.X/src/application_callbacks/callbacks_config_mem.c:139-148`).

Suggested fix -- add before the shared RAM writes:

```c
__disable_irq();
```

Cortex-M's `HAL_NVIC_SystemReset()` includes a `__DSB()` barrier that flushes the write
buffer, so the magic flag is reliably visible after reset. The risk here is the interrupt
window, not the write ordering.

### Misleading comment about cleanup

**File:** `BasicNode/Core/Src/application_callbacks/callbacks_config_mem.c:71`

Comment says "STM32_DriverLibDrivers_cleanup_before_handoff will be called by the library"
but the `HAL_NVIC_SystemReset()` call fires immediately -- the library never gets a chance
to call cleanup. The comment should be removed or the cleanup should be called explicitly
before reset if peripheral teardown matters.

### `cleanup_before_handoff()` only clears NVIC register 0

**File:** `BasicNodeBootloader/Core/Src/openlcb_c_lib_bootloader/drivers/bootloader_drivers_openlcb.c`

Only clears `NVIC->ICER[0]` and `NVIC->ICPR[0]`. STM32F407 has up to 82 interrupts
spanning 3 ICER/ICPR registers (IRQ 0-31, 32-63, 64-81). If the bootloader or a future
change enables any interrupt above IRQ 31, it will not be cleared before jumping to the app.

Currently safe since only CAN1 (IRQ 19-22) and SysTick are used, but fragile.

Suggested fix:

```c
for (int i = 0; i < 3; i++) {
    NVIC->ICER[i] = 0xFFFFFFFF;
    NVIC->ICPR[i] = 0xFFFFFFFF;
}
```

### `NO_CHECKSUM` is globally enabled

**File:** Shared `bootloader_types.h:218`

`#define NO_CHECKSUM` is uncommented. All checksum validation is skipped -- both at boot and
after firmware write. Any corrupted image will be jumped to. Acceptable for development, must
be addressed before production.

## LOW

### Node ID is hardcoded

**File:** `BasicNodeBootloader/Core/Src/openlcb_c_lib_bootloader/drivers/bootloader_drivers_openlcb.c`

Returns `0x050101012201ULL` for all boards. Two STM32 boards on the same CAN bus will have
identical node IDs. TODO comment already present about reading from protected flash sector
at `NODEID_FLASH_ADDRESS`.

### `finalize_flash()` checksum validation is stubbed

Returns 0 unconditionally without validating the written image. Gated by `NO_CHECKSUM`.
Once the post-link checksum tool is integrated, implement the validation logic here.

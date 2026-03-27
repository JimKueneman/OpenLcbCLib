# TI MSPM0G3507 Bootloader — Lessons Learned

This document records the non-obvious problems, surprises, and design decisions
encountered while porting the OpenLCB standalone bootloader to the
MSPM0G3507 LaunchPad (LP-MSPM0G3507) using TI CCS Theia.

---

## 1. Flash Write Alignment: 8 Bytes with ECC

### The constraint

The MSPM0G3507 flash controller requires writes to be aligned to 8-byte (64-bit)
boundaries.  The `DL_FlashCTL_programMemoryFromRAM64WithECCGenerated()` API
writes exactly 8 bytes and generates ECC (Error-Correcting Code) in hardware.
Writing fewer than 8 bytes or writing to a non-8-byte-aligned address will fail.

### Cortex-M0+ unaligned access restriction

The Cortex-M0+ core does NOT support unaligned memory access.  Casting a
`uint8_t*` pointer to `uint32_t*` will hard fault if the pointer is not
naturally 4-byte aligned.  The driver copies incoming data through a stack-local
`uint32_t aligned_data[2]` buffer using `memcpy()` to guarantee alignment:

```c
uint32_t aligned_data[2];
memcpy(aligned_data, source_data, FLASH_WRITE_ALIGN);
DL_FlashCTL_programMemoryFromRAM64WithECCGenerated(FLASHCTL, dest, aligned_data);
```

### Trailing data padding

Any remaining bytes fewer than 8 are padded with 0xFF (erased flash value):

```c
uint8_t pad_buffer[FLASH_WRITE_ALIGN];
memset(pad_buffer, 0xFF, sizeof(pad_buffer));
memcpy(pad_buffer, source_data, remaining);
memcpy(pad_aligned, pad_buffer, FLASH_WRITE_ALIGN);
```

---

## 2. Flash Erase and Write Require `__disable_irq()` / `__enable_irq()`

### Why

The MSPM0 flash controller cannot be read while an erase or program operation
is in progress.  If an interrupt fires during a flash operation and the ISR
tries to execute code from flash (which is almost all code), the CPU will hard
fault.

### The solution

Both `erase_flash_page()` and `write_flash_bytes()` bracket the flash operation
with `__disable_irq()` / `__enable_irq()`:

```c
__disable_irq();
/* ... erase or write ... */
__enable_irq();
```

Each early-return error path must also call `__enable_irq()` before returning.

### Contrast with other platforms

- **STM32F407:** The flash controller handles concurrent reads (from a different
  bank or while the CPU stalls), so interrupt disable is not strictly required.
- **ESP32:** Flash operations go through the partition API which handles
  synchronisation internally.
- **dsPIC33:** Flash operations use NVM controller with WREN bit; MCC's
  `FLASH_WriteRow24()` internally handles the unlock sequence.

---

## 3. `.noinit` Cleanup Must Use the `request` Parameter, Not `DL_SYSCTL_getResetCause()`

### Problem

The original `initialize_hardware()` checked `DL_SYSCTL_getResetCause()` to
decide whether to clear shared `.noinit` variables.  This approach is fragile:

- TI's reset-cause register bits may be sticky across resets.
- After a programmer reset, `bootloader_cached_alias` in `.noinit` RAM contains
  garbage.  A stale 16-bit value used as a 12-bit CAN alias overflows the alias
  field and corrupts the CAN identifier.

### Solution

`initialize_hardware(request)` bases all cleanup on the `request` parameter:

```c
void BootloaderDriversOpenlcb_initialize_hardware(bootloader_request_t request) {
    bootloader_request_flag = 0;

    if (request != BOOTLOADER_REQUESTED_BY_APP) {
        bootloader_cached_alias = 0;
    }

    __disable_irq();
    SYSCFG_DL_init();
    __enable_irq();
}
```

No `DL_SYSCTL_getResetCause()` inspection is needed.  The `request` parameter
from `is_bootloader_requested()` is unambiguous.

This pattern is consistent across all platforms (dsPIC, STM32, ESP32).

---

## 4. Sector Unprotect Before Every Erase and Write

### Why

The MSPM0 flash controller requires explicit sector unprotection before any
erase or program operation:

```c
DL_FlashCTL_unprotectSector(FLASHCTL, address, DL_FLASHCTL_REGION_SELECT_MAIN);
```

Forgetting this call results in a silent failure — the erase/write command
completes but flash contents are unchanged.

### Clear status first

Before each operation, call `DL_FlashCTL_executeClearStatus(FLASHCTL)` to
reset the flash controller's status register.  Stale error flags from a
previous operation can cause the next operation to report failure even if it
would have succeeded.

---

## 5. The Bootloader Jumps via the Vector Table (ARM Cortex-M Pattern)

### How the jump works

The MSPM0G3507 is an ARM Cortex-M0+.  The bootloader jumps to the application
using the same vector-table pattern as STM32:

```c
uint32_t *app_vectors = (uint32_t *) APP_FLASH_START;
SCB->VTOR = APP_FLASH_START;
__set_MSP(app_vectors[0]);
void (*app_reset)(void) = (void (*)(void)) app_vectors[1];
app_reset();
```

- `app_vectors[0]` is the application's initial stack pointer
- `app_vectors[1]` is the address of `Reset_Handler`
- `SCB->VTOR` must be updated so the application's interrupt vectors are used

### Peripherals must be torn down first

`cleanup_before_handoff()` stops SysTick, disables all NVIC interrupts, clears
pending flags, and resets the MCAN peripheral:

```c
SysTick->CTRL = 0;
SysTick->LOAD = 0;
SysTick->VAL  = 0;
NVIC->ICER[0] = 0xFFFFFFFF;
NVIC->ICPR[0] = 0xFFFFFFFF;
DL_MCAN_reset(MCAN0_INST);
```

---

## 6. `NO_CHECKSUM` Must Be Enabled During Development

Same as all other platforms: the `bootloader_app_header_t` struct must be
patched by a post-link tool with correct CRC values.  Until that tool is
integrated, define `NO_CHECKSUM` in `bootloader_types.h`.

---

## 7. Shared RAM Requires a `.noinit` Linker Section

Same pattern as STM32: both the bootloader and application linker scripts must
define a `SHARED_NOINIT` memory region at the **same physical SRAM address**
with a `.noinit (NOLOAD)` section.  The CCS Theia linker uses `.cmd` files
instead of GNU `.ld` files, but the concept is identical:

```
MEMORY
{
    /* ... */
    SHARED_NOINIT : origin = 0x20027FF0, length = 0x0010
}

SECTIONS
{
    .noinit : {} > SHARED_NOINIT type = NOLOAD
}
```

The exact addresses depend on the device's SRAM size and the agreed boundary
between application RAM and shared RAM.

---

## 8. Double-Call Bug in `Bootloader_init()` (Library Fix)

The old `Bootloader_init()` called `is_bootloader_requested()` a second time
internally.  Since the first call (in `Bootloader_entry()`) had already consumed
the magic flag, the second call always returned `BOOTLOADER_NOT_REQUESTED`.
On app drop-back, the bootloader would bounce back to the application.

**Fix (library-level):** `Bootloader_init()` now takes a `bootloader_request_t request`
parameter.  `Bootloader_entry()` calls `is_bootloader_requested()` once and
passes the result through.

---

## 9. SysTick as 100 ms Timer Source

The bootloader uses SysTick for the 100 ms tick needed by CAN alias
negotiation.  The `SysTick_Handler` ISR increments a counter:

```c
static volatile uint8_t _tick_counter = 0;

void SysTick_Handler(void) {
    _tick_counter++;
}
```

`get_100ms_timer_tick()` returns this counter.  SysTick must be configured
(via `SYSCFG_DL_init()`) to fire every 100 ms.

**Important:** SysTick must be stopped in `cleanup_before_handoff()` before
jumping to the application, or the application's SysTick configuration will
conflict.

---

## Summary Checklist for a New TI MSPM0 Bootloader Port

- [ ] Align all flash writes to 8 bytes; pad trailing data with 0xFF
- [ ] Use `memcpy()` through aligned buffers — never cast `uint8_t*` to `uint32_t*`
- [ ] Disable interrupts around flash erase/write operations
- [ ] Call `DL_FlashCTL_executeClearStatus()` and `DL_FlashCTL_unprotectSector()` before every flash operation
- [ ] Add `SHARED_NOINIT` region to both linker scripts at the same address
- [ ] Clear `bootloader_cached_alias` in `initialize_hardware()` except on app drop-back
- [ ] Enable `NO_CHECKSUM` until the post-link CRC tool is ready
- [ ] Stop SysTick, disable NVIC, reset MCAN in `cleanup_before_handoff()`
- [ ] Verify `Bootloader_init()` signature matches library (takes `request` parameter)

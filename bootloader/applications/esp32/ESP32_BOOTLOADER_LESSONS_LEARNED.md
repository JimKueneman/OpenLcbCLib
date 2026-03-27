# ESP32 Bootloader — Lessons Learned

This document records the non-obvious problems, surprises, and design decisions
encountered while porting the OpenLCB standalone bootloader to the ESP32
(ESP32-DevKitC V1) using Arduino framework + ESP-IDF.

---

## 1. ESP32 Uses OTA Partitions, Not Raw Flash Addresses

### How it differs from bare-metal targets

On STM32, dsPIC33, and TI MSPM0, the bootloader writes firmware directly to a
known flash address range.  On ESP32, flash is managed by the ESP-IDF partition
table.  The bootloader must use the OTA (Over-The-Air) API to target the
inactive OTA partition:

```c
_ota_partition = esp_ota_get_next_update_partition(NULL);
```

All flash operations (erase, write, read) convert the library's mapped address
back to a partition-relative offset:

```
part_offset = mapped_addr - ESP32_FLASH_MMAP_BASE - partition->address
```

`finalize_flash()` calls `esp_ota_set_boot_partition()` to commit the update.
On the next `esp_restart()`, the ROM second-stage bootloader boots from the
newly written partition.

### Why `jump_to_application()` is just `esp_restart()`

There is no bare-metal vector-table jump on ESP32.  The ROM bootloader always
runs first and selects the active OTA partition.  The bootloader library's
`jump_to_application()` simply triggers `esp_restart()` and the ROM bootloader
handles the rest.

---

## 2. Shared RAM Uses RTC NOINIT, Not a `.noinit` Linker Section

### The problem

ESP32's normal SRAM is zeroed by the ROM bootloader on every reset.  The
standard `.noinit` section trick used on STM32 and dsPIC does not work.

### The solution

ESP-IDF provides `RTC_NOINIT_ATTR`, which places variables in RTC slow memory.
This region survives both software resets (`esp_restart()`) and deep sleep:

```c
RTC_NOINIT_ATTR volatile uint32_t bootloader_request_flag;
RTC_NOINIT_ATTR volatile uint16_t bootloader_cached_alias;
```

No linker script modifications are needed — ESP-IDF's default linker script
already defines the `rtc_noinit` segment.

### Caveat

RTC NOINIT memory contains garbage after a full power cycle, just like `.noinit`
on other platforms.  The same cleanup pattern applies: `initialize_hardware()`
must clear stale values on every reset except app drop-back.

---

## 3. `.noinit` Cleanup Must Use the `request` Parameter, Not `esp_reset_reason()`

### Problem

The original `initialize_hardware()` checked `esp_reset_reason()` to decide
whether to clear shared RTC NOINIT variables.  This approach is fragile:

- `esp_reset_reason()` returns the reason for the MOST RECENT reset, but the
  RTC NOINIT values may have been written several resets ago.
- After POR, `esp_reset_reason()` returns `ESP_RST_POWERON`, but the RTC memory
  may contain stale garbage from a previous session.
- The `bootloader_cached_alias` is 16 bits but the CAN alias field is 12 bits.
  A stale value like 0xFFFF overflows into the MTI field, producing a corrupt
  CAN frame.

### Solution

`initialize_hardware(request)` now bases all cleanup on the `request` parameter:

```c
void BootloaderDriversOpenlcb_initialize_hardware(bootloader_request_t request) {
    bootloader_request_flag = 0;  /* always clear */

    if (request != BOOTLOADER_REQUESTED_BY_APP) {
        bootloader_cached_alias = 0;  /* stale on POR/button/programmer */
    }

    /* ... peripheral init ... */
}
```

No `esp_reset_reason()` inspection is needed.  The `request` parameter from
`is_bootloader_requested()` is unambiguous.

This pattern is consistent across all platforms (dsPIC, STM32, TI MSPM0).

---

## 4. Flash Write Alignment (4-Byte Minimum)

### The constraint

ESP32 flash writes must be aligned to 4 bytes.  The `esp_partition_write()`
API enforces this — writes of fewer than 4 bytes, or writes to non-4-byte-aligned
offsets, will fail with `ESP_ERR_INVALID_SIZE`.

### How the driver handles it

`write_flash_bytes()` processes data in `FLASH_WRITE_ALIGN` (4-byte) chunks.
Any trailing sub-4-byte remainder is padded with 0xFF (erased flash value) and
written as a full 4-byte block:

```c
if (remaining > 0U) {
    uint8_t pad[FLASH_WRITE_ALIGN];
    memset(pad, 0xFF, sizeof(pad));
    memcpy(pad, source_data, remaining);
    esp_partition_write(_ota_partition, part_offset, pad, FLASH_WRITE_ALIGN);
}
```

---

## 5. TWAI (CAN) Peripheral Must Be Stopped Before Handoff

### Why

The ESP32 TWAI (Two-Wire Automotive Interface, ESP32's CAN controller) must be
stopped and its driver uninstalled before restarting into the application.  If
left running, the application's TWAI initialisation will fail because the driver
is already installed.

```c
void BootloaderDriversOpenlcb_cleanup_before_handoff(void) {
    twai_stop();
    twai_driver_uninstall();
}
```

### Contrast with STM32

On STM32, `cleanup_before_handoff()` also stops SysTick and disables all NVIC
interrupts.  On ESP32, the ROM bootloader handles CPU state reset, so only
peripheral teardown is needed.

---

## 6. `NO_CHECKSUM` Must Be Enabled During Development

The same as STM32 and TI: the `bootloader_app_header_t` struct must be patched
by a post-link tool with the correct CRC values.  Until that tool is integrated,
define `NO_CHECKSUM` in `bootloader_types.h` so the bootloader skips CRC
validation and jumps to the application based solely on the first-word blank check.

---

## 7. Double-Call Bug in `Bootloader_init()` (Library Fix)

The old `Bootloader_init()` called `is_bootloader_requested()` a second time
internally.  Since the first call (in `Bootloader_entry()`) had already consumed
the magic flag, the second call always returned `BOOTLOADER_NOT_REQUESTED`.
On app drop-back, the bootloader would bounce back to the application instead of
staying in bootloader mode.

**Fix (library-level):** `Bootloader_init()` now takes a `bootloader_request_t request`
parameter.  `Bootloader_entry()` calls `is_bootloader_requested()` once and
passes the result through.

---

## Summary Checklist for a New ESP32 Bootloader Port

- [ ] Use `RTC_NOINIT_ATTR` for shared RAM variables (not `.noinit` section)
- [ ] Clear `bootloader_cached_alias` in `initialize_hardware()` on every reset except app drop-back
- [ ] Use `esp_ota_get_next_update_partition()` to resolve the write target
- [ ] Call `esp_ota_set_boot_partition()` in `finalize_flash()` to commit the update
- [ ] Pad flash writes to 4-byte alignment
- [ ] Stop and uninstall TWAI driver in `cleanup_before_handoff()`
- [ ] Enable `NO_CHECKSUM` until the post-link CRC tool is ready
- [ ] Verify `Bootloader_init()` signature matches library (takes `request` parameter)

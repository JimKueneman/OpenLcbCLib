# How To Modify This Demo For Your ESP32

This guide walks through every change needed to adapt the ESP32-DevKitC V1
bootloader demo to a different ESP32 board or variant. It covers both the
BasicNodeBootloader project (the bootloader itself) and the BasicNode project
(the application it boots).

The demo is built around two PlatformIO projects that share a common RTC NOINIT
RAM region for handshake data. Both projects need coordinated changes when you
retarget.

**Key difference from STM32/dsPIC/MSPM0:** The ESP32 does not use custom linker
scripts or direct flash address management. Flash is managed by the ESP-IDF
partition table and OTA API. There is no vector-table jump -- the ROM
second-stage bootloader selects the active partition on every reset.

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Understand the Flash Layout](#2-understand-the-flash-layout)
3. [Create the PlatformIO Projects](#3-create-the-platformio-projects)
4. [Partition Table](#4-partition-table)
5. [Flash Constants](#5-flash-constants)
6. [App Header Offset](#6-app-header-offset)
7. [Flash Operations](#7-flash-operations)
8. [CAN Configuration](#8-can-configuration)
9. [GPIO and LEDs](#9-gpio-and-leds)
10. [Boot Button](#10-boot-button)
11. [Cleanup Before Handoff](#11-cleanup-before-handoff)
12. [Shared RAM (Handshake)](#12-shared-ram-handshake)
13. [Application app_header.c](#13-application-app_headerc)
14. [Node ID](#14-node-id)
15. [Post-Link Checksum Tool](#15-post-link-checksum-tool)
16. [Building and Flashing](#16-building-and-flashing)
17. [Verification Checklist](#17-verification-checklist)

---

## 1. Prerequisites

- PlatformIO Core (CLI) or PlatformIO IDE (VS Code extension) installed
- The datasheet and technical reference manual for your ESP32 variant
- Know which GPIO pins your board uses for CAN TX/RX and any LEDs/buttons
- A CAN transceiver module (e.g. SN65HVD230) connected to the ESP32

### Supported ESP32 variants

The demo targets the original ESP32 (dual-core Xtensa). It should work on
ESP32-S2, ESP32-S3, ESP32-C3, and ESP32-C6 with minor pin and peripheral
changes, but those variants have not been tested.

---

## 2. Understand the Flash Layout

On ESP32, flash is not divided by linker scripts. Instead, ESP-IDF uses a
**partition table** that defines named regions in SPI flash. The bootloader
uses the OTA (Over-The-Air) partition scheme:

```
0x00000000 +---------------------------+
           | ESP-IDF 2nd-stage         |
           | bootloader (factory)      |
0x00009000 +---------------------------+
           | NVS (20 KB)               |  Key-value store for node ID, etc.
0x0000E000 +---------------------------+
           | OTA data (8 KB)           |  Tracks which OTA slot is active
0x00010000 +---------------------------+
           | ota_0 (1920 KB)           |  Application or bootloader image
           |   +-- app_header at +0x120|
0x001F0000 +---------------------------+
           | ota_1 (1920 KB)           |  The other image
           |   +-- app_header at +0x120|
0x003D0000 +---------------------------+
           | (unused, 192 KB)          |
0x00400000 +---------------------------+  4 MB flash end
```

**Key differences from bare-metal targets:**

- There is no "bootloader flash region" that you size manually. Both the
  bootloader and the application are full ESP-IDF images that each occupy an
  OTA slot.
- The ESP-IDF ROM bootloader (not ours) runs first on every reset and picks
  the active OTA partition from the `otadata` region.
- `esp_ota_get_next_update_partition(NULL)` always returns the INACTIVE slot
  (the one not currently running), which is the write target.
- Flash addresses are partition-relative, not absolute. All flash operations
  use `esp_partition_read()`, `esp_partition_write()`, and
  `esp_partition_erase_range()` with a byte offset from the partition start.

---

## 3. Create the PlatformIO Projects

Start with two PlatformIO projects:

- **BasicNodeBootloader** -- the bootloader. Framework: `arduino`.
- **BasicNode** -- your application. Framework: `arduino`.

Both projects use `platform = espressif32` and `board = esp32dev` (or your
board). The `board` setting controls flash size, PSRAM, upload speed, etc.

**Project structure:**

```
esp32/
  shared/
    bootloader_shared_ram.h     -- shared between both projects
    bootloader_shared_ram.c     -- RTC NOINIT variable definition
    partitions.csv              -- partition table (shared)
  BasicNodeBootloader/
    platformio.ini
    src/
      main.cpp                  -- wires DI structs, calls Bootloader_entry()
      application_drivers/
        bootloader_drivers_can.c/.h
        bootloader_drivers_openlcb.c/.h
      openlcb_c_lib_bootloader/ -- symlink to bootloader/src/
  BasicNode/
    platformio.ini
    src/
      main.cpp                  -- normal application entry point
      app_header.c              -- linker-placed CRC header
      application_callbacks/
        callbacks_config_mem.c  -- freeze handler drops back to bootloader
      openlcb_c_lib/            -- symlink to main library src/
```

**Lesson learned:** Both projects must include the `shared/` directory in their
build source filter:

```ini
build_src_filter = ...
                   +<../../shared/*.c>
                   +<../../shared/*.h>
```

---

## 4. Partition Table

**File:** `shared/partitions.csv`

Both `platformio.ini` files reference the same partition table:

```ini
board_build.partitions = ../../shared/partitions.csv
```

The demo partition table provides two equal-sized OTA slots on a 4 MB flash:

```csv
# Name,     Type, SubType, Offset,     Size,       Flags
nvs,        data, nvs,     0x9000,     0x5000,
otadata,    data, ota,     0xE000,     0x2000,
ota_0,      app,  ota_0,   0x10000,    0x1E0000,
ota_1,      app,  ota_1,   0x1F0000,   0x1E0000,
```

**What to change for your board:**

| Flash size | Adjustment |
|------------|------------|
| 2 MB | Shrink each OTA slot to ~960 KB (0xF0000) |
| 8 MB | Enlarge each OTA slot, or add a data partition |
| 16 MB | Same -- use larger slots or extra partitions |

**Constraints:**

- Each OTA slot must be large enough for your application image (typically
  800 KB - 1.5 MB with Arduino framework overhead).
- The `otadata` partition is required for OTA to work. Do not remove it.
- NVS is used for persistent node ID storage. Size 0x5000 (20 KB) is typical.
- Both OTA slots should be the same size.

**Important:** If you use the default PlatformIO partition table (no
`board_build.partitions` line), it may not include OTA slots, and the OTA API
calls will fail at runtime.

---

## 5. Flash Constants

**File:** `bootloader_drivers_openlcb.h`

```c
#define ESP32_FLASH_MMAP_BASE      0x3F400000UL   /* DROM window base */
#define ESP32_FLASH_SECTOR_SIZE    4096U           /* 4 KB erase granularity */
#define FLASH_WRITE_ALIGN          4U              /* 4-byte write alignment */
```

**What to change:**

- `ESP32_FLASH_MMAP_BASE` -- this is the same for all ESP32 variants
  (0x3F400000). For ESP32-S2/S3, check the technical reference manual for the
  DROM mapping base address (may differ).
- `ESP32_FLASH_SECTOR_SIZE` -- 4 KB on all current ESP32 variants.
- `FLASH_WRITE_ALIGN` -- 4 bytes on all current ESP32 variants.

Unlike STM32, there is no sector table because ESP32 flash has uniform 4 KB
sectors. The `get_flash_page_info()` function is a simple mask-and-align:

```c
*page_start        = address & ~((uint32_t)(ESP32_FLASH_SECTOR_SIZE - 1U));
*page_length_bytes = ESP32_FLASH_SECTOR_SIZE;
```

---

## 6. App Header Offset

**File:** `bootloader_drivers_openlcb.h`

```c
#define APP_HEADER_OFFSET          0x00000120UL
```

The ESP32 app header lives in the `.rodata_custom_desc` linker section, which
ESP-IDF places at a fixed offset of 0x120 in the firmware.bin image:

```
Offset  Content
0x00    esp_image_header_t          (24 bytes)
0x18    esp_image_segment_header_t  (8 bytes)
0x20    esp_app_desc_t              (256 bytes, ends at 0x11F)
0x120   .rodata_custom_desc         <-- app_header
```

**Do not change this value** unless you are certain your ESP-IDF version places
`.rodata_custom_desc` at a different offset. The 0x120 offset is determined by
the ESP-IDF linker script template, not by us.

**Why 0x120 and not a custom offset:** On bare-metal targets (STM32, dsPIC,
MSPM0), you choose the app header offset freely and place it with a custom
linker section. On ESP32, the firmware.bin is a segment-based image format with
a mandatory `esp_app_desc_t` structure at offset 0x20. You cannot insert a
custom section at an arbitrary file offset. The `.rodata_custom_desc` section
is the ESP-IDF-provided extension point immediately after `esp_app_desc_t`.

**Lesson learned:** The original demo used 0x100, which falls INSIDE the
256-byte `esp_app_desc_t` structure. This would have corrupted the ELF SHA256
hash and reserved fields. Always use 0x120.

---

## 7. Flash Operations

**File:** `bootloader_drivers_openlcb.c`

### Address translation

All flash operations convert a library-facing "mapped address" to a
partition-relative byte offset:

```c
static uint32_t _mapped_to_offset(uint32_t mapped_addr) {
    return mapped_addr - ESP32_FLASH_MMAP_BASE - _ota_partition->address;
}
```

This pattern is unique to ESP32. On STM32/dsPIC, the flash address IS the
physical address. On ESP32, the library sees the DROM-mapped address but must
use a partition-relative offset for all `esp_partition_*()` calls.

### Inactive partition is not memory-mapped

The ESP32 MMU only maps the RUNNING partition into the DROM address space. The
INACTIVE partition (the one the bootloader writes to) is NOT accessible via
`memcpy()`. All reads must use `esp_partition_read()`:

```c
void BootloaderDriversOpenlcb_read_flash_bytes(uint32_t flash_addr,
                                                void *dest_ram,
                                                uint32_t size_bytes) {
    _ensure_ota_partition();
    uint32_t part_offset = _mapped_to_offset(flash_addr);
    esp_partition_read(_ota_partition, part_offset, dest_ram, size_bytes);
}
```

### Write alignment

ESP32 flash writes must be a multiple of 4 bytes. The driver pads any trailing
remainder with 0xFF:

```c
if (remaining > 0U) {
    uint8_t pad[FLASH_WRITE_ALIGN];
    memset(pad, 0xFF, sizeof(pad));
    memcpy(pad, source_data, remaining);
    esp_partition_write(_ota_partition, part_offset, pad, FLASH_WRITE_ALIGN);
}
```

### Checksum computation

Because the inactive partition is not memory-mapped, the checksum function
reads flash in 256-byte chunks and accumulates CRC-16-IBM incrementally:

```c
#define CHECKSUM_CHUNK_SIZE 256U
uint8_t chunk[CHECKSUM_CHUNK_SIZE];

while (remaining > 0) {
    uint32_t to_read = (remaining < CHECKSUM_CHUNK_SIZE) ? remaining : CHECKSUM_CHUNK_SIZE;
    esp_partition_read(_ota_partition, part_offset, chunk, to_read);
    /* ... feed bytes into CRC accumulators ... */
}
```

This keeps stack usage bounded regardless of image size. The CRC nibble tables
are duplicated from `bootloader_crc.c` to avoid changing the core library API.

### Finalize flash

After all firmware data is written, `finalize_flash()`:

1. Reads the app header from the newly written partition via `esp_partition_read()`
2. Validates `app_size` is within partition bounds
3. Recomputes pre-header and post-header CRCs and compares them against the
   stored values
4. If valid, calls `esp_ota_set_boot_partition()` to commit the update

The `app_size` value in the header is the end of segment data in the
firmware.bin, which excludes the trailing ESP-IDF checksum byte and SHA256 hash.
This avoids a circular dependency (see Section 15).

---

## 8. CAN Configuration

**File:** `bootloader_drivers_can.c`, `bootloader_drivers_openlcb.c`

### TWAI peripheral

ESP32 uses the TWAI (Two-Wire Automotive Interface) peripheral for CAN. The
demo initialises it in `initialize_hardware()`:

```c
twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
    GPIO_NUM_21,    /* TX pin */
    GPIO_NUM_22,    /* RX pin */
    TWAI_MODE_NORMAL);

twai_timing_config_t  t_config = TWAI_TIMING_CONFIG_125KBITS();
twai_filter_config_t  f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

twai_driver_install(&g_config, &t_config, &f_config);
twai_start();
```

**What to change:**

- **TX/RX pins:** Change `GPIO_NUM_21` and `GPIO_NUM_22` to match your board's
  CAN transceiver connections. Nearly any GPIO pin can be used on ESP32 -- the
  TWAI peripheral is routed through the GPIO matrix.
- **Baud rate:** OpenLCB uses 125 kbps. The `TWAI_TIMING_CONFIG_125KBITS()`
  macro handles this automatically for the ESP32's APB clock. No manual
  prescaler calculation needed.

### Polling mode

The bootloader uses zero-timeout polling -- no FreeRTOS task or interrupt
handler:

```c
/* RX: return immediately if no frame available */
esp_err_t err = twai_receive(&message, 0);

/* TX: return false immediately if queue is full */
return (twai_transmit(&message, 0) == ESP_OK);
```

The TWAI TX FIFO preserves insertion order, so the one-frame-at-a-time
workaround needed on STM32 bxCAN is not required here.

### Pin mapping for different boards

| Board | Typical TX | Typical RX | Notes |
|-------|-----------|-----------|-------|
| ESP32-DevKitC V1 | GPIO 21 | GPIO 22 | Demo default |
| ESP32-WROOM-32 | Any GPIO | Any GPIO | Check your transceiver wiring |
| ESP32-S3 | Any GPIO | Any GPIO | Uses TWAI peripheral (same API) |
| ESP32-C3 | GPIO 0-5 | GPIO 0-5 | Fewer pins, check availability |

---

## 9. GPIO and LEDs

**File:** `bootloader_drivers_openlcb.h`, `bootloader_drivers_openlcb.c`

```c
#define BOOTLOADER_LED_PIN         2    /* built-in blue LED */
```

The demo multiplexes all bootloader LED functions onto the single built-in LED
on GPIO 2. The LED functions are:

| Function | What it signals |
|----------|----------------|
| BOOTLOADER_LED_ACTIVE | Blinking while bootloader is running |
| BOOTLOADER_LED_WRITING | Flash write in progress |
| BOOTLOADER_LED_CSUM_ERROR | Checksum validation failed |
| BOOTLOADER_LED_REQUEST | Bootloader was explicitly requested by app |
| BOOTLOADER_LED_FRAME_LOST | CAN frame dropped |

**What to change:**

- If your board has a different LED pin, change `BOOTLOADER_LED_PIN`.
- If you have multiple LEDs, add GPIO defines and update `set_status_led()` to
  map each `bootloader_led_enum` to a separate pin.
- If you have no LEDs, make `set_status_led()` a no-op.

---

## 10. Boot Button

**File:** `bootloader_drivers_openlcb.h`, `bootloader_drivers_openlcb.c`

```c
#define BOOTLOADER_BOOT_BUTTON_PIN 0    /* GPIO 0, active LOW */
```

Holding GPIO 0 low during power-on forces bootloader mode. Most ESP32 boards
have a "BOOT" button on GPIO 0 that is already wired active-low with an
external pull-up.

```c
if (gpio_get_level((gpio_num_t) BOOTLOADER_BOOT_BUTTON_PIN) == 0) {
    return BOOTLOADER_REQUESTED_BY_BUTTON;
}
```

**What to change:**

- If your board has the boot button on a different pin, change the define.
- If your button is active-high, flip the comparison to `== 1`.
- If you have no button, remove the button check. The app-to-bootloader magic
  RAM handshake still works without it.

---

## 11. Cleanup Before Handoff

**File:** `bootloader_drivers_openlcb.c`

```c
void BootloaderDriversOpenlcb_cleanup_before_handoff(void) {
    twai_stop();
    twai_driver_uninstall();
}
```

This tears down the TWAI driver before restarting. If left running, the
application's TWAI initialisation will fail because the driver is already
installed.

**Contrast with STM32:** On STM32, `cleanup_before_handoff()` also stops
SysTick and disables all NVIC interrupts. On ESP32, the ROM bootloader
handles CPU state reset, so only peripheral teardown is needed.

**What to change:** If you add other peripherals to the bootloader (SPI,
I2C, etc.), de-initialise them here.

---

## 12. Shared RAM (Handshake)

**Files:** `shared/bootloader_shared_ram.h`, `shared/bootloader_shared_ram.c`

### How it works

The application drops back to the bootloader by writing a magic value to RTC
NOINIT memory and calling `esp_restart()`:

```c
/* In the application's freeze callback: */
bootloader_shared_ram.cached_alias = statemachine_info->openlcb_node->alias;
bootloader_shared_ram.request_flag = BOOTLOADER_REQUEST_MAGIC;  /* 0xB00710AD */
esp_restart();
```

After the reset, the bootloader checks `request_flag`. If it matches the
magic value, it enters firmware-update mode using the cached alias.

### Why RTC NOINIT (not .noinit)

ESP32's normal SRAM is zeroed by the ROM bootloader on every reset. The
`.noinit` section trick used on STM32 does not work. ESP-IDF provides
`RTC_NOINIT_ATTR`, which places variables in RTC slow memory. This region
survives `esp_restart()` and deep sleep but contains garbage after a true
power-on or brownout.

```c
/* bootloader_shared_ram.c */
RTC_NOINIT_ATTR bootloader_shared_ram_t bootloader_shared_ram;
```

No linker script modifications are needed -- ESP-IDF's default linker script
already defines the `rtc_noinit` segment.

### Cleanup

`initialize_hardware()` clears the handshake variables based on the `request`
parameter:

```c
bootloader_shared_ram.request_flag = 0;  /* always clear */

if (request != BOOTLOADER_REQUESTED_BY_APP) {
    bootloader_shared_ram.cached_alias = 0;  /* stale on POR/button */
}
```

**Lesson learned:** Do not use `esp_reset_reason()` for cleanup decisions. It
returns the reason for the most recent reset, but the RTC NOINIT values may
have been written several resets ago. The `request` parameter from
`is_bootloader_requested()` is unambiguous.

### Struct wrapper

The fields are wrapped in a struct to prevent the linker from reordering them
independently between the bootloader and application builds:

```c
typedef struct {
    volatile uint32_t request_flag;
    volatile uint16_t cached_alias;
} bootloader_shared_ram_t;
```

**What to change:** Nothing, unless you add fields to the handshake. If you do,
add them inside the struct and recompile both projects.

---

## 13. Application app_header.c

**File:** `BasicNode/src/app_header.c`

This file places a 36-byte zero-initialized struct in the `.rodata_custom_desc`
linker section. ESP-IDF positions this section at file offset 0x120 in the
firmware.bin, immediately after `esp_app_desc_t`.

```c
__attribute__((section(".rodata_custom_desc")))
__attribute__((used))
const app_header_image_t app_header = { 0U };
```

**Critical rules:**

1. This must be a `.c` file, not `.cpp`. There is a known ESP-IDF bug where
   `__attribute__((section(".rodata_custom_desc")))` is silently ignored in
   C++ translation units.
2. The `__attribute__((used))` prevents the linker from discarding the symbol.
3. The `platformio.ini` must include `-u app_header` in `build_flags` to force
   the linker to pull in the symbol:

```ini
build_flags = -std=gnu99 -u app_header
```

4. The `_Static_assert` guard catches size mismatches at compile time if the
   library's `bootloader_app_header_t` changes.

**What to change:** Nothing. Copy this file as-is to your application project.

---

## 14. Node ID

**File:** `bootloader_drivers_openlcb.c`

The bootloader reads the 48-bit OpenLCB node ID from ESP-IDF NVS
(Non-Volatile Storage):

```c
uint64_t BootloaderDriversOpenlcb_get_persistent_node_id(void) {
    nvs_flash_init();
    nvs_handle_t handle;
    uint64_t node_id = 0x050101012201ULL;  /* fallback */

    if (nvs_open("openlcb", NVS_READONLY, &handle) == ESP_OK) {
        uint64_t stored;
        if (nvs_get_u64(handle, "node_id", &stored) == ESP_OK) {
            node_id = stored;
        }
        nvs_close(handle);
    }
    return node_id;
}
```

**What to change:**

- Change the fallback node ID to your own unique value.
- The application must write the node ID to NVS during production programming
  (namespace `"openlcb"`, key `"node_id"`, stored as `uint64_t`).
- Every OpenLCB node must have a globally unique 48-bit node ID.

---

## 15. Post-Link Checksum Tool

After building the application, run `hex2bin.py` to produce a
checksum-patched binary:

```bash
python3 hex2bin.py \
    --input-format bin \
    --arch esp32 \
    --app-header-offset 0x120 \
    firmware.bin \
    BasicNode.esp32.debug.boot.bin
```

### What `--arch esp32` does

The `--arch esp32` flag tells `hex2bin.py` to perform three extra steps after
patching the CRC fields:

1. **Recomputes the ESP-IDF XOR checksum byte** -- the firmware.bin has a
   trailing XOR checksum (seed 0xEF) over all segment data. Patching the
   app_header CRC fields (which are inside the DROM segment) invalidates it.
2. **Recomputes the SHA256 hash** -- if the `append_digest` flag is set (byte
   0x17 in the image header), the firmware.bin has a trailing 32-byte SHA256
   hash. This is also recomputed.
3. **Sets `app_size` to the segment data end** -- the checksum byte and SHA256
   hash depend on the CRC values in the header, creating a circular dependency.
   To break it, `app_size` is set to the end of segment data (before the
   trailing checksum padding). The on-target bootloader reads `app_size` from
   the header and uses it to determine the CRC extent, so both sides agree.

### Build script

Add your target to `create_boot_images.sh`:

```bash
ESP32_BIN="${APPS_DIR}/esp32/BasicNode/.pio/build/esp32dev/firmware.bin"
ESP32_OUT="$(dirname "${ESP32_BIN}")/BasicNode.esp32.debug.boot.bin"
if [ -f "${ESP32_BIN}" ]; then
    echo "  BUILD ${ESP32_OUT}"
    python3 "${HEX2BIN}" \
        --input-format bin \
        --arch esp32 \
        --app-header-offset 0x120 \
        "${ESP32_BIN}" \
        "${ESP32_OUT}"
else
    echo "  SKIP  ${ESP32_BIN} (not found)"
fi
```

### NO_CHECKSUM during development

The shared `bootloader_types.h` has a `#define NO_CHECKSUM` that disables all
checksum validation -- both at boot and after firmware write. With this flag
enabled the bootloader only checks that flash is not blank before jumping.

**Leave `NO_CHECKSUM` enabled** until your post-link checksum tool is
integrated into the build. Without it, every application image will fail the
CRC check and the bootloader will refuse to jump. Comment it out once
`hex2bin.py` is producing patched binaries for your target.

---

## 16. Building and Flashing

### Initial setup

1. Build BasicNodeBootloader:

```bash
cd bootloader/applications/esp32/BasicNodeBootloader
pio run
```

2. Flash the bootloader to the ESP32 via USB:

```bash
pio run -t upload
```

3. Build BasicNode (the application):

```bash
cd bootloader/applications/esp32/BasicNode
pio run
```

4. Run hex2bin to patch the checksums:

```bash
python3 bootloader/tools/hex2bin/hex2bin.py \
    --input-format bin \
    --arch esp32 \
    --app-header-offset 0x120 \
    .pio/build/esp32dev/firmware.bin \
    .pio/build/esp32dev/BasicNode.esp32.debug.boot.bin
```

5. Flash the application. Because both projects share the same partition
   table, PlatformIO writes to whichever OTA slot is configured. For initial
   setup, use `esptool.py` to write directly to ota_0:

```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 \
    write_flash 0x10000 .pio/build/esp32dev/firmware.bin
```

6. Power-cycle the board. The ROM bootloader selects ota_0 (the application).

### How the boot sequence works

1. ESP32 ROM bootloader runs (every reset)
2. Checks `otadata` partition for the active OTA slot
3. Loads and runs the active slot's image
4. If the active slot is the BOOTLOADER:
   - Checks `request_flag` in RTC NOINIT memory
   - Checks the BOOT button
   - If neither is set, validates the APPLICATION in the other OTA slot
   - If valid, calls `esp_ota_set_boot_partition()` + `esp_restart()` to jump
5. If the active slot is the APPLICATION:
   - Normal application operation
   - On Config Tool Freeze command, writes magic to RTC NOINIT and restarts

### Firmware update via OpenLCB Config Tool

After initial setup, firmware updates are done over the network:

1. Build the application and run `hex2bin.py` to produce the patched `.bin`
2. In the Config Tool, connect to the node
3. Send a Freeze command to the firmware address space (0xEF)
4. The application writes the magic flag + cached alias to RTC NOINIT and
   calls `esp_restart()`
5. The bootloader starts, sees the magic flag, enters firmware-update mode
6. The Config Tool sends the binary data via Memory Configuration datagrams
7. The bootloader erases, writes, and verifies the firmware
8. On success, calls `esp_ota_set_boot_partition()` and reboots
9. The ROM bootloader loads the new application from the updated OTA slot

### Verify with esptool

After building and patching, verify the image structure:

```bash
esptool.py image_info .pio/build/esp32dev/firmware.bin
```

This shows the image header, segments, checksum status, and SHA256 hash.

---

## 17. Verification Checklist

Work through this list after making all changes:

### Partition table
- [ ] `partitions.csv` has `ota_0`, `ota_1`, `otadata`, and `nvs` partitions
- [ ] Both OTA slots are large enough for your application image
- [ ] Both `platformio.ini` files reference the same `partitions.csv`
- [ ] OTA slot sizes are equal

### Flash constants
- [ ] `ESP32_FLASH_MMAP_BASE` matches your ESP32 variant's DROM base
- [ ] `ESP32_FLASH_SECTOR_SIZE` is 4096
- [ ] `FLASH_WRITE_ALIGN` is 4
- [ ] `APP_HEADER_OFFSET` is 0x120

### App header
- [ ] `app_header.c` is in the APPLICATION project (not the bootloader)
- [ ] `app_header.c` is a `.c` file (not `.cpp`)
- [ ] `app_header.c` uses `__attribute__((section(".rodata_custom_desc")))`
- [ ] `app_header.c` uses `__attribute__((used))`
- [ ] `platformio.ini` has `-u app_header` in `build_flags`
- [ ] `_Static_assert` verifies 36-byte size

### CAN
- [ ] TWAI TX/RX pins match your board's CAN transceiver
- [ ] Baud rate = 125 kbps (TWAI_TIMING_CONFIG_125KBITS)
- [ ] Polling mode with zero timeout (no FreeRTOS task)
- [ ] TWAI accept-all filter enabled

### GPIO
- [ ] LED pin matches your board
- [ ] Boot button pin and polarity correct
- [ ] GPIO direction and pull mode configured in `initialize_hardware()`

### Shared RAM handshake
- [ ] Both projects compile `bootloader_shared_ram.c`
- [ ] `RTC_NOINIT_ATTR` used (not `.noinit` section)
- [ ] `initialize_hardware()` clears stale values on cold boot
- [ ] Cleanup uses `request` parameter (not `esp_reset_reason()`)
- [ ] Application freeze callback writes magic + alias before `esp_restart()`

### Checksum tool
- [ ] `hex2bin.py` uses `--arch esp32` and `--app-header-offset 0x120`
- [ ] `hex2bin.py` uses `--input-format bin` (not hex)
- [ ] `hex2bin.py` produces non-zero checksums (not all 0x0000)
- [ ] `esptool.py image_info` shows valid checksum after patching
- [ ] `NO_CHECKSUM` enabled in `bootloader_types.h` during development

### Bootloader cleanup
- [ ] `cleanup_before_handoff()` calls `twai_stop()` + `twai_driver_uninstall()`
- [ ] No other peripherals left running

### Integration
- [ ] Bootloader builds and uploads without errors
- [ ] Application builds and uploads without errors
- [ ] Boot button forces bootloader mode
- [ ] Application drops back to bootloader (magic RAM handshake)
- [ ] Bootloader jumps to application after successful checksum
- [ ] Checksum error LED lights when image is corrupt or unpatched
- [ ] Firmware update via Config Tool completes successfully
- [ ] Node reboots and runs the new firmware after update

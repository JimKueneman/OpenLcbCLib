# OpenLCB Bootloader Porting Guide

## Part 1: Overview

### 1.1 What This Bootloader Does
- OpenLCB Firmware Upgrade Standard (section 5.5) compliance
- Datagram-based firmware transfer (memory space 0xEF)
- Works on any CAN-capable microcontroller

### 1.2 Architecture Summary
- Library code (`bootloader/src/`) is platform-independent
- All hardware-specific code lives in application drivers
- Dependency injection via function pointer structs (not #include)
- Two DI tables: CAN driver and OpenLCB driver

### 1.3 Flash Layout Concept
- Bootloader region: starts at reset vector, small as possible
- Application region: remainder of flash
- Node ID region: small reserved area (optional, can be in app header)
- Application header: fixed offset in app region, contains checksum and metadata

## Part 2: Creating the Bootloader Application

### 2.1 Project Setup
- Create a new project targeting your MCU
- Add the bootloader library source files
- Do NOT include the main OpenLCB library — the bootloader is standalone

### 2.2 Custom Linker Script
- Why you need a custom linker script (flash partitioning)
- Disabling the default linker script (platform-specific instructions)
  - TI CCS Theia (tiarmclang)
  - GCC (arm-none-eabi, avr-gcc, xc16)
  - IAR
- Defining memory regions (BOOT_FLASH, APP_FLASH, NODEID_FLASH, SRAM)
- Calculating sizes: bootloader size + headroom, app gets the rest
- The `.noinit` section: what it is and why you need it
  - Prevents C startup from zeroing a variable
  - Used for the bootloader request magic value handshake
  - Platform-specific syntax for each compiler/chip family

### 2.3 Linker Script Examples by Platform
- TI MSPM0 (tiarmclang / CCS Theia)
  - `.cmd` file format
  - `__attribute__((section(".noinit")))` for the flag variable
- STM32 / ARM GCC (arm-none-eabi-gcc)
  - `.ld` file format
  - Same `__attribute__` syntax
- Nordic nRF52 / NXP LPC (arm-none-eabi-gcc)
  - Same as STM32 GCC — just different memory addresses
- AVR (avr-gcc)
  - `.ld` file format
  - `__attribute__((section(".noinit")))` — same syntax, different addresses
- dsPIC / PIC24 (XC16)
  - `.gld` file format
  - `__attribute__((persistent))` instead of `.noinit`
  - No need for linker section — `persistent` is built-in to XC16
- PIC32 (XC32)
  - `.ld` file format
  - `__attribute__((persistent))` or `__attribute__((section(".persist")))`
- ESP32 (ESP-IDF / GCC Xtensa)
  - Uses `RTC_NOINIT_ATTR` macro (places in RTC slow memory)
  - Regular SRAM is NOT preserved across `esp_restart()`
- Renesas RA / RX (e2 studio GCC or CC-RX)
  - GCC: same as ARM GCC `.noinit` section
  - CC-RX: `#pragma section` syntax
- IAR (any target)
  - `__no_init` keyword

### 2.4 Implementing the CAN Driver (`bootloader_can_driver.h`)
- `initialize()` — set up the CAN peripheral, bit rate, filters
- `transmit()` — send a single CAN frame, return success/fail
- `receive()` — poll for an incoming CAN frame, return 0 or 1
- `timer_ms()` — return a free-running millisecond counter
- Filter configuration: accept all extended frames (29-bit ID)

### 2.5 Implementing the OpenLCB Driver (`bootloader_driver.h`)
- `initialize_hardware()` — MCU init, clock, pins, peripherals
  - Must detect power-on reset and clear `bootloader_request_flag`
  - Platform-specific reset cause register (RSTCAUSE, RCON, RCC_CSR, MCUSR, etc.)
- `is_bootloader_requested()` — returns a `bootloader_request_t` enum:
  - `BOOTLOADER_NOT_REQUESTED` — normal boot, jump to app if checksum valid
  - `BOOTLOADER_REQUESTED_BY_APP` — magic value found in RAM. The application already received Freeze from the CT and dropped back. The library sets `firmware_active = 1` so PIP immediately reports `PSI_FIRMWARE_UPGRADE_ACTIVE`. The CT does NOT need to send Freeze again.
  - `BOOTLOADER_REQUESTED_BY_BUTTON` — hardware button held at power-on. The library starts with `firmware_active = 0` so PIP reports `PSI_FIRMWARE_UPGRADE`. The CT must send Freeze before transferring firmware.
  - Check magic value first (return `REQUESTED_BY_APP`), then button (return `REQUESTED_BY_BUTTON`)
  - Clear the magic value after reading it (so next cold boot doesn't re-enter)
- `jump_to_application()` — relocate vector table (if applicable) and jump
  - The library has already verified the app is present (flash_min[0] != 0xFFFFFFFF) and called `cleanup_before_handoff()` before calling this function — the driver does NOT need to re-check for erased flash or call cleanup
  - Cortex-M: disable IRQs, set VTOR, load MSP from app vector table, jump to reset handler
  - dsPIC: function pointer to app start address (IVT redirection needed separately)
  - AVR: jump to app section or use IVSEL
- `reboot()` — trigger a software reset
  - Cortex-M: `NVIC_SystemReset()`
  - dsPIC: `asm("RESET")`
  - AVR: watchdog reset or `jmp 0`
- `get_persistent_node_id()` — read 6-byte node ID from flash/EEPROM
  - Handle erased flash (all 0xFF) with a fallback for bringup
- `get_flash_boundaries()` — return app flash start, end, and header pointer
- `get_flash_page_info()` — return page/sector start and size for a given address
- `erase_flash_page()` — erase one page/sector
  - Must run from RAM on some chips (MSPM0, some STM32)
  - Return 0 on success, error code on failure
- `write_flash_bytes()` — write data to flash
  - Handle alignment requirements (8-byte on MSPM0, 4-byte on STM32, etc.)
  - Pad the last write if data is not aligned
  - Return 0 on success, error code on failure
- `finalize_flash()` — validate checksum after all writes complete
- `get_cached_alias()` — return the alias the app was using, or 0 for cold boot
  - Reads `bootloader_cached_alias` from shared `.noinit` RAM (defined in `bootloader_types.h`)
  - The application writes its current alias here before resetting
  - Clear the variable after reading (so next cold boot returns 0)
  - Must also be cleared in `initialize_hardware()` on power-on reset alongside the magic flag

### 2.6 The Magic Value Handshake (App-to-Bootloader Drop-Back)
- What the magic value is and why it works
- Why a 32-bit magic value (1-in-4-billion vs 50/50 for a single bit)
- Why SRAM survives software reset but contains garbage after power-on
- The full flow:
  1. App receives Freeze command from Configuration Tool
  2. App writes its current CAN alias to `bootloader_cached_alias`
  3. App writes `BOOTLOADER_REQUEST_MAGIC` to `bootloader_request_flag`
  4. App calls platform-specific software reset
  5. Bootloader starts, `is_bootloader_requested()` sees magic → returns `BOOTLOADER_REQUESTED_BY_APP`
  6. Library sets `firmware_active = 1` (PIP reports Firmware Upgrade Active)
  7. `get_cached_alias()` reads `bootloader_cached_alias` → skips CID negotiation
  8. Bootloader sends Init Complete (one frame)
  9. CT sees Init Complete → proceeds with firmware transfer

### 2.7 The bootloader_main.c
- Wiring the two DI tables (CAN driver and OpenLCB driver)
- The main loop: call the state machine tick function
- No RTOS, no interrupts needed — pure polling

### 2.8 Generating a Raw Binary (.bin) for Firmware Transfer

The CT sends the raw contents of a file over datagram writes.  The application
must be built as a raw binary (.bin) — not ELF (.out) or hex (.hex).

#### TI CCS Theia (tiarmclang)

1. Open **Project → Properties → Build → Arm Objcopy Utility**
2. In the **Format of output file** (`--output-target`, `-O`) field, type `binary` (all lowercase).  If there is a dropdown and `binary` is not listed, click into the text box and type it manually.
3. In the **Output file name** field, change the extension to `.bin`.  Use: `${BuildArtifactFileBaseName}.bin`
4. (Optional) Verify at the bottom of the page — you should see `--output-target binary` or `-O binary` in the command-line summary.
5. Click **Apply and Close**, then **Clean** and **Build**.
6. The `.bin` file appears in the `Debug/` (or `Release/`) folder.

**Warning:** If the `.bin` file is unexpectedly huge (gigabytes), it means there are large gaps between memory sections.  Raw binary format fills gaps with zeros.  Make sure the linker script does not leave holes between flash sections.

#### GCC (arm-none-eabi)

Add a post-build step or Makefile rule:
```
arm-none-eabi-objcopy -O binary MyApp.elf MyApp.bin
```

#### XC16 / XC32 (Microchip)

Use the hex-to-bin conversion in the post-build step or use a tool like `hex2bin`.

## Part 3: Creating the Application (Main Firmware)

### 3.1 Application Linker Script
- Must start at APP_FLASH address, not 0x00000000
- Vector table at app start address
- All code and const data in APP_FLASH region
- Application header at fixed offset (e.g., APP_FLASH_START + 0xC0)

### 3.2 Application Header
- Structure: `bootloader_app_header_t`
- Fields: magic, app_size, checksum, version, etc.
- Placed by the linker at the known offset
- The bootloader reads this to validate the app before jumping

### 3.3 Handling the Freeze Command
- Application must handle Memory Configuration Freeze for space 0xEF
- On receiving Freeze:
  1. Send Datagram OK reply
  2. Write current CAN alias to `bootloader_cached_alias`
  3. Write `BOOTLOADER_REQUEST_MAGIC` to `bootloader_request_flag`
  4. Call software reset (NVIC_SystemReset, asm("RESET"), etc.)
- The bootloader takes over from here — app does NOT send Init Complete
- Both `bootloader_cached_alias` and `bootloader_request_flag` are declared in `bootloader_types.h` and defined with `.noinit` placement in the platform driver

### 3.4 Interrupt Vector Table Considerations
- Cortex-M (MSPM0, STM32, nRF52, etc.): VTOR register handles relocation automatically
  - Bootloader sets VTOR to app address before jumping — done
  - No shared interrupt handling needed
- dsPIC (no alternate IVT): requires interrupt redirection
  - Bootloader owns the IVT at 0x0004
  - Shared jump table in RAM with function pointers to app handlers
  - Bootloader trap handlers check a redirect flag and call through the table
  - Application registers its handlers into the jump table on startup
- AVR: IVSEL bit moves IVT between boot section and app section
  - Some AVRs have this, some don't — check your datasheet

### 3.5 Reporting Firmware Upgrade Support (PIP Bits)
- `PSI_FIRMWARE_UPGRADE` (0x000020) = "I support firmware upgrade" — set when `firmware_active` is 0. Tells the CT: "you can send me a Freeze command to start a firmware transfer."
- `PSI_FIRMWARE_UPGRADE_ACTIVE` (0x000010) = "firmware transfer is in progress" — set when `firmware_active` is 1. Tells the CT: "I am ready to receive firmware data, no Freeze needed."
- These two bits are mutually exclusive — only one is set at a time
- The library handles this automatically based on `firmware_active` state

## Part 4: The Firmware Upgrade Sequence (What Happens on the Bus)

### 4.1 Normal Boot (No Firmware Update)
1. Bootloader starts
2. `is_bootloader_requested()` returns false
3. Bootloader validates app checksum via header
4. If valid: jump to application
5. If invalid: stay in bootloader mode, log in to bus, wait for firmware

### 4.2 Button-Forced Bootloader Entry
1. User holds button during power-on
2. `is_bootloader_requested()` returns `BOOTLOADER_REQUESTED_BY_BUTTON`
3. `firmware_active = 0` — PIP reports `PSI_FIRMWARE_UPGRADE` ("I support it")
4. Bootloader logs in with full CID negotiation (no cached alias)
5. Sends Init Complete
6. Optionally sends PCER "Firmware Upgrade Requested by Hardware Switch"
7. CT discovers node, sends PIP → sees `PSI_FIRMWARE_UPGRADE`
8. CT sends Freeze → bootloader sets `firmware_active = 1`, sends Init Complete
9. CT sends PIP → sees `PSI_FIRMWARE_UPGRADE_ACTIVE` ("transfer in progress")
10. CT sends Write datagrams (64 bytes each, space 0xEF, offset 0+)
11. Bootloader writes data to flash
12. CT sends Unfreeze → bootloader validates checksum, reboots to application

### 4.3 Application Drop-Back (Freeze Command)
1. CT sends Freeze (space 0xEF) to running application
2. Application sends Datagram OK, sets magic, saves alias, resets
3. Bootloader starts, sees magic → `BOOTLOADER_REQUESTED_BY_APP`
4. `firmware_active = 1` — PIP reports `PSI_FIRMWARE_UPGRADE_ACTIVE` immediately
5. Uses cached alias → skips CID → sends Init Complete (one frame)
6. CT was waiting for Init Complete → proceeds
7. CT sends PIP → sees `PSI_FIRMWARE_UPGRADE_ACTIVE` (no Freeze needed)
8. CT sends Write datagrams (64 bytes each, space 0xEF, offset 0+)
9. Bootloader writes data to flash
10. CT sends Unfreeze → bootloader validates checksum, reboots to application

### 4.4 Error Handling During Transfer
- Checksum failure after all writes: respond with error 0x2088
- Incompatible firmware: respond with error 0x1088
- Corrupted data: respond with error 0x1089
- On any error: stay in bootloader mode, accept retry from offset 0
- Must survive power cycle and re-enter bootloader mode

## Part 5: Testing

### 5.1 Unit Testing with Mocks
- All DI functions can be mocked for host-side testing
- Test the state machine logic without real hardware
- Simulate CID negotiation, datagram reception, flash writes

### 5.2 Hardware Testing Checklist
- [ ] Cold boot with valid app → jumps to application
- [ ] Cold boot with erased flash → stays in bootloader, logs in
- [ ] Button held at boot → enters bootloader mode
- [ ] App drop-back via Freeze → bootloader mode with cached alias
- [ ] Full firmware transfer via datagrams → app updated and running
- [ ] Firmware transfer with bad checksum → error, stays in bootloader
- [ ] Power cycle during transfer → re-enters bootloader, accepts retry
- [ ] Unfreeze after successful transfer → reboots to new application

## Appendix A: Feature Flags

Define these in `bootloader_types.h` (or as a compiler `-D` flag) before building the library.

| Flag | Effect |
|---|---|
| `NO_CHECKSUM` | Bypasses `_check_application_checksum()` at boot time and skips checksum validation in `finalize_flash()`. The library still checks `flash_min[0] != 0xFFFFFFFF` before jumping — a blank flash device stays in bootloader mode correctly. Use during development before the post-link checksum tool is integrated. |

---

## Appendix B: Constants and Memory Space Numbers
- Firmware Space: 0xEF
- `BOOTLOADER_REQUEST_MAGIC`: 0xB00710AD
- Well-Known Event IDs:
  - Firmware Corrupted: 01.01.00.00.00.00.06.01
  - Firmware Upgrade Requested by Hardware Switch: 01.01.00.00.00.00.06.02

## Appendix C: Troubleshooting
- Bootloader enters bootloader mode on every power cycle
  - Check that `initialize_hardware()` clears the magic flag on POR
  - Check that `.noinit` section is correctly defined in linker script
- CT never sees Init Complete after Freeze
  - Check that the app actually resets (not stuck in Freeze handler)
  - Check that `get_cached_alias()` returns the correct alias
- Flash writes fail silently
  - Some chips require flash operations to run from RAM
  - Check alignment requirements
- Application doesn't start after successful update
  - Check VTOR is set correctly before jump
  - Check app vector table has valid stack pointer and reset handler
  - Check application linker script starts at the correct address

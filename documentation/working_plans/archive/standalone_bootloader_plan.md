# Standalone C Bootloader Plan

**Status:** COMPLETE -- Standalone bootloader fully implemented. See `bootloader/`
directory for source (`src/`), platform demos (`applications/dspic/`, `applications/stm32_cubeide/`,
`applications/ti_theia/`, `applications/esp32/`), and test suite.

## Context

The current TurnoutBoss bootloader pulls in the entire OpenLcbCLib library (~15,000+ lines) using `OPENLCB_COMPILE_BOOTLOADER`. This wastes ~20-40KB of flash. The goal is a standalone C bootloader (~900 lines) that:

- Uses **datagrams only** (no streams) for firmware transfer
- Implements OpenMRN's **triple CRC-16-IBM** checksum validation (stronger than TurnoutBoss's 8-bit byte sum)
- Is **platform-independent** via driver interfaces — DI used **only** for hardware and transport abstraction
- **Two-layer architecture**: protocol layer (transport-agnostic) + transport layer (CAN, replaceable)
- Runs **polled** (no interrupts, no RTOS, no dynamic memory)

### DI Policy (differs from main library CLAUDE.md)

The main library's CLAUDE.md requires all cross-module calls go through interface structs (function pointers) for testability. **This bootloader is a separate codebase** with a different DI policy:

- **DI is used ONLY** for hardware and transport abstraction (the `extern` functions in `bootloader_driver.h`, `bootloader_can_driver.h`, and `bootloader_transport.h`)
- **No function pointer structs** for protocol internals — protocol modules call each other directly via `#include` and direct function calls
- **Rationale**: code size is the priority; function pointer indirection adds overhead that matters in a bootloader. DI exists solely to make the bootloader portable across hardware and transport layers.

### Reference implementations studied
- OpenMRN `Bootloader.hxx` (1151 lines C++, standalone, single-file state machine)
- OpenMRN `bootloader_driver.h` (17 extern driver functions)
- OpenMRN `Crc.cxx` (table-driven CRC-16-IBM with triple variant)
- TurnoutBoss bootloader (full library, dsHex format, dsPIC33)

---

## File Structure

Same pattern as the main library's application demos (e.g., `applications/xcode/BasicNode/`).
**Top-level folder** — completely self-contained, separate from the main library.

### Repository layout

```
OpenLcbCLib/
    src/                                ← main library (unchanged)
    test/                               ← main library tests (unchanged)
    applications/                       ← main library demos (unchanged)

    bootloader/                         ← NEW — everything bootloader lives here
        src/                 ← copy this folder into IDE projects
            openlcb/                    ← core protocol (don't modify)
                bootloader.c           — main loop, boot decision (~200 lines)
                bootloader.h           — public API
                bootloader_protocol.c  — PIP, SNIP, VerifyNodeID, memconfig (~300 lines)
                bootloader_protocol.h
                bootloader_transport.h — transport interface (extern functions)
                bootloader_driver.h    — hardware driver interface (extern functions)
                bootloader_types.h     — state struct, enums, constants
            crc/                        ← checksum library (don't modify)
                bootloader_crc.c       — software CRC-16-IBM (~80 lines)
                bootloader_crc.h
                bootloader_app_header.h — app_header struct + checksum validation
            drivers/canbus/             ← CAN transport (don't modify, swappable)
                bootloader_can.c       — alias SM, framing, datagram assembly (~300 lines)
                bootloader_can.h
                bootloader_can_driver.h — CAN-specific driver externs
        test/                           ← bootloader tests (CMake, self-contained)
            CMakeLists.txt
            bootloader_Test.cxx
        applications/                   ← example IDE projects per platform
            dspic33/                    ← TurnoutBoss example
            stm32/                      ← STM32 example
```

### IDE project layout (what the user creates)

```
MyBootloaderProject/                    ← IDE project root
    main.c                              ← user writes (calls Bootloader_entry)
    bootloader_user_config.h            ← user writes (SNIP defines, config)
    application_drivers/                ← user writes platform implementations
        dspic33_bootloader_drivers.c    ← implements BootloaderDriver_* + BootloaderCanDriver_*
        dspic33_bootloader_drivers.h
    src/                     ← COPIED from bootloader/src/
        openlcb/                        ← don't touch
        crc/                            ← don't touch
        drivers/canbus/                 ← don't touch
```

This is identical to how the main library works:
- `openlcb_c_lib/` → `src/`
- `application_drivers/` → `application_drivers/`
- `openlcb_user_config.h` → `bootloader_user_config.h`

### Architecture Diagram
```
+----------------------------------------------------------+
| User Files (main.c, bootloader_user_config.h)            |
+----------------------------------------------------------+
        |
        v
+----------------------------------------------------------+
| Protocol Layer (src/openlcb/)                 |
|   bootloader.c — boot decision, main loop                |
|   bootloader_protocol.c — memconfig, PIP, SNIP, Verify  |
|   Calls BootloaderTransport_* externs                    |
+----------------------------------------------------------+
        |  BootloaderTransport_* (extern functions)
        v
+----------------------------------------------------------+
| Transport Layer (src/drivers/canbus/)         |
|   bootloader_can.c — alias SM, CAN framing, datagrams   |
|   Implements BootloaderTransport_* functions              |
|   Calls BootloaderCanDriver_* externs                    |
+----------------------------------------------------------+
        |  BootloaderCanDriver_* (extern functions)
        v
+----------------------------------------------------------+
| Application Drivers (application_drivers/)               |
|   BootloaderDriver_*: flash, LED, identity, lifecycle    |
|   BootloaderCanDriver_*: CAN read/send frame             |
+----------------------------------------------------------+

    CRC (src/crc/) used by protocol layer for checksum validation
```

**Completely separate codebase** — no files from the main library are included.

---

## Interfaces (Three Layers of Extern Functions)

All DI is via `extern` functions. Swap the .c file to change the implementation.

### 1. Transport Interface (bootloader_transport.h) — protocol calls transport

The protocol layer calls these. The CAN transport implements them. A future TCP or serial transport would provide its own .c file implementing the same externs.

| Function | Purpose |
|----------|---------|
| `BootloaderTransport_init()` | Initialize transport (CAN: start alias negotiation) |
| `BootloaderTransport_loop()` | Transport tick (CAN: poll RX, try send TX, run alias SM) |
| `BootloaderTransport_is_initialized()` | → bool: transport ready |
| `BootloaderTransport_send_addressed(mti, dest, data, len)` | Send addressed message |
| `BootloaderTransport_send_global(mti, data, len)` | Send global message |
| `BootloaderTransport_send_datagram_ok(dest)` | Send Datagram Received OK |
| `BootloaderTransport_send_datagram_rejected(dest, error)` | Send Datagram Rejected |
| `BootloaderTransport_send_datagram(dest, data, len)` | Send response datagram |

The transport calls back into the protocol layer via direct includes (not externs):
- `BootloaderProtocol_on_datagram_received(src, data, len)`
- `BootloaderProtocol_on_addressed_message(mti, src, data, len)`
- `BootloaderProtocol_on_global_message(mti, data, len)`

### 2. Hardware Driver Interface (bootloader_driver.h) — shared across all transports

| Category | Function | Purpose |
|----------|----------|---------|
| **Lifecycle** | `BootloaderDriver_hw_set_to_safe()` | Disable interrupts, safe GPIO |
| | `BootloaderDriver_hw_init()` | Clock, peripherals, LEDs |
| | `BootloaderDriver_request_bootloader()` | Check GPIO/magic word/app flag → bool |
| | `BootloaderDriver_application_entry()` | Jump to app (never returns) |
| | `BootloaderDriver_reboot()` | MCU reset (never returns) |
| **Flash** | `BootloaderDriver_get_flash_boundaries()` | → flash_min, flash_max, app_header ptr |
| | `BootloaderDriver_get_flash_page_info()` | → page_start, page_length for address |
| | `BootloaderDriver_erase_flash_page()` | Erase one page (incl. reset vector protection) |
| | `BootloaderDriver_write_flash()` | Program flash (incl. reset vector protection) |
| | `BootloaderDriver_flash_complete()` | Post-write validation → 0 or error code |
| **Checksum** | `BootloaderDriver_checksum_data()` | Compute 128-bit checksum (default: sw CRC) |
| **Identity** | `BootloaderDriver_node_id()` | → 48-bit node ID |
| **LED** | `BootloaderDriver_led()` | Set status LED (ACTIVE, WRITING, CSUM_ERROR, etc.) |

### 3. CAN Driver Interface (bootloader_can_driver.h) — CAN transport only

| Function | Purpose |
|----------|---------|
| `BootloaderCanDriver_read_can_frame(frame)` | Poll CAN RX → bool |
| `BootloaderCanDriver_try_send_can_frame(frame)` | Attempt CAN TX → bool |
| `BootloaderCanDriver_node_alias()` | → suggested alias (0 = auto-generate) |

### SNIP strings
**Hardcoded** as `#define` constants in `bootloader_types.h`. Each platform overrides via compiler flags (`-DBOOTLOADER_SNIP_MANUFACTURER="MyCompany"`).

### Key differences from OpenMRN
- **Two-layer split**: transport + hardware (OpenMRN mixes them in one file)
- Pure C signatures
- No `raw_erase`/`raw_write` — reset vector protection is the driver's responsibility
- Transport is swappable (CAN today, TCP/serial tomorrow)

---

## CRC / Checksum Design

### app_header struct (in flash, embedded in application image)
```c
#define BOOTLOADER_CHECKSUM_COUNT 4

struct bootloader_app_header {
    uint32_t app_size;
    uint32_t checksum_pre[BOOTLOADER_CHECKSUM_COUNT];   // CRC of flash_min..app_header
    uint32_t checksum_post[BOOTLOADER_CHECKSUM_COUNT];  // CRC of app_header+sizeof..app_end
};
```

### Software CRC (bootloader_crc.c)
Port OpenMRN's table-driven CRC-16-IBM from `Crc.cxx`:
- Two 16-entry lookup tables (CRC16_IBM_HI, CRC16_IBM_LO) — 64 bytes ROM total
- `BootloaderCrc_crc16_ibm(data, len)` → single uint16_t
- `BootloaderCrc_crc3_crc16_ibm(data, len, checksum[3])` → triple CRC (overall, odd bytes, even bytes)
- Polynomial 0xA001, init 0x0000, no final XOR
- Test vector: "123456789" → 0xBB3D

### Checksum validation (bootloader_app_header.h)
`Bootloader_check_application_checksum()`:
1. Get flash boundaries from driver
2. Compute pre-checksum: `BootloaderDriver_checksum_data(flash_min, pre_size, checksum)`
3. Compare against `app_header->checksum_pre[]`
4. Compute post-checksum for remaining bytes
5. Compare against `app_header->checksum_post[]`

Platforms with HW CRC override `BootloaderDriver_checksum_data()`. Others call `BootloaderCrc_crc3_crc16_ibm()`.

---

## State Machines

### CAN Transport Init (inside bootloader_can.c — internal to transport)
```
PICK_ALIAS → SEND_CID_7 → SEND_CID_6 → SEND_CID_5 → SEND_CID_4
→ WAIT_RID → SEND_RID → SEND_AMD → SEND_INIT_COMPLETE → INITIALIZED
```
Alias generation: `BootloaderCanDriver_node_alias()`, fallback XOR node ID segments. On conflict: pick new alias. This state machine is invisible to the protocol layer — it just sees `BootloaderTransport_is_initialized()` return true when ready.

### Main Loop (bootloader.c — protocol layer)
```
Bootloader_loop():
    1. BootloaderTransport_loop()           — transport does its work
    2. Check reset request → BootloaderDriver_reboot()
    3. Update busy LED
```
The transport calls `BootloaderProtocol_on_*()` callbacks during its loop when complete messages arrive. The protocol handlers call `BootloaderTransport_send_*()` to reply.

### Boot Decision
```
Bootloader_entry():
    BootloaderDriver_hw_set_to_safe()
    BootloaderDriver_hw_init()
    if (!BootloaderDriver_request_bootloader()):
        if (Bootloader_check_application_checksum()):
            BootloaderDriver_application_entry()    // jump to app
    memset(&state, 0, sizeof(state))
    while (true): Bootloader_loop()
```

---

## Datagram Firmware Write Handling

### Frame dispatch (from _handle_input_frame)
- Frame type 0x1A/0x1B (first/only) targeting our alias → `_handle_memory_config_frame()`
- Frame type 0x1C/0x1D (middle/final) → append to write buffer, flush on final

### Memory Config commands handled

| Command | Action |
|---------|--------|
| FREEZE (0xEF) | Reset state machine (same as OpenMRN) |
| WRITE (0xEF) | Extract 32-bit address, validate space=0xEF, buffer data, flush to flash |
| UNFREEZE (0xEF) | Call `BootloaderDriver_flash_complete()`, request reset |
| RESET | Send Datagram OK, request reboot |
| Get Options | Reply datagram with capabilities |
| Get Address Space Info | Reply datagram with space 0xEF metadata |

### Write buffer
64 bytes (one datagram's worth). Filled from first-frame data byte + middle/final frame data. Flushed on final frame or buffer full:
1. `BootloaderDriver_get_flash_page_info()` — check if at page boundary
2. If page boundary: `BootloaderDriver_erase_flash_page()`
3. `BootloaderDriver_write_flash()`
4. Advance offset, clear buffer to 0xFF

### Error handling
- Wrong space → reject SPACE_NOT_KNOWN
- Out of bounds → reject OUT_OF_BOUNDS
- Out of order → reject OUT_OF_ORDER
- Buffer busy → reject BUFFER_UNAVAILABLE

---

## Protocol Messages (bootloader_protocol.c)

### Messages handled

| RX Message | Response |
|------------|----------|
| AME (with matching or empty node ID) | AMD frame |
| CID for our alias | RID frame |
| Alias conflict (non-CID) | Reset to PICK_ALIAS |
| Verify Node ID (addressed) | Verified Node ID |
| Verify Node ID (global) | Verified Node ID |
| Protocol Support Inquiry | PIP Reply: DATAGRAM \| MEMORY_CONFIGURATION \| FIRMWARE_UPGRADE_ACTIVE |
| SNIP Request | Multi-frame SNIP response |
| Datagram (Memory Config) | See datagram section above |
| Other addressed message | Optional Interaction Rejected |

### SNIP response
Built entirely from hardcoded `#define` constants (manufacturer, model, hw/sw version, user name, user description). Sent as multi-frame addressed message using the datagram output buffer (expanded to 72 bytes to hold SNIP payload).

### Datagram output pipeline
For Get Options / Address Space Info replies: build payload in `datagram_payload[72]`, send as multi-frame datagram, wait for Datagram OK from receiver.

---

## Implementation Sequence

| Phase | What | Location (under `src/`) | Depends On |
|-------|------|-------------------------------------|-----------|
| 1 | CRC library | `crc/bootloader_crc.c/.h` + `crc/bootloader_app_header.h` | Nothing |
| 2 | Core types + interfaces | `openlcb/bootloader_types.h` + `openlcb/bootloader_transport.h` + `openlcb/bootloader_driver.h` | Phase 1 |
| 3 | CAN transport | `drivers/canbus/bootloader_can_driver.h` + `drivers/canbus/bootloader_can.c/.h` | Phase 2 |
| 4 | Protocol handlers | `openlcb/bootloader_protocol.c/.h` | Phase 2 |
| 5 | Main + user config | `openlcb/bootloader.c/.h` + top-level `bootloader_user_config.h` + `main.c` | Phase 3, 4 |
| 6 | Test suite | `test/bootloader_Test.cxx` (CMake only, for test builds) | Phase 5 |
| 7 | dsPIC33 platform | `application_drivers/dspic33_bootloader_drivers.c` (example) | Phase 6 |
| 8 | CRC patcher tool | Standalone build tool for app_header | Phase 1 |

---

## Testing (bootloader_Test.cxx)

Standalone test — links against `bootloader_standalone` library, NOT openlcb. Mock driver and transport functions with static arrays for CAN frames and flash memory.

### Test categories (~30 tests)

1. **CRC** (4): known vectors, triple CRC, empty data, alignment
2. **Alias init** (5): default, auto-generate, conflict, CID reply, full sequence
3. **Protocol replies** (5): PIP, SNIP, Verify addressed/global, AME
4. **Datagram handling** (6): single-frame write, multi-frame, wrong space, OOB, non-memconfig, out-of-order
5. **Memory Config commands** (5): FREEZE, UNFREEZE success/fail, RESET, Get Options, Get Address Space Info
6. **Boot decision** (3): requested, valid checksum, invalid checksum
7. **Flash write** (3): page boundary erase, mid-page, multi-datagram sequence
8. **Application checksum** (3): valid, invalid pre, invalid post

### Build integration (test/CMakeLists.txt)
```cmake
# In bootloader/test/CMakeLists.txt — self-contained, separate from main library tests
add_library(bootloader_standalone STATIC
    ${ROOT_DIR}/bootloader/src/openlcb/bootloader.c
    ${ROOT_DIR}/bootloader/src/openlcb/bootloader_protocol.c
    ${ROOT_DIR}/bootloader/src/drivers/canbus/bootloader_can.c
    ${ROOT_DIR}/bootloader/src/crc/bootloader_crc.c
)
# NO dependency on openlcb library
```

---

## TurnoutBoss Migration Summary

1. **Remove** all `src/openlcb/` files from bootloader MPLAB X project
2. **Add** the 4 bootloader .c files from `openlcb/`, `crc/`, `drivers/canbus/`
3. **Create** `application_drivers/dspic33_bootloader_drivers.c` implementing all `BootloaderDriver_*` + `BootloaderCanDriver_*` functions (adapt from existing `dspic33_can_drivers` and `dspic33_drivers`)
4. **Remove** dsHex parser — new bootloader receives raw binary
5. **Upgrade** checksum from 8-bit byte sum to triple CRC-16-IBM
6. **Add** `bootloader_app_header` to application linker script at known flash address
7. **Create** post-link CRC patcher tool to compute and embed checksums in .bin
8. **Alias handoff** preserved: `BootloaderDriver_node_alias()` reads `CommonLoaderApp_node_alias` from persistent RAM
9. **Expected flash savings**: ~20-40KB program memory

---

## Style Guides

All code must follow the project style guides:
- `documentation/style_guides/StyleGuide_v4.md` — C coding conventions (naming, indentation, braces, etc.)
- `documentation/style_guides/Doxygen_StyleGuide_v2_0.md` — documentation/comment conventions

Key rules that apply to the bootloader:
- Filenames: all lowercase, underscores between words
- Public functions: `PascalCaseModule_lowercase_description()`
- Private functions/vars: `_prefixed`, declared `static`
- Types: `_t` suffix for structs, `_enum` for enums
- 4 spaces indentation, no tabs, max 120 char lines
- Header guards: `__PATH_MODULE_NAME__`
- `extern "C" { }` wrappers in all headers

---

## Key Reference Files

- `documentation/style_guides/StyleGuide_v4.md` — coding style (MUST follow)
- `documentation/style_guides/Doxygen_StyleGuide_v2_0.md` — documentation style (MUST follow)
- `/Users/jimkueneman/Documents/openmrn/src/openlcb/Bootloader.hxx` — primary reference (state machine, datagram handling)
- `/Users/jimkueneman/Documents/openmrn/src/openlcb/bootloader_hal.h` — driver contract reference (OpenMRN's version)
- `/Users/jimkueneman/Documents/openmrn/src/utils/Crc.cxx` — CRC-16-IBM algorithm to port
- `/Users/jimkueneman/Documents/openmrn/src/openlcb/ApplicationChecksum.hxx` — checksum validation logic
- `documentation/working_plans/turnoutboss_bootloader_reference.md` — TurnoutBoss patterns
- `documentation/working_plans/openmrn_bootloader_reference.md` — OpenMRN analysis

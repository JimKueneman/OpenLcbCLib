# TurnoutBoss Bootloader Reference - Patterns & Architecture

Reference document for creating a standalone C bootloader that does NOT use the OpenLcbCLib library.
Source: `/Users/jimkueneman/Documents/LCC Projects/TurnoutBoss2_Firmware/TurnoutBossBootloader.X/`

---

## 1. High-Level Architecture

The TurnoutBoss bootloader uses the **full OpenLcbCLib library** in a stripped-down configuration
via the `OPENLCB_COMPILE_BOOTLOADER` feature flag. The goal of this document is to extract the
patterns used so a **library-free** standalone bootloader can be written.

### Current Architecture (uses full library)

```
+-------------------------------------------------------+
| Application Layer                                      |
|   turnoutboss_bootloader_main.c - boot logic, main()   |
|   callbacks_config_mem.c - freeze/unfreeze/fw_write     |
|   turnoutboss_bootloader_hex_file_statemachine.c        |
+-------------------------------------------------------+
| OpenLcbCLib (via OPENLCB_COMPILE_BOOTLOADER flag)       |
|   CAN alias login, datagram handler, memory config ops  |
|   SNIP, Protocol Support Inquiry, Message Network       |
+-------------------------------------------------------+
| Platform Drivers                                        |
|   dspic33_can_drivers, dspic33_drivers, timer, uart     |
+-------------------------------------------------------+
| Hardware (dsPIC33EP512MC506)                            |
+-------------------------------------------------------+
```

### What the Library Provides (that a standalone bootloader must replicate)

1. CAN alias allocation (login state machine)
2. CAN frame assembly/disassembly (multi-frame messages)
3. Datagram protocol (receive/ACK/reject)
4. Memory Configuration Protocol dispatch (freeze/write/unfreeze/reset)
5. Protocol Support Inquiry replies
6. SNIP replies
7. Verify Node ID replies

---

## 2. Memory Map (dsPIC33EP512MC506)

```
Address Range          Purpose
------------------------------------------------------------
0x000000 - 0x0001FE   Hardware reset vector (GOTO __reset)
0x000200 - 0x00AFFE   Bootloader code (linker-reserved)
0x00B000 - 0x0547FF   Application code (writable by bootloader)
0x054000               Global Node ID storage (2 instructions = 6 bytes)
0x054800               Checksum storage page (start_addr, end_addr, checksum)
0x055000               Configuration bits page (DO NOT ERASE)
0x055800               End of flash
------------------------------------------------------------
0x001000               Shared persistent data (in RAM/data memory)
```

### Persistent Data at DATA_START_ADDRESS (0x1000)

Shared between bootloader and application via `__attribute__((persistent address(...)))`:

| Offset | Size    | Variable                              | Purpose |
|--------|---------|---------------------------------------|---------|
| +0     | 2 bytes | `CommonLoaderApp_bootloader_state`    | Bitfield: interrupt_redirect, started_from_app, started_from_bootloader, do_start, update_succeeded |
| +2     | 2 bytes | `CommonLoaderApp_node_alias`          | Preserved CAN alias for seamless handoff |
| +4     | 8 bytes | `CommonLoaderApp_node_id`             | Cached node ID |
| +12    | 36 bytes| `CommonLoaderApp_jumptable`           | VIVT interrupt redirect table |

### Reset Vector Strategy

- **Bootloader reset vector** at 0x0200: `goto __reset` (in `TurnoutBossBootloader.X/reset_vector.s`)
- **Application reset vector** at 0xB000: `goto __reset` (in `TurnoutBoss.X/reset_vector.s`)
- Hardware always boots to 0x0000 which jumps to bootloader at 0x0200
- Bootloader decides whether to stay or jump to application at 0xB000

---

## 3. Boot Entry Decision Flow

```
main()
  |-- Disable interrupts (_GIE = 0)
  |-- _initialize_state()          [clear state on POR/BOR only]
  |-- _initialize()                [HW setup, CAN init, OpenLCB init]
  |-- Extract node ID from flash, fallback to EEPROM, fallback to default
  |
  |-- Was started_from_app set?
  |     YES:
  |       |-- Run bootloader (alias already known from app)
  |       |-- After update: validate checksum
  |       |-- If invalid: reboot (loop back to bootloader)
  |       |-- If valid: fall through to start app
  |     NO (cold boot):
  |       |-- Is application checksum valid?
  |       |     YES: skip bootloader, jump to app
  |       |     NO:  run bootloader (must get firmware loaded)
  |
  |-- Disable interrupts
  |-- Set started_from_bootloader = true
  |-- Jump to APPLICATION_START_ADDRESS (0xB000)
```

### Key: Two Entry Paths

1. **Cold boot (POR/BOR)** - bootloader checks if valid app exists in flash
   - Valid app: jump directly to app (bootloader is transparent)
   - No valid app: enter bootloader mode, wait for firmware
2. **App-initiated** - application sets `started_from_app=true` and reboots
   - Bootloader inherits the app's CAN alias for seamless transition
   - Node appears to stay online during firmware upgrade

---

## 4. Node Configuration for Bootloader Mode

### Feature Flag

```c
#define OPENLCB_COMPILE_BOOTLOADER
```

This single flag in `openlcb_user_config.h` triggers the library's cascade:
- Auto-defines `OPENLCB_COMPILE_DATAGRAMS`
- Auto-defines `OPENLCB_COMPILE_MEMORY_CONFIGURATION`
- Auto-defines `OPENLCB_COMPILE_FIRMWARE`

### What Gets Compiled OUT

The `#ifndef OPENLCB_COMPILE_BOOTLOADER` guards in the library exclude:

| Module | Excluded Functions |
|--------|-------------------|
| `protocol_config_mem_read_handler.c` | **Entire file** (no reads needed) |
| `protocol_config_mem_write_handler.c` | All non-firmware write spaces (CDI, config mem, ACDI user, train function config) |
| `protocol_config_mem_operations_handler.c` | Lock/reserve, get unique ID, update complete, factory reset |
| `openlcb_config.c` | All read space wiring, non-firmware write wiring, lock/reserve/factory_reset wiring |

### What REMAINS Active

| Function | Purpose |
|----------|---------|
| CAN alias login state machine | Get on the bus |
| Datagram handler | Receive memory config commands |
| Memory Config Operations: options, address_space_info, freeze, unfreeze, reset | Firmware upgrade sequence |
| Memory Config Write: firmware space (0xEF) only | Receive firmware data |
| SNIP | Identify the bootloader node |
| Protocol Support Inquiry | Advertise capabilities |
| Message Network (Verify Node ID, etc.) | Basic node participation |

### Protocol Support Bits Advertised

```c
PSI_DATAGRAM | PSI_MEMORY_CONFIGURATION | PSI_SIMPLE_NODE_INFORMATION | PSI_FIRMWARE_UPGRADE
```

### Address Space Configuration

Only firmware space (0xEF) is marked `present = 1`. All other spaces (0xFF, 0xFE, 0xFD, 0xFC, 0xFB) have `present = 0`.

---

## 5. Firmware Upgrade Protocol Sequence

### OpenLCB Message Flow

```
Updater Tool                          Bootloader
  |                                      |
  |  [Application reboot with            |
  |   started_from_app = true]           |
  |                                      |
  |-- Memory Config FREEZE (0xEF) ------>|  callbacks_config_mem.c: CallbacksConfigMem_freeze()
  |     - Sets firmware_upgrade_active    |    - Reset hex file state machine
  |     - Sends Initialization event     |    - Send Producer/Consumer event
  |<------------- Datagram OK -----------|
  |                                      |
  |-- Memory Config WRITE (0xEF) ------->|  callbacks_config_mem.c: CallbacksConfigMem_firmware_write()
  |     addr=0, data=<dsHex bytes>       |    - Feed each byte to hex statemachine
  |<------------- Datagram OK -----------|    - Returns error if write fails
  |                                      |
  |-- Memory Config WRITE (0xEF) ------->|  [repeat for all firmware data]
  |<------------- Datagram OK -----------|
  |   ...                                |
  |                                      |
  |-- Memory Config UNFREEZE (0xEF) ---->|  callbacks_config_mem.c: CallbacksConfigMem_unfreeze()
  |     - Clears firmware_upgrade_active  |    - Check update_succeeded flag
  |     - Issues asm("RESET")            |    - CPU reset, bootloader restarts
  |                                      |
```

### Transport: Datagram-Only

The TurnoutBoss bootloader uses **datagram-based writes only** (no stream support).
Each write datagram carries a chunk of the dsHex file content.

---

## 6. Firmware Data Format: dsHex

The firmware image is NOT raw binary. It is a custom text format called **dsHex** (Microchip-specific
hex variant processed by the `Hex2dsHex` tool).

### dsHex File Structure

```
;00000024MustangpeakEngineeringTurnoutBoss2.0     <-- GUID header
:00006000000004000000C2F0000040F6....              <-- data blocks
:00006800000004000000C2F0000040F6....              <-- more data blocks
:FFFFFFFF                                          <-- EOF marker
```

### Hex File State Machine States

```
HEX_STATE_FIND_COLON (0)       - Scan for ':' (data block) or ';' (GUID header)
HEX_STATE_READ_ADDRESS (1)     - Read 8 hex chars -> 32-bit start address of erase block
HEX_STATE_READ_INSTRUCTION_COUNT (2) - Read 8 hex chars -> instruction count in block
HEX_STATE_READ_DATA (3)        - Read pairs of 8-hex-char dwords, write to flash
HEX_STATE_READ_GUID_LEN (4)    - Read 8 hex chars -> length of GUID string
HEX_STATE_READ_GUID (5)        - Verify GUID matches expected key
```

### Processing Flow Per Data Block

1. Read `:` -> transition to read address
2. Read 8 hex chars as 32-bit address
   - If address == 0xFFFFFFFF: EOF, validate checksum, return success/fail
   - If valid address in application range: erase the flash page at that address
3. Read 8 hex chars as instruction count
4. Read pairs of dwords (2 x 8 hex chars = 2 x 24-bit instructions)
   - Write each pair using `FLASH_WriteDoubleWord24(address, dword1, dword2)`
   - Read-back verify each write
   - Advance address by 4 (2 instructions per double-word)
5. When instruction_count reached, go back to FIND_COLON

### GUID Validation

Before any data blocks, the file starts with `;` followed by:
- 8 hex chars = GUID length
- ASCII chars = GUID string (e.g., "MustangpeakEngineeringTurnoutBoss2.0")

If the GUID doesn't match, the file is rejected. This prevents flashing wrong firmware.

### Address Range Protection

```c
if ((address >= APPLICATION_START_ADDRESS) && (address < APPLICATION_END_ADDRESS))
    // OK to erase and write
```

- `APPLICATION_START_ADDRESS` = 0x00B000
- `APPLICATION_END_ADDRESS` = 0x055800
- Bootloader area (0x0200-0xAFFE) and config bits page (0x055000+) are protected

---

## 7. Checksum Validation

Checksum data is stored at `APPLICATION_CHECKSUM_ADDRESS` (0x054800):

| Offset | Content | Size |
|--------|---------|------|
| +0     | Start address | 24-bit instruction word |
| +2     | End address | 24-bit instruction word |
| +4     | Expected checksum | 24-bit instruction word (only low 8 bits used) |

### Algorithm

```
running_checksum = 0
for each instruction address from start to end (step 2):
    word = FLASH_ReadWord24(address)
    running_checksum += (word & 0xFF) + ((word >> 8) & 0xFF) + ((word >> 16) & 0xFF)

valid = (expected_checksum & 0xFF) == (running_checksum & 0xFF)
```

This is a simple 8-bit byte-sum checksum across the entire application flash region.

---

## 8. Node ID Resolution

Priority order:
1. **Flash** at `GLOBAL_NODE_ID_ADDRESS` (0x054000) - read two 24-bit instruction words, combine into 48-bit node ID
2. **EEPROM** at last page (offset 1008) - read 6 bytes via SPI EEPROM driver
3. **Default** `0x0501010107FF` - hardcoded fallback

The flash-based node ID is embedded in the application firmware image by the build tools.
The EEPROM-based node ID is the persistent user-configured ID.

---

## 9. Interrupt Redirection (VIVT)

Since the bootloader owns the hardware interrupt vectors (they're in flash at 0x0000-0x01FE),
the application cannot install its own ISRs directly.

### Solution: Software Virtual Interrupt Vector Table (VIVT)

1. Bootloader defines all ISR handlers (traps, CAN, UART, timers)
2. Each ISR checks `CommonLoaderApp_bootloader_state.interrupt_redirect`:
   - **false** (bootloader mode): handle locally or halt
   - **true** (app mode): call function pointer from `CommonLoaderApp_jumptable`
3. Application populates `CommonLoaderApp_jumptable` with its ISR addresses on startup
4. Application sets `interrupt_redirect = true`

### VIVT Jump Table Entries

```c
typedef struct {
    parameterless_callback_t oscillatorfail_hander;
    parameterless_callback_t addresserror_hander;
    parameterless_callback_t stackerror_hander;
    parameterless_callback_t matherror_hander;
    parameterless_callback_t dmacerror_hander;
    parameterless_callback_t timer_2_hander;
    parameterless_callback_t u1_tx_hander;
    parameterless_callback_t u1_rx_hander;
    parameterless_callback_t c1_hander;
    parameterless_callback_t timer_1_hander;
} vivt_jumptable_t;
```

---

## 10. Interface Configuration Structs

The bootloader wires two configuration structs for the library:

### CAN Config (`can_config_t`)

```c
.transmit_raw_can_frame  = &Dspic33CanDrivers_transmit_raw_can_frame,
.is_tx_buffer_clear      = &Dspic33CanDrivers_is_can_tx_buffer_clear,
.lock_shared_resources   = &Dspic33Drivers_lock_shared_resources,
.unlock_shared_resources = &Dspic33Drivers_unlock_shared_resources,
.on_rx                   = &CallbacksCan_on_rx,        // toggle yellow LED
.on_tx                   = &CallbacksCan_on_tx,        // toggle blue LED
.on_alias_change         = &CallbacksCan_on_alias_change,
```

### OpenLCB Config (`openlcb_config_t`)

```c
.lock_shared_resources   = &Dspic33Drivers_lock_shared_resources,
.unlock_shared_resources = &Dspic33Drivers_unlock_shared_resources,
.config_mem_read         = &Dspic33Drivers_config_mem_read,
.config_mem_write        = &Dspic33Drivers_config_mem_write,
.reboot                  = &Dspic33Drivers_reboot,
.factory_reset           = &Dspic33Drivers_factory_reset,
.freeze                  = &CallbacksConfigMem_freeze,
.unfreeze                = &CallbacksConfigMem_unfreeze,
.firmware_write          = &CallbacksConfigMem_firmware_write,
.on_100ms_timer          = &CallbacksOlcb_on_100ms_timer,  // LED blinker
```

---

## 11. App-to-Bootloader Handoff (Alias Preservation)

When the application wants to enter bootloader mode:

1. App saves its CAN alias to `CommonLoaderApp_node_alias` (persistent RAM)
2. App sets `CommonLoaderApp_bootloader_state.started_from_app = true`
3. App triggers CPU reset

On reboot, the bootloader:

1. Detects `started_from_app == true`
2. Creates node with the **preserved alias** (skips alias negotiation)
3. Sets `node->state.permitted = true` (already logged in)
4. Sets `node->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE`
5. Sets `node->state.firmware_upgrade_active = true`
6. Registers the alias in the mapping table

Result: the node appears to stay online seamlessly during the firmware upgrade.

---

## 12. Comparison: TurnoutBoss vs OpenMRN Bootloader

| Aspect | TurnoutBoss (current) | OpenMRN |
|--------|----------------------|---------|
| Language | C | C++ |
| Library usage | Full OpenLcbCLib (stripped via flag) | Standalone `Bootloader.hxx` |
| Code size | Large (entire lib compiled in) | ~1150 lines single file |
| Transport | Datagram only | Datagram or Stream |
| Data format | dsHex (text, per-page blocks) | Raw binary |
| Checksum | 8-bit byte sum | 4x uint32_t pre/post checksums |
| GUID validation | String match in dsHex file | N/A (app_header checksum) |
| Alias handoff | Yes (persistent RAM) | Yes (magic word + RAM) |
| Interrupt model | VIVT redirect table | No interrupts (polled) |
| Flash write | MCC FLASH_WriteDoubleWord24 | HAL erase_page/write_flash |
| Entry decision | POR/BOR flags + started_from_app | Magic word + GPIO + checksum |

---

## 13. Minimal Messages for Standalone Bootloader

A standalone bootloader (no library) needs to handle ONLY these CAN-level interactions:

### Must Implement

| Message | Direction | Purpose |
|---------|-----------|---------|
| CID (4 frames) | TX | Alias negotiation |
| RID | TX | Claim alias |
| AMD | TX | Announce mapping |
| Initialization Complete | TX | Announce node is online |
| Verify Node ID (addressed) | RX -> TX | Reply with Verified Node ID |
| Verify Node ID (global) | RX -> TX | Reply with Verified Node ID |
| AME (Alias Map Enquiry) | RX -> TX | Reply with AMD |
| Protocol Support Inquiry | RX -> TX | Reply with protocol bits |
| SNIP Request | RX -> TX | Reply with SNIP data |
| Datagram (single/first/middle/final) | RX | Receive memory config commands |
| Datagram Received OK | TX | Acknowledge received datagrams |
| Datagram Rejected | TX | Reject invalid datagrams |
| Memory Config FREEZE cmd | RX | Start firmware session |
| Memory Config WRITE cmd (space 0xEF) | RX | Receive firmware data |
| Memory Config UNFREEZE cmd | RX | End firmware session |
| Memory Config RESET cmd | RX | Reboot |
| Memory Config Get Options | RX -> TX | Reply with capabilities |
| Memory Config Get Address Space Info | RX -> TX | Reply with space 0xEF info |

### Can Omit

| Message | Why |
|---------|-----|
| Events (produce/consume/identify) | Not needed for firmware upgrade |
| Memory Config READ (any space) | Bootloader doesn't serve reads |
| Memory Config WRITE (spaces other than 0xEF) | Only firmware space |
| Lock/Reserve | Not needed |
| Get Unique ID | Not needed |
| Factory Reset | Not needed |
| Stream protocol | Datagram-only is sufficient for embedded |
| CDI | No configuration in bootloader |

---

## 14. Key Implementation Details for Standalone Version

### CAN Frame Format Reminders

- 29-bit extended identifier
- Bits 28-27: priority (0b01 for OpenLCB)
- Bits 26-24: frame type
- Bits 23-12: source alias (for CAN control frames) or varies
- Bits 11-0: varies by frame type

### Datagram Reassembly

The bootloader must handle multi-frame datagrams:
- Frame type 0x1A = datagram first frame (up to 8 data bytes)
- Frame type 0x1B = datagram only frame (complete in one frame)
- Frame type 0x1C = datagram middle frame
- Frame type 0x1D = datagram final frame

Accumulate data bytes across frames. On final/only frame, dispatch the complete datagram.

### Memory Config Datagram Format

```
Byte 0:    0x20 (Memory Configuration command)
Byte 1:    Command/subcommand
Bytes 2+:  Command-specific data
```

Key commands for bootloader:
- 0x00 = Write (with address space in byte 6 or encoded in cmd)
- 0x08 = Get Options
- 0x07 = Get Address Space Info
- 0x01 = Freeze
- 0x02 = Unfreeze
- 0x09 = Reset/Reboot

### Flash Write Considerations (dsPIC33)

- Flash is organized in pages (erase blocks)
- Writes are double-word (2 instructions = 48 bits per write)
- Must erase entire page before writing
- Read-back verification after each write
- Protect bootloader region and config bits page

---

## 15. Summary: What to Build

The standalone bootloader needs these components:

1. **CAN driver** - raw frame TX/RX (reuse existing `dspic33_can_drivers`)
2. **CAN alias state machine** - CID/RID/AMD/INIT sequence (~100 lines)
3. **Frame dispatcher** - route incoming frames by type (~150 lines)
4. **Datagram assembler** - multi-frame accumulation + ACK/reject (~200 lines)
5. **Memory config command handler** - freeze/write/unfreeze/reset/options/space_info (~200 lines)
6. **PIP/SNIP responder** - canned replies (~50 lines)
7. **dsHex parser** - existing `turnoutboss_bootloader_hex_file_statemachine.c` (reuse as-is)
8. **Flash driver** - existing MCC `flash.h/flash.s` (reuse as-is)
9. **Boot decision logic** - checksum validation + entry decision (~50 lines)
10. **Persistent state** - shared RAM variables for app/bootloader handoff

Estimated new code: ~750 lines of C (excluding reused drivers and dsHex parser).
This replaces pulling in the entire OpenLcbCLib (~15,000+ lines) for the bootloader build.

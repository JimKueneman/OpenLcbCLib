# OpenMRN Bootloader Reference - Patterns & Architecture

Reference document for creating an OpenLcbCLib-compatible bootloader in C.
Source: `/Users/jimkueneman/Documents/openmrn/src/openlcb/Bootloader.hxx` and related files.

---

## 1. High-Level Architecture

The OpenMRN bootloader is a **standalone minimal OpenLCB stack** whose sole purpose is firmware upgrades. It runs on bare metal with no RTOS, no dynamic allocation, and no interrupts. The entire protocol engine is polled.

### Layers

```
+-------------------------------------------------------+
| Layer 4: Bootloader Client (remote updater tool)       |
|   BootloaderClient.hxx - sends firmware to target      |
+-------------------------------------------------------+
| Layer 3: Protocol Handlers                             |
|   Memory Config (FREEZE/WRITE/UNFREEZE/RESET)          |
|   Stream Protocol (initiate/data/proceed/complete)      |
|   Datagram Protocol (multi-frame reassembly)            |
+-------------------------------------------------------+
| Layer 2: Protocol State Machine (Bootloader.hxx)       |
|   CAN alias init, main loop, frame dispatch             |
+-------------------------------------------------------+
| Layer 1: HAL Interface (bootloader_hal.h)              |
|   Flash ops, CAN send/recv, HW init, LED, checksum     |
+-------------------------------------------------------+
| Hardware (board-specific BootloaderHal.hxx)             |
+-------------------------------------------------------+
```

### Key Design Decisions

- **Single-file implementation** - `Bootloader.hxx` is ~1150 lines containing the entire state machine
- **Compile-time feature selection** - `#define BOOTLOADER_STREAM` or `#define BOOTLOADER_DATAGRAM`
- **Polled, non-blocking** - no interrupts in the bootloader loop; stability over performance
- **Global state struct** - single `BootloaderState` holds all protocol state
- **No cross-module function pointer indirection** - direct function calls since there is only one "module"

---

## 2. Boot Entry Flow

```
bootloader_entry()                     [never returns]
  |-- bootloader_hw_set_to_safe()      [disable interrupts, safe GPIO]
  |-- bootloader_hw_init()             [clock, CAN, LEDs]
  |-- bootloader_init()
  |     |-- request_bootloader()       [check GPIO pin or magic word]
  |     |-- if NOT requested:
  |     |     check_application_checksum()
  |     |-- if valid app && not requested:
  |     |     application_entry()      [jump to app, never returns]
  |     |-- else:
  |           memset(&state_, 0, sizeof(state_))
  |
  |-- while(true):
        bootloader_loop()              [returns true to reboot]
```

### Magic Word Entry
Applications can request bootloader mode by writing `0x76b89b1eU` to
`__bootloader_magic_ptr` (a linker-defined symbol in RAM) before rebooting.

### Checksum Validation
The `app_header` struct embedded in application flash contains:
- `app_size` (uint32_t) - total application binary length
- `checksum_pre[4]` (uint32_t) - checksum of bytes from flash_min to app_header
- `checksum_post[4]` (uint32_t) - checksum of bytes from app_header to app_end

Validation checksums both regions separately. If either fails, bootloader stays active.

---

## 3. CAN Alias Initialization State Machine

The bootloader must claim a CAN alias before participating on the bus.

```
PICK_ALIAS --> SEND_CID_7 --> SEND_CID_6 --> SEND_CID_5 --> SEND_CID_4
    --> WAIT_RID --> SEND_RID --> SEND_AMD --> SEND_NMRANET_INIT --> INITIALIZED
```

**Alias generation:**
1. Try `nmranet_alias()` (saved from app or default)
2. If zero, XOR 12-bit chunks of the node ID
3. On conflict, advance: `alias = (alias + (nodeID & 0xffe) + 1759) & 0xfff`

**Conflict handling:**
- If someone sends CID for our alias: reply with RID
- If any other frame uses our alias: reset to PICK_ALIAS, choose new alias

---

## 4. Main Loop Structure

```c
bool bootloader_loop() {
    // 1. Handle re-init request (re-announce on bus)
    // 2. Poll CAN hardware for incoming frame
    // 3. Update busy LED
    // 4. Try to send pending output frame
    // 5. Check for reset request (reboot if idle)
    // 6. Process input frame if present
    // 7. Run init state machine if not yet initialized
    // 8. Send stream proceed if pending (BOOTLOADER_STREAM)
    // 9. Send outgoing datagram frames if pending
    return false;  // true = reboot
}
```

**Single input / single output buffer model:**
- `state_.input_frame` + `input_frame_full` flag
- `state_.output_frame` + `output_frame_full` flag
- If output buffer full, handlers return without consuming input (re-try next loop)

---

## 5. Frame Dispatch (handle_input_frame)

```
CAN frame received
  |-- Error/RTR/non-EFF? -> drop
  |-- Non-OpenLCB priority? -> drop
  |-- Alias matches ours?
  |     |-- CID frame? -> reply RID (alias defense)
  |     |-- Other? -> reset to PICK_ALIAS
  |-- Control message?
  |     |-- AME (with our nodeID or empty)? -> reply AMD
  |     |-- Other control? -> drop
  |-- Datagram start/only frame (0x1A/0x1B to our alias)?
  |     |-- data[0] == CONFIGURATION? -> handle_memory_config_frame()
  |     |-- else -> reject_datagram()
  |-- Datagram middle/final frame (0x1C/0x1D)? [BOOTLOADER_DATAGRAM only]
  |     |-- Copy data to write buffer, flush on final
  |-- Stream data frame (0x1F)? [BOOTLOADER_STREAM only]
  |     |-- handle_stream_data()
  |-- Addressed/global message (0x19)?
  |     |-- Addressed to us? -> handle_addressed_message(mti)
  |     |-- Global? -> handle_global_message(mti)
  |-- Else -> drop
```

---

## 6. Memory Configuration Protocol Commands

The bootloader responds to a subset of the Memory Configuration Protocol:

### COMMAND_FREEZE (space 0xEF)
- Signals firmware download is about to begin
- Resets state machine (poor man's reset): `memset(&state_, 0, sizeof(state_))`
- Causes re-initialization on the bus

### COMMAND_WRITE (datagram mode only)
- Single-datagram write to firmware space
- Data format: `[0x20] [cmd] [addr32] [space] [data...]`
- First frame sets offset, normalizes to physical address
- Multi-frame datagrams: middle/final frames append to write buffer
- Flushes to flash on final frame, ACKs datagram

### COMMAND_WRITE_STREAM (stream mode only)
- Initiates a stream-based write session
- Replies with write response datagram (REPLY_PENDING flag)
- Sets up stream state (src_id, offset)
- Actual data arrives via STREAM_DATA frames

### COMMAND_UNFREEZE (space 0xEF)
- Signals firmware download complete
- Calls `flash_complete()` for validation
- If error, rejects datagram with error code
- If success, ACKs and requests reset

### COMMAND_RESET
- Sends datagram OK, sets `request_reset = 1`
- Bootloader reboots when idle

---

## 7. Stream Protocol Flow

```
Client                              Bootloader
  |                                     |
  |-- Write Stream datagram ----------->|  (sets up stream state)
  |<--------- Datagram OK + PENDING ----|
  |<--------- Write Stream Reply dg ----|
  |                                     |
  |-- Stream Initiate Request --------->|  (negotiate buffer size)
  |<--------- Stream Initiate Reply ----|  (accepted, buffer size)
  |                                     |
  |-- Stream Data (7 bytes/frame) ----->|  (accumulate in write buffer)
  |-- Stream Data ---------------------->|
  |-- ... (buffer fills) -------------->|
  |<--------- Stream Proceed -----------|  (buffer consumed, flush to flash)
  |-- Stream Data ---------------------->|
  |-- ... ------------------------------>|
  |                                     |
  |-- Stream Complete ----------------->|  (flush remaining, close stream)
  |                                     |
  |-- Unfreeze datagram --------------->|  (validate, prepare reboot)
  |<--------- Datagram OK -------------|
```

**Buffer negotiation:**
- Client proposes max buffer size
- Bootloader halves until <= WRITE_BUFFER_SIZE (power-of-2 rounding down)
- Stream proceed sent when `stream_buffer_remaining <= 0`

**Stream data handling:**
- First byte of each CAN frame is stream_id, remaining 1-7 bytes are payload
- Data copied into `g_write_buffer[write_buffer_index]`
- When buffer full (`write_buffer_index >= WRITE_BUFFER_SIZE`): `flush_flash_buffer()`

---

## 8. Datagram Protocol Flow

```
Client                              Bootloader
  |                                     |
  |-- Write datagram (first/only) ----->|  [addr + first data byte]
  |     If only frame:                  |
  |<--------- Datagram OK -------------|  [flush immediately]
  |     If first of multi:             |
  |-- Middle frame(s) ----------------->|  [append to buffer]
  |-- Final frame ---------------------->|  [flush + Datagram OK]
  |                                     |
  |-- Next Write datagram ------------->|  [repeat...]
```

- Max 64-byte write buffer (one datagram worth)
- Each datagram carries address + data; much slower than stream
- ACK on every datagram = significant overhead

---

## 9. Flash Write Pattern

```c
void flush_flash_buffer() {
    address = state_.write_buffer_offset;
    get_flash_page_info(address, &page_start, &page_length);
    if (page_start == address) {
        erase_flash_page(address);       // Only at page boundary
    }
    write_flash(address, g_write_buffer, state_.write_buffer_index);
    state_.write_buffer_offset += state_.write_buffer_index;
    state_.write_buffer_index = 0;
    memset(g_write_buffer, 0xff, WRITE_BUFFER_SIZE);  // Reset to erased state
}
```

**Address translation:**
- OpenLCB memory space offset 0 = `flash_min` (from `get_flash_boundaries`)
- `normalize_write_buffer_offset()` adds `flash_min` to logical offset
- Bounds check: offset must be < (`flash_max` - `flash_min`)

**Reset vector protection (TI Tiva example):**
- `erase_flash_page()`: if erasing page 0, immediately writes back stack pointer + reset_handler
- `write_flash()`: if writing to flash_start, skips first 8 bytes (reset vector)
- `checksum_data()`: also skips first 8 bytes when checksumming from flash_start
- This prevents bricking if power is lost during page 0 erase/write

---

## 10. HAL Interface Summary

Functions the platform must implement:

| Function | Purpose |
|----------|---------|
| `bootloader_hw_set_to_safe()` | Disable interrupts, safe GPIO (pre-BSS init) |
| `bootloader_hw_init()` | Clock, CAN peripheral, LEDs (post-BSS init) |
| `request_bootloader()` | Check GPIO/magic word for bootloader entry |
| `application_entry()` | Jump to application (never returns) |
| `bootloader_reboot()` | MCU reset (never returns) |
| `read_can_frame(frame*)` | Poll CAN RX, return true if frame available |
| `try_send_can_frame(frame&)` | Attempt CAN TX, return false if busy |
| `get_flash_boundaries(min, max, hdr)` | Return writable flash region and app header location |
| `get_flash_page_info(addr, start, len)` | Return page containing address |
| `erase_flash_page(addr)` | Erase one flash page (blocking) |
| `write_flash(addr, data, size)` | Program flash (blocking, 4-byte aligned) |
| `raw_erase_flash_page(addr)` | Erase without reset vector protection |
| `raw_write_flash(addr, data, size)` | Write without reset vector protection |
| `flash_complete()` | Post-write validation; return 0 or error code |
| `checksum_data(data, size, out)` | Compute checksum (4x uint32_t output) |
| `nmranet_nodeid()` | Return 48-bit OpenLCB node ID |
| `nmranet_alias()` | Return suggested CAN alias (0 = auto-generate) |
| `bootloader_led(led, value)` | Set status LED (ACTIVE, WRITING, CSUM_ERROR, etc.) |

---

## 11. Protocol Support Advertised

```
Stream mode:  DATAGRAM | STREAM | MEMORY_CONFIGURATION | FIRMWARE_UPGRADE_ACTIVE
Datagram mode: DATAGRAM | MEMORY_CONFIGURATION | FIRMWARE_UPGRADE_ACTIVE
```

Responds to:
- Protocol Support Inquiry (PIP)
- Verify Node ID (addressed and global)
- Datagram OK (for outgoing datagram ACK tracking)
- Optional Interaction Rejected (for unsupported MTIs)

---

## 12. Error Codes

| Code | Meaning |
|------|---------|
| `0x2088` | Write checksum failed |
| `0x1088` | Incompatible firmware |
| `0x1089` | Corrupted data |
| `0x1000` | Permanent error (generic) |

Firmware space: `0xEF`

---

## 13. Configuration Macros

```c
#define BOOTLOADER_STREAM       // Enable stream-based transfer
// OR
#define BOOTLOADER_DATAGRAM     // Enable datagram-based transfer

#define WRITE_BUFFER_SIZE 1024  // Stream: 1KB recommended
// (datagram defaults to 64)

#define FLASH_SPACE 0xEF        // Memory config space ID
#define STREAM_ID 0x5A          // Local stream identifier
#define CHECKSUM_COUNT 4        // Number of uint32_t words per checksum

#define BOOTLOADER_LOOP_HOOK()  // Optional hook called each loop iteration
```

---

## 14. Adaptation Notes for OpenLcbCLib

### What to Reuse Directly
1. **Boot entry flow** - checksum validation, magic word entry, GPIO check
2. **CAN alias initialization sequence** - same CID/RID/AMD/INIT pattern
3. **Memory config command set** - FREEZE/WRITE/WRITE_STREAM/UNFREEZE/RESET
4. **Flash write pattern** - buffered writes with page-boundary erase
5. **Stream flow control** - buffer negotiation, proceed messages
6. **App header / checksum structure** - pre/post checksum validation
7. **HAL interface concept** - clean separation of platform-specific operations

### What Needs OpenLcbCLib-Style Adaptation
1. **C not C++** - rewrite in pure C, no namespaces/templates
2. **Interface structs** - HAL functions wired via function pointer structs (per CLAUDE.md pattern), not direct extern calls. This enables test mocking.
3. **Feature flag guards** - all code wrapped in `OPENLCB_COMPILE_BOOTLOADER`, sub-features in `OPENLCB_COMPILE_BOOTLOADER_STREAM` / `OPENLCB_COMPILE_BOOTLOADER_DATAGRAM`
4. **Naming conventions** - `OpenLcb_Bootloader_init()`, `_flush_flash_buffer()` for private
5. **State struct as parameter** - pass `bootloader_state_t *state` instead of global, for testability
6. **Separate files** - split into `openlcb_bootloader.h` / `openlcb_bootloader.c` following project file pattern (not single-header)
7. **Linker script templates** - provide example .ld files showing flash partitioning
8. **Integration with existing CAN driver** - use OpenLcbCLib's CAN driver interface struct rather than raw HAL calls

### Stream vs Datagram Tradeoffs
| Aspect | Stream | Datagram |
|--------|--------|----------|
| Speed | Fast (large buffers) | Slow (64-byte ACK cycles) |
| RAM cost | ~1KB write buffer | ~64 bytes |
| Code size | ~400 extra lines | ~200 extra lines |
| Complexity | Higher (negotiate, proceed) | Lower (write + ACK) |
| Best for | Production firmware updates | Low-resource targets |

### Memory Layout (from linker scripts)
```
+---------------------------+ flash_max
|  Application code/data    |
|  ...                      |
|  app_header (checksum)    |
|  ...                      |
|  Application vectors      |
+---------------------------+ flash_min
|  Bootloader code/data     |
|  Bootloader vectors       |
+---------------------------+ 0x00000000
```

The bootloader occupies the lowest flash addresses and owns the reset vector.
The application's entry point is stored at a known vector offset (e.g., vector 13 on Cortex-M).

---

## 15. Key Source File Locations (OpenMRN)

| File | Lines | Purpose |
|------|-------|---------|
| `src/openlcb/Bootloader.hxx` | 1151 | Complete bootloader state machine |
| `src/openlcb/bootloader_hal.h` | 237 | HAL interface definition |
| `src/openlcb/ApplicationChecksum.hxx` | 77 | Checksum validation logic |
| `src/openlcb/FirmwareUpgradeDefs.hxx` | 60 | Error codes and space definitions |
| `src/openlcb/BootloaderClient.hxx` | 815 | Client-side firmware updater |
| `src/openlcb/BootloaderPort.hxx` | 97 | Test bridge (HAL <-> CAN hub) |
| `boards/ti-tm4c123-generic/BootloaderHal.hxx` | 249 | TI Tiva HAL implementation |
| `boards/ti-tm4c123-generic/bootloader.ld` | ~4KB | Linker script example |
| `src/openlcb/Bootloader.cxxtest` | ~26KB | Stream-mode tests |
| `src/openlcb/BootloaderDg.cxxtest` | ~30KB | Datagram-mode tests |
| `applications/bootloader_client/main.hxx` | ~12KB | CLI firmware update tool |

---

## 16. Spec Compliance Analysis

Analysis of `Bootloader.hxx` (and supporting files) against the OpenLCB standards
located at `/Users/jimkueneman/Documents/OpenLcb Documents/`.

Standards referenced:
- **FW-S** = FirmwareUpgradeS.pdf
- **FW-TN** = FirmwareUpgradeTN.pdf
- **MC-S** = MemoryConfigurationS.pdf
- **MC-TN** = MemoryConfigurationTN.pdf
- **DG-S** = DatagramTransportS.pdf
- **ST-S** = StreamTransportS.pdf (drafts)
- **ST-TN** = StreamTransportTN.pdf (drafts)
- **CAN-S** = CanFrameTransferS.pdf
- **MN-S** = MessageNetworkS.pdf

### 16.1 COMPLIANT

| # | Spec Requirement | Reference | Implementation |
|---|-----------------|-----------|----------------|
| C1 | Node in FW Upgrade state SHALL support writable space 0xEF | FW-S 4 | `FLASH_SPACE = MemoryConfigDefs::SPACE_FIRMWARE` (0xEF); accepts Write and Write Stream to this space |
| C2 | Node SHALL be standards-compliant OpenLCB node in FW Upgrade state | FW-S 4 | Runs full alias init, sends Init Complete, responds to PIP, Verify Node ID |
| C3 | PIP SHALL set Firmware Upgrade Active bit (0x000010) | FW-S 5.3 | `PIP_REPLY_VALUE` includes `Defs::FIRMWARE_UPGRADE_ACTIVE` |
| C4 | PIP SHALL include Datagram bit | FW-S 5.3 | `PIP_REPLY_VALUE` includes `Defs::DATAGRAM` |
| C5 | PIP SHALL include Stream bit when stream mode enabled | FW-S 5.4.1 | `#ifdef BOOTLOADER_STREAM` adds `Defs::STREAM` to PIP |
| C6 | PIP SHALL include Memory Configuration bit | FW-S 5.3 | `PIP_REPLY_VALUE` includes `Defs::MEMORY_CONFIGURATION` |
| C7 | Freeze for space 0xEF SHALL cause re-initialization | FW-S 5.2 | `COMMAND_FREEZE` does `memset(&state_, 0, sizeof(state_))`, causing full CAN alias re-init and new Init Complete |
| C8 | Unfreeze SHALL validate and reboot | FW-S 5.2 | `COMMAND_UNFREEZE` calls `flash_complete()`, then ACKs and sets `request_reset` |
| C9 | Reset/Reboot command SHALL reset to power-on state | MC-S 4.24 | `COMMAND_RESET` sets `request_reset`, which calls `bootloader_reboot()` |
| C10 | CAN alias reservation: 4 CID frames with Node ID portions | CAN-S 6.2.1 | States `SEND_CID_7/6/5/4` send CID with bits 47-36, 35-24, 23-12, 11-0 |
| C11 | RID SHALL be sent after CID sequence | CAN-S 6.2.1 | `SEND_RID` follows `WAIT_RID` |
| C12 | AMD SHALL be sent to transition to Permitted state | CAN-S 6.2.2 | `SEND_AMD` sends AMD with full Node ID |
| C13 | Initialization Complete SHALL be sent after AMD | MN-S 3.4.1 | `SEND_NMRANET_INIT` sends MTI_INITIALIZATION_COMPLETE with Node ID |
| C14 | Alias conflict on CID: reply with RID | CAN-S 6.2.5 | Lines 774-784: detects CID for our alias, replies RID |
| C15 | Alias conflict on non-CID: pick new alias | CAN-S 6.2.5 | Lines 786-789: resets `init_state` to PICK_ALIAS |
| C16 | Alias value SHALL NOT be zero | CAN-S 6.3 | `PICK_ALIAS` loops until non-zero: `while (!state_.alias)` |
| C17 | AME with matching Node ID or no data: reply AMD | CAN-S 6.2.3 | Lines 796-807: checks `dlc == 0 || is_can_frame_nodeid()`, replies AMD |
| C18 | Addressed Verify Node ID: reply Verified Node ID | MN-S 3.4.2 | `handle_addressed_message` case `MTI_VERIFY_NODE_ID_ADDRESSED` replies with global Verified Node ID + Node ID payload |
| C19 | PSI: reply with PSR containing protocol flags | MN-S 3.4.3 | `handle_addressed_message` case `MTI_PROTOCOL_SUPPORT_INQUIRY` replies with 6-byte PIP_REPLY |
| C20 | Datagram received: SHALL reply OK or Rejected | DG-S 6 | All datagram handlers either call `set_can_frame_addressed(MTI_DATAGRAM_OK)` or `reject_datagram()` |
| C21 | Write to wrong space: reject with error | MC-S 4.3 | `COMMAND_WRITE` checks `data[6] != FLASH_SPACE`, rejects with `ERROR_SPACE_NOT_KNOWN` |
| C22 | Out-of-bounds address: reject | MC-S 4.3 | `normalize_write_buffer_offset()` bounds-checks against flash_max - flash_min, rejects with `ERROR_OUT_OF_BOUNDS` |
| C23 | Unknown command: reject datagram | MC-S 4.3 | Default case in `handle_memory_config_frame()` calls `reject_datagram()` with `UNIMPLEMENTED` |
| C24 | Stream buffer negotiation: final size <= proposed | ST-S 4.2 | Lines 594-600: halves `WRITE_BUFFER_SIZE` until <= proposed_size |
| C25 | Stream Data: DID in byte 0, payload in bytes 1-7 | ST-S 4.3/7.1 | `handle_stream_data()` checks `data[0] == STREAM_ID`, copies `data[1..dlc-1]` |
| C26 | Stream Proceed sent when buffer consumed | ST-S 6.3 | Lines 1107-1117: sends `MTI_STREAM_PROCEED` with src_id + STREAM_ID when `stream_proceed_pending` |
| C27 | Stream Complete: flush remaining data | ST-S 6.4 | `handle_stream_complete()` flushes remaining write buffer |
| C28 | Aborted transfer: node stays in FW Upgrade state | FW-S 5.4.3 | On error, bootloader continues running; no transition out of bootloader mode |
| C29 | After power cycle with bad firmware: enter FW Upgrade state | FW-TN 2.6 | `bootloader_init()` checks `check_application_checksum()`; on failure stays in bootloader |
| C30 | Datagram Write Reply Pending flag | MC-S 4.8 | Write Stream handler sets `DatagramDefs::REPLY_PENDING` in Datagram OK flags byte (line 491) |
| C31 | EFF frames only; ignore RTR/ERR | CAN-S 4 | `handle_input_frame()` lines 756-762: drops ERR, RTR, non-EFF |
| C32 | Non-OpenLCB priority frames: ignore | CAN-S 4 | Lines 765-770: drops non-NORMAL_PRIORITY frames |
| C33 | Reset vector protection prevents bricking | FW-TN | TI HAL `erase_flash_page()` re-writes reset vector after erasing page 0 |
| C34 | Datagram middle/final frame sequencing | DG-S 7.2 | `BOOTLOADER_DATAGRAM` code tracks `incoming_datagram_pending` state, rejects out-of-order with `OUT_OF_ORDER` |
| C35 | Flash write buffer initialized to 0xFF (erased state) | impl detail | `init_flash_write_buffer()` does `memset(g_write_buffer, 0xff, WRITE_BUFFER_SIZE)` |

### 16.2 NON-COMPLIANT / DEVIATIONS

| # | Spec Requirement | Reference | Issue |
|---|-----------------|-----------|-------|
| NC1 | **200ms wait between CID and RID** | CAN-S 6.2.1 | `WAIT_RID` is a single state that immediately advances to `SEND_RID` on the next loop iteration. There is NO 200ms delay. On a fast MCU this could be microseconds. The spec says "Wait at least 200 milliseconds." This is a clear violation. On slow CAN buses this might not cause practical problems but other nodes may not have time to detect the alias conflict. |
| NC2 | **Global Verify Node ID with Node ID: reply only if matching** | MN-S 3.4.2 | `handle_global_message()` for `MTI_VERIFY_NODE_ID_GLOBAL` always replies regardless of whether the frame contains a Node ID that doesn't match. The spec says: with no Node ID = reply; with matching Node ID = reply; with non-matching Node ID = do NOT reply. The implementation doesn't check. |
| NC3 | **Write Stream Reply SHALL include Source Stream ID and Destination Stream ID** | MC-S 4.12 | The Write Stream Reply datagram is built by echoing back `can_dlc - 1` bytes of the input command. The Write Stream command is `[0x20][0x20][addr4][space][srcStreamID]` = 8 bytes, so the reply echoes 7 bytes = `[0x20][0x30][addr4][space]`. Per MC-S 4.12, the OK reply should be `[0x20][0x30][addr4][space][srcStreamID][dstStreamID]`. Both stream IDs are **missing** from the reply. The client (BootloaderClient.hxx) likely doesn't validate these fields strictly, masking the issue. |
| NC4 | **Well-Known Events for firmware state** | FW-S 5.2 | The spec says a node MAY emit PCER with "Firmware Corrupted" (01.01.00.00.00.00.06.01) or "Firmware Upgrade Requested by Hardware Switch" (01.01.00.00.00.00.06.02). The bootloader does NOT emit these events. This is a MAY (not SHALL) so technically not a violation, but it's a gap for discoverability. |
| NC5 | **Firmware Upgrade Protocol bit in Operating state** | FW-S 5.3 | The spec says in Operating state, PIP SHALL set "Firmware Upgrade Protocol" bit (0x000020). The bootloader only runs in Firmware Upgrade state, so it never sets this bit. However, the **application** running on the same hardware should set it. This is an application-side concern, not a bootloader bug per se, but worth noting: the application firmware must advertise this bit. |
| NC6 | **Freeze response: Datagram OK not always sent** | FW-S 5.5 | The spec notes "a Datagram Received OK reply is NOT always returned to [Freeze]." OpenMRN's implementation does `memset(&state_, 0, ...)` on Freeze, which wipes the output buffer before any ACK is sent. This is actually spec-conformant (the spec allows omission), but it means the client MUST NOT rely on receiving Datagram OK to Freeze — the client must wait for Init Complete instead. Documenting because it's a deliberate design choice, not an oversight. |
| NC7 | **No Get Address Space Information support** | MC-S 4.16 | The spec says a reply IS REQUIRED for Get Address Space Information queries (present or not present). The bootloader does not handle this command — it falls through to `reject_datagram()` with UNIMPLEMENTED. A compliant CT could query space 0xEF info before writing, and would get an error instead of the expected present/not-present reply. |
| NC8 | **No Get Configuration Options support** | MC-S 4.14 | Similar to NC7, the Get Configuration Options command tells the CT what the node supports (write lengths, supported commands). Not implemented; falls to reject. A strict CT implementation might query this before starting transfer. |
| NC9 | **Alias generation algorithm differs from recommended** | CAN-S 6.3 / CAN-TN 6.1 | The preferred algorithm is a 48-bit LFSR PRNG. The bootloader uses a simpler XOR-fold of the node ID for the initial alias, and a linear congruential step for subsequent attempts: `(alias + (nodeID & 0xffe) + 1759) & 0xfff`. This SHOULD work (spec only says "shall not be zero" and has collision probability requirements), but doesn't match the recommended PRNG. |
| NC10 | **AME with no data: should discard cached alias mappings** | CAN-S 6.2.3 | The spec says upon receipt of AME with no data, a node must discard alias-to-NodeID mappings for other nodes. The bootloader doesn't cache any mappings (it only tracks `write_src_alias`), so this is technically N/A, but the handler doesn't explicitly clear `write_src_alias` either. Minimal risk. |
| NC11 | **Non-CID alias conflict in Permitted state: SHALL send AMR** | CAN-S 6.2.5 | On non-CID alias conflict, the bootloader resets to `PICK_ALIAS` (line 789) but does NOT send an AMR frame first. The spec says: "immediately transition to Inhibited, send AMR, stop using the alias." The missing AMR means other nodes won't know the alias was released. |
| NC12 | **Stream Initiate Reply: Reject format** | ST-S 4.2, 6.1 | When rejecting a stream, the bootloader uses `0b01000010` and `0b00100000` in bytes 4-5 (lines 585-590). The current draft spec encodes this differently, but the spec is being updated to match OpenMRN. **Treat OpenMRN as reference.** |
| NC13 | **Stream Data Proceed format** | ST-S 4.4 | The bootloader sends 4 bytes: `[stream_src_id][STREAM_ID][0][0]` (lines 1112-1116). The current draft spec says 2 bytes: `[DID][SID]`. The spec is being updated to match OpenMRN. **Treat OpenMRN as reference.** |
| NC14 | **750ms response time requirement** | MN-S 3.7 | The spec says nodes shall send required protocol messages within 750ms. The polled bootloader depends on loop speed and flash write timing. A large flash erase (some MCUs take 20-100ms per page) could cause the bootloader to miss this deadline. The spec allows "unless a specific protocol says otherwise" — flash operations might be considered an exception, but no such exception is formally defined. |

### 16.3 AMBIGUOUS / IMPLEMENTATION NOTES

| # | Topic | Notes |
|---|-------|-------|
| A1 | **Stream spec is being updated to match OpenMRN** | StreamTransportS.pdf and StreamTransportTN.pdf are in `drafts/`. The spec is being revised to align with how OpenMRN implements streams, so NC12 and NC13 should be treated as the **reference behavior** rather than deviations. Build the OpenLcbCLib bootloader to match OpenMRN's stream implementation. |
| A2 | **Datagram-mode write: 64-byte payload rule** | FW-S 5.4.2 says "exactly 64 bytes of payload in each datagram except the last." The bootloader doesn't enforce this on the receiving side — it accepts any payload length. This is fine (receiver can be lenient), but a compliant CT must send exactly 64 bytes. |
| A3 | **Checksum algorithm is HAL-defined** | The spec doesn't mandate a specific checksum algorithm. The `app_header` and `checksum_data()` approach is an OpenMRN convention, not an OpenLCB requirement. Other implementations could use CRC32, SHA, etc. |
| A4 | **COMMAND_ENTER_BOOTLOADER** | The bootloader handles this (line 371, falls through from FREEZE) as equivalent to Freeze. This command doesn't appear in the standard spec — it may be an OpenMRN extension. |
| A5 | **Write datagram: address space in byte 6** | For space 0xEF (not 0xFD/FE/FF), the spec requires the space byte in position 6 with byte 1 bits 1:0 = 0b00. The bootloader checks `data[6] != FLASH_SPACE` which is correct. However, it doesn't verify byte 1's space-encoding bits. A malformed datagram with byte 1 = 0x01 (space 0xFD) and byte 6 = 0xEF would be accepted when it should be rejected. Low risk but technically a parsing gap. |
| A6 | **No Inhibited-state AME suppression** | CAN-S 6.2.3 says a node in Inhibited state shall NOT reply to AME. The bootloader's AME handler (line 796) doesn't check whether it's in Permitted state (init_state == INITIALIZED). During CID/RID sequence it could respond to AME when it shouldn't. |

### 16.4 COMPLIANCE SUMMARY

**Overall**: The OpenMRN bootloader is a practical, working implementation that handles the core firmware upgrade flow correctly. It is compliant with the major protocol requirements (Freeze/Unfreeze lifecycle, Write/WriteStream data transfer, PIP advertisement, alias allocation, datagram handling).

**Critical gaps for an OpenLcbCLib re-implementation to fix:**
1. **NC1**: Add 200ms delay between CID sequence and RID (mandatory per CAN-S)
2. **NC2**: Filter global Verify Node ID by Node ID content
3. **NC3**: Include stream IDs in Write Stream Reply datagram
4. **NC11**: Send AMR before picking new alias on non-CID conflict
5. **NC7/NC8**: Consider implementing Get Address Space Info and Get Config Options (at minimum return "present, writable" for space 0xEF)

**Lower priority fixes:**
6. **NC12/NC13**: Not actual issues — stream spec is being updated to match OpenMRN behavior. Use OpenMRN as the reference for stream message formats.
7. **NC9**: Use the recommended PRNG for alias generation
8. **NC14**: Ensure flash operations don't block beyond 750ms (may require chunked erase)
9. **A5/A6**: Tighten input validation

---

## 17. Spec Reference Quick-Links

All specs located under `/Users/jimkueneman/Documents/OpenLcb Documents/`:

| Spec | Path | Key Sections |
|------|------|-------------|
| Firmware Upgrade Standard | `standards/FirmwareUpgradeS.pdf` | Sec 4 (states), 5.2 (transitions), 5.3 (PIP bits), 5.4 (data transfer), 5.5 (full sequence) |
| Firmware Upgrade TN | `standards/FirmwareUpgradeTN.pdf` | Sec 1 (recovery), 2.6 (wire-level sequence) |
| Memory Configuration Standard | `standards/MemoryConfigurationS.pdf` | Sec 4.8 (Write), 4.11 (Write Stream), 4.12 (Write Stream Reply), 4.21-22 (Freeze/Unfreeze), 4.24 (Reset) |
| Memory Configuration TN | `standards/MemoryConfigurationTN.pdf` | Sec 2.4.21 (Freeze semantics), 3.1.3 (write timing) |
| Datagram Transport Standard | `standards/DatagramTransportS.pdf` | Sec 4.2 (OK), 4.3 (Rejected), 6 (interactions), 7.1 (CAN frames) |
| Stream Transport Standard | `drafts/StreamTransportS.pdf` | Sec 4.1-4.6 (messages), 6.1-6.4 (interactions), 7.1 (CAN) |
| Stream Transport TN | `drafts/StreamTransportTN.pdf` | Sec 2.4 (message details), 3.3 (CAN considerations) |
| CAN Frame Transfer Standard | `standards/CanFrameTransferS.pdf` | Sec 5 (states), 6.2 (alias allocation), 6.2.5 (collision) |
| Message Network Standard | `standards/MessageNetworkS.pdf` | Sec 3.2 (states), 3.3 (messages), 3.4 (interactions), 3.7 (timing) |

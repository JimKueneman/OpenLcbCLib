# Bootloader Architecture

## Overview

The bootloader is a standalone OpenLCB node that supports firmware upload over
CAN bus using the Memory Configuration protocol. It implements only the subset
of OpenLCB required for discovery (PIP, SNIP, Verify Node ID) and firmware
transfer (Freeze, Write, Unfreeze, Reboot).

The codebase is split into four modules with clean header interfaces. Each
module has one job and communicates through a defined contract. Direct
cross-module `#include` of implementation files is permitted (no DI function
pointer structs between protocol modules), but hardware hooks use DI structs
for testability and maintainability.

---

## Module Map

```
bootloader/src/
  openlcb/
    bootloader.c/.h                   Top-level entry, init/loop
    bootloader_openlcb_statemachine.c/.h  Protocol dispatch (PIP, SNIP, memconfig)
    bootloader_types.h                Types, constants, DI structs
  drivers/canbus/
    bootloader_can.h                  CAN ID constants, shared declarations
    bootloader_can_statemachine.c/.h  Alias reservation, collision, frame dispatch
    bootloader_rx.c/.h                RX polling, filtering, datagram assembly
    bootloader_tx.c/.h                Frame sending (non-blocking + blocking multi-frame)
  crc/
    bootloader_crc.c/.h               CRC-16/IBM triple checksum
```

---

## Call Flow

```
main.c
  BootloaderDriversOpenlcb_initialize_hardware()
  Bootloader_entry(&can_driver, &openlcb_driver)
    bootloader.c
      Bootloader_init()
        BootloaderCanSM_init(can_driver, openlcb_driver)
          BootloaderRx_init(can_driver)
          BootloaderTx_init(can_driver)
          BootloaderOpenlcbSM_init(openlcb_driver)
        Check: is_bootloader_requested? checksum valid? -> run or jump to app

      Bootloader_loop()  [called forever]
        BootloaderCanSM_loop()
          BootloaderRx_poll(alias, tick)       <- drain HW FIFO, assemble datagrams
          check collision flags                <- RID or restart alias
          if initializing: alias state machine (CID, wait, RID, AMD, Init Complete)
          if initialized:
            check BootloaderRx_has_error()     <- send datagram rejected if assembly failed
            check BootloaderRx_has_datagram()  <- dispatch complete datagrams
            dequeue non-datagram frames, dispatch:
              CAN control frames   -> internal (AME reply)
              Addressed messages   -> BootloaderOpenlcbSM_on_addressed_message()
              Global messages      -> BootloaderOpenlcbSM_on_global_message()

          Protocol handlers call back into CAN SM send helpers:
            BootloaderCanSM_send_addressed()  -> BootloaderTx_send_multiframe()
            BootloaderCanSM_send_global()     -> BootloaderTx_send_frame()
            BootloaderCanSM_send_datagram()   -> single or multi-frame (FIRST/MIDDLE/FINAL)
            BootloaderCanSM_send_datagram_ok/rejected()
```

---

## Module Details

### 1. RX Engine (`bootloader_rx.c/.h`)

**Responsibility:** Poll CAN hardware, filter frames, buffer non-datagram
frames, detect alias collisions, and assemble multi-frame CAN datagrams into
complete OpenLCB datagrams.

**Interface:**

| Function | Description |
|---|---|
| `BootloaderRx_init(can_driver)` | Store driver pointer, reset buffer and flags |
| `BootloaderRx_poll(our_alias, current_tick)` | Drain HW FIFO; filter; assemble datagrams; set collision flags |
| `BootloaderRx_has_frame()` | True if buffered non-datagram frame(s) available |
| `BootloaderRx_get_frame(frame)` | Dequeue oldest non-datagram frame; returns false if empty |
| `BootloaderRx_has_datagram()` | True if a complete datagram is ready |
| `BootloaderRx_get_datagram(src_alias, buffer, len)` | Copy out the assembled datagram; clears ready flag |
| `BootloaderRx_has_error()` | True if a datagram assembly error occurred |
| `BootloaderRx_get_error()` | Return error struct (error_code, src_alias); clears flag |
| `BootloaderRx_collision_cid()` | True if CID collision seen since last clear |
| `BootloaderRx_collision_non_cid()` | True if non-CID collision seen since last clear |
| `BootloaderRx_clear_collision_flags()` | Reset both collision flags |

**Internal state:** Circular buffer of `bootloader_can_frame_t` (64 entries)
for non-datagram frames; 72-byte datagram assembly buffer with source alias,
in-progress flag, last-tick timestamp, and completion/error flags.

**Datagram assembly:** DATAGRAM_ONLY frames produce an immediate complete
datagram. DATAGRAM_FIRST starts assembly, DATAGRAM_MIDDLE appends, and
DATAGRAM_FINAL completes. Errors are flagged (not sent directly) for:
- MIDDLE/FINAL with no matching FIRST in progress
- New FIRST arriving while previous assembly is incomplete
- Assembly timeout (3 seconds with no continuation frame)

The CAN SM loop checks `has_error()` after each poll and sends the rejection
through `BootloaderCanSM_send_datagram_rejected()`. This avoids a circular
dependency between the RX engine and the send path.

**Filter rules — what the RX engine keeps vs discards:**

| Frame Type | Keep/Discard | Reason |
|---|---|---|
| CID with our alias | Flag only (`collision_cid`) | Must send RID to defend alias |
| Non-CID control frame with our alias as source | Flag only (`collision_non_cid`) | Must go Inhibited, send AMR, restart alias |
| Datagram frames addressed to us (dest alias in CAN ID) | Assemble | Firmware write data from host |
| Addressed messages for us (dest alias in data bytes) | Buffer | Must respond to PIP, SNIP, Verify Addressed, Datagram OK/Rejected |
| Verify Node ID Global (MTI 0x0490) | Buffer | Spec says "shall reply" with Verified Node ID |
| Everything else | Discard | Not for us, or global traffic we don't handle |

### 2. TX Engine (`bootloader_tx.c/.h`)

**Responsibility:** All outbound CAN framing. Takes OpenLCB-level parameters
(MTI, aliases, payload) and constructs the correct CAN frames — CAN ID bit
packing, frame type selection, multi-frame chunking, and addressed message
framing flags. The CAN SM delegates all sending here.

**Interface:**

| Function | Description |
|---|---|
| `BootloaderTx_init(can_driver)` | Store driver pointer |
| `BootloaderTx_send_frame(frame)` | Non-blocking raw frame send; returns true if sent, false if HW busy |
| `BootloaderTx_send_multiframe(mti, src, dest, alias, tick, data, len)` | Blocking addressed message; MULTIFRAME_FIRST/MIDDLE/FINAL framing in data[0] |
| `BootloaderTx_send_global(mti, src, alias, tick, data, len)` | Blocking global message; MTI in CAN ID variable field |
| `BootloaderTx_send_datagram(src, dest, alias, tick, data, len)` | Blocking datagram; ONLY for ≤8 bytes, FIRST/MIDDLE/FINAL for larger |
| `BootloaderTx_send_datagram_ok(src, dest, alias, tick, flags)` | Blocking Datagram Received OK with flags byte |
| `BootloaderTx_send_datagram_rejected(src, dest, alias, tick, error)` | Blocking Datagram Rejected with error code |

**Why blocking multi-frame drains RX:** During layout startup, the CAN bus can
be saturated with high-priority traffic (Initialization Complete = priority 0,
event identification = priority 1). SNIP replies (priority 2) and datagrams
(priority 3) lose CAN arbitration and may stall. The TX spin loop calls
`BootloaderRx_poll()` each iteration to keep the HW FIFO drained so incoming
frames are not lost.

### 3. CAN State Machine (`bootloader_can_statemachine.c/.h`)

**Responsibility:** Alias reservation (CID sequence + 200ms wait + RID + AMD +
Initialization Complete), collision response, AME handling, frame dispatch to
the OpenLCB state machine, and send helpers. The send helpers are thin wrappers
that supply the current alias and tick, then delegate all CAN framing to the
TX engine.

**Interface:**

| Function | Description |
|---|---|
| `BootloaderCanSM_init(can_driver, openlcb_driver)` | Init RX, TX, OpenLCB SM; start alias reservation |
| `BootloaderCanSM_loop()` | One main-loop pass: poll RX, collisions, errors, datagrams, dispatch |
| `BootloaderCanSM_is_initialized()` | True after alias negotiation complete |
| `BootloaderCanSM_alias()` | Current 12-bit alias |
| `BootloaderCanSM_send_addressed(mti, dest, node_id, data, len)` | Delegate to `BootloaderTx_send_multiframe` |
| `BootloaderCanSM_send_global(mti, data, len)` | Delegate to `BootloaderTx_send_global` |
| `BootloaderCanSM_send_datagram(dest, node_id, data, len)` | Delegate to `BootloaderTx_send_datagram` |
| `BootloaderCanSM_send_datagram_ok(dest, node_id)` | Delegate to `BootloaderTx_send_datagram_ok` |
| `BootloaderCanSM_send_datagram_rejected(dest, node_id, error)` | Delegate to `BootloaderTx_send_datagram_rejected` |

**Alias reservation states:**

```
SEND_CID7 -> SEND_CID6 -> SEND_CID5 -> SEND_CID4 -> WAIT_200MS -> SEND_RID
  -> SEND_AMD -> SEND_INIT_COMPLETE -> INITIALIZED
```

**Collision handling:**
- `collision_cid` flag set: send RID frame to defend alias
- `collision_non_cid` flag set: send AMR, restart alias reservation

### 4. OpenLCB State Machine (`bootloader_openlcb_statemachine.c/.h`)

**Responsibility:** Protocol-level message dispatch. Handles PIP, SNIP, Verify
Node ID, OIR, and Memory Configuration commands (Freeze, Write, Unfreeze,
Reboot, Get Options, Get Address Space Info).

**Interface:**

| Function | Description |
|---|---|
| `BootloaderOpenlcbSM_init(openlcb_driver)` | Store driver pointer, reset state |
| `BootloaderOpenlcbSM_on_datagram_received(src, node_id, data, len)` | Dispatch to memconfig handler |
| `BootloaderOpenlcbSM_on_addressed_message(mti, src, node_id, data, len)` | Dispatch PIP, SNIP, Verify, Datagram OK; OIR for unknown |
| `BootloaderOpenlcbSM_on_global_message(mti, data, len)` | Handle Verify Node ID Global |

**Handled MTIs:**

| MTI | Handler |
|---|---|
| `0x0828` Protocol Support Inquiry | Reply with PIP bits (Datagram, MemConfig, SNIP, Firmware) |
| `0x0DE8` Simple Node Info Request | Reply with SNIP data (manufacturer, model, versions) |
| `0x0488` Verify Node ID Addressed | Reply with Verified Node ID |
| `0x0490` Verify Node ID Global | Reply with Verified Node ID (match check if Node ID present) |
| `0x0A28` Datagram OK Reply | Acknowledge (no action needed) |
| Unknown | Optional Interaction Rejected with error code + rejected MTI |

**Memory Configuration commands:**

| Command | Action |
|---|---|
| Freeze (0xA1) | Activate firmware mode, reset write state |
| Write (0x00, space in byte 6) | Validate space=0xEF, write data to flash, send write reply datagram |
| Unfreeze (0xA0) | Finalize flash via `compute_checksum_helper`, verify checksum |
| Reboot (0xA9) | Request reset |
| Get Options (0x80) | Reply with supported capabilities |
| Get Address Space Info (0x84) | Reply present/not-present for firmware space |

**Write flow (with flow control):**

```
Host sends Write datagram
  -> Bootloader sends Datagram Received OK (with Reply Pending flag)
  -> Bootloader erases page if needed, writes flash
  -> On success: send Write Reply OK datagram (0x20, 0x10, addr, space)
  -> On failure: send Write Reply Fail datagram (0x20, 0x18, addr, space, error_code)
Host waits for write reply before sending next block
```

This two-phase reply (ACK + response datagram) throttles the host so it cannot
outrun flash erase/write timing. Without it, the host would fire the next write
immediately after the ACK.

---

## Dependency Injection

Hardware hooks use DI structs defined in `bootloader_types.h`. The application
wires them up at startup in `bootloader_main.c`.

### `bootloader_can_driver_t`

Used by RX and TX engines.

| Field | Type | Purpose |
|---|---|---|
| `read_received_frame` | `bool (*)(bootloader_can_frame_t *)` | Poll HW for next received frame |
| `try_send_frame` | `bool (*)(const bootloader_can_frame_t *)` | Attempt to send; false if HW busy |
| `get_cached_alias` | `uint16_t (*)(void)` | Return alias passed from application, or 0 for auto-generation |

### `bootloader_openlcb_driver_t`

Used by CAN SM, OpenLCB SM, and bootloader.c.

| Field | Type | Purpose |
|---|---|---|
| `get_persistent_node_id` | `uint64_t (*)(void)` | Return 48-bit OpenLCB Node ID |
| `get_100ms_timer_tick` | `uint8_t (*)(void)` | Monotonic 100ms tick counter |
| `set_status_led` | `void (*)(led_enum, bool)` | Control status LEDs |
| `is_bootloader_requested` | `bool (*)(void)` | Check if bootloader entry was requested |
| `jump_to_application` | `void (*)(void)` | Jump to application code |
| `reboot` | `void (*)(void)` | Reset the processor |
| `initialize_hardware` | `void (*)(void)` | Reset peripherals and initialize hardware |
| `get_flash_boundaries` | `void (*)(...)` | Return app flash min/max and header pointer |
| `get_flash_page_info` | `void (*)(...)` | Return page start and length for an address |
| `erase_flash_page` | `uint16_t (*)(...)` | Erase one flash page; 0=success, error code on failure |
| `write_flash` | `uint16_t (*)(...)` | Write data to flash; 0=success, error code on failure |
| `finalize_flash` | `uint16_t (*)(checksum_helper)` | Validate written firmware; receives `compute_checksum` as helper |
| `compute_checksum` | `void (*)(...)` | Compute checksum over a data range |

### Wiring example (TI Theia MSPM0G3507)

```c
// bootloader_main.c
const bootloader_can_driver_t can_driver = {
    .read_received_frame = BootloaderDriversCan_read_received_frame,
    .try_send_frame      = BootloaderDriversCan_try_send_frame,
    .get_cached_alias    = BootloaderDriversCan_get_cached_alias_passed_from_application
};

const bootloader_openlcb_driver_t openlcb_driver = {
    .get_persistent_node_id = BootloaderDriversOpenlcb_get_persistent_node_id,
    .get_100ms_timer_tick   = BootloaderDriversOpenlcb_get_100ms_timer_tick,
    .set_status_led         = BootloaderDriversOpenlcb_set_status_led,
    .is_bootloader_requested = BootloaderDriversOpenlcb_is_bootloader_requested,
    .jump_to_application    = BootloaderDriversOpenlcb_jump_to_application,
    .reboot                 = BootloaderDriversOpenlcb_reboot,
    .initialize_hardware    = BootloaderDriversOpenlcb_initialize_hardware,
    .get_flash_boundaries   = BootloaderDriversOpenlcb_get_flash_boundaries,
    .get_flash_page_info    = BootloaderDriversOpenlcb_get_flash_page_info,
    .erase_flash_page       = BootloaderDriversOpenlcb_erase_flash_page,
    .write_flash            = BootloaderDriversOpenlcb_write_flash,
    .finalize_flash         = BootloaderDriversOpenlcb_finalize_flash,
    .compute_checksum       = BootloaderDriversOpenlcb_compute_checksum
};

BootloaderDriversOpenlcb_initialize_hardware();
Bootloader_entry(&can_driver, &openlcb_driver);
```

Application drivers are split into two files matching the DI struct split:
- `bootloader_drivers_can.h/.c` — CAN hardware (MCAN RX/TX, cached alias)
- `bootloader_drivers_openlcb.h/.c` — OpenLCB hardware (flash, lifecycle, LED, timer, identity, checksum)

---

## CAN Priority and Bus Contention

OpenLCB message priority on CAN (lower = wins arbitration):

| Priority | Messages |
|---|---|
| 0 | Initialization Complete, Verified Node ID |
| 1 | Producer/Consumer Events, Identify Events |
| 2 | SNIP Reply, PIP Reply |
| 3 | Datagram frames |

During layout startup, the bus is flooded with priority 0-1 traffic. The
bootloader's responses (SNIP = priority 2, datagrams = priority 3) lose
arbitration to every one of those frames. The TX engine's blocking multi-frame
send handles this by calling `BootloaderRx_poll()` while spinning, keeping the
RX FIFO drained even if TX stalls for an extended period.

CAN hardware filtering cannot help here. The startup flood consists of frame
format 1 (global) messages where the destination (if any) is in the data
payload, not the CAN ID. The MCAN hardware can only filter on the 29-bit CAN
ID, so it cannot distinguish "addressed to us" from "addressed to someone
else" for format 1 frames.

---

## Design Differences from Main Library

| Aspect | Main Library | Bootloader |
|---|---|---|
| Inter-module coupling | DI structs (function pointers) wired in `openlcb_config.c` | Direct `#include` and function calls between protocol modules |
| Feature flags | `OPENLCB_COMPILE_*` ifdefs | None — bootloader has no conditional compilation |
| Hardware abstraction | DI structs | DI structs (same pattern) |
| Code priority | Flexibility, extensibility | Minimal code size, clarity |
| Buffer strategy | Dispatcher queues, sibling dispatch | Circular buffer (non-datagram) + single assembly buffer (datagrams) |
| Datagram assembly | In-flight buffer list, multiple concurrent datagrams | Single 72-byte assembly buffer, one datagram at a time |
| Write flow control | Reply pending + response datagram | Same pattern — ACK with pending, then Write Reply OK/Fail |

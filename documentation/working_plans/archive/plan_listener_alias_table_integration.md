<!--
  ============================================================
  STATUS: IMPLEMENTED (core wiring complete; hold-until-AMD deferred — v1.0.0 known limitation)

  Verified against codebase 2026-03-17:

  IMPLEMENTED:
  - DI pointers on interface_can_rx_message_handler_t: listener_set_alias,
    listener_clear_alias_by_alias, listener_flush_aliases (wired in
    _build_rx_message_handler() in can_config.c under #ifdef OPENLCB_COMPILE_TRAIN).
  - AMD handler: ListenerAliasTable_set_alias() called and
    _release_held_messages_for_listener() helper present (can_rx_message_handler.c).
  - AMR handler: listener_clear_alias_by_alias DI call present.
  - TX alias resolution: nullable listener_find_by_node_id DI call present in
    can_tx_statemachine.c lines 149-165.
  - ListenerAliasTable_initialize() called from CanConfig_initialize() in
    can_config.c (line 303) under #ifdef OPENLCB_COMPILE_TRAIN. [Fixed 2026-03-15]
  - _build_tx_statemachine() in can_config.c wires
    _tx_sm.listener_find_by_node_id = &ListenerAliasTable_find_by_node_id
    under #ifdef OPENLCB_COMPILE_TRAIN (line 205). [Fixed 2026-03-15]
    Consist forwarding is now functional.

  KNOWN LIMITATION (accepted for v1.0.0):
  - Hold-until-AMD (Step 2): last_frame() in can_rx_message_handler.c does NOT
    call _check_hold_for_alias_resolution(). Attach messages go to the FIFO
    immediately. The first consist command to a brand-new listener may be dropped
    if the alias is not yet in the table; subsequent commands succeed once the
    AMD reply arrives.
  ============================================================
-->

# Listener Alias Table — CAN Layer Integration Plan (v5)

## Overview

The `alias_mapping_listener` module (`.h`, `.c`, `_Test.cxx`) already exists and is
compiled into the build. It provides a static table mapping listener Node IDs to
their 12-bit CAN aliases. This plan wires it into the CAN transport layer so that
consist-forwarded messages can resolve `dest_alias` transparently.

**This version replaces v4** by incorporating implementation progress, resolved
design decisions, the `openlcb_msg_timer_t` union refactor, and the Periodic
Listener Alias Verification design.

---

## Implementation Status

| Step | Description | Status |
|------|-------------|--------|
| 1 | Initialization — `can_config.c` | **NOT STARTED** |
| 2 | Detect attach in CAN RX — hold-until-AMD | **NOT STARTED** |
| 3 | AMD releases held messages | **IMPLEMENTED** (AMD handler expanded, `_release_held_messages_for_listener()` helper added) |
| 4 | AMR clears listener alias | **IMPLEMENTED** (DI call added to AMR handler) |
| 5 | Detach cleanup — reference counting | **SKIPPED** (lazy cleanup decision — see Resolved Decisions) |
| 6 | TX alias resolution | **IMPLEMENTED** (DI lookup in `can_tx_statemachine.c`) |
| 7 | Periodic Listener Alias Verification | **DESIGNED** (not yet implemented — see new section below) |

### Other Completed Work

| Item | Status |
|------|--------|
| DI pointers added to `can_rx_message_handler.h` (`listener_register`, `listener_set_alias`, `listener_clear_alias_by_alias`) | **IMPLEMENTED** |
| DI pointer added to `can_tx_statemachine.h` (`listener_find_by_node_id`) | **IMPLEMENTED** |
| `openlcb_msg_timer_t` union refactor (replaced packed `timerticks` field) | **IMPLEMENTED** and tested |
| Style guide updated (single-line `continue`/`break` alongside `return`) | **DONE** |
| Style fixes in `drivers/canbus/*.c` (31 fixes across 7 files) | **DONE** |
| Style fixes in `src/openlcb/*.c` | **NOT DONE** (halted by user) |

---

## Prerequisites — Already Implemented

The following systems are in place and tested. This plan builds on them.

### AMR alias invalidation (`plan_amr_alias_invalidation.md`)

| Defense | Where | What it does |
|---------|-------|-------------|
| BufferList scrub | `_check_and_release_messages_by_source_alias()` | Frees all messages with `source_alias == released_alias` |
| FIFO invalidation | `OpenLcbBufferFifo_check_and_invalidate_messages_by_source_alias()` | Sets `state.invalid` on any FIFO message with `source_alias == released_alias` |
| Pop-phase guard | `OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message()` | Discards `state.invalid` messages before protocol dispatch |
| TX guard | `CanTxStatemachine_send_openlcb_message()` | Discards `state.invalid` messages before transmission |

### 3-second timeout (`openlcb_buffer_list.c`)

`OpenLcbBufferList_check_timeouts()` runs every main-loop iteration. Any message
in the BufferList with `state.inprocess == true` for >= 30 ticks (3 seconds) is
freed automatically. This is the safety net for the hold-until-AMD pattern: if
AMD never arrives, the held attach message times out and is dropped. The protocol
layer never sees it, and the node's internal state stays consistent.

### `openlcb_msg_timer_t` union (`openlcb_types.h`)

The old packed `uint8_t timerticks` field has been replaced with a union:

```c
typedef union {

    uint8_t assembly_ticks;     /**< Full 8-bit tick for multi-frame assembly timeout */

    struct {

        uint8_t tick_snapshot : 5;  /**< 5-bit tick snapshot (0-31) for datagram retry timeout */
        uint8_t retry_count : 3;   /**< Datagram retry counter (0-7) */

    } datagram;

} openlcb_msg_timer_t;
```

All references updated across the codebase. All 36 test suites (1,193 tests) pass.

---

## Architecture — Hold-Until-AMD Pattern

```
CAN Bus
  │
  ▼
CAN RX Statemachine
  │
  ├─ last_frame() / single_frame()
  │      │
  │      ├── Is this a Train Listener Attach? ──► YES:
  │      │                                          │
  │      │                                          ├── Extract listener node_id from payload
  │      │                                          ├── ListenerAliasTable_register(node_id)
  │      │                                          ├── Is alias already resolved (non-zero)?
  │      │                                          │     YES: Release from LIST → push to FIFO (normal)
  │      │                                          │     NO:  Keep in LIST (state.inprocess = true)
  │      │                                          │          Reset timer.assembly_ticks for fresh 3s window
  │      │                                          │          Queue targeted AME via CanBufferFifo
  │      │                                          │
  │      │                                          └── (message waits for AMD or 3s timeout)
  │      │
  │      └── NOT attach: Release from LIST → push to FIFO (normal, unchanged)
  │
  ▼
AMD frame arrives
  │
  ├── ListenerAliasTable_set_alias(node_id, alias)        ◄── IMPLEMENTED
  ├── Scan LIST for held messages matching this node_id    ◄── IMPLEMENTED
  │     For each match:
  │       ├── state.inprocess = false
  │       ├── Release from LIST
  │       └── Push to OpenLCB FIFO
  │
  ▼
3-second timeout (EXISTING — no new code)
  │
  ├── OpenLcbBufferList_check_timeouts()
  │     Any still-held message with elapsed >= 30 ticks:
  │       ├── Release from LIST
  │       ├── Free buffer
  │       └── Attach command silently dropped — protocol layer never sees it
  │
  ▼
AMR frame arrives
  │
  ├── _check_and_release_messages_by_source_alias(alias) ◄── EXISTING
  ├── OpenLcbBufferFifo_check_and_invalidate_messages_    ◄── EXISTING
  │     by_source_alias(alias)
  ├── _interface->listener_clear_alias_by_alias(alias)   ◄── IMPLEMENTED (via DI)
  │
  ▼
TX path
  │
  ├── If dest_alias == 0 && dest_id != 0:               ◄── IMPLEMENTED
  │     Look up ListenerAliasTable_find_by_node_id(dest_id)
  │     If alias resolved: fill in dest_alias, transmit
  │     If not resolved: drop message (return true to clear it)
  │
  └── Normal transmission
```

## What the Existing Systems Already Cover

| Scenario | Covered by | New code needed? |
|----------|------------|-----------------|
| Incoming message from released alias is in FIFO | `OpenLcbBufferFifo_check_and_invalidate_messages_by_source_alias()` sets `state.invalid`; pop-phase guard discards | **No** |
| Forwarded consist msg is being TX'd when AMR arrives (mid-flight) | Let it finish (contiguous-frame requirement) — accepted exposure | **No** |
| Any message has `state.invalid` when popped from FIFO | Pop-phase guard discards | **No** |
| Any message has `state.invalid` when reaching TX path | TX guard discards | **No** |
| Any message from released alias is in BufferList | `_check_and_release_messages_by_source_alias()` frees it immediately | **No** |
| Held attach message never gets AMD response | 3-second timeout frees it | **No** |
| Held attach message's sender sends AMR | BufferList scrub frees it immediately | **No** |
| Listener node sends AMR while attach is held | 3-second timeout drops it. Listener table entry cleared by DI call. | **No** |

---

## Step 1: Initialization — `can_config.c` — NOT STARTED

**File**: `src/drivers/canbus/can_config.c`

**What**: Call `ListenerAliasTable_initialize()` at startup. Wire the DI function
pointers for both `interface_can_rx_message_handler_t` and
`interface_can_tx_statemachine_t`.

**Where**: Add to `CanConfig_initialize()` after `AliasMappings_initialize()`.

```c
#include "alias_mapping_listener.h"

// In CanConfig_initialize(), after AliasMappings_initialize():
ListenerAliasTable_initialize();
```

Wire DI pointers (gated on train feature flag):
```c
// In interface_can_rx_message_handler_t setup:
.listener_register = ListenerAliasTable_register,
.listener_set_alias = ListenerAliasTable_set_alias,
.listener_clear_alias_by_alias = ListenerAliasTable_clear_alias_by_alias,

// In interface_can_tx_statemachine_t setup:
.listener_find_by_node_id = ListenerAliasTable_find_by_node_id,
```

---

## Step 2: Detect attach in CAN RX — NOT STARTED

**File**: `src/drivers/canbus/can_rx_message_handler.c`

**What**: After `last_frame()` completes message assembly, inspect the message. If
it is a Train Listener Attach (`MTI_TRAIN_PROTOCOL`, instruction byte =
`TRAIN_LISTENER_CONFIG`, sub-command = `TRAIN_LISTENER_ATTACH`), register the
listener Node ID and decide whether to hold or release.

**Design**: Add a static helper `_check_hold_for_alias_resolution()` called at the
end of `CanRxMessageHandler_last_frame()`, after `state.inprocess = false` but
BEFORE `OpenLcbBufferList_release()` and `OpenLcbBufferFifo_push()`.

Modified flow:
```c
    target_openlcb_msg->state.inprocess = false;

    if (_check_hold_for_alias_resolution(target_openlcb_msg)) {

        return;  // message stays in LIST, AME queued

    }

    OpenLcbBufferList_release(target_openlcb_msg);
    OpenLcbBufferFifo_push(target_openlcb_msg);
```

The helper:
```c
static bool _check_hold_for_alias_resolution(openlcb_msg_t *msg) {

    if (msg->mti != MTI_TRAIN_PROTOCOL) { return false; }

    uint8_t instruction = msg->payload->basic[0];
    if (instruction != TRAIN_LISTENER_CONFIG) { return false; }

    uint8_t sub_command = msg->payload->basic[1];
    if (sub_command != TRAIN_LISTENER_ATTACH) { return false; }

    // Listener Node ID at payload offset 3 (byte 0=instruction, 1=sub-cmd, 2=flags, 3-8=node_id)
    node_id_t listener_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(msg, 3);

    if (!_interface->listener_register) { return false; }  // listener table not linked in

    listener_alias_entry_t *entry = _interface->listener_register(listener_id);

    if (!entry) { return false; }  // table full — let it through, alias resolution will fail at TX

    if (entry->alias != 0) { return false; }  // alias already resolved — release normally

    // Hold: alias unresolved — keep in LIST, queue AME
    msg->state.inprocess = true;
    msg->timer.assembly_ticks = _interface->get_current_tick();  // fresh 3s window

    _queue_targeted_ame(msg->dest_alias, listener_id);

    return true;  // caller skips release/push

}
```

The AME helper:
```c
static void _queue_targeted_ame(uint16_t source_alias, node_id_t node_id) {

    can_msg_t *ame_msg = _interface->can_buffer_store_allocate_buffer();

    if (ame_msg) {

        ame_msg->identifier = RESERVED_TOP_BIT
                | CAN_CONTROL_FRAME_AME
                | source_alias;
        CanUtilities_copy_node_id_to_payload(ame_msg, node_id, 0);
        CanBufferFifo_push(ame_msg);

    }

}
```

**AME source alias**: Uses `msg->dest_alias` — the alias of our train node that
received the attach command.

**Timeout safety**: If AMD never arrives, the held message has
`state.inprocess = true` and a fresh `timer.assembly_ticks`. After 3 seconds,
`OpenLcbBufferList_check_timeouts()` frees it automatically.

**Detach**: NOT held. Detach commands go straight through to the protocol layer.

**`single_frame()` path**: Not needed. Attach payload is instruction (1) +
sub-cmd (1) + flags (1) + node_id (6) = 9 bytes. With 2 bytes of dest_alias
overhead for addressed messages, that's 11 bytes — exceeds the 8-byte CAN
payload. Attach is always multi-frame and always goes through `last_frame()`.

---

## Step 3: AMD releases held messages — IMPLEMENTED

**File**: `src/drivers/canbus/can_rx_message_handler.c`

**What**: When an AMD frame arrives, store the alias in the listener table AND
scan the OpenLcbBufferList for any held attach messages waiting on this Node ID.

**Implementation**: AMD handler expanded with DI call to `listener_set_alias()`
and call to `_release_held_messages_for_listener()`. Both are in the current code.

---

## Step 4: AMR clears listener alias — IMPLEMENTED

**File**: `src/drivers/canbus/can_rx_message_handler.c`

**What**: DI call to `listener_clear_alias_by_alias()` added to AMR handler after
existing BufferList scrub and FIFO invalidation.

---

## Step 5: Detach cleanup — SKIPPED (lazy cleanup)

**Decision**: Use lazy cleanup. No reference counting. Stale entries are harmless
— the alias stays valid (or gets updated by AMD/AMR). The table is sized for
worst case (`USER_DEFINED_MAX_LISTENERS_PER_TRAIN * USER_DEFINED_TRAIN_NODE_COUNT`).
`ListenerAliasTable_unregister()` exists but won't be called in this integration.

---

## Step 6: TX alias resolution — IMPLEMENTED

**File**: `src/drivers/canbus/can_tx_statemachine.c`

**What**: Before transmitting a forwarded consist message, resolve `dest_alias`
from the listener alias table if it is zero. DI lookup via
`listener_find_by_node_id`. Returns `true` (drop) if alias unresolvable.

---

## Step 7: Periodic Listener Alias Verification — DESIGNED, NOT IMPLEMENTED

### Problem

If a listener node disappears without sending AMR (power loss, crash, etc.), the
listener alias table retains a stale alias. Forwarded consist commands are sent to
a nonexistent node. No error is returned — fire-and-forget forwarding. This is the
same exposure the standard alias mapping table has for all nodes, but for consist
listeners the impact is more significant (silent message loss for train operations).

### Design

**Approach**: The train node periodically sends targeted AME frames to verify each
registered listener's alias is still valid. If no AMD reply within 3 seconds, the
alias is stale and gets cleared.

**Changes to `listener_alias_entry_t`** (`can_types.h`):

```c
typedef struct {

    node_id_t node_id;          /**< 48-bit OpenLCB Node ID (0 = empty slot) */
    uint16_t alias;             /**< 12-bit CAN alias (0 = unresolved) */
    uint8_t verify_ticks;       /**< Tick snapshot when AME was sent (for 3s timeout) */
    uint16_t verify_pending : 1; /**< true = AME sent, waiting for AMD reply */

} listener_alias_entry_t;
```

**State machine per entry**:

```
Empty (node_id == 0)
  │
  ├── register() → Registered (alias == 0, verify_pending == 0)
  │
  ▼
Registered
  │
  ├── set_alias() → Resolved (alias != 0, verify_pending == 0)
  │
  ▼
Resolved
  │
  ├── check_verifications() countdown expires → Verifying
  │     send targeted AME, verify_pending = 1, verify_ticks = current_tick
  │
  ▼
Verifying
  │
  ├── set_alias() called (AMD arrived) → Resolved (verify_pending = 0)
  │
  ├── 3s timeout (no AMD) → Stale detected
  │     alias = 0, verify_pending = 0
  │     Entry reverts to Registered state
  │
  ▼
(entry stays Registered — future AMD re-populates alias)
```

**New function**: `ListenerAliasTable_check_verifications(uint8_t current_tick)`

- Returns `node_id_t` — the node_id to probe (caller builds targeted AME)
- Returns 0 if nothing to do this cycle
- Round-robin scans one entry per call
- Uses static `_scan_index` to track position
- Uses static `uint16_t _probe_countdown` for 30-second probe interval (300 ticks)
- When countdown expires: find next resolved, non-pending entry, set `verify_pending = 1`, snapshot `verify_ticks`, return its `node_id`
- When `verify_pending == 1` and elapsed >= 30 ticks (3s): alias is stale, clear it

**Modifications to existing functions**:

- `ListenerAliasTable_set_alias()`: Also clear `verify_pending = 0` (AMD arrived = verified)
- `ListenerAliasTable_clear_alias_by_alias()`: Also clear `verify_pending = 0`
- `ListenerAliasTable_flush_aliases()`: Also clear `verify_pending = 0` on all entries

**Caller integration** (`can_main_statemachine.c` main loop):

```c
node_id_t probe_id = ListenerAliasTable_check_verifications(current_tick);

if (probe_id != 0) {

    // Build and queue targeted AME for probe_id
    // (use train node's alias as source)

}
```

**Timing**:
- Probe interval: 30 seconds (uint16_t countdown, 300 ticks at 100ms)
- Verification timeout: 3 seconds (uint8_t snapshot, 30 ticks)
- Round-robin: one listener per 30-second cycle

**Edge cases**:
- Listener reboots and sends AMD with new alias → `set_alias()` updates + clears pending
- AME buffer allocation fails → skip this cycle, try again next interval
- Multiple listeners gone → detected one per probe cycle (30s each)
- Global AME received → `flush_aliases()` zeros all aliases and pending flags; AMD replies re-populate

---

## Files Modified Summary

| # | File | Change | Status |
|---|------|--------|--------|
| 1 | `can_config.c` | `#include`, init call, wire DI function pointers | NOT STARTED |
| 2 | `can_rx_message_handler.h` | Add 3 listener DI pointers to interface struct | **DONE** |
| 3 | `can_rx_message_handler.c` | AMD expansion, AMR DI call, `_release_held_messages_for_listener()` | **DONE** (hold logic in `last_frame()` NOT YET) |
| 4 | `can_tx_statemachine.h` | Add `listener_find_by_node_id` to interface struct | **DONE** |
| 5 | `can_tx_statemachine.c` | DI alias resolution in `send_openlcb_message()` | **DONE** |
| 6 | `can_types.h` | Add `verify_ticks`, `verify_pending` to `listener_alias_entry_t` | NOT STARTED (Step 7) |
| 7 | `alias_mapping_listener.c/.h` | Add `check_verifications()`, update `set_alias`/`clear`/`flush` | NOT STARTED (Step 7) |
| 8 | `can_main_statemachine.c` | Caller integration for periodic verification | NOT STARTED (Step 7) |
| 9 | `openlcb_types.h` | `openlcb_msg_timer_t` union (replaces `timerticks`) | **DONE** |

## Files NOT Modified

| File | Why |
|------|-----|
| `openlcb_buffer_fifo.c/.h` | `check_and_invalidate_messages_by_source_alias()` already exists |
| `openlcb_buffer_list.c/.h` | Timeout mechanism already handles held-message cleanup |
| `openlcb_main_statemachine.c` | Pop-phase guard already discards `state.invalid` messages |
| `protocol_train_handler.c` | Protocol layer untouched — layer separation |
| `openlcb_config.c/.h` | No callback wiring needed |
| `CMakeLists.txt` | All source files already listed |

## New Tests Required

| File | Tests | Status |
|------|-------|--------|
| `can_rx_message_handler_Test.cxx` | Attach held when alias unresolved; attach released when AMD arrives; attach released immediately when alias already resolved; held message times out after 3s; detach NOT held; AMR clears listener table entry; AMD for non-listener is no-op | NOT STARTED |
| `can_tx_statemachine_Test.cxx` | TX resolves alias from listener table; TX drops message when alias unresolvable; `state.invalid` checked before alias resolution | NOT STARTED |
| `alias_mapping_listener_Test.cxx` | Verification: AME sent after countdown; AMD clears pending; stale detected after 3s timeout; flush clears pending | NOT STARTED |

## Resolved Decisions

1. **Lazy cleanup (not reference counting)**: Detach does NOT unregister listener
   from alias table. Stale entries are harmless. Table pre-sized for worst case.

2. **Stale-alias-without-AMR**: Same exposure the standard alias mapping table has
   for all nodes. Accepted as spec-level limitation. Mitigated by Step 7 (periodic
   verification).

3. **`openlcb_msg_timer_t` union**: Replaced dual-purpose packed `uint8_t timerticks`
   with clean union. `assembly_ticks` (8-bit) for multi-frame timeout.
   `datagram.tick_snapshot` (5-bit) + `datagram.retry_count` (3-bit) for datagram
   retry. Eliminates mask/shift operations.

4. **Dependency injection for listener table**: All calls go through nullable
   function pointers. NULL = feature not linked in, skip the call.

## Open Questions

1. **CAN RX layer parsing OpenLCB payload**: The hold-until-AMD pattern requires
   `can_rx_message_handler.c` to understand `MTI_TRAIN_PROTOCOL`,
   `TRAIN_LISTENER_CONFIG`, and `TRAIN_LISTENER_ATTACH`. The CAN layer already
   knows about MTIs for frame type selection. Alternatively, a `check_hold_message`
   callback could keep this knowledge out of the CAN layer.

2. **Global AME handling**: Should the AME handler call
   `ListenerAliasTable_flush_aliases()` when a global AME (empty payload) is
   received? This would ensure stale aliases are not served during the
   re-population window. Incoming AMD replies will re-populate via
   `ListenerAliasTable_set_alias()`.

## Pickup Checklist (Remaining Work)

When resuming this plan, implement in this order:

1. **Step 1**: Wire DI pointers in `can_config.c` and call `ListenerAliasTable_initialize()`
2. **Step 2**: Implement hold-until-AMD in `last_frame()` (`_check_hold_for_alias_resolution()` and `_queue_targeted_ame()`)
3. **Step 7**: Implement periodic verification (struct changes, `check_verifications()`, caller integration)
4. **Global AME**: Decide and implement `flush_aliases` call in AME handler
5. **Tests**: Write tests for Steps 2, 7, and the existing Step 3/4/6 implementations
6. **Style fixes**: Complete single-line style fixes in `src/openlcb/*.c` files (~60+ violations remain)

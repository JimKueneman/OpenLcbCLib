<!--
  ============================================================
  STATUS: IMPLEMENTED (wiring gap fixed 2026-03-15)

  The forwarding logic IS fully present in protocol_train_handler.c:
  _forward_to_next_listener(), _load_forwarded_command(), TRAIN_INSTRUCTION_P_BIT (0x80),
  P-bit masking in dispatch, and source-skip loop prevention (source_id check at
  listener iteration).

  The listener alias table module IS implemented in alias_mapping_listener.h/c
  (not listener_alias_table.h/c as the plan named it), with all required functions:
  ListenerAliasTable_initialize(), register(), unregister(), set_alias(),
  find_by_node_id(), flush_aliases(), clear_alias_by_alias().

  The alias resolution IS in can_tx_statemachine.c via the nullable DI pointer
  listener_find_by_node_id (lines 149-165).

  Two wiring steps that were previously missing in can_config.c have been fixed:

  1. ListenerAliasTable_initialize() is now called from CanConfig_initialize()
     after AliasMappings_initialize(), guarded by #ifdef OPENLCB_COMPILE_TRAIN.

  2. _build_tx_statemachine() now wires _tx_sm.listener_find_by_node_id to
     &ListenerAliasTable_find_by_node_id under #ifdef OPENLCB_COMPILE_TRAIN.
     The TX alias resolution branch is now active; consist-forwarded messages
     will resolve correctly at runtime.

  Note: hold-until-AMD (plan_listener_alias_table_integration Step 2) remains
  unimplemented — attach messages are forwarded to the protocol layer immediately
  without waiting for AMD. This is an accepted limitation: the first consist
  command to a newly-attached listener may be dropped if the alias is not yet
  resolved, but retries will succeed once the AMD reply populates the table.
  ============================================================
-->

# Plan: Item 3 -- Consist Forwarding (Train Control)

## 1. Summary

When a train node receives Set Speed, Set Function, or Emergency Stop commands, it updates
local state but does NOT forward adjusted commands to listener (consist member) nodes. The
listener infrastructure (attach/detach/query, flags storage in `train_listener_entry_t`) is
fully implemented, but the actual forwarding logic is missing. Consists will not function.

This plan covers the complete solution in two parts:

- **Part A — On-Demand Listener Alias Resolution:** A CAN-driver-only module that resolves
  remote node aliases on demand via AME/AMD exchanges, with a bounded table sized to the total
  listener capacity across all local train nodes. The protocol layer simply sets `dest_alias = 0`
  with a valid `dest_id`, and the CAN TX path transparently resolves it.

- **Part B — Forwarding Logic:** After local state updates in `_handle_set_speed()`,
  `_handle_set_function()`, and `_handle_emergency_stop()`, iterate the listener list and send
  forwarded commands with flag-based adjustments (direction reversal, function linking) and
  the P bit set (bit 7 of instruction byte) per spec.

**Standard:** TrainControlS Sections 4.3, 4.4, 6.5; TrainControlTN Sections 2.5-2.6;
CanFrameTransferS Sections 6.1-6.2

---

## Design Decisions & Rationale

### Why Not a Global Passive Cache?

The original design passively cached every AMD seen on the bus into a fixed-size table. This
approach fails because:

1. **Unbounded layout size.** A layout may have 1 or 10,000 train nodes. Any fixed global cache
   will either overflow or waste memory on small systems.
2. **Embedded targets.** RAM is scarce. Allocating 16 or 100 cache slots "just in case" is
   unacceptable when only 6 listeners per train are ever used.

### On-Demand AME Resolution

Instead, we resolve aliases **on demand** using AME (Alias Map Enquiry):

- **When:** At listener attach time. When the CAN RX handler pushes a Listener Attach message
  to the OpenLCB FIFO, it also fires a targeted AME for the listener's node_id. The AMD reply
  arrives shortly after and populates the table entry.
- **Where:** Entirely in the CAN driver layer. The CAN RX handler already has the pattern for
  sending CAN frames directly (`can_buffer_store_allocate_buffer()` → build frame →
  `CanBufferFifo_push()`), as used by the CID→RID handler.
- **Table size:** `USER_DEFINED_MAX_LISTENERS_PER_TRAIN × USER_DEFINED_TRAIN_NODE_COUNT`.
  With typical values of 6 × 4 = 24 entries. Bounded, compile-time-known, no waste.

### Layer Separation

The protocol layer never touches CAN aliases. The forwarding code sets `dest_alias = 0` and
`dest_id = listener_node_id`. The CAN TX path resolves the alias transparently from the table.
This keeps the CAN/protocol boundary clean.

### Locking

The alias table is written by the CAN RX interrupt (AMD arrival) and read by the CAN TX main
thread (alias resolution in `CanTxStatemachine_send_openlcb_message()`). The existing
`lock_shared_resources`/`unlock_shared_resources` in `CanMainStateMachine_run()` already
protects the CAN TX path reads. The CAN RX interrupt writes are safe because the lock disables
the CAN RX interrupt during the lock window. Same pattern as FIFO and buffer store access.

### Global AME Flush Requirement

Per CanFrameTransferS: "Upon receipt of an Alias Mapping Enquiry frame with no data content,
in either Inhibited or Permitted state, the receiving node must discard any information it is
retaining that maps aliases to/from Node IDs other than its own."

When a global AME (empty payload) is received, all entries in the listener alias table must be
flushed (aliases zeroed, node_ids preserved). The next forwarded message will fail the alias
lookup (`dest_alias == 0`), causing a retry. The global AME also triggers all nodes to send AMD
replies, which will re-populate the table entries.

### P Bit (Forwarded Message Marker)

Per TrainControlS Section 4.3: the P bit is **bit 7 of byte 0** (the instruction byte) of the
Train Control Command payload. It is NOT part of the MTI.

- P=0: Message sent directly from a throttle to a train node.
- P=1: Message forwarded from a train node to a listener node.

So forwarded instruction bytes become:
- `TRAIN_SET_SPEED_DIRECTION` (0x00) → `0x80`
- `TRAIN_SET_FUNCTION` (0x01) → `0x81`
- `TRAIN_EMERGENCY_STOP` (0x02) → `0x82`

Define: `#define TRAIN_P_BIT  0x80`

### Source-Skip Rule (Loop Prevention)

Per TrainControlS Section 6.5: "Even if the source node of the incoming message is on the
listener list, the message is not forwarded to the source node to avoid message loops."

The forwarding helpers must receive `source_node_id` (from the incoming message's `source_id`)
and skip any listener whose `node_id` matches it.

### Forward Both P=0 and P=1 Messages

Per TrainControlTN Section 2.6.5: "Incoming Train Control Operations with P=1 are also
forwarded; during forwarding they skip the link on which they came from."

The dispatch must call the forwarding helpers regardless of whether the incoming instruction
byte has P=0 or P=1. The P bit in the outgoing forwarded message is always set to 1. The
source-skip rule prevents loops in chained consists.

---

## Part A: On-Demand Listener Alias Resolution

### A1. Add Types to `can_types.h`

**File:** `src/drivers/canbus/can_types.h`

```c
typedef struct listener_alias_entry_struct {

    node_id_t node_id;  /**< Listener node ID (from protocol layer attach). 0 = unused slot. */
    uint16_t alias;     /**< Resolved CAN alias (0 = not yet resolved). */

} listener_alias_entry_t;
```

Table depth is computed, not user-defined:
```c
#define LISTENER_ALIAS_TABLE_DEPTH \
    (USER_DEFINED_MAX_LISTENERS_PER_TRAIN * USER_DEFINED_TRAIN_NODE_COUNT)
```

### A2. New Module `listener_alias_table.h` + `.c`

**Files:** `src/drivers/canbus/listener_alias_table.h` and `listener_alias_table.c`

| Function | Purpose |
|----------|---------|
| `ListenerAliasTable_initialize()` | Zero all entries |
| `ListenerAliasTable_register(node_id)` | Add node_id with alias=0 (first-fit empty slot). Returns pointer or NULL if full. |
| `ListenerAliasTable_unregister(node_id)` | Remove entry (listener detached) |
| `ListenerAliasTable_set_alias(node_id, alias)` | Called when AMD arrives — stores alias for matching node_id |
| `ListenerAliasTable_find_by_node_id(node_id)` | **Primary TX-path query** — returns entry (caller checks alias != 0) |
| `ListenerAliasTable_flush_aliases()` | Zero all alias fields but preserve node_ids (for global AME) |
| `ListenerAliasTable_clear_alias_by_alias(alias)` | Zero alias field for entry matching this alias (for AMR) |

Implementation: static array of `listener_alias_entry_t[LISTENER_ALIAS_TABLE_DEPTH]`.
Linear scan. Validates node_id non-zero, alias range 0x001-0xFFF.

### A3. Send AME at Listener Attach Time

**File:** `src/drivers/canbus/can_rx_message_handler.c`

The CAN RX handler already knows when a listener attach message arrives — it's an addressed
Train Protocol message with the attach sub-command in the payload. However, the CAN RX handler
currently doesn't inspect OpenLCB payload semantics (it just reassembles and pushes to the FIFO).

**Simplest correct approach:**

Add to `protocol_train_handler.h` interface:
```c
    /** @brief OPTIONAL. Called when a listener is attached. Triggers alias resolution. */
    void (*on_listener_alias_needed)(node_id_t listener_node_id);
```

Wire in `openlcb_config.c` to a function that:
1. Calls `ListenerAliasTable_register(listener_node_id)`.
2. Calls a CAN-layer function to send the targeted AME.

The CAN-layer function `ListenerAliasTable_send_ame(node_id)` uses the existing buffer
allocation + FIFO push pattern.

Similarly, add:
```c
    /** @brief OPTIONAL. Called when a listener is detached. Frees alias table entry. */
    void (*on_listener_alias_released)(node_id_t listener_node_id);
```

### A4. Populate Table from AMD Replies

**File:** `src/drivers/canbus/can_rx_message_handler.c`

In `CanRxMessageHandler_amd_frame()`, after `_check_for_duplicate_alias()`:
```c
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    node_id_t node_id = CanUtilities_extract_can_payload_as_node_id(can_msg);
    ListenerAliasTable_set_alias(node_id, source_alias);
```

This is a no-op if the node_id isn't in the table (not one of our listeners). Only entries
we care about get populated.

### A5. Handle AMR — Invalidate Single Entry

**File:** `src/drivers/canbus/can_rx_message_handler.c`

In `CanRxMessageHandler_amr_frame()`, after `_check_for_duplicate_alias()`:
```c
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    ListenerAliasTable_clear_alias_by_alias(source_alias);
```

When that node re-claims an alias and sends AMD, `A4` repopulates the entry.

### A6. Handle Global AME — Flush All Aliases

**File:** `src/drivers/canbus/can_rx_message_handler.c`

In `CanRxMessageHandler_ame_frame()`, at the top of the global query path (the
`can_msg->payload_count == 0` branch, after existing AMD response loop):

```c
    ListenerAliasTable_flush_aliases();
```

This zeros all alias fields but preserves node_ids. The AMD replies triggered by the global
AME will re-populate via `A4`.

### A7. Resolve dest_alias in CAN TX Path

**File:** `src/drivers/canbus/can_tx_statemachine.c`

At the top of `CanTxStatemachine_send_openlcb_message()`, before any frame construction:

```c
    if (openlcb_msg->dest_alias == 0 && openlcb_msg->dest_id != 0) {

        listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(openlcb_msg->dest_id);

        if (entry && entry->alias != 0) {

            openlcb_msg->dest_alias = entry->alias;

        } else {

            return false;  // Cannot resolve — caller retries on next loop

        }

    }
```

This runs in the CAN main statemachine thread, which already uses `lock_shared_resources`
around FIFO/buffer operations. The alias table read is protected by the same lock window.

Cache miss returns `false` (same as "TX buffer busy"). The message stays in the FIFO and
retries. The AMD reply will arrive and populate the table before the next retry succeeds.

### A8. Wire Initialization

**File:** `src/drivers/canbus/can_config.c`

In `CanConfig_initialize()`, after `AliasMappings_initialize()`:
```c
    ListenerAliasTable_initialize();
```

### A9. Tests for Listener Alias Table

**File:** `src/drivers/canbus/listener_alias_table_Test.cxx`

- Basic: initialize, all entries cleared
- Register: returns pointer, fills to capacity, returns NULL on overflow
- Unregister: by node_id, clears entry, ignores nonexistent
- Set alias: sets alias for registered node_id, ignores unregistered
- Find by node_id: returns entry, returns NULL for unregistered
- Flush aliases: all aliases zeroed, node_ids preserved
- Clear alias by alias: matching entry alias zeroed, others untouched
- Boundary: rejects node_id 0, alias 0 in set_alias, alias > 0xFFF

**File:** `src/drivers/canbus/can_rx_message_handler_Test.cxx` (add to existing)

- AMD frame populates listener alias table (register node_id first, then AMD → alias stored)
- AMD frame for non-listener node_id is ignored (table unchanged)
- AMR frame clears alias for matching entry
- Global AME flushes all aliases in table

---

## Part B: Forwarding Logic

### B1. Add P bit define and verify listener flag constants

**File:** `src/openlcb/openlcb_defines.h`

Add P bit define:
```c
    /** @brief P bit (bit 7 of instruction byte): set on forwarded messages. */
#define TRAIN_P_BIT  0x80
```

Existing listener flag constants are correct and match the spec:
```c
#define TRAIN_LISTENER_FLAG_REVERSE       0x02   // Reverse direction in consist
#define TRAIN_LISTENER_FLAG_LINK_F0       0x04   // Link F0 (headlight)
#define TRAIN_LISTENER_FLAG_LINK_FN       0x08   // Link Fn functions
#define TRAIN_LISTENER_FLAG_HIDE          0x80   // Hide from enumeration
```

### B2. Add send callback and alias callbacks to interface

**File:** `src/openlcb/protocol_train_handler.h`

```c
    /** @brief Send a train control command to a listener node. REQUIRED for consist forwarding. */
    void (*send_train_command)(openlcb_node_t *source_node, node_id_t dest_node_id,
                               uint8_t *payload, uint16_t payload_count);

    /** @brief OPTIONAL. Called when a listener is attached. Triggers alias resolution at CAN layer. */
    void (*on_listener_alias_needed)(node_id_t listener_node_id);

    /** @brief OPTIONAL. Called when a listener is detached. Frees alias table entry at CAN layer. */
    void (*on_listener_alias_released)(node_id_t listener_node_id);
```

The `send_train_command` callback builds an `MTI_TRAIN_PROTOCOL` message with `dest_alias = 0`
and `dest_id = dest_node_id`. The CAN TX path resolves the alias transparently via the listener
alias table.

### B3. Call alias callbacks from attach/detach

**File:** `src/openlcb/protocol_train_handler.c`

In `_handle_listener_config()` TRAIN_LISTENER_ATTACH case, after successful attach
(result == 0):
```c
    if (_interface->on_listener_alias_needed) {

        _interface->on_listener_alias_needed(listener_id);

    }
```

In `_handle_listener_config()` TRAIN_LISTENER_DETACH case, after successful detach
(result == 0):
```c
    if (_interface->on_listener_alias_released) {

        _interface->on_listener_alias_released(listener_id);

    }
```

### B4. Speed forwarding helper

**File:** `src/openlcb/protocol_train_handler.c`

The helper takes `source_node_id` to skip the source of the incoming message (loop prevention).

```c
static void _forward_speed_to_listeners(openlcb_node_t *node, uint16_t speed,
                                         node_id_t source_node_id) {

    train_state_t *state = node->train_state;

    if (!state || !_interface || !_interface->send_train_command) { return; }

    for (uint8_t i = 0; i < state->listener_count; i++) {

        train_listener_entry_t *listener = &state->listeners[i];

        if (listener->node_id == 0) { continue; }

        if (listener->node_id == source_node_id) { continue; }

        uint16_t adjusted_speed = speed;

        if (listener->flags & TRAIN_LISTENER_FLAG_REVERSE) {

            adjusted_speed ^= 0x8000;  // Toggle direction bit in float16

        }

        uint8_t payload[3];
        payload[0] = TRAIN_SET_SPEED_DIRECTION | TRAIN_P_BIT;
        payload[1] = (adjusted_speed >> 8) & 0xFF;
        payload[2] = adjusted_speed & 0xFF;

        _interface->send_train_command(node, listener->node_id, payload, 3);

    }

}
```

### B5. Function forwarding helper

```c
static void _forward_function_to_listeners(openlcb_node_t *node,
                                            uint32_t fn_address, uint16_t fn_value,
                                            node_id_t source_node_id) {

    train_state_t *state = node->train_state;

    if (!state || !_interface || !_interface->send_train_command) { return; }

    for (uint8_t i = 0; i < state->listener_count; i++) {

        train_listener_entry_t *listener = &state->listeners[i];

        if (listener->node_id == 0) { continue; }

        if (listener->node_id == source_node_id) { continue; }

        if (fn_address == 0 && !(listener->flags & TRAIN_LISTENER_FLAG_LINK_F0)) { continue; }

        if (fn_address > 0 && !(listener->flags & TRAIN_LISTENER_FLAG_LINK_FN)) { continue; }

        uint8_t payload[6];
        payload[0] = TRAIN_SET_FUNCTION | TRAIN_P_BIT;
        payload[1] = (fn_address >> 16) & 0xFF;
        payload[2] = (fn_address >> 8) & 0xFF;
        payload[3] = fn_address & 0xFF;
        payload[4] = (fn_value >> 8) & 0xFF;
        payload[5] = fn_value & 0xFF;

        _interface->send_train_command(node, listener->node_id, payload, 6);

    }

}
```

### B6. Emergency Stop forwarding helper

Per TrainControlTN Section 2.6.5: "Set Speed and Emergency Stop message needs to be acted upon
(and forwarded) by the train, independently of whether P=0 or P=1."

```c
static void _forward_emergency_stop_to_listeners(openlcb_node_t *node,
                                                   node_id_t source_node_id) {

    train_state_t *state = node->train_state;

    if (!state || !_interface || !_interface->send_train_command) { return; }

    for (uint8_t i = 0; i < state->listener_count; i++) {

        train_listener_entry_t *listener = &state->listeners[i];

        if (listener->node_id == 0) { continue; }

        if (listener->node_id == source_node_id) { continue; }

        uint8_t payload[1];
        payload[0] = TRAIN_EMERGENCY_STOP | TRAIN_P_BIT;

        _interface->send_train_command(node, listener->node_id, payload, 1);

    }

}
```

### B7. Dispatch: strip P bit, call handlers, forward

The existing dispatch in `ProtocolTrainHandler_handle_train_command()` uses
`instruction = byte 0` directly as the switch key. Since the P bit is bit 7, the instruction
values 0x80-0x82 won't match the existing cases (0x00-0x02).

**Fix:** Mask out the P bit before switching, and pass the source_id to forwarding helpers.

```c
void ProtocolTrainHandler_handle_train_command(openlcb_statemachine_info_t *statemachine_info) {

    if (!statemachine_info) { return; }

    if (!statemachine_info->incoming_msg_info.msg_ptr) { return; }

    uint8_t raw_instruction = OpenLcbUtilities_extract_byte_from_openlcb_payload(
            statemachine_info->incoming_msg_info.msg_ptr, 0);
    uint8_t instruction = raw_instruction & ~TRAIN_P_BIT;

    node_id_t source_node_id = statemachine_info->incoming_msg_info.msg_ptr->source_id;

    switch (instruction) {

        case TRAIN_SET_SPEED_DIRECTION:

            _handle_set_speed(statemachine_info);
            _forward_speed_to_listeners(
                    statemachine_info->openlcb_node,
                    OpenLcbUtilities_extract_word_from_openlcb_payload(
                            statemachine_info->incoming_msg_info.msg_ptr, 1),
                    source_node_id);
            break;

        case TRAIN_SET_FUNCTION:

            _handle_set_function(statemachine_info);
            _forward_function_to_listeners(
                    statemachine_info->openlcb_node,
                    _extract_fn_address(statemachine_info->incoming_msg_info.msg_ptr, 1),
                    OpenLcbUtilities_extract_word_from_openlcb_payload(
                            statemachine_info->incoming_msg_info.msg_ptr, 4),
                    source_node_id);
            break;

        case TRAIN_EMERGENCY_STOP:

            _handle_emergency_stop(statemachine_info);
            _forward_emergency_stop_to_listeners(
                    statemachine_info->openlcb_node,
                    source_node_id);
            break;

        /* remaining cases unchanged */
        ...
    }

}
```

Note: The forwarding runs for both P=0 (from throttle) and P=1 (from another consist member)
incoming messages. The source-skip rule prevents loops: the source node_id is never in its own
listener list, and chained consists skip the link they came from.

### B8. Wire callbacks in `openlcb_config.c`

In the `#ifdef OPENLCB_COMPILE_TRAIN` section:

- Wire `send_train_command` to an implementation that allocates a BASIC buffer, sets
  `mti = MTI_TRAIN_PROTOCOL`, `dest_alias = 0`, `dest_id = dest_node_id`, copies payload,
  and pushes to the OpenLCB FIFO.

- Wire `on_listener_alias_needed` to a function that calls
  `ListenerAliasTable_register(node_id)` and then `ListenerAliasTable_send_ame(node_id)`.

- Wire `on_listener_alias_released` to a function that calls
  `ListenerAliasTable_unregister(node_id)`.

### B9. Tests for forwarding

**File:** `src/openlcb/protocol_train_handler_Test.cxx`

- Speed forwarded to one listener (no flags) — verify P bit set (0x80)
- Speed forwarded with direction reversal (TRAIN_LISTENER_FLAG_REVERSE)
- Function F0 forwarded when TRAIN_LISTENER_FLAG_LINK_F0 set — verify P bit set (0x81)
- Function F0 NOT forwarded when TRAIN_LISTENER_FLAG_LINK_F0 clear
- Function Fn forwarded when TRAIN_LISTENER_FLAG_LINK_FN set
- Emergency stop forwarded to listeners — verify P bit set (0x82)
- No forwarding with empty listener list
- Source node skipped: listener matching source_id not forwarded to (loop prevention)
- Forwarding works for P=1 incoming messages (chained consists)
- Alias callbacks invoked on attach/detach
- Alias unknown — message retried (stays in FIFO, returns false from TX path)

---

## File Change Summary

| File | Change |
|------|--------|
| `src/drivers/canbus/can_types.h` | Add `listener_alias_entry_t`, `LISTENER_ALIAS_TABLE_DEPTH` |
| `src/drivers/canbus/listener_alias_table.h` | **New** — public API |
| `src/drivers/canbus/listener_alias_table.c` | **New** — implementation + AME send helper |
| `src/drivers/canbus/listener_alias_table_Test.cxx` | **New** — test suite |
| `src/drivers/canbus/can_rx_message_handler.c` | Add table population in `amd_frame()`, alias clear in `amr_frame()`, flush in `ame_frame()` global path |
| `src/drivers/canbus/can_rx_message_handler_Test.cxx` | Add AMD/AMR/global-AME alias table tests |
| `src/drivers/canbus/can_tx_statemachine.c` | Add dest_alias resolution at entry point |
| `src/drivers/canbus/can_config.c` | Add `ListenerAliasTable_initialize()` call |
| `src/openlcb/openlcb_defines.h` | Add `TRAIN_P_BIT` define |
| `src/openlcb/protocol_train_handler.h` | Add `send_train_command`, `on_listener_alias_needed`, `on_listener_alias_released` to interface |
| `src/openlcb/protocol_train_handler.c` | Add forwarding helpers (speed, function, estop), P bit masking in dispatch, source-skip logic, call alias callbacks from listener attach/detach |
| `src/openlcb/openlcb_config.c` | Wire `send_train_command`, `on_listener_alias_needed`, `on_listener_alias_released` |
| `src/openlcb/protocol_train_handler_Test.cxx` | Add forwarding + P bit + source-skip + alias callback tests |
| `src/drivers/canbus/CMakeLists.txt` | Add listener_alias_table source + test |

---

## Message Flow Diagrams

### Listener Attach + Alias Resolution
```
Controller                  Our Train Node (CAN RX)         Listener Node
    |                              |                              |
    |-- Listener Attach (node_id)->|                              |
    |                              |-- register(node_id) in table |
    |                              |-- AME (targeted, node_id) -->|
    |                              |                              |
    |                              |<--- AMD (alias, node_id) ----|
    |                              |-- set_alias(node_id, alias)  |
    |                              |   (table entry complete)     |
```

### Speed Forwarding (P bit + source-skip)
```
Controller                  Our Train Node                  Listener Node
    |                              |                              |
    |-- Set Speed (P=0, speed) --->|                              |
    |                              |-- update local state         |
    |                              |-- callback to application    |
    |                              |-- _forward_speed_to_listeners|
    |                              |   (skip source = Controller) |
    |                              |   builds msg: P=1,           |
    |                              |   dest_alias=0,              |
    |                              |   dest_id=listener_node_id   |
    |                              |                              |
    |            CAN TX path resolves alias from table            |
    |                              |                              |
    |                              |-- Set Speed (P=1, adj) ----->|
```

### Chained Consist Forwarding (P=1 → P=1)
```
Throttle       Train A              Train B              Train C
    |              |                    |                    |
    |-- Speed P=0->|                    |                    |
    |              |-- update state     |                    |
    |              |-- Speed P=1 ------>|                    |
    |              |                    |-- update state     |
    |              |                    |-- Speed P=1 ------>|
    |              |                    |   (skip source=A)  |
    |              |                    |                    |
    Note: B forwards to C but skips A (source of the incoming P=1 msg)
```

### Global AME Flush + Recovery
```
Some Node                   Our Train Node (CAN RX)         Listener Node
    |                              |                              |
    |-- Global AME (empty) ----->>|                              |
    |                              |-- flush_aliases() (all=0)   |
    |                              |-- send own AMD(s) as normal  |
    |                              |                              |
    |                              |<--- AMD (alias, node_id) ----|
    |                              |-- set_alias() re-populates   |
    |                              |                              |
    |   (next forwarded msg works normally)                       |
```

---

## Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| Alias stale after reallocation | LOW | AMR clears entry; next AMD re-populates |
| Table full | NONE | Table sized exactly to total listener capacity — cannot overflow |
| Message loop (no source-skip) | **FIXED** | Forwarding helpers skip `source_node_id` |
| P bit not set on forwarded msgs | **FIXED** | Instruction byte ORed with `TRAIN_P_BIT` (0x80) |
| Emergency stop not forwarded | **FIXED** | New `_forward_emergency_stop_to_listeners()` |
| P=1 msgs not re-forwarded | **FIXED** | Dispatch masks P bit; forwards regardless of P value |
| Alias not yet resolved on first forward | LOW | TX returns false, message retries. AMD arrives within ms of AME. |
| Global AME flushes all aliases | LOW | AMD replies re-populate automatically. Brief retry window. |
| AME buffer allocation fails | LOW | Alias stays 0, forwarded messages retry. Next AMD from any source populates. |

---

## Build & Verify

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

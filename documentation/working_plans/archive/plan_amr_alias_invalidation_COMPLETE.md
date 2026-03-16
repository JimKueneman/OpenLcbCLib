<!--
  ============================================================
  STATUS: IMPLEMENTED
  The `invalid` flag exists in `openlcb_msg_state_t`, FIFO scrub function `OpenLcbBufferFifo_check_and_invalidate_messages_by_source_alias()` is wired, and both the pop-phase guard and TX guard are in place.
  ============================================================
-->

# AMR Alias Invalidation — General CAN-Layer Defense

## Problem

When a remote node releases its 12-bit CAN alias via an AMR (Alias Map Reset)
frame, the OpenLCB CAN Frame Transfer Standard (Section 3.5.5, adopted July
2024) requires other nodes to stop using that alias within **100 ms**.  Messages
already in the pipeline that reference the stale alias must be invalidated to
meet this deadline and avoid delivering frames to a node that may no longer own
that alias.

### Why 100 ms is very tight

On a 125 kbps CAN bus with extended frames, one frame takes ~1.4-1.8 ms.  A
low-priority node losing arbitration repeatedly could see 10-50 ms per frame on
a loaded bus.  A full PCER-with-payload (up to ~33 frames, all required
contiguous by spec) could take 60-500+ ms depending on contention — easily
exceeding the 100 ms window.  During that blocking send, the main loop is
stalled: no FIFO pop, no AMR processing, nothing.

This is a pre-existing architectural exposure, not specific to the listener
alias table.  The defense below is a general-purpose mechanism that protects
all message types.

## Design

Four pipeline stages, four defense mechanisms:

```
AMR arrives for alias X
  │
  ├─ 1. BufferList scrub (immediate)
  │     _check_and_release_messages_by_source_alias(X)
  │     Scan OpenLcbBufferList for all messages with
  │     source_alias == X.  Free the buffer, NULL the slot.
  │     Rationale: sender is gone — partial assemblies will never
  │     complete, and completed messages should not generate replies
  │     to a stale alias.  Freeing immediately reclaims scarce
  │     buffer slots.
  │
  ├─ 2. FIFO scrub (lazy invalidation)
  │     OpenLcbBufferFifo_check_and_invalidate_messages_by_source_alias(X)
  │     Scan OpenLcbBufferFifo for incoming messages with
  │     source_alias == X.  Set state.invalid = true.
  │     Rationale: these are completed incoming messages from the
  │     released node.  Processing them could generate replies to a
  │     stale alias that may now belong to a different node.  When
  │     they reach the pop phase, state.invalid causes them to be
  │     dropped.
  │
  └─ 3. Existing: _check_for_duplicate_alias()
        (unchanged — handles OUR alias conflicts)


FIFO pop-phase guard — PRIMARY defense (transport-agnostic)
  │
  └─ In OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message():
       After popping, if state.invalid is set, free the message
       immediately.  No protocol dispatch, no node enumeration.
       Rationale: this is the transport-agnostic choke point.  Every
       message passes through here regardless of transport.  Catches
       invalidated messages before any protocol handler sees them.
       Future non-CAN transports can also set state.invalid and benefit
       from this guard without transport-specific code.


CAN TX path guard — belt-and-suspenders (CAN-specific)
  │
  └─ Top of CanTxStatemachine_send_openlcb_message():
       if (state.invalid) return true;   // message consumed, discarded
       Rationale: secondary defense.  Catches any message that was
       invalidated after popping but before transmission (unlikely but
       possible if future code paths set the flag late).  Costs one
       branch per TX — negligible.


Mid-flight messages (no action)
  │
  └─ If a multi-frame message is already in the blocking while loop
     inside send_openlcb_message(), let it finish.  Interrupting mid-
     sequence would leave the receiver with partial frames and violate
     the contiguous-frame requirement for PCER-with-payload.  The 100 ms
     spec window accounts for in-flight messages.
```

### Why `state.invalid` instead of zeroing `source_alias`

An earlier design zeroed `source_alias` as a sentinel to signal invalidity.
This works (source_alias == 0 is always illegal) but has drawbacks:

- **Overloads a field** with two meanings: "actual source alias" and "this
  message is invalid."  Anyone reading the FIFO scrub sees
  `msg->source_alias = 0` and must understand the convention.
- **Obscures intent**: debugging shows a zeroed alias, not an explicit
  "invalidated by AMR" flag.
- **CAN-specific**: checking `source_alias` in the OpenLCB layer would leak
  transport knowledge.

`state.invalid` is self-documenting, transport-agnostic, and can be checked at
any pipeline stage without layer violations.  It costs one bit in a struct
that already has 6 spare bits.

### CAN FIFO (`CanBufferFifo`) — not affected

The CAN FIFO holds raw `can_msg_t` control frames (RID, AMD, AMR, AME).  These
are all broadcast with our source alias — none reference a remote alias as a
destination.  No scrub needed.  `CanTxStatemachine_send_can_message()` is also
unaffected — it sends pre-built control frames that don't carry the
`openlcb_msg_state_t` structure.

---

## Step 0: Add `invalid` flag — `openlcb_types.h`

**What**: Add a single-bit `invalid` flag to `openlcb_msg_state_t`.

**Current code** (line 438):

```c
typedef struct {

    bool allocated : 1;     /**< Buffer is in use */
    bool inprocess : 1;     /**< Multi-frame message being assembled */

} openlcb_msg_state_t;
```

**Modified**:

```c
typedef struct {

    bool allocated : 1;     /**< Buffer is in use */
    bool inprocess : 1;     /**< Multi-frame message being assembled */
    bool invalid : 1;       /**< Message invalidated (e.g. AMR) — TX shall discard */

} openlcb_msg_state_t;
```

**Notes**:
- No size change — the struct already occupies 1 byte with 6 bits spare.
- The flag must be cleared by `OpenLcbUtilities_clear_openlcb_message()`.
  Verify this function zeroes the entire `state` struct (it does:
  `openlcb_msg->state.allocated`, `.inprocess` are set individually, but the
  struct is zero-initialized via the allocation path in
  `OpenLcbBufferStore_allocate_buffer()`).

---

## Step 1: BufferList scrub — `can_rx_message_handler.c`

**What**: When AMR arrives, scan the `OpenLcbBufferList` for all messages
from the released alias.  Free them immediately — partial assemblies will
never complete, and completed messages should not generate replies to a
stale alias.

**New static helper**:

```c
static void _check_and_release_messages_by_source_alias(uint16_t alias) {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        openlcb_msg_t *msg = OpenLcbBufferList_index_of(i);

        if (!msg) { continue; }
        if (msg->source_alias != alias) { continue; }

        OpenLcbBufferList_release(msg);
        OpenLcbBufferStore_free_buffer(msg);

    }

}
```

**Notes**:
- `LEN_MESSAGE_BUFFER` is typically 4-8 entries.  Negligible scan cost.
- Frees all messages from the released alias regardless of `inprocess` state.
- Must include `openlcb_buffer_store.h` for `OpenLcbBufferStore_free_buffer()`.

---

## Step 2: FIFO scrub — `openlcb_buffer_fifo.c/.h`

**What**: New public function that walks the circular buffer and sets
`state.invalid` on incoming messages from a released alias.

**New function in `openlcb_buffer_fifo.h`**:

```c
extern void OpenLcbBufferFifo_check_and_invalidate_messages_by_source_alias(uint16_t alias);
```

**Implementation in `openlcb_buffer_fifo.c`**:

```c
void OpenLcbBufferFifo_check_and_invalidate_messages_by_source_alias(uint16_t alias) {

    if (alias == 0) {

        return;

    }

    uint8_t index = _openlcb_msg_buffer_fifo.tail;

    while (index != _openlcb_msg_buffer_fifo.head) {

        openlcb_msg_t *msg = _openlcb_msg_buffer_fifo.list[index];

        if (msg && msg->source_alias == alias) {

            msg->state.invalid = true;

        }

        index = index + 1;
        if (index >= LEN_MESSAGE_FIFO_BUFFER) {

            index = 0;

        }

    }

}
```

**Notes**:
- Walks from tail to head (all occupied slots).
- Matches on `source_alias` (incoming messages FROM the released node).
  The FIFO holds incoming messages — `source_alias` identifies the sender.
  Processing these could generate replies to a stale alias that may now
  belong to a different node.
- Sets `state.invalid = true`.  Does NOT remove the message from the FIFO —
  the circular buffer head/tail pointers stay untouched.
- FIFO is typically 2-3 messages deep.  AMR is extremely rare.

---

## Step 3: AMR handler expansion — `can_rx_message_handler.c`

**What**: Add BufferList and FIFO scrubs to the existing AMR handler.

**Current code** (line 488):

```c
void CanRxMessageHandler_amr_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

}
```

**Modified**:

```c
void CanRxMessageHandler_amr_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

    uint16_t alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);

    _check_and_release_messages_by_source_alias(alias);

    OpenLcbBufferFifo_check_and_invalidate_messages_by_source_alias(alias);

}
```

**New includes needed**:

```c
#include "../../openlcb/openlcb_buffer_store.h"   // already present
#include "../../openlcb/openlcb_buffer_fifo.h"     // already present
#include "../../openlcb/openlcb_buffer_list.h"     // already present
```

All three includes are already present in `can_rx_message_handler.c`.  No new
dependencies.

---

## Step 4: Pop-phase guard — `openlcb_main_statemachine.c`

**What**: After popping a message from the FIFO, check `state.invalid` and
discard before any protocol dispatch.  This is the **primary defense** — it
is transport-agnostic and catches invalidated messages at the earliest
protocol-layer point.

**Current code** (line 864):

```c
bool OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message(void) {

    if (!_statemachine_info.incoming_msg_info.msg_ptr) {

        _interface->lock_shared_resources();
        _statemachine_info.incoming_msg_info.msg_ptr = OpenLcbBufferFifo_pop();
        _interface->unlock_shared_resources();

        _statemachine_info.current_tick = _interface->get_current_tick();

        return (!_statemachine_info.incoming_msg_info.msg_ptr);

    }

    return false;

}
```

**Modified**:

```c
bool OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message(void) {

    if (!_statemachine_info.incoming_msg_info.msg_ptr) {

        _interface->lock_shared_resources();
        _statemachine_info.incoming_msg_info.msg_ptr = OpenLcbBufferFifo_pop();
        _interface->unlock_shared_resources();

        if (_statemachine_info.incoming_msg_info.msg_ptr &&
                _statemachine_info.incoming_msg_info.msg_ptr->state.invalid) {

            _free_incoming_message(&_statemachine_info);

            return true;

        }

        _statemachine_info.current_tick = _interface->get_current_tick();

        return (!_statemachine_info.incoming_msg_info.msg_ptr);

    }

    return false;

}
```

**Notes**:
- Check goes after `unlock_shared_resources()` — the lock only protects the
  FIFO pop, not the message content.
- `_free_incoming_message()` frees the buffer and NULLs `msg_ptr`.
- Returns `true` (idle / nothing to process) so the caller moves on.
  The next main-loop iteration will try to pop the next message.
- `_free_incoming_message()` is a file-scoped static — no new dependencies.
- Future transports (e.g. TCP/IP) that also push to the OpenLCB FIFO can set
  `state.invalid` for their own invalidation reasons and benefit from this
  guard without any transport-specific code in the protocol layer.

---

## Step 5: TX guard (belt-and-suspenders) — `can_tx_statemachine.c`

**What**: Add a `state.invalid` check at the top of
`CanTxStatemachine_send_openlcb_message()`.  This is a secondary defense
in the CAN transport layer — catches any edge case where a message is
invalidated after popping but before transmission.

**Current code** (line 129):

```c
bool CanTxStatemachine_send_openlcb_message(openlcb_msg_t* openlcb_msg) {

    can_msg_t worker_can_msg;
    uint16_t payload_index = 0;

    if (!_interface->is_tx_buffer_empty()) {

        return false;

    }
    // ...
```

**Modified**:

```c
bool CanTxStatemachine_send_openlcb_message(openlcb_msg_t* openlcb_msg) {

    if (openlcb_msg->state.invalid) {

        return true;  // invalidated message — discard, signal consumed

    }

    can_msg_t worker_can_msg;
    uint16_t payload_index = 0;

    if (!_interface->is_tx_buffer_empty()) {

        return false;

    }
    // ...
```

**Return value**: `true` (message consumed) rather than `false` (retry later).
An invalidated message is permanently dead — retrying is pointless.  Returning
`true` causes the caller (`OpenLcbMainStatemachine_handle_outgoing_openlcb_message`)
to clear the `outgoing_msg_info.valid` flag and move on.

---

## Step 6: Clear flag on allocation — verify `openlcb_buffer_store.c`

**What**: Confirm that `state.invalid` is cleared when a buffer is allocated.

`OpenLcbBufferStore_allocate_buffer()` already zeroes the entire message struct
before returning it.  Since `invalid` is a bitfield in `openlcb_msg_state_t`,
it will be 0 (false) after allocation.  No code change needed — just verify
in testing.

Also confirm `OpenLcbUtilities_clear_openlcb_message()` clears the state
properly.  Currently it sets `.allocated` and `.inprocess` individually but
does not touch other bits.  Add `openlcb_msg->state.invalid = false;` to
this function for completeness.

---

## Files Modified Summary

| # | File | Change |
|---|------|--------|
| 1 | `src/openlcb/openlcb_types.h` | Add `bool invalid : 1` to `openlcb_msg_state_t` |
| 2 | `src/openlcb/openlcb_utilities.c` | Clear `state.invalid` in `clear_openlcb_message()` |
| 3 | `src/drivers/canbus/can_rx_message_handler.c` | New `_check_and_release_messages_by_source_alias()` helper; expanded `amr_frame()` handler |
| 4 | `src/openlcb/openlcb_buffer_fifo.c` | New `OpenLcbBufferFifo_check_and_invalidate_messages_by_source_alias()` function |
| 5 | `src/openlcb/openlcb_buffer_fifo.h` | Declaration of new function |
| 6 | `src/openlcb/openlcb_main_statemachine.c` | Pop-phase `state.invalid` guard — **primary defense** |
| 7 | `src/drivers/canbus/can_tx_statemachine.c` | `state.invalid` guard at top of `send_openlcb_message()` — belt-and-suspenders |

## Files NOT Modified

| File | Why |
|------|-----|
| `openlcb_buffer_list.c/.h` | Existing API (`index_of`, `release`) is sufficient |
| `openlcb_buffer_store.c` | Allocation already zeroes state — verify only |
| `can_config.c/.h` | No initialization or wiring changes |
| `can_buffer_fifo.c/.h` | CAN FIFO holds control frames only — no stale aliases |
| `can_tx_statemachine.h` | No API change |

## New Tests Required

| File | Tests |
|------|-------|
| `openlcb_buffer_fifo_Test.cxx` | `check_and_invalidate_messages_by_source_alias()`: sets `state.invalid` on matching `source_alias`; does not affect non-matching entries; does not affect empty FIFO; handles wraparound in circular buffer |
| `can_rx_message_handler_Test.cxx` | AMR scrubs BufferList: all messages from released alias are freed (regardless of inprocess state); non-matching messages untouched.  AMR scrubs FIFO: marks incoming messages from released alias as invalid |
| `openlcb_main_statemachine_Test.cxx` | Pop-phase guard: invalid message freed immediately after pop, not dispatched to nodes; non-invalid messages dispatched normally |
| `can_tx_statemachine_Test.cxx` | `state.invalid == true`: returns true (consumed); message not transmitted.  `state.invalid == false`: normal transmission unaffected |

## Interaction with Listener Alias Table Plan

This plan is **independent** of the listener alias table integration
(`plan_listener_alias_table_integration.md`).  The AMR defense here is a
general-purpose mechanism that protects all message types.  When the listener
plan is also implemented, the AMR handler will additionally call
`ListenerAliasTable_clear_alias_by_alias(alias)` — but that is additive and
does not conflict with the scrubs defined here.

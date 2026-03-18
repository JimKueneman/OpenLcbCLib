# Plan: Unified Sibling Dispatch (Zero-FIFO, Sequential)

Date: 2026-03-17

## Overview

Replace the current FIFO-based loopback mechanism with OpenMRN-style sequential
dispatch. Instead of copying outgoing messages to a FIFO and processing them
later, dispatch each outgoing message directly to sibling virtual nodes — one
node at a time, one `_run()` call per step — before producing the next outgoing
message.

**Key insight:** Stop simulating a bus between virtual nodes. The statemachine
already iterates nodes one at a time. After sending an outgoing message to the
wire, show that same message to each sibling before moving on. Zero buffer
allocation. Zero FIFO. One message in flight at a time — same as OpenMRN.

---

## Phased Delivery

### Phase 1 — Main Statemachine (this document)
Implement and prove the sibling dispatch mechanism in `openlcb_main_statemachine.c`.
All sections below describe Phase 1 changes only.

### Phase 2 — Login Statemachine (see end of document)
Apply the same pattern to `openlcb_login_statemachine.c` after Phase 1 is
verified working. Phase 1 uses the Path B wrapper as a temporary bridge for
login messages. Phase 2 replaces that with a proper first-class implementation.

---

## Architecture: Was vs Is

### Was (FIFO-based loopback)

```
    outgoing msg → wire
                 → allocate buffer, copy, push to FIFO
                 → (later) pop from FIFO, dispatch to siblings
                 → each sibling may respond → more FIFO copies
                 → unbounded buffer accumulation
```

### Is (sequential sibling dispatch)

```
    outgoing msg → wire
                 → hold in outgoing slot (don't clear yet)
                 → dispatch to sibling A (one _run() call)
                 → dispatch to sibling B (one _run() call)
                 → if sibling responds, send THAT to wire first
                 → then show THAT response to other siblings
                 → only after all siblings processed, clear slot
                 → reenumerate for next outgoing msg
```

Zero extra buffers. The outgoing message is already sitting in the
`outgoing_msg_info` slot. Sibling responses use the second
`statemachine_info`'s own static outgoing slot. No pool allocation.

---

## Detailed Design

### New Static State

**File:** `src/openlcb/openlcb_main_statemachine.c`

#### Was (lines 85-92):

```c
    /** @brief Stored callback interface pointer for protocol handler dispatch. */
static const interface_openlcb_main_statemachine_t *_interface;

    /** @brief Static state machine context for message routing and node enumeration. */
static openlcb_statemachine_info_t _statemachine_info;

    /** @brief Tracks whether any train node matched during the current enumeration. */
static bool _train_search_match_found;
```

#### Is:

```c
    /** @brief Stored callback interface pointer for protocol handler dispatch. */
static const interface_openlcb_main_statemachine_t *_interface;

    /** @brief Static state machine context for message routing and node enumeration. */
static openlcb_statemachine_info_t _statemachine_info;

    /** @brief Second context for sibling dispatch of outgoing messages.
     *  Uses its own outgoing slot so sibling responses don't corrupt the
     *  main dispatch. Shares the same protocol handler functions. */
static openlcb_statemachine_info_t _sibling_statemachine_info;

    /** @brief Tracks whether any train node matched during the current enumeration. */
static bool _train_search_match_found;

    /** @brief TRUE while we are iterating siblings for an outgoing message. */
static bool _sibling_dispatch_active;
```

The `_sibling_statemachine_info` has its own:
- `outgoing_msg_info` (static outgoing slot — siblings write responses here)
- `incoming_msg_info` (points to the main outgoing message being shown)
- `openlcb_node` (tracks which sibling we're currently dispatching to)

---

### Initialize the Second Context

**File:** `src/openlcb/openlcb_main_statemachine.c`

#### Was — `OpenLcbMainStatemachine_initialize()` (lines 107-124):

```c
void OpenLcbMainStatemachine_initialize(
            const interface_openlcb_main_statemachine_t *interface_openlcb_main_statemachine) {

    _interface = interface_openlcb_main_statemachine;

    _statemachine_info.outgoing_msg_info.msg_ptr = &_statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_msg;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload =
            (openlcb_payload_t *) _statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_payload;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload_type = STREAM;
    OpenLcbUtilities_clear_openlcb_message(_statemachine_info.outgoing_msg_info.msg_ptr);
    OpenLcbUtilities_clear_openlcb_message_payload(_statemachine_info.outgoing_msg_info.msg_ptr);
    _statemachine_info.outgoing_msg_info.msg_ptr->state.allocated = true;

    _statemachine_info.incoming_msg_info.msg_ptr = NULL;
    _statemachine_info.incoming_msg_info.enumerate = false;
    _statemachine_info.openlcb_node = NULL;

}
```

#### Is:

```c
void OpenLcbMainStatemachine_initialize(
            const interface_openlcb_main_statemachine_t *interface_openlcb_main_statemachine) {

    _interface = interface_openlcb_main_statemachine;

    // Main context — unchanged
    _statemachine_info.outgoing_msg_info.msg_ptr = &_statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_msg;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload =
            (openlcb_payload_t *) _statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_payload;
    _statemachine_info.outgoing_msg_info.msg_ptr->payload_type = STREAM;
    OpenLcbUtilities_clear_openlcb_message(_statemachine_info.outgoing_msg_info.msg_ptr);
    OpenLcbUtilities_clear_openlcb_message_payload(_statemachine_info.outgoing_msg_info.msg_ptr);
    _statemachine_info.outgoing_msg_info.msg_ptr->state.allocated = true;

    _statemachine_info.incoming_msg_info.msg_ptr = NULL;
    _statemachine_info.incoming_msg_info.enumerate = false;
    _statemachine_info.openlcb_node = NULL;

    // Sibling context — same pattern, its own outgoing slot
    _sibling_statemachine_info.outgoing_msg_info.msg_ptr = &_sibling_statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_msg;
    _sibling_statemachine_info.outgoing_msg_info.msg_ptr->payload =
            (openlcb_payload_t *) _sibling_statemachine_info.outgoing_msg_info.openlcb_msg.openlcb_payload;
    _sibling_statemachine_info.outgoing_msg_info.msg_ptr->payload_type = STREAM;
    OpenLcbUtilities_clear_openlcb_message(_sibling_statemachine_info.outgoing_msg_info.msg_ptr);
    OpenLcbUtilities_clear_openlcb_message_payload(_sibling_statemachine_info.outgoing_msg_info.msg_ptr);
    _sibling_statemachine_info.outgoing_msg_info.msg_ptr->state.allocated = true;

    _sibling_statemachine_info.incoming_msg_info.msg_ptr = NULL;
    _sibling_statemachine_info.incoming_msg_info.enumerate = false;
    _sibling_statemachine_info.openlcb_node = NULL;

    _sibling_dispatch_active = false;

}
```

---

### Remove `_loopback_to_siblings()` Entirely

**File:** `src/openlcb/openlcb_main_statemachine.c`

#### Was (lines 142-206):

```c
    /**
     * @brief Copies the outgoing message into the incoming FIFO for sibling
     * dispatch.
     *
     * @details Allocates a buffer from the store sized to the outgoing
     * payload, copies header fields and payload bytes, marks the copy as
     * loopback, and pushes it to the head of the incoming FIFO so it is
     * processed next.  Asserts if the pool is exhausted — the buffer pool
     * must be sized so this never happens (see Buffer Pool Sizing in
     * documentation/plans/sibling_dispatch_plan.md).
     */
static void _loopback_to_siblings(void) {

    openlcb_msg_t *out = _statemachine_info.outgoing_msg_info.msg_ptr;

    // Pick the smallest pool that fits the payload
    payload_type_enum ptype;

    if (out->payload_count <= LEN_MESSAGE_BYTES_BASIC) {

        ptype = BASIC;

    } else if (out->payload_count <= LEN_MESSAGE_BYTES_DATAGRAM) {

        ptype = DATAGRAM;

    } else if (out->payload_count <= LEN_MESSAGE_BYTES_SNIP) {

        ptype = SNIP;

    } else {

        ptype = STREAM;

    }

    _interface->lock_shared_resources();
    openlcb_msg_t *copy = OpenLcbBufferStore_allocate_buffer(ptype);
    _interface->unlock_shared_resources();

    assert (copy && "loopback buffer allocation failed — pool too small");

    // Copy header
    copy->mti          = out->mti;
    copy->source_alias = out->source_alias;
    copy->source_id    = out->source_id;
    copy->dest_alias   = out->dest_alias;
    copy->dest_id      = out->dest_id;
    copy->payload_count = out->payload_count;

    // Copy payload bytes
    for (uint16_t i = 0; i < out->payload_count; i++) {

        *copy->payload[i] = *out->payload[i];

    }

    // Mark as loopback — prevents re-loopback and enables self-skip
    copy->state.loopback = true;

    _interface->lock_shared_resources();
    OpenLcbBufferFifo_push_to_head(copy);
    _interface->unlock_shared_resources();

}
```

#### Is:

**DELETE ENTIRELY.** No buffer allocation, no FIFO push. Sibling dispatch
happens inline via the run loop steps below.

---

### New Sibling Dispatch Functions

**File:** `src/openlcb/openlcb_main_statemachine.c`

These are new functions. No "was" — they don't exist today.

```c
    /**
     * @brief Begins sibling dispatch of the outgoing message.
     *
     * @details Called after handle_outgoing sends the message to the wire.
     * Points the sibling context's incoming_msg_info at the main outgoing
     * message, then fetches the first node for sibling iteration.
     *
     * @return true if sibling dispatch started, false if only 1 node (no siblings)
     */
static bool _sibling_dispatch_begin(void) {

    if (_interface->openlcb_node_get_count() <= 1) {

        return false;

    }

    // Point sibling's incoming at the outgoing message we just sent
    _sibling_statemachine_info.incoming_msg_info.msg_ptr =
            _statemachine_info.outgoing_msg_info.msg_ptr;
    _sibling_statemachine_info.incoming_msg_info.enumerate = false;

    // Mark the outgoing as loopback so self-skip works in does_node_process_msg
    _sibling_statemachine_info.incoming_msg_info.msg_ptr->state.loopback = true;

    // Get first sibling node
    _sibling_statemachine_info.openlcb_node =
            _interface->openlcb_node_get_first(OPENLCB_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX);

    _sibling_dispatch_active = true;

    return true;

}

    /**
     * @brief Sends the sibling's pending outgoing response to the wire.
     *
     * @return true if a message was pending (caller should retry), false if idle
     */
static bool _sibling_handle_outgoing(void) {

    if (_sibling_statemachine_info.outgoing_msg_info.valid) {

        if (_interface->send_openlcb_msg(_sibling_statemachine_info.outgoing_msg_info.msg_ptr)) {

            _sibling_statemachine_info.outgoing_msg_info.valid = false;

        }

        return true;

    }

    return false;

}

    /**
     * @brief Re-enters the sibling handler for multi-message enumerate responses.
     *
     * @return true if re-enumeration active, false if complete
     */
static bool _sibling_handle_reenumerate(void) {

    if (_sibling_statemachine_info.incoming_msg_info.enumerate) {

        _interface->process_main_statemachine(&_sibling_statemachine_info);

        return true;

    }

    return false;

}

    /**
     * @brief Dispatches the outgoing message to the current sibling node.
     *
     * @details Skips the originating node (self-skip handled by
     * does_node_process_msg via source_id comparison). Dispatches to nodes
     * in RUNSTATE_RUN only.
     *
     * @return true if dispatch occurred or node skipped, false if no node
     */
static bool _sibling_dispatch_current(void) {

    if (!_sibling_statemachine_info.openlcb_node) {

        _sibling_dispatch_active = false;

        return false;

    }

    if (_sibling_statemachine_info.openlcb_node->state.run_state == RUNSTATE_RUN) {

        _interface->process_main_statemachine(&_sibling_statemachine_info);

    }

    return true;

}

    /**
     * @brief Advances to the next sibling node.
     *
     * @return true if advanced (more siblings or reached end), false if not active
     */
static bool _sibling_dispatch_advance(void) {

    if (!_sibling_statemachine_info.openlcb_node) {

        return false;

    }

    _sibling_statemachine_info.openlcb_node =
            _interface->openlcb_node_get_next(OPENLCB_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX);

    if (!_sibling_statemachine_info.openlcb_node) {

        // All siblings processed — sibling dispatch complete
        _sibling_dispatch_active = false;
        _sibling_statemachine_info.incoming_msg_info.msg_ptr = NULL;

        return true;

    }

    return true;

}
```

**Note:** `OPENLCB_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX` is a new enumerator
key constant. The existing internal keys (in `openlcb_defines.h` lines 1196-1206) are:

```c
// Internal keys — all four slots currently occupied
#define OPENLCB_MAIN_STATMACHINE_NODE_ENUMERATOR_INDEX   (MAX_USER_ENUM_KEYS_VALUES + 0)  // = 4
#define OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX  (MAX_USER_ENUM_KEYS_VALUES + 1)  // = 5
#define CAN_STATEMACHINE_NODE_ENUMRATOR_KEY              (MAX_USER_ENUM_KEYS_VALUES + 2)  // = 6
#define DATAGRAM_TIMEOUT_ENUM_KEY                        (MAX_USER_ENUM_KEYS_VALUES + 3)  // = 7
```

User keys (0-3) are reserved for application code — the plan must NOT use them.
All 4 internal slots are taken, so `MAX_INTERNAL_ENUM_KEYS_VALUES` must be
expanded from 4 to 5.

**File:** `src/openlcb/openlcb_defines.h`

#### Was (line 1176):
```c
#define MAX_INTERNAL_ENUM_KEYS_VALUES 4
```

#### Is:
```c
#define MAX_INTERNAL_ENUM_KEYS_VALUES 5
```

Then add the new key immediately after `DATAGRAM_TIMEOUT_ENUM_KEY`:

```c
    /** @brief Enumeration key used by sibling dispatch in main state machine */
#define OPENLCB_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX   (MAX_USER_ENUM_KEYS_VALUES + 4)  // = 8
```

No array resize needed — `MAX_NODE_ENUM_KEY_VALUES` is computed as
`MAX_USER_ENUM_KEYS_VALUES + MAX_INTERNAL_ENUM_KEYS_VALUES` so it grows
automatically from 8 to 9.

---

### Modified `handle_outgoing_openlcb_message()`

**File:** `src/openlcb/openlcb_main_statemachine.c`

#### Was (lines 892-921):

```c
bool OpenLcbMainStatemachine_handle_outgoing_openlcb_message(void) {

    if (_statemachine_info.outgoing_msg_info.valid) {

        if (_interface->send_openlcb_msg(_statemachine_info.outgoing_msg_info.msg_ptr)) {

            // Loopback to siblings only when there are multiple nodes and
            // the trigger was NOT itself a loopback (one-level cascade prevention).
            if (_interface->openlcb_node_get_count() > 1) {

                if (!_statemachine_info.incoming_msg_info.msg_ptr ||
                        !_statemachine_info.incoming_msg_info.msg_ptr->state.loopback) {

                    _loopback_to_siblings();

                }

            }

            _statemachine_info.outgoing_msg_info.valid = false; // done

        }

        return true; // keep trying till it can get sent

    }

    return false;

}
```

#### Is:

```c
bool OpenLcbMainStatemachine_handle_outgoing_openlcb_message(void) {

    if (_statemachine_info.outgoing_msg_info.valid) {

        if (_interface->send_openlcb_msg(_statemachine_info.outgoing_msg_info.msg_ptr)) {

            // Start sibling dispatch if multiple nodes exist.
            // The outgoing slot stays valid until sibling dispatch completes.
            if (!_sibling_dispatch_begin()) {

                // Single node — no siblings, clear immediately
                _statemachine_info.outgoing_msg_info.valid = false;

            }

        }

        return true;

    }

    return false;

}
```

Key changes:
- Removed `_loopback_to_siblings()` call entirely
- Removed the one-level loopback guard (no longer needed — no FIFO copies)
- Outgoing slot stays valid during sibling dispatch (message still in slot for
  siblings to read)
- Cleared only after sibling dispatch completes (in run loop below)

---

### Modified Run Loop

**File:** `src/openlcb/openlcb_main_statemachine.c`

#### Was — `OpenLcbMainStatemachine_run()` (lines 1081-1119):

```c
void OpenLcbMainStatemachine_run(void) {

    // Get any pending message out first
    if (_interface->handle_outgoing_openlcb_message()) {

        return;

    }

    // If the message handler needs to send multiple messages then
    // enumerate the same incoming/outgoing message again
    if (_interface->handle_try_reenumerate()) {

        return;

    }

    // Pop the next incoming message and dispatch it to the active node
    if (_interface->handle_try_pop_next_incoming_openlcb_message()) {

        return;

    }

    // Grab the first OpenLcb Node
    if (_interface->handle_try_enumerate_first_node()) {

        return;

    }

    // Enumerate all the OpenLcb Nodes
    if (_interface->handle_try_enumerate_next_node()) {

        return;

    }

}
```

#### Is:

```c
void OpenLcbMainStatemachine_run(void) {

    // ── Priority 1: Send pending main outgoing message ──────────────
    // If valid and NOT in sibling dispatch, try to send to wire.
    // If valid and IN sibling dispatch, skip (held for siblings to read).
    if (!_sibling_dispatch_active) {

        if (_interface->handle_outgoing_openlcb_message()) {

            return;

        }

    }

    // ── Priority 2: Sibling dispatch of the outgoing message ────────
    // After sending to wire, show the outgoing msg to each sibling.
    // One step per _run() call — same cadence as node enumeration.
    if (_sibling_dispatch_active) {

        // 2a: Send any pending sibling response to wire first
        if (_sibling_handle_outgoing()) {

            return;

        }

        // 2b: If sibling handler is mid-enumerate, continue it
        if (_sibling_handle_reenumerate()) {

            return;

        }

        // 2c: Dispatch to current sibling node
        if (_sibling_dispatch_current()) {

            // After dispatch, advance to next sibling for next _run()
            _sibling_dispatch_advance();

            // If dispatch just completed (no more siblings), clear main slot
            if (!_sibling_dispatch_active) {

                _statemachine_info.outgoing_msg_info.msg_ptr->state.loopback = false;
                _statemachine_info.outgoing_msg_info.valid = false;

            }

            return;

        }

    }

    // ── Priority 3: Re-enumerate main handler for multi-message ─────
    if (_interface->handle_try_reenumerate()) {

        return;

    }

    // ── Priority 4: Pop next incoming from wire FIFO ────────────────
    if (_interface->handle_try_pop_next_incoming_openlcb_message()) {

        return;

    }

    // ── Priority 5: Enumerate first node for incoming message ───────
    if (_interface->handle_try_enumerate_first_node()) {

        return;

    }

    // ── Priority 6: Enumerate next node ─────────────────────────────
    if (_interface->handle_try_enumerate_next_node()) {

        return;

    }

}
```

---

### Path B: Application Helper Sends (Wrapper)

Application helpers (`OpenLcbApplication_send_event_pc_report`,
`OpenLcbApplicationTrain_send_set_speed`, etc.) call `_interface->send_openlcb_msg()`
directly — Path B. Today this goes straight to the transport layer with no
sibling visibility.

**Solution:** Replace the transport-level DI callback with a wrapper that
sends to wire AND queues the message for sibling dispatch.

**File:** `src/openlcb/openlcb_main_statemachine.c` (new function)

```c
    /** @brief Single-slot pending message for Path B sibling dispatch.
     *  Holds a copy of the last application-layer send so the run loop
     *  can dispatch it to siblings. */
static openlcb_stream_message_t _path_b_pending_msg;
static openlcb_msg_t *_path_b_pending_ptr;
static bool _path_b_pending;

    /**
     * @brief Wrapper around the transport send that adds sibling dispatch.
     *
     * @details Called by application helpers via send_openlcb_msg DI.
     * -# Sends the message to the wire via the real transport callback
     * -# Copies the message into the Path B pending slot
     * -# The run loop will dispatch it to siblings on subsequent calls
     *
     * @param msg  Pointer to the outgoing message (often stack-allocated)
     * @return true if wire send succeeded, false if transport busy
     *
     * @note NOT static — must be visible to openlcb_config.c for DI wiring.
     * Declared in openlcb_main_statemachine.h.
     */
bool OpenLcbMainStatemachine_send_with_sibling_dispatch(openlcb_msg_t *msg) {

    // Send to wire via the real transport function
    if (!_interface->send_openlcb_msg(msg)) {

        return false;

    }

    // If only one node, no siblings to notify
    if (_interface->openlcb_node_get_count() <= 1) {

        return true;

    }

    // Copy into pending slot for sibling dispatch by run loop
    openlcb_msg_t *pending = &_path_b_pending_msg.openlcb_msg;

    pending->mti           = msg->mti;
    pending->source_alias  = msg->source_alias;
    pending->source_id     = msg->source_id;
    pending->dest_alias    = msg->dest_alias;
    pending->dest_id       = msg->dest_id;
    pending->payload_count = msg->payload_count;
    pending->state.loopback = true;

    for (uint16_t i = 0; i < msg->payload_count; i++) {

        *pending->payload[i] = *msg->payload[i];

    }

    _path_b_pending = true;

    return true;

}
```

**IMPORTANT:** This function must be declared in
`src/openlcb/openlcb_main_statemachine.h` so `openlcb_config.c` can reference it:

```c
    /** @brief Transport wrapper — sends to wire and queues for sibling dispatch. */
bool OpenLcbMainStatemachine_send_with_sibling_dispatch(openlcb_msg_t *msg);
```

This wrapper is NOT wired into `_main_sm.send_openlcb_msg`
(that stays as the real transport for the main statemachine's outgoing slot).
It is wired ONLY into the application-layer DI pointers:

**File:** `src/openlcb/openlcb_config.c`

#### Was (lines 269, 613):

```c
_app_train.send_openlcb_msg = &CanTxStatemachine_send_openlcb_message;   // line 269
_app.send_openlcb_msg       = &CanTxStatemachine_send_openlcb_message;   // line 613
```

#### Is:

```c
_app_train.send_openlcb_msg = &OpenLcbMainStatemachine_send_with_sibling_dispatch;
_app.send_openlcb_msg       = &OpenLcbMainStatemachine_send_with_sibling_dispatch;
```

The main statemachine and login statemachine DI pointers stay unchanged:

```c
_main_sm.send_openlcb_msg   = &CanTxStatemachine_send_openlcb_message;   // unchanged
_login_sm.send_openlcb_msg  = &CanTxStatemachine_send_openlcb_message;   // unchanged
```

Why: Path A (main statemachine) already has sibling dispatch built into the
run loop. Path A doesn't need the wrapper — it dispatches to siblings via
steps 2a-2c. Login doesn't need sibling dispatch (login messages are handled
by the main statemachine when they come in as incoming messages).

---

### Path B Sibling Dispatch in Run Loop

The run loop needs one more check: if a Path B message is pending for
sibling dispatch, handle it the same way as a Path A outgoing.

#### Is — addition to `OpenLcbMainStatemachine_run()`:

Insert between Priority 2 and Priority 3:

```c
    // ── Priority 2.5: Path B pending sibling dispatch ───────────────
    // Application helpers sent a message to wire; now show it to siblings.
    if (_path_b_pending && !_sibling_dispatch_active) {

        // Point sibling context at the pending message
        _sibling_statemachine_info.incoming_msg_info.msg_ptr = _path_b_pending_ptr;
        _sibling_statemachine_info.incoming_msg_info.enumerate = false;

        _sibling_statemachine_info.openlcb_node =
                _interface->openlcb_node_get_first(OPENLCB_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX);

        _sibling_dispatch_active = true;
        _path_b_pending = false;

        return;

    }
```

Once `_sibling_dispatch_active` is true, the existing Priority 2 block handles
the rest (dispatch to each sibling, one per `_run()` call). When complete, the
`_path_b_pending_msg` data is no longer referenced.

**Limitation:** If the application sends multiple Path B messages in a tight
loop (e.g., `_forward_estop_to_listeners()`), only the LAST message gets sibling
dispatch. Each send overwrites the pending slot.

**Mitigation options:**
1. Accept it — most Path B sends are individual PCERs or train commands.
   The e-stop forward case is the only tight loop, and listeners receiving
   e-stop are typically external nodes, not local siblings.
2. Small FIFO (depth 8-16) for Path B pending messages instead of a single slot.
   This is bounded by protocol, not unbounded like the old approach.
3. Block in the wrapper until sibling dispatch completes (busy-wait calling
   `_run()` from inside the wrapper). This gives full backpressure but adds
   complexity and may not be safe from all call contexts.

---

### Login Statemachine — Phase 1 Temporary Bridge

**File:** `src/openlcb/openlcb_login_statemachine.c`

#### Was — `openlcb_config.c` login wiring (line 318):

```c
_login_sm.send_openlcb_msg = &CanTxStatemachine_send_openlcb_message;
```

#### Phase 1 Is — wire to Path B wrapper:

```c
_login_sm.send_openlcb_msg = &OpenLcbMainStatemachine_send_with_sibling_dispatch;
```

Login statemachine code itself is **unchanged** in Phase 1.

**What this gives us:** Login messages (Init Complete, Verified Node ID) reach
sibling nodes via the Path B pending slot. Siblings already in `RUNSTATE_RUN`
will see them.

**Known limitation of this approach:** Login sends P/C Identified messages
via enumerate — potentially many per node. The Path B single pending slot means
only the last P/C Identified in an enumerate sequence gets sibling dispatch.
This is accepted for Phase 1. Phase 2 fixes it properly.

**Why CAN hardware echo is not sufficient:** CAN hardware does NOT echo
transmitted frames back to the sender's receive buffer. Sibling virtual nodes
on the same device never see outgoing messages unless explicitly dispatched.

---

## Trace: Identify Events Global with 3 Nodes × 4 Events

Setup: Nodes A, B, C. Each has 4 produced events. External node sends
Identify Events Global. No loopback FIFO involved.

```
_run() #1:   Pop incoming (Identify Events Global from wire FIFO)
_run() #2:   Enumerate first node → dispatch to Node A
             A's handler: set enumerate=true, produce P.Id A.e1 → outgoing slot valid
_run() #3:   handle_outgoing → send P.Id A.e1 to wire
             _sibling_dispatch_begin() → sibling dispatch active, get first sibling
_run() #4:   sibling dispatch → dispatch P.Id A.e1 to Node B (B sees it, no response)
             advance to next sibling
_run() #5:   sibling dispatch → dispatch P.Id A.e1 to Node C (C sees it, no response)
             advance → no more siblings, clear outgoing, _sibling_dispatch_active=false
_run() #6:   reenumerate → A's handler produces P.Id A.e2 → outgoing valid
_run() #7:   handle_outgoing → send P.Id A.e2 to wire, begin sibling dispatch
_run() #8:   sibling dispatch → P.Id A.e2 to Node B
_run() #9:   sibling dispatch → P.Id A.e2 to Node C, done
_run() #10:  reenumerate → A produces P.Id A.e3
_run() #11:  send to wire, begin sibling dispatch
_run() #12:  P.Id A.e3 to B
_run() #13:  P.Id A.e3 to C, done
_run() #14:  reenumerate → A produces P.Id A.e4 (last, clears enumerate)
_run() #15:  send to wire, begin sibling dispatch
_run() #16:  to B
_run() #17:  to C, done
_run() #18:  reenumerate → enumerate not set, falls through
             enumerate next node → dispatch to Node B
             B's handler: set enumerate=true, produce P.Id B.e1
_run() #19:  send P.Id B.e1 to wire, begin sibling dispatch
_run() #20:  P.Id B.e1 to Node A
_run() #21:  P.Id B.e1 to Node C, done
...
```

**Buffers in flight at any point: ZERO extra.** The outgoing message is in the
static `_statemachine_info.outgoing_msg_info` slot (already allocated at init).
Sibling responses go into `_sibling_statemachine_info.outgoing_msg_info`
(also already allocated at init). No pool allocations. No FIFO copies.

**Total `_run()` calls for 3 nodes × 4 events:**
- Per event: 1 send + 2 sibling dispatches = 3 calls
- Per node: 4 events × 3 = 12 calls + 1 initial dispatch = 13 calls
- Total: 3 nodes × ~13 + 1 pop = ~40 calls

Compare to single-node today: 4 events × 2 (send + reenumerate) × 3 nodes = 24
calls. The overhead is the sibling dispatch steps — about 2× more `_run()` calls.
Each call is cheap (no allocation, no copy, just a function pointer dispatch).

---

## Trace: Sibling Responds During Dispatch

Setup: Node A sends Init Complete. Sibling dispatch shows it to Node B.
Node B's handler responds with Identify Events (requests A's events).

```
_run() #1:   handle_outgoing → Init Complete to wire
             _sibling_dispatch_begin()
_run() #2:   sibling dispatch → Init Complete to Node B
             B's handler produces Identify Events → sibling outgoing valid
             advance to Node C
_run() #3:   _sibling_handle_outgoing() → send B's Identify Events to wire
             (sibling outgoing cleared)
_run() #4:   sibling dispatch → Init Complete to Node C (no response)
             advance → no more siblings → _sibling_dispatch_active = false
             clear main outgoing
_run() #5:   reenumerate (main) → enumerate not set, falls through
             pop incoming → empty, falls through
             enumerate first/next → no message, falls through
             (B's Identify Events went to wire only — siblings don't see it
              unless something on the wire sends it back)
```

**Wait — B's Identify Events response went to the wire but NOT to sibling A.**
This is the depth > 1 problem. B's response was sent via the sibling
statemachine's outgoing slot, which goes straight to `send_openlcb_msg` (wire
transport) without its own sibling dispatch.

**This is the Scenario 7 problem.** The sequential dispatch handles one level
correctly but doesn't chain.

---

## Handling Depth > 1: Nested Sibling Responses

When a sibling's handler produces a response during sibling dispatch, that
response also needs to reach other siblings. Two approaches:

### Option A: Queue sibling responses for dispatch (small FIFO)

When `_sibling_handle_outgoing()` sends a sibling response to the wire, also
copy it into a small pending queue (same as Path B). The run loop will pick
it up after the current sibling dispatch completes.

This is a small bounded FIFO (depth 2-3 covers all real protocol chains:
Init Complete → Identify Events → P/C Identified = depth 2).

### Option B: Recursive sibling dispatch

After `_sibling_handle_outgoing()` sends the response to wire, immediately
start a nested sibling dispatch of that response before continuing with the
current dispatch. This requires saving and restoring the current sibling
iteration position (a stack of 2-3 levels).

### Recommendation: Option A

A small fixed FIFO (depth 5) for "sibling responses pending dispatch" is
simpler and sufficient:

```c
#define SIBLING_RESPONSE_QUEUE_DEPTH 5

static openlcb_stream_message_t _sibling_response_queue[SIBLING_RESPONSE_QUEUE_DEPTH];
static uint8_t _sibling_response_queue_head;
static uint8_t _sibling_response_queue_tail;

    /** @brief High-water mark for runtime monitoring of chain depth. */
static uint8_t _sibling_response_queue_high_water;
```

When `_sibling_handle_outgoing()` sends a response to wire, it copies the
message into this queue and updates the high-water mark. When the current
sibling dispatch completes, the run loop checks the queue and starts a new
sibling dispatch for the next queued response.

The queue depth is bounded by the maximum protocol interaction chain depth,
NOT by the number of events or nodes. Depth 5 is conservative — known real
chains reach depth 2. The high-water counter lets application code observe
the actual max depth reached at runtime and confirm no overflow occurs.

---

## Failure Scenario Resolution

| # | Scenario | Resolution |
|---|----------|-----------|
| 1 | Enumerate + buffer accumulation | **Eliminated.** No buffers allocated. Sibling dispatch happens between each enumerate step. One message at a time. |
| 2 | App callback reentrancy | **Eliminated.** Path B wrapper copies to pending slot, run loop dispatches to siblings later. No inline dispatch, no reentrancy. |
| 3 | Combined enum + callback | **Eliminated.** Path A uses run loop sibling dispatch. Path B uses wrapper + pending slot. Both are sequential, non-reentrant. |
| 4 | App layer bypasses loopback | **Fixed.** Wrapper intercepts all Path B sends and copies to pending slot for sibling dispatch. |
| 5 | Broadcast time rollover | **Fixed.** Same as #4. Timer tick sends go through wrapper. Last of 3 messages gets sibling dispatch (single slot limitation — see mitigation options above). |
| 6 | Train listener forwarding | **Eliminated.** Same as #1 — forwarding uses enumerate pattern, sibling dispatch interleaved. |
| 7 | One-level loopback guard | **Partially fixed.** Level 1 works (direct sibling dispatch). Level 2+ requires the sibling response queue (Option A above). Bounded FIFO of depth 3 covers all real protocol chains. |

---

## Implementation Cost

| Item | Count | Details |
|------|-------|---------|
| Modified files | 3 | `openlcb_main_statemachine.c`, `openlcb_config.c`, `openlcb_defines.h` |
| New files | 0 | Everything goes in existing files |
| New static state | ~2 KB | Second `openlcb_statemachine_info_t` + Path B pending msg + response queue |
| Deleted code | ~65 lines | `_loopback_to_siblings()` function + FIFO push/copy logic |
| New functions | 6 | `_sibling_dispatch_begin/current/advance`, `_sibling_handle_outgoing/reenumerate`, `_send_with_sibling_dispatch` |
| Modified functions | 2 | `handle_outgoing_openlcb_message()`, `_run()` |
| New enumerator key | 1 | `OPENLCB_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX` |
| Application code changes | 0 | All changes internal to library |
| Transport driver changes | 0 | Wire path unchanged |
| User config file changes | 0 | DI rewiring is inside `openlcb_config.c` |

---

## Resolved Design Decisions

1. **Path B tight-loop limitation — ACCEPTED as-is.**
   The single pending slot is sufficient. The `_forward_estop_to_listeners()`
   concern is moot: the lead listener receives the original e-stop message and
   its response flows through the normal protocol path, triggering sibling
   dispatch automatically. No tight-loop workaround needed.

2. **Sibling response chain depth — DEPTH 5 with high-water monitoring.**
   Use `SIBLING_RESPONSE_QUEUE_DEPTH 5`. Known real chains reach depth 2 at most.
   A `_sibling_response_queue_high_water` counter tracks the maximum depth seen
   at runtime so applications can verify no overflow occurs in production.

3. **Test impact — TESTS MUST BE UPDATED.**
   The existing tests for `_loopback_to_siblings()` must be replaced with tests
   for the new sibling dispatch mechanism. The old function is deleted entirely.

---

## Implementation Notes

### `state.loopback` Flag Timing on the Main Outgoing Slot

The plan sets `state.loopback = true` on the main outgoing message at the
start of `_sibling_dispatch_begin()` so that `does_node_process_msg()` can
use it for self-skip (it checks both `loopback` flag and `source_id`). After
all siblings are dispatched the flag is cleared (plan run loop, final step of
Priority 2c).

**Why this is safe:**
- Protocol handlers only READ `incoming_msg_info.msg_ptr` — they never read
  the outgoing slot's state flags. Setting loopback on the outgoing slot does
  not affect handler logic.
- The sibling context points `incoming_msg_info.msg_ptr` at the main outgoing
  slot, so `does_node_process_msg()` sees loopback=true via the sibling context
  only. The main context is not dispatching during this period.
- The flag is cleared before `outgoing_msg_info.valid = false`, ensuring the
  next handler that fills the slot starts clean.

### `_sibling_handle_outgoing()` Must Feed the Sibling Response Queue

When `_sibling_handle_outgoing()` successfully sends a sibling response to
wire, it must also copy that response into the sibling response queue for
further sibling dispatch. This is NOT shown explicitly in the new functions
section above — the implementation must include this copy step:

```c
static bool _sibling_handle_outgoing(void) {

    if (_sibling_statemachine_info.outgoing_msg_info.valid) {

        if (_interface->send_openlcb_msg(_sibling_statemachine_info.outgoing_msg_info.msg_ptr)) {

            // Queue the response for sibling dispatch (depth > 1 chains)
            _sibling_response_queue_push(_sibling_statemachine_info.outgoing_msg_info.msg_ptr);

            _sibling_statemachine_info.outgoing_msg_info.valid = false;

        }

        return true;

    }

    return false;

}
```

Without this, sibling responses (e.g., Node B's "Identify Events" in response
to Init Complete) go to the wire but are never shown to sibling Node C.
This is the depth > 1 gap identified in the trace above.

---

## Pre-existing Issues Observed (Out of Scope for This Plan)

The deep-dive audit found one pre-existing inconsistency worth noting for a
future fix:

**Enumerate flag location inconsistency:**
- `openlcb_login_statemachine.c` line 224 checks `outgoing_msg_info.enumerate`
- `openlcb_main_statemachine.c` line 934 checks `incoming_msg_info.enumerate`
- Both are doing the same thing (re-enter handler for next message in sequence)
  but reading from different fields of `openlcb_statemachine_info_t`.
- This does not affect the current plan (the sibling context uses
  `incoming_msg_info.enumerate` — same as the main statemachine, which is the
  code path sibling dispatch calls). But it should be investigated and
  unified in a separate cleanup.

---

---

## Phase 2: Login Statemachine Full Sibling Dispatch — IMPLEMENTED

> **Prerequisite:** Phase 1 verified working in main statemachine.
> **Status:** Implemented 2026-03-17. All 1,170 tests pass.

### Problem with Phase 1 Bridge

The Path B wrapper gives login messages single-slot treatment. During login,
each node sends P/C Identified for every event it produces/consumes via the
enumerate pattern. With N events, N messages are sent in rapid succession, each
one overwriting the pending slot. Only the last reaches siblings.

Siblings that are already in `RUNSTATE_RUN` need to see ALL of them — they use
this information to build their event consumer/producer knowledge.

### Solution: Mirror the Main Statemachine Pattern

Apply the same sequential sibling dispatch infrastructure to
`openlcb_login_statemachine.c`.

### New Static State

**File:** `src/openlcb/openlcb_login_statemachine.c`

```c
    /** @brief Second context for sibling dispatch of login outgoing messages. */
static openlcb_statemachine_info_t _sibling_statemachine_info;

    /** @brief TRUE while iterating siblings for a login outgoing message. */
static bool _sibling_dispatch_active;
```

Initialized in `OpenLcbLoginStatemachine_initialize()` using the same pattern
as the main statemachine's second context (same field-by-field setup).

### New Enumerator Key

**File:** `src/openlcb/openlcb_defines.h`

```c
    /** @brief Enumeration key used by login statemachine sibling dispatch */
#define OPENLCB_LOGIN_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX  (MAX_USER_ENUM_KEYS_VALUES + 5)
```

Also expand `MAX_INTERNAL_ENUM_KEYS_VALUES` from 5 to 6.

### New Interface Function Pointer

The login statemachine's sibling dispatch must call `process_main_statemachine`
because receiving siblings handle login messages through the main protocol
handlers (ProtocolMessageNetwork, ProtocolEventTransport) — not the login
handlers. Add to `interface_openlcb_login_statemachine_t`:

```c
    /** @brief Dispatches a message through the main protocol handler table.
     *  Used by sibling dispatch to deliver login outgoing messages to
     *  siblings already in RUNSTATE_RUN. */
void (*process_main_statemachine)(openlcb_statemachine_info_t *statemachine_info);
```

Wire in `openlcb_config.c`:

```c
_login_sm.process_main_statemachine = &OpenLcbMainStateMachine_process;
```

### Modified `handle_outgoing_openlcb_message()`

Same pattern as Phase 1 main statemachine change: after sending to wire, call
`_sibling_dispatch_begin()` instead of clearing the slot immediately.

### Modified Run Loop

`OpenLcbLoginStatemachine_run()` gains the same Priority 2 sibling dispatch
block as the main statemachine run loop. `_sibling_dispatch_current()` calls
`_interface->process_main_statemachine(&_sibling_statemachine_info)`.

### Revert Phase 1 Bridge

Once Phase 2 is in place, revert the `openlcb_config.c` login wiring back to
the real transport:

```c
_login_sm.send_openlcb_msg = &CanTxStatemachine_send_openlcb_message;
```

Login's sibling dispatch is now handled inline in the login run loop, not
via the Path B wrapper.

### Implementation Cost (Phase 2 incremental)

| Item | Count | Details |
|------|-------|---------|
| Modified files | 3 | `openlcb_login_statemachine.c`, `openlcb_login_statemachine.h`, `openlcb_config.c`, `openlcb_defines.h` |
| New static state | ~1 KB | Second `openlcb_statemachine_info_t` + `_sibling_dispatch_active` |
| New functions | 4 | Same `_sibling_dispatch_begin/current/advance/handle_outgoing` pattern as Phase 1 |
| Modified functions | 2 | `handle_outgoing_openlcb_message()`, `_run()` |
| New enumerator key | 1 | `OPENLCB_LOGIN_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX` |
| New interface field | 1 | `process_main_statemachine` in login interface struct |

---

## Phase 3: CAN Layer — Alias Collision Prevention and AME Listener Repopulate

> **Status:** IMPLEMENTED — 2026-03-17.
> **Prerequisite:** Phase 1 verified working. Independent of Phase 2.

These two changes close the CAN transport layer gaps for virtual nodes.
They require no new data structures, no new files, and no changes to the
OpenLCB message layer. Both operate on existing shared infrastructure
(alias mapping table, listener alias table).

### 3A: Sibling Alias Collision Prevention at Generation Time

#### Problem

When multiple virtual nodes share a single physical instance, each node
independently generates a 12-bit CAN alias via its LFSR seed. The current
code in `state_generate_alias()` only rejects alias `0x000` (invalid per
spec). It does not check whether a sibling virtual node already holds the
generated alias.

Two siblings could derive the same alias, both register it in separate
mapping table slots (since `alias_mapping_register()` matches on `node_id`,
not `alias`), and neither would detect the collision. External nodes would
see duplicate aliases on the bus.

#### Why No New Infrastructure Is Needed

- **`alias_mapping_find_mapping_by_alias(uint16_t alias)`** — already wired
  into `interface_can_login_message_handler_t`. Returns a pointer to the
  mapping entry if the alias exists in the table, NULL if not.
- **`_generate_seed(seed)`** — advances the LFSR one step. Already used in
  the `while (alias == 0)` retry loop inside `state_generate_alias()`.
- **`_generate_alias(seed)`** — derives a 12-bit alias from the current seed.
- **Conflict-retry path** — when a duplicate is detected externally, the RX
  ISR sets `is_duplicate` on the mapping entry. The main loop calls
  `_process_duplicate_aliases()` which calls `alias_mapping_unregister()`
  first (clearing the entry completely), then `_reset_node()` which sets
  `run_state = RUNSTATE_GENERATE_SEED`. The old alias is removed from the
  mapping table before `state_generate_alias()` runs again.

#### Proposed Change

**File:** `src/drivers/canbus/can_login_message_handler.c`
**Function:** `CanLoginMessageHandler_state_generate_alias()`

Widen the existing `while (alias == 0)` rejection loop to also reject
aliases already present in the mapping table:

```c
while ((can_statemachine_info->openlcb_node->alias == 0) ||
        (_interface->alias_mapping_find_mapping_by_alias(
            can_statemachine_info->openlcb_node->alias) != (void*) 0)) {

    can_statemachine_info->openlcb_node->seed =
            _generate_seed(can_statemachine_info->openlcb_node->seed);
    can_statemachine_info->openlcb_node->alias =
            _generate_alias(can_statemachine_info->openlcb_node->seed);

}
```

#### Why This Is Safe

- **No stale self-match:** On re-login after conflict,
  `_process_duplicate_aliases()` calls `alias_mapping_unregister()` before
  `_reset_node()`. The old entry is fully cleared before the node re-enters
  `state_generate_alias()`.
- **First login:** No entries exist in the mapping table yet.
- **Bounded retry:** The alias space is 12 bits (4095 valid values). The
  number of virtual nodes is small (typically 2-20). The LFSR cycles through
  the full space. Finding an unused alias is near-instant.

#### Scope

- One function modified, one condition widened
- Zero new interface pointers, data structures, or files

---

### 3B: AME Listener Alias Repopulate for Local Virtual Nodes

#### Problem

A global AME (empty payload) triggers a listener alias flush per
CanFrameTransferS §6.2.3. After the flush, listener entries for remote nodes
are repopulated as their AMD responses arrive off the wire. But local virtual
nodes never send AMD on the wire to themselves — CAN hardware does not echo
transmitted frames. Their listener entries stay empty until the next external
interaction resolves them.

#### Why CAN Frame Loopback Is Not Needed

The listener table and alias mapping table are shared data structures. On a
global AME, one handler can do all the work in a single pass:

1. Flush the listener alias cache
2. Immediately repopulate entries whose `node_id` matches a local virtual
   node (alias is already known from the alias mapping table)
3. Send AMDs for all local virtual nodes (external nodes need them)

This avoids per-node propagation of the AME, preventing flush-repopulate-flush
race conditions and duplicate AMD responses.

#### Why No Per-Node CAN Control Dispatch Is Needed

The global handlers in `can_rx_message_handler.c` already defend ALL local
aliases because they search the shared mapping table:

- **CID → RID Defense:** `alias_mapping_find_mapping_by_alias()` searches
  all entries. Any local virtual node's alias is defended.
- **AME → AMD (targeted):** `alias_mapping_find_mapping_by_node_id()` finds
  any local virtual node and responds with AMD.
- **AME → AMD (global):** Explicit loop over `ALIAS_MAPPING_BUFFER_DEPTH`.
  Every permitted virtual node gets an AMD response.
- **AMR scrub:** All operations are alias-based, covering all virtual nodes.
- **Duplicate alias detection:** Checks all local aliases in every received
  frame.

Sibling-to-sibling CAN control loopback is redundant — the global handlers
already operate on behalf of all local nodes using shared data.

#### Proposed Change — Incoming Global AME Handler

**File:** `src/drivers/canbus/can_rx_message_handler.c`
**Function:** `CanRxMessageHandler_ame_frame()` — global AME branch

Add `listener_set_alias()` call inside the existing alias mapping loop,
immediately after the flush and before the AMD response:

```c
    // Global AME: discard cached listener aliases per CanFrameTransferS §6.2.3
    if (_interface->listener_flush_aliases) {

        _interface->listener_flush_aliases();

    }

    alias_mapping_info_t *alias_mapping_info =
            _interface->alias_mapping_get_alias_mapping_info();

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (alias_mapping_info->list[i].alias != 0x00 &&
                alias_mapping_info->list[i].is_permitted) {

            // Repopulate listener table for local virtual nodes immediately.
            // Their aliases are already known — no need to wait for AMD off the wire.
            if (_interface->listener_set_alias) {

                _interface->listener_set_alias(
                        alias_mapping_info->list[i].node_id,
                        alias_mapping_info->list[i].alias);

            }

            // Send AMD for this local alias (external nodes need it)
            outgoing_can_msg = _interface->can_buffer_store_allocate_buffer();

            if (outgoing_can_msg) {

                outgoing_can_msg->identifier = RESERVED_TOP_BIT |
                        CAN_CONTROL_FRAME_AMD |
                        alias_mapping_info->list[i].alias;
                CanUtilities_copy_node_id_to_payload(
                        outgoing_can_msg,
                        alias_mapping_info->list[i].node_id, 0);
                CanBufferFifo_push(outgoing_can_msg);

            }

        }

    }
```

`listener_set_alias()` is already in the interface and already wired. If
the `node_id` is not in the listener table it is a no-op (safe).

#### Proposed Change — Self-Originated Global AME

**File:** `src/drivers/canbus/can_main_statemachine.c` (new function)
**File:** `src/drivers/canbus/can_main_statemachine.h` (declaration)

New function `CanMainStatemachine_send_global_alias_enquiry()` performs the
same one-shot pattern when we originate the global AME ourselves:

1. Flush listener alias cache
2. Repopulate local virtual node entries + send AMDs
3. Queue the global AME (triggers AMD responses from external nodes)

This requires two OPTIONAL interface pointers on the CAN main statemachine
interface:

```c
void (*listener_flush_aliases)(void);
void (*listener_set_alias)(node_id_t node_id, uint16_t alias);
```

Wired conditionally in `can_config.c`:

```c
#ifdef OPENLCB_COMPILE_TRAIN
    _main_sm.listener_flush_aliases = &ListenerAliasTable_flush_aliases;
    _main_sm.listener_set_alias = &ListenerAliasTable_set_alias;
#endif
```

#### Scope

- One modified function: `CanRxMessageHandler_ame_frame()`
- One new public function: `CanMainStatemachine_send_global_alias_enquiry()`
- Two new OPTIONAL interface pointers on CAN main statemachine
- Two wiring lines in `can_config.c`
- Zero new files, zero CAN frame loopback infrastructure

---

## Test Matrix

### Phase 1 — Main Statemachine (`openlcb_main_statemachine_Test.cxx`)

| Concern | Test(s) |
|---------|---------|
| Single node: no sibling overhead | `sibling_single_node_no_dispatch` |
| Global visibility: all siblings see outgoing | `sibling_3_nodes_global_visibility` |
| Self-skip: originator does not process own message | `sibling_self_skip_during_dispatch` |
| Response chain: sibling response dispatched to others | `sibling_response_chain_depth_2` |
| Enumerate interleaving: N events × M nodes | `sibling_enumerate_interleaving_4_nodes_10_events` |
| Stress: 4 nodes × 100 events | `sibling_stress_4_nodes_100_events` |
| Path B wrapper: application helper outgoing | `sibling_path_b_wrapper`, `sibling_path_b_single_node_no_pending` |
| Transport busy retry | `sibling_transport_busy_retry` |
| Addressed message filtering | `sibling_addressed_message_filtering` |
| Response queue high-water tracking | `sibling_response_queue_high_water_tracking` |
| Stream addressed routing (5 MTI cases) | `sibling_stream_init_request_routes_to_destination_only`, `sibling_stream_init_reply_routes_to_destination_only`, `sibling_stream_send_and_proceed_route_to_destination_only`, `sibling_stream_reply_routes_back_to_originating_sibling`, `sibling_stream_complete_routes_to_destination_only` |
| Stream 3-node isolation under load | `sibling_stream_third_sibling_isolation_under_load` |

### Phase 2 — Login Statemachine (`openlcb_login_statemachine_Test.cxx`)

| Concern | Test(s) |
|---------|---------|
| Single node: no sibling dispatch | `sibling_dispatch_single_node_no_dispatch` |
| 2-node Init Complete dispatch | `sibling_dispatch_two_nodes_init_complete` |
| 3-node full dispatch | `sibling_dispatch_three_nodes` |
| Mixed run states: only RUNSTATE_RUN siblings | `sibling_dispatch_mixed_run_states` |
| Enumerate with 10 events | `sibling_dispatch_enumerate_10_events` |
| 50-node stress (no events) | `sibling_dispatch_50_nodes_stress` |
| 50-node stress (with events) | `sibling_dispatch_50_nodes_with_events_stress` |
| Loopback flag lifecycle | `sibling_dispatch_loopback_flag_lifecycle` |
| Zero pool allocations (50 nodes) | `sibling_dispatch_zero_pool_allocations_50_nodes` |
| Handler produces response | `sibling_dispatch_handler_produces_response` |
| Response send fails then retries | `sibling_dispatch_response_send_fails_then_retries` |
| Handler sets enumerate flag | `sibling_dispatch_handler_sets_enumerate` |
| Response + enumerate combined | `sibling_dispatch_response_and_enumerate` |
| Run loop skips outgoing during dispatch | `run_loop_skips_outgoing_during_sibling_dispatch` |

### Phase 3A — Alias Collision Prevention (`can_login_message_handler_Test.cxx`)

| Concern | Test(s) |
|---------|---------|
| Basic alias generation | `generate_alias` |
| Zero alias rejection | `generate_alias_rejects_zero` |
| Alias valid range | `alias_within_valid_range` |
| Alias registration | `alias_registration` |
| Sibling collision rejected (2 nodes) | `sibling_alias_collision_rejected` |
| Multiple siblings unique aliases (4 nodes) | `multiple_siblings_unique_aliases` |

### Phase 3B — AME Listener Repopulate (`can_main_statemachine_Test.cxx`)

| Concern | Test(s) |
|---------|---------|
| Global AME flushes listener table | `global_ame_flushes_listener_table` |
| Global AME queues AME frame | `global_ame_queues_ame_frame` |
| Repopulates local aliases + queues AMDs | `global_ame_repopulates_and_sends_amds_for_local_nodes` |
| AMD frames contain correct node IDs | `global_ame_amd_frames_have_correct_content` |
| Skips non-permitted entries | `global_ame_skips_non_permitted_entries` |
| No nodes: only AME queued | `global_ame_no_nodes_only_ame_queued` |
| Stress at max alias mapping depth | `global_ame_stress_max_virtual_nodes` |

---

## Related Documents

- `OpenLcbCLib_OpenMRN_message_comparison.md` — full comparison analysis
- `plan_virtual_node_verification_protocol.md` — draft conformance testing spec

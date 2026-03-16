<!--
  ============================================================
  STATUS: IMPLEMENTED
  The loopback bit, `OpenLcbBufferFifo_push_to_head()`, `_loopback_to_siblings()`, self-skip logic, and cascade-prevention flag check are all present and correct.
  ============================================================
-->

# Virtual Node Sibling Dispatch — Implementation Plan

**Date:** 6 Mar 2026
**Status:** COMPLETED
**Depends on:** None (uses existing buffer store and FIFO infrastructure)

---

## Problem

When one virtual node sends a message, sibling virtual nodes within the same
library instance never see it.  The outgoing message goes directly to
`CanTxStatemachine_send_openlcb_message()` and never enters the incoming FIFO.

**Example:** A multi-train decoder has trains A, B, C as virtual nodes.  An
external throttle sends Set Speed to train A.  Train A's consist handler
forwards the command to listener B.  The forwarded message goes out on CAN —
but train B (on the same device) never receives it because it never enters the
incoming FIFO.

---

## Solution Overview

After the main statemachine successfully transmits an outgoing message, copy it
into a newly allocated buffer, mark it as `loopback`, and push it to the
**head** of the incoming FIFO so it is processed next.  During dispatch, skip
the originating node (self-skip).  Messages marked as loopback never generate
new loopback copies (one-level prevention).

Pushing to the head instead of the tail ensures the loopback copy is processed
immediately on the next `run()` cycle, before any other queued messages.  This
caps the maximum concurrent loopback buffers at **1** (or 2 during a two-phase
datagram enumerate), regardless of FIFO depth or traffic load.

This is unconditional — no compile flag.  The library supports multiple
virtual nodes as a core feature, so sibling dispatch is a correctness fix,
not an optional add-on.  The buffer pool must be sized to accommodate the
additional allocation per outgoing message (see **Buffer Pool Sizing** below).

---

## Files Modified

| File | Change |
|------|--------|
| `openlcb_types.h` | Add `loopback` bit to `openlcb_msg_state_t` |
| `openlcb_buffer_fifo.h` | Add `OpenLcbBufferFifo_push_to_head()` declaration |
| `openlcb_buffer_fifo.c` | Implement `OpenLcbBufferFifo_push_to_head()` |
| `openlcb_main_statemachine.c` | Add `_loopback_to_siblings()`, call from outgoing handler, add self-skip |
| `openlcb_main_statemachine.h` | No changes (loopback is internal) |
| `openlcb_buffer_fifo_Test.cxx` | Tests for `push_to_head` |
| `openlcb_main_statemachine_Test.cxx` | Sibling dispatch tests |

No DI wiring changes needed — `openlcb_main_statemachine.c` already calls
`OpenLcbBufferStore_free_buffer()` directly (in `_free_incoming_message()`),
so it can also call `OpenLcbBufferStore_allocate_buffer()` and
`OpenLcbBufferFifo_push_to_head()` directly.

---

## Step 1 — Add `loopback` bit to `openlcb_msg_state_t`

**File:** `openlcb_types.h` (line ~437)

**WAS:**
```c
typedef struct {
    bool allocated : 1;     // Buffer is in use
    bool inprocess : 1;     // Multi-frame message being assembled
    bool invalid : 1;       // Message invalidated (e.g. AMR) — shall be discarded
} openlcb_msg_state_t;
```

**IS:**
```c
typedef struct {
    bool allocated : 1;     // Buffer is in use
    bool inprocess : 1;     // Multi-frame message being assembled
    bool invalid : 1;       // Message invalidated (e.g. AMR) — shall be discarded
    bool loopback : 1;      // Sibling dispatch copy — skip source node, no re-loopback
} openlcb_msg_state_t;
```

**Rationale:** Adding a fourth bit-field does not change struct packing.  All
code that initializes `openlcb_msg_state_t` uses `OpenLcbUtilities_clear_openlcb_message()`,
which zeroes the struct — so `loopback` defaults to `false` without any
initialization changes.

---

## Step 2 — Add `OpenLcbBufferFifo_push_to_head()`

**Files:** `openlcb_buffer_fifo.h`, `openlcb_buffer_fifo.c`

Adds a new FIFO function that inserts a message at the front of the queue
(next to be popped), instead of the back.  This ensures loopback copies are
processed immediately on the next `run()` cycle, capping concurrent loopback
buffers at 1.

**`openlcb_buffer_fifo.h` — add declaration:**
```c
        /**
         * @brief Inserts a message pointer at the head of the FIFO.
         *
         * @details The inserted message becomes the next one returned by
         * OpenLcbBufferFifo_pop().  Used by sibling dispatch to ensure
         * loopback copies are processed immediately, minimizing the time
         * the loopback buffer is allocated.
         *
         * @param new_msg  Pointer to an @ref openlcb_msg_t allocated from OpenLcbBufferStore.
         *
         * @return The queued message pointer on success, or NULL if the FIFO is full.
         */
    extern openlcb_msg_t *OpenLcbBufferFifo_push_to_head(openlcb_msg_t *new_msg);
```

**`openlcb_buffer_fifo.c` — add implementation:**
```c
    /**
    * @brief Inserts a message pointer at the head of the FIFO.
    *
    * @details Algorithm:
    * -# Compute previous tail position with wraparound
    * -# If prev == head the FIFO is full, return NULL
    * -# Move tail back, store pointer at new tail, return the pointer
    *
    * @verbatim
    * @param new_msg Pointer to @ref openlcb_msg_t allocated from OpenLcbBufferStore
    * @endverbatim
    *
    * @return The queued pointer on success, or NULL if the FIFO is full
    */
openlcb_msg_t *OpenLcbBufferFifo_push_to_head(openlcb_msg_t *new_msg) {

    uint8_t prev = (_openlcb_msg_buffer_fifo.tail == 0)
        ? LEN_MESSAGE_FIFO_BUFFER - 1
        : _openlcb_msg_buffer_fifo.tail - 1;

    if (prev != _openlcb_msg_buffer_fifo.head) {

        _openlcb_msg_buffer_fifo.tail = prev;
        _openlcb_msg_buffer_fifo.list[prev] = new_msg;

        return new_msg;

    }

    return NULL;

}
```

**How it works:** The circular buffer uses `head` for insertion and `tail` for
removal.  Normal `push` advances `head`; `push_to_head` decrements `tail`.
The full-detection sentinel slot works identically in both directions —
if `prev == head`, the FIFO is full.

---

## Step 3 — Add `_loopback_to_siblings()` helper

**File:** `openlcb_main_statemachine.c`

Add a new static helper near the top (after `_free_incoming_message()`):

```c
    /**
     * @brief Copies the outgoing message into the incoming FIFO for sibling
     * dispatch.
     *
     * @details Allocates a buffer from the store sized to the outgoing
     * payload, copies header fields and payload bytes, marks the copy as
     * loopback, and pushes it to the head of the incoming FIFO so it is
     * processed next.  Asserts if the pool is exhausted — the buffer pool
     * must be sized so this never happens (see Buffer Pool Sizing).
     */
static void _loopback_to_siblings(void) {

    openlcb_msg_t *out = _statemachine_info.outgoing_msg_info.msg_ptr;

    // Pick the smallest pool that fits the payload
    payload_type_enum ptype;

    if (out->payload_count <= LEN_MESSAGE_BYTES_BASIC)
        ptype = BASIC;
    else if (out->payload_count <= LEN_MESSAGE_BYTES_DATAGRAM)
        ptype = DATAGRAM;
    else if (out->payload_count <= LEN_MESSAGE_BYTES_SNIP)
        ptype = SNIP;
    else
        ptype = STREAM;

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
    for (uint16_t i = 0; i < out->payload_count; i++)
        copy->payload[i] = out->payload[i];

    // Mark as loopback — prevents re-loopback and enables self-skip
    copy->state.loopback = true;

    _interface->lock_shared_resources();
    OpenLcbBufferFifo_push_to_head(copy);
    _interface->unlock_shared_resources();

}
```

**Rationale:**
- **Smallest pool:** Avoids wasting STREAM (512-byte) buffers on 8-byte
  replies.  Most protocol replies fit in BASIC (16 bytes).
- **Allocation failure is fatal:** `assert()` fires if the pool is exhausted.
  Silent failure would be a protocol violation (OpenLCB requires reliable
  delivery).  The pool must be sized to prevent this — see **Buffer Pool
  Sizing** section.  `assert()` is already included in
  `openlcb_main_statemachine.c`.
- **Lock/unlock:** Same pattern used by `_free_incoming_message()`.

---

## Step 4 — Call loopback from `handle_outgoing_openlcb_message()`

**File:** `openlcb_main_statemachine.c` (line ~812)

**WAS:**
```c
bool OpenLcbMainStatemachine_handle_outgoing_openlcb_message(void) {

    if (_statemachine_info.outgoing_msg_info.valid) {

        if (_interface->send_openlcb_msg(_statemachine_info.outgoing_msg_info.msg_ptr)) {

            _statemachine_info.outgoing_msg_info.valid = false; // done

        }

        return true; // keep trying till it can get sent

    }

    return false;

}
```

**IS:**
```c
bool OpenLcbMainStatemachine_handle_outgoing_openlcb_message(void) {

    if (_statemachine_info.outgoing_msg_info.valid) {

        if (_interface->send_openlcb_msg(_statemachine_info.outgoing_msg_info.msg_ptr)) {

            // Loopback to siblings only when the trigger was NOT itself
            // a loopback (one-level cascade prevention).
            if (!_statemachine_info.incoming_msg_info.msg_ptr ||
                    !_statemachine_info.incoming_msg_info.msg_ptr->state.loopback) {

                _loopback_to_siblings();

            }

            _statemachine_info.outgoing_msg_info.valid = false; // done

        }

        return true; // keep trying till it can get sent

    }

    return false;

}
```

**Cascade prevention logic:**
- `incoming_msg_info.msg_ptr` is always valid during outgoing handling (set in
  pop step, freed only after all nodes are enumerated).
- If the incoming is NULL (edge case) — loopback is safe (no cascade risk).
- If the incoming has `state.loopback == true` — the outgoing is a *reply to
  a loopback message*.  Looping that reply back would start a cascade.  Skip.

**Result:** Exactly one level of loopback.  Original external messages generate
loopback copies.  Replies to loopback copies go out on CAN only.

---

## Step 5 — Self-skip in `does_node_process_msg()`

**File:** `openlcb_main_statemachine.c` (line ~158)

**WAS:**
```c
bool OpenLcbMainStatemachine_does_node_process_msg(openlcb_statemachine_info_t *statemachine_info) {

    if (!statemachine_info->openlcb_node) {

        return false;

    }

    return ( (statemachine_info->openlcb_node->state.initialized) &&
            (
            ((statemachine_info->incoming_msg_info.msg_ptr->mti & MASK_DEST_ADDRESS_PRESENT) !=
                        MASK_DEST_ADDRESS_PRESENT) || // if not addressed process it
            ((... dest alias/id match ...)) ||
            (statemachine_info->incoming_msg_info.msg_ptr->mti ==
                        MTI_VERIFY_NODE_ID_GLOBAL) // special case
            )
            );
}
```

**IS:**
```c
bool OpenLcbMainStatemachine_does_node_process_msg(openlcb_statemachine_info_t *statemachine_info) {

    if (!statemachine_info->openlcb_node) {

        return false;

    }

    // Self-skip: the originating node must not process its own loopback copy
    if (statemachine_info->incoming_msg_info.msg_ptr->state.loopback &&
            statemachine_info->openlcb_node->id ==
            statemachine_info->incoming_msg_info.msg_ptr->source_id) {

        return false;

    }

    return ( (statemachine_info->openlcb_node->state.initialized) &&
            (
            ((statemachine_info->incoming_msg_info.msg_ptr->mti & MASK_DEST_ADDRESS_PRESENT) !=
                        MASK_DEST_ADDRESS_PRESENT) ||
            ((... dest alias/id match ...)) ||
            (statemachine_info->incoming_msg_info.msg_ptr->mti ==
                        MTI_VERIFY_NODE_ID_GLOBAL)
            )
            );
}
```

**Why `source_id` (48-bit) not `source_alias` (12-bit):**
Alias is CAN-layer only and may be 0 in non-CAN transports.  Node ID is
always valid and unique.

---

## Step 6 — Add `#include` to `openlcb_main_statemachine.c`

**File:** `openlcb_main_statemachine.c`

The loopback helper calls `OpenLcbBufferStore_allocate_buffer()` and
`OpenLcbBufferFifo_push_to_head()` directly.  Check if these headers are
already included; if not, add:

```c
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"
```

These may already be present — verify before adding.

---

## Step 7 — Tests

### 7a — FIFO `push_to_head` tests

**File:** `openlcb_buffer_fifo_Test.cxx`

#### Test: `push_to_head_single_message`
- Push one message via `push_to_head`
- Pop → verify it is the same message
- FIFO is empty

#### Test: `push_to_head_is_next_popped`
- Push message A via normal `push` (tail)
- Push message B via `push_to_head` (head)
- Pop → verify it is message B (head has priority)
- Pop → verify it is message A
- FIFO is empty

#### Test: `push_to_head_multiple`
- Push message A via normal `push`
- Push message B via `push_to_head`
- Push message C via `push_to_head`
- Pop order: C, B, A (most recent head-push first)

#### Test: `push_to_head_full_fifo_returns_null`
- Fill the FIFO to capacity via normal `push`
- Call `push_to_head` → returns NULL
- Existing FIFO contents unchanged

#### Test: `push_to_head_interleaved_with_push`
- Push A (tail), push B (head), push C (tail), push D (head)
- Pop order: D, B, A, C

#### Test: `push_to_head_wraparound`
- Fill FIFO partially, pop several, then `push_to_head` to force tail
  wraparound past index 0
- Pop and verify correct message — confirms wraparound arithmetic is correct

### 7b — Fix mock node enumeration

**File:** `openlcb_main_statemachine_Test.cxx`

The existing mock `_OpenLcbNode_get_first` / `_OpenLcbNode_get_next` return
a single pre-set pointer instead of iterating the allocated node pool.
Replace them with the real functions so tests can dispatch to multiple nodes.

The existing mock `_OpenLcbNode_get_first` / `_OpenLcbNode_get_next` return
a single pre-set pointer instead of iterating the allocated node pool.
Replace them with the real functions so tests can dispatch to multiple nodes.

**WAS:**
```c
.openlcb_node_get_first = &_OpenLcbNode_get_first,   // mock — returns fixed pointer
.openlcb_node_get_next  = &_OpenLcbNode_get_next,    // mock — returns fixed pointer
```

**IS:**
```c
.openlcb_node_get_first = &OpenLcbNode_get_first,    // real — iterates allocated nodes
.openlcb_node_get_next  = &OpenLcbNode_get_next,     // real — iterates allocated nodes
```

Existing tests that set `node_get_first` / `node_get_next` globals will need
minor updates: allocate the nodes they want instead of setting mock pointers.

### 7c — Component-level tests

These test `_loopback_to_siblings()` and `does_node_process_msg()` in
isolation, without running the full dispatch loop.  Test setup needs at
least 2 virtual nodes (node A and node B).

#### Test 1: `sibling_dispatch_global_message_delivered_to_sibling`
- Node A produces an outgoing global message (e.g., PCER)
- Call `handle_outgoing_openlcb_message()` → message sent via mock
  `send_openlcb_msg`
- Verify FIFO is NOT empty — loopback copy is at head, pop it
- Verify loopback copy has `state.loopback == true`
- Verify `mti`, `source_id`, `source_alias`, `payload_count` match original
- Dispatch to Node B → `does_node_process_msg()` returns true
- Dispatch to Node A → `does_node_process_msg()` returns false (self-skip)

#### Test 2: `sibling_dispatch_addressed_message_delivered_to_target`
- Node A produces an addressed message to Node B (dest_alias = B's alias)
- Call `handle_outgoing_openlcb_message()` → sent + loopback pushed to head
- Pop loopback copy (it is next in FIFO)
- Dispatch to Node B → `does_node_process_msg()` returns true
- Dispatch to Node A → returns false (self-skip even if addressed)

#### Test 3: `sibling_dispatch_single_node_no_loopback_processed`
- Only Node A exists
- Node A produces an outgoing message
- Loopback copy is pushed to FIFO
- Pop and dispatch to Node A → `does_node_process_msg()` returns false
- Message is freed normally — no processing occurs

#### Test 4: `sibling_dispatch_no_re_loopback`
- Node A produces an outgoing message → loopback copy pushed
- Pop loopback copy, dispatch to Node B
- Node B produces a reply (sets `outgoing_msg_info.valid = true`)
- Call `handle_outgoing_openlcb_message()` for Node B's reply
- Verify FIFO is empty after send — no second loopback was generated
  (because incoming was `state.loopback == true`)

#### Test 5: `sibling_dispatch_buffer_exhaustion_graceful`
- Exhaust the buffer store (allocate all buffers)
- Node A produces an outgoing message
- `handle_outgoing_openlcb_message()` succeeds (send returns true)
- `_loopback_to_siblings()` silently skips (allocation fails)
- Verify FIFO is empty — no loopback copy
- Verify `outgoing_msg_info.valid` is false — outgoing still completed

### 7d — End-to-end dispatch loop tests

These exercise the full `OpenLcbMainStatemachine_run()` loop with real node
enumeration, verifying that loopback messages actually arrive at sibling
nodes through normal dispatch.

#### Test 6: `sibling_dispatch_e2e_global_reaches_sibling`
- Allocate Node A (id=0x060504030201) and Node B (id=0x070605040302),
  both initialized and in RUNSTATE_RUN
- Push a global message (e.g., MTI_VERIFIED_NODE_ID) to the incoming FIFO
  with `source_id` = some external node
- Call `run()` repeatedly until FIFO is empty and no outgoing pending
- If Node A's handler produces a global reply (e.g., Verified Node ID):
  - Verify `send_openlcb_msg` was called (reply went out on bus)
  - Verify loopback copy was pushed to FIFO
  - Continue calling `run()` — loopback is popped and dispatched
  - Verify Node B's handler was called for the loopback message
  - Verify Node A's handler was NOT called for its own loopback (self-skip)

#### Test 7: `sibling_dispatch_e2e_addressed_reply_reaches_sibling`
- Allocate Node A and Node B, both initialized
- Push an addressed message to Node A (e.g., Protocol Support Inquiry)
- Call `run()` — Node A processes and produces a reply addressed to
  the external source
- Verify reply was sent on bus AND loopback copy in FIFO
- Continue `run()` — loopback dispatched to all nodes
- Node A skips (self-skip), Node B checks (but dest doesn't match, so
  `does_node_process_msg()` returns false for addressed loopback) — this
  confirms addressed replies don't get incorrectly delivered to siblings

#### Test 8: `sibling_dispatch_e2e_no_cascade`
- Allocate Node A and Node B
- Push a global message from external source
- Run until Node A produces a reply → loopback pushed → dispatched to
  Node B → Node B produces a reply to the loopback
- Verify Node B's reply goes out on bus but does NOT push a second
  loopback (incoming was loopback, so cascade is blocked)
- Verify FIFO drains completely — no infinite loop

---

## Message Flow Diagram

```
External CAN message arrives
        │
        ▼
┌──────────────────┐
│  Incoming FIFO   │◄──push-to-head────────────────┐
│  (pop from head) │                                │
└────────┬─────────┘                                │
         │                                          │
         ▼                                          │
┌──────────────────┐     ┌───────────────────┐      │
│  Node A dispatch │────▶│  Handler produces │      │
│  (self-skip if   │     │  outgoing msg     │      │
│   loopback &&    │     └────────┬──────────┘      │
│   source==self)  │              │                  │
└──────────────────┘              ▼                  │
                         ┌───────────────────┐      │
                         │  send_openlcb_msg │      │
                         │  (CAN TX)         │      │
                         └────────┬──────────┘      │
                                  │                  │
                                  ▼                  │
                         ┌───────────────────┐      │
                         │  _loopback_to_    │      │
                         │  siblings()       │──────┘
                         │  (if incoming NOT │
                         │   loopback)       │
                         └───────────────────┘

Next run() cycle:
  pop → gets loopback (it's at head, ahead of any queued external msgs)
    → dispatch to Node B (processes)
    → dispatch to Node A (self-skip)
    → free loopback buffer
  pop → gets next external message (normal processing resumes)
```

---

## Risks and Mitigations

| Risk | Mitigation |
|------|-----------|
| Buffer exhaustion (extra buffer per TX) | Push-to-head caps concurrent loopback buffers at 1 (2 during enumerate). Pool needs only +2 headroom. `assert()` fires on exhaustion. See **Buffer Pool Sizing**. |
| Infinite loop (reply → loopback → reply → ...) | One-level prevention: replies to loopback messages are NOT re-looped. |
| Performance (alloc + copy per TX) | BASIC pool (16 bytes) used for small messages — minimal overhead. |
| Self-delivery (node processes own message) | `source_id` match in `does_node_process_msg()`. |

---

## Buffer Pool Sizing

### The Existing Constraint

The library already depends on the buffer pool never being exhausted.  This is
not new to sibling dispatch — it is a fundamental property of the architecture.
If CAN RX cannot allocate a buffer for an incoming frame, that frame is lost.
There is no back-pressure mechanism to tell the sender to slow down (datagrams
have their own retry/reject flow, but general messages do not).

The outgoing path is immune to this problem because it uses a static buffer
embedded in `_statemachine_info` — not a pool allocation.  The dispatch loop
always makes forward progress: the `enumerate` stall blocks message intake for
2-3 `run()` cycles, but the static outgoing buffer ensures the handler always
completes and frees the incoming buffer back to the pool.

### What Sibling Dispatch Adds

Each outgoing message now also consumes one pool buffer for the loopback copy.
Because the loopback is pushed to the **head** of the FIFO, it is always the
next message processed.  This means at most **1** loopback buffer is alive at
any instant during normal dispatch (or **2** during a two-phase datagram
enumerate that produces 2 outgoing messages before the incoming is freed).

This is a bounded constant — it does not grow with FIFO depth or traffic load.
Compare to push-to-tail, where every outgoing reply queues a loopback behind
all existing FIFO entries and the concurrent count grows with queue depth.

**Without sibling dispatch:** At any instant, the pool holds:
- Incoming messages queued in the FIFO (1 per pending CAN message)
- The one incoming message currently being dispatched
- `last_received_datagram` saves (up to 1 per node)

**With sibling dispatch (push-to-head):** Add:
- At most 1 loopback buffer (2 during two-phase datagram enumerate)

### Sizing Guidance

The default pool depths in `openlcb_user_config.h` are:

| Pool     | Depth | Payload Size |
|----------|-------|-------------|
| BASIC    | 32    | 16 bytes    |
| DATAGRAM | 4     | 72 bytes    |
| SNIP     | 4     | 256 bytes   |
| STREAM   | 1     | 512 bytes   |
| **Total**| **41**|             |

Most protocol replies (Verified Node ID, Protocol Support Reply, event
messages) fit in BASIC (16 bytes).  Loopback copies are allocated from the
smallest pool that fits the payload, so BASIC absorbs most of the additional
load.

**Rule of thumb:** The BASIC pool depth should be at least:

```
(max concurrent incoming messages) + 2 + margin
```

The `+ 2` accounts for the worst case of 2 concurrent loopback buffers during
a two-phase datagram enumerate.  Under normal (non-enumerate) dispatch, only
1 extra buffer is needed.  Because push-to-head ensures the loopback is
processed immediately, the count does not scale with the number of virtual
nodes or FIFO depth.

For a 4-node train decoder with default BASIC depth of 32, there is ample
headroom.

### Runtime Monitoring

The buffer store already provides peak allocation counters:

- `OpenLcbBufferStore_basic_messages_max_allocated()`
- `OpenLcbBufferStore_datagram_messages_max_allocated()`
- `OpenLcbBufferStore_snip_messages_max_allocated()`
- `OpenLcbBufferStore_stream_messages_max_allocated()`

Applications can read these at runtime (e.g., via diagnostic CDI fields) to
verify that pool depths have adequate headroom.  If the peak allocation
approaches the pool depth, the application should increase the corresponding
`USER_DEFINED_*_BUFFER_DEPTH` in `openlcb_user_config.h`.

---

## Design Notes

- **No DI changes needed.** The main statemachine already calls buffer
  store/FIFO directly (see `_free_incoming_message()` at line 137).  The
  loopback helper follows the same pattern.

- **Why not loopback at the CAN layer?** The CAN TX path converts OpenLCB
  messages to CAN frames and fragments them.  Looping back at that level
  would require re-assembly.  Looping at the OpenLCB layer keeps the copy
  as a complete `openlcb_msg_t` — no fragmentation/reassembly overhead.

- **`enumerate` case works naturally.** When a handler sets
  `outgoing_msg_info.enumerate = true` (multi-message), each outgoing message
  passes through `handle_outgoing_openlcb_message()` separately.  Each gets
  its own loopback copy if the trigger was not loopback.  No special handling.

- **Timer-triggered messages are unaffected.** Timer callbacks (heartbeat,
  etc.) use separate send paths, not the main statemachine's outgoing buffer.
  They do not enter `handle_outgoing_openlcb_message()`.

- **Why push-to-head instead of push-to-tail?** Two benefits: (1) Caps
  concurrent loopback buffers at 1 regardless of FIFO depth or traffic —
  with push-to-tail the count grows with queue depth.  (2) Siblings see the
  message sooner — the loopback is processed on the very next `run()` cycle
  after TX, before any other queued external messages.  OpenLCB does not
  require message ordering at the transport level, so reordering is safe.

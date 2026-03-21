<!--
  ============================================================
  STATUS: IMPLEMENTED
  `ProtocolDatagramHandler_check_timeouts(uint8_t current_tick)` is declared and defined with `DATAGRAM_TIMEOUT_TICKS = 30` and bit-packed retry-count logic matching the plan exactly.
  ============================================================
-->

# Plan: Item 2 -- Datagram Timeout and Retry Limit (also covers Item 6)

## 1. Summary

`ProtocolDatagramHandler_100ms_timer_tick()` is an empty placeholder. The standard requires a
3-second default timeout on sent datagrams awaiting acknowledgement. Additionally, there is no
retry limit on temporary rejections -- an infinite retry loop is possible.

**This plan covers both Item 2 (timeout) and Item 6 (retry limit).**

The fix packs both timeout and retry count into the existing `openlcb_msg_t.timerticks` (uint8_t)
field, using zero additional memory:

```
Bits 7-5: retry count (0-7, abandon at 3)
Bits 4-0: tick counter (0-31, timeout at 30 = 3 seconds)
```

Abandon the pending datagram if either:
- Tick counter reaches 30 (no response within 3 seconds of this attempt)
- Retry count reaches 3 (too many temporary rejections)

On each temporary rejection + resend, the retry count increments and the tick counter resets
to 0, giving each attempt a fresh 3-second window.

**Thread safety -- tick parameter approach:**

A shared `volatile uint8_t _global_100ms_tick` counter is incremented by the timer tick (which
may fire from interrupt or thread context). This is the sole action of the timer interrupt --
it never touches per-buffer fields. The global clock is owned by `openlcb_config.c` and is
**not read directly by any protocol module**. Instead, the main loop reads the clock once per
iteration and passes it down as a `uint8_t current_tick` parameter to each module's periodic
service function. This follows the dependency-injection pattern established in
`plan_global_clock_refactor.md`.

When a datagram buffer is allocated, `timerticks` is set to 0 by
`OpenLcbUtilities_load_openlcb_message()`. The buffer's `timerticks` field stores the retry
count (upper 3 bits) and the **snapshot of the current tick** at the start of each attempt
(lower 5 bits). The main loop, running under lock, computes elapsed time via
`(current_tick - snapshot) & DATAGRAM_TICK_MASK` and frees the buffer if it exceeds the
threshold.

**Rejection handler tick access:** The rejection handler
`ProtocolDatagramHandler_datagram_rejected()` runs from the main statemachine dispatch when
processing an incoming Datagram Rejected message -- NOT from the periodic services. To give
it access to the tick without module coupling, the main statemachine sets
`statemachine_info->current_tick` at the start of each dispatch cycle (before routing to any
handler). This means every handler dispatched through `openlcb_statemachine_info_t` has the
current tick available without calling any getter.

This means:
- **Timer tick context:** Only increments a single global `volatile uint8_t`. Zero per-buffer
  access. Zero risk of racing with buffer ownership.
- **Main loop context:** Receives tick as parameter, reads per-buffer `timerticks`, frees
  under lock. Lock disables interrupts, preventing races with Rx buffer allocation.
- **No module coupling:** `protocol_datagram_handler.c` does NOT include `openlcb_config.h`
  and does NOT call `OpenLcb_get_global_100ms_tick()`. The tick arrives as a function
  parameter or through `statemachine_info->current_tick`.

## 2. Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/openlcb/openlcb_types.h` | Add `uint8_t current_tick` to `openlcb_statemachine_info_t` |
| `src/openlcb/protocol_datagram_handler.c` | Implement `ProtocolDatagramHandler_100ms_timer_tick(uint8_t current_tick)` (no-op or removed); add `ProtocolDatagramHandler_check_timeouts(uint8_t current_tick)` for main loop; update rejection handler to use `statemachine_info->current_tick`; stamp buffer on first send |
| `src/openlcb/protocol_datagram_handler.h` | Declare `ProtocolDatagramHandler_check_timeouts(uint8_t current_tick)` |
| `src/openlcb/openlcb_main_statemachine.c` | Set `statemachine_info->current_tick` at the start of each dispatch cycle |
| `src/drivers/canbus/can_main_statemachine.c` | Call `ProtocolDatagramHandler_check_timeouts(tick)` from main loop periodic services |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

### Tests

| File | Change |
|------|--------|
| `src/openlcb/protocol_datagram_handler_Test.cxx` | Add tests for timeout, retry count, overflow guard, and combined scenarios |

## 3. Implementation Steps

### Step 1. Add `current_tick` to `openlcb_statemachine_info_t`

In `openlcb_types.h`, add the field:

```c
        /** @brief Complete context passed to protocol handler functions. */
    typedef struct {

        openlcb_node_t *openlcb_node;
        openlcb_incoming_msg_info_t incoming_msg_info;
        openlcb_outgoing_stream_msg_info_t outgoing_msg_info;
        uint8_t current_tick;

    } openlcb_statemachine_info_t;
```

Then in `openlcb_main_statemachine.c`, at the start of the dispatch cycle (before routing
the incoming message to any handler), set the tick:

```c
    _statemachine_info.current_tick = current_tick;
```

Where `current_tick` comes from the periodic services parameter chain established by the
global clock refactor. This gives every handler -- including the datagram rejection handler --
access to the current tick without any getter call.

### Step 2. Define bit-field macros

In `protocol_datagram_handler.c`:

```c
    /** @brief Default datagram timeout in 100ms ticks (3 seconds). */
#define DATAGRAM_TIMEOUT_TICKS 30

    /** @brief Maximum datagram retry attempts before abandoning. */
#define DATAGRAM_MAX_RETRIES 3

    /** @brief Bit masks and shifts for packing retry count + tick snapshot into timerticks.
     *
     *  Bits 7-5: retry count (0-7)
     *  Bits 4-0: tick snapshot (snapshot of current_tick at start of attempt)
     */
#define DATAGRAM_RETRY_SHIFT  5
#define DATAGRAM_RETRY_MASK   0xE0
#define DATAGRAM_TICK_MASK    0x1F
```

### Step 3. Stamp the buffer on first send

When a datagram is first assigned to `node->last_received_datagram`, stamp the current
tick into the lower 5 bits. The `current_tick` value is available from the calling context
(either the periodic services parameter or `statemachine_info->current_tick`).

Find the location where `last_received_datagram` is first assigned and add:

```c
    node->last_received_datagram->timerticks =
            current_tick & DATAGRAM_TICK_MASK;
```

This sets retry count = 0 (upper 3 bits clear) and snapshot = current tick (lower 5 bits).

### Step 4. Make the timer tick a no-op (or remove the per-node iteration)

`ProtocolDatagramHandler_100ms_timer_tick(uint8_t current_tick)` receives the tick as a
parameter from the main loop's periodic services but no longer needs to iterate nodes. The
per-module timer tick can remain as an empty function (for future use) or be removed. Keeping
it as an empty stub is simplest for backwards compatibility:

```c
void ProtocolDatagramHandler_100ms_timer_tick(uint8_t current_tick) {

    // Datagram timeout is handled by
    // ProtocolDatagramHandler_check_timeouts() in the main loop.

}
```

### Step 5. Implement main-loop timeout check (lock + free)

Add a new public function that receives `current_tick` as a parameter:

```c
    /**
     * @brief Scan all nodes for timed-out or max-retried pending datagrams and free them.
     *
     * @details Algorithm:
     * -# Lock shared resources
     * -# Iterate all allocated nodes
     * -# If a node has last_received_datagram != NULL:
     *    -# Extract tick snapshot (lower 5 bits) and retry count (upper 3 bits)
     *    -# Compute elapsed = (current_tick - snapshot) & DATAGRAM_TICK_MASK
     *    -# If elapsed >= DATAGRAM_TIMEOUT_TICKS OR retry count >= DATAGRAM_MAX_RETRIES:
     *       -# Free the buffer and NULL the pointer
     *       -# Clear the resend_datagram flag
     * -# Unlock shared resources
     *
     * @param current_tick  Current value of the global 100ms tick, passed from the main loop.
     *
     * @note Called from the main processing loop, NOT from the timer interrupt.
     */
void ProtocolDatagramHandler_check_timeouts(uint8_t current_tick) {

    _interface->lock_shared_resources();

    openlcb_node_t *node = OpenLcbNode_get_first(DATAGRAM_TIMEOUT_ENUM_KEY);

    while (node) {

        if (node->last_received_datagram) {

            uint8_t snapshot = node->last_received_datagram->timerticks & DATAGRAM_TICK_MASK;
            uint8_t retries = (node->last_received_datagram->timerticks & DATAGRAM_RETRY_MASK) >> DATAGRAM_RETRY_SHIFT;
            uint8_t elapsed = (current_tick - snapshot) & DATAGRAM_TICK_MASK;

            if (elapsed >= DATAGRAM_TIMEOUT_TICKS || retries >= DATAGRAM_MAX_RETRIES) {

                OpenLcbBufferStore_free_buffer(node->last_received_datagram);
                node->last_received_datagram = NULL;
                node->state.resend_datagram = false;

            }

        }

        node = OpenLcbNode_get_next(DATAGRAM_TIMEOUT_ENUM_KEY);

    }

    _interface->unlock_shared_resources();

}
```

### Step 6. Update the rejection handler to increment retry count

In `ProtocolDatagramHandler_datagram_rejected()`, when a temporary error is received, increment
the retry count (upper 3 bits) and reset the tick snapshot (lower 5 bits) to the current tick,
giving the retry a fresh 3-second window.

The rejection handler runs from the main statemachine dispatch (processing an incoming Datagram
Rejected message), NOT from the periodic services. The tick is available via
`statemachine_info->current_tick`, which the main statemachine sets at the start of each
dispatch cycle:

```c
void ProtocolDatagramHandler_datagram_rejected(openlcb_statemachine_info_t *statemachine_info) {

    if ((OpenLcbUtilities_extract_word_from_openlcb_payload(
            statemachine_info->incoming_msg_info.msg_ptr, 0) & ERROR_TEMPORARY) == ERROR_TEMPORARY) {

        if (statemachine_info->openlcb_node->last_received_datagram) {

            uint8_t retries = (statemachine_info->openlcb_node->last_received_datagram->timerticks
                    & DATAGRAM_RETRY_MASK) >> DATAGRAM_RETRY_SHIFT;

            retries++;

            if (retries < DATAGRAM_MAX_RETRIES) {

                statemachine_info->openlcb_node->state.resend_datagram = true;

            }

            // Pack: new retry count in upper 3 bits, fresh tick snapshot in lower 5 bits
            statemachine_info->openlcb_node->last_received_datagram->timerticks =
                    (retries << DATAGRAM_RETRY_SHIFT) |
                    (statemachine_info->current_tick & DATAGRAM_TICK_MASK);

        }

    } else {

        ProtocolDatagramHandler_clear_resend_datagram_message(statemachine_info->openlcb_node);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}
```

Note: when `retries` reaches 3, `resend_datagram` is NOT set. The main loop's
`check_timeouts()` will see `retries >= DATAGRAM_MAX_RETRIES` and free the buffer on its
next pass.

### Step 7. Declare in header

In `protocol_datagram_handler.h`:

```c
        /**
         * @brief Scans for timed-out or max-retried pending datagrams and frees them.
         *
         * @details Must be called from the main processing loop, not from an
         * interrupt. Acquires the shared resource lock internally.
         *
         * @param current_tick  Current value of the global 100ms tick, passed from the main loop.
         */
    extern void ProtocolDatagramHandler_check_timeouts(uint8_t current_tick);
```

### Step 8. Wire into the main loop

Call `ProtocolDatagramHandler_check_timeouts(tick)` from the main processing loop's periodic
services. The `tick` value is already available from `_run_periodic_services()` (see
`plan_global_clock_refactor.md`). The function acquires its own lock internally.

### Step 9. Confirm timerticks reset path

`openlcb_msg_t.timerticks` is reset to 0 by `OpenLcbUtilities_load_openlcb_message()` when the
buffer is originally built. When first assigned to `last_received_datagram`, we overwrite it
with the current tick snapshot (Step 3). Value: retry count = 0, snapshot = current_tick.
Correct starting point.

## 4. Test Changes

All changes are in `src/openlcb/protocol_datagram_handler_Test.cxx`.

### Test 1: Timeout fires after 3 seconds (no response)

```cpp
TEST(ProtocolDatagramHandler, timeout_fires_after_3_seconds)
{
    // Setup: create node, assign last_received_datagram
    // Stamp timerticks with current_tick snapshot (e.g., current_tick = 5)
    // Call check_timeouts(current_tick = 35)  -- elapsed = 30
    // Verify last_received_datagram is NULL and resend_datagram is false
}
```

### Test 2: No timeout before 3 seconds

```cpp
TEST(ProtocolDatagramHandler, no_timeout_before_3_seconds)
{
    // Setup: create node with pending datagram stamped at current_tick = 5
    // Call check_timeouts(current_tick = 34)  -- elapsed = 29
    // Verify last_received_datagram is still valid
}
```

### Test 3: Retry count increments on temporary rejection

```cpp
TEST(ProtocolDatagramHandler, retry_count_increments_on_temp_rejection)
{
    // Setup: node with pending datagram, statemachine_info.current_tick = 20
    // Receive temporary rejection (statemachine_info->current_tick used for new snapshot)
    // Verify: retry count = 1 (upper bits = 001), tick snapshot = 20
    // Verify: resend_datagram == true
}
```

### Test 4: Max retries reached -- datagram abandoned

```cpp
TEST(ProtocolDatagramHandler, max_retries_abandons_datagram)
{
    // Setup: node with pending datagram
    // Receive 3 temporary rejections (set statemachine_info.current_tick before each call)
    // Verify resend_datagram is NOT set on 3rd rejection
    // Call check_timeouts(current_tick)
    // Verify last_received_datagram is NULL and resend_datagram is false
}
```

### Test 5: Tick snapshot overflow guard (wrap-around arithmetic)

```cpp
TEST(ProtocolDatagramHandler, tick_snapshot_wraps_correctly)
{
    // Setup: node with pending datagram, snapshot at current_tick = 28
    // Call check_timeouts(current_tick = 3)  -- elapsed = (3 - 28) & 0x1F = 7
    // Verify NOT timed out (elapsed 7 < 30)
    // Call check_timeouts(current_tick = 26)  -- elapsed = (26 - 28) & 0x1F = 30
    // Verify timed out (elapsed 30 >= 30)
}
```

### Test 6: OK reply clears before timeout

```cpp
TEST(ProtocolDatagramHandler, ok_reply_clears_pending)
{
    // Setup: create node with pending datagram
    // Call datagram_received_ok
    // Verify last_received_datagram is NULL
    // Call check_timeouts(current_tick = 100) with large elapsed
    // Verify no crash (nothing to timeout)
}
```

### Test 7: Combined -- retry then timeout on final attempt

```cpp
TEST(ProtocolDatagramHandler, retry_then_timeout)
{
    // Setup: node with pending datagram
    // Receive 2 temporary rejections (set statemachine_info.current_tick for each)
    // (retry count = 2, tick snapshot reset each time)
    // Call check_timeouts(current_tick + 30) with 30 elapsed since last snapshot
    // Verify last_received_datagram is NULL (timed out on 3rd attempt)
}
```

### Test 8: Permanent error still clears immediately

```cpp
TEST(ProtocolDatagramHandler, permanent_error_clears_immediately)
{
    // Setup: node with pending datagram
    // Receive permanent rejection (0x1xxx)
    // Verify last_received_datagram is NULL, retry count irrelevant
}
```

### Verification procedure

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

## 5. Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| timerticks snapshot wrap-around | NONE | Subtraction with mask: `(current - snapshot) & 0x1F` handles wrap correctly for durations up to 31 ticks (~3.1 seconds). Timeout threshold is 30, well within range. |
| Timer/main-loop race | NONE | Timer only increments `volatile uint8_t _global_100ms_tick`. Main loop reads it once and passes the value as a parameter. Per-buffer fields are read under lock. Lock disables interrupts. |
| No module coupling | NONE | `protocol_datagram_handler.c` does not include `openlcb_config.h` and does not call `OpenLcb_get_global_100ms_tick()`. The tick arrives as a function parameter (`current_tick`) for periodic services and through `statemachine_info->current_tick` for message dispatch handlers. This follows the dependency-injection pattern from `plan_global_clock_refactor.md`. |
| Retry count overflow | VERY LOW | 3 bits hold 0-7. Max retries is 3. Even if somehow called more, 7 is the ceiling. |
| Existing tests | LOW | Existing `_100ms_timer_tick` test calls the function with no nodes -- still passes (now a no-op). Existing rejection tests will need updated assertions for the new timerticks bit layout. |
| No struct changes needed | NONE | Reuses existing `openlcb_msg_t.timerticks`. Zero impact on struct layouts. |
| `current_tick` field added to `openlcb_statemachine_info_t` | LOW | Adds 1 byte to the statemachine info struct. The struct is a local variable in the main statemachine, not allocated dynamically. Trivial memory cost. |

## 6. Estimated Effort

| Task | Time |
|------|------|
| Add `current_tick` to `openlcb_statemachine_info_t` | 5 min |
| Set `current_tick` in main statemachine dispatch | 5 min |
| Define bit-field macros | 5 min |
| Stamp buffer on first send | 5 min |
| Implement check_timeouts (lock + free) | 15 min |
| Update rejection handler | 10 min |
| Declare in header | 5 min |
| Wire into main loop | 10 min |
| Add 8 tests | 30 min |
| Build and run full test suite | 5 min |
| **Total** | **~95 min** |

## 7. Item 6 Disposition

This plan fully covers Item 6 (No Datagram Retry Limit Enforcement). The retry count in the
upper 3 bits of timerticks enforces a maximum of 3 retries. Item 6 can be marked as covered
by this plan and does not need a separate implementation.

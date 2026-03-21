<!--
  ============================================================
  STATUS: IMPLEMENTED
  `OpenLcbBufferList_check_timeouts()` is fully implemented with `BUFFER_LIST_INPROCESS_TIMEOUT_TICKS = 30`, iterating buffers, checking `assembly_ticks`, and freeing stale in-progress buffers.
  ============================================================
-->

# Plan: Item 7 -- No Timeout on Partially Assembled Multi-Frame Datagrams

## 1. Summary

If a sender begins a multi-frame CAN sequence (FIRST frame) but never sends the LAST frame
(e.g., sender crash), the partially assembled buffer in `OpenLcbBufferList` remains allocated
permanently with `state.inprocess == true`. No timer reclaims orphaned in-progress buffers.

This affects all multi-frame message types — datagrams, SNIP replies, events with payload, etc.

**Clock access — no direct coupling to the global clock:**

The incoming CAN frames are processed in the Rx interrupt or thread context
(`can_rx_message_handler.c`). The Rx context exclusively owns the buffer list — it creates
FIRST-frame buffers, appends MIDDLE frames, and releases on LAST frame. No other context
allocates buffers in the list.

The Rx handler accesses the global 100ms tick counter through its interface function pointer
`_interface->get_current_tick()`. It never includes `openlcb_config.h` and never calls
`OpenLcb_get_global_100ms_tick()` directly. The wiring code (`can_config.c`) connects the
function pointer to the getter. This follows the library's dependency-injection pattern.

The main-loop safety-net scan function `OpenLcbBufferList_check_timeouts(uint8_t current_tick)`
receives the tick as a parameter from the caller. The caller obtains the tick through the
main loop's parameter chain (ultimately from `openlcb_config.c`), not by calling the getter
directly from the buffer list module.

**The Rx context handles its own cleanup:**

- On FIRST frame: stamp `msg->timerticks = _interface->get_current_tick()` (snapshot the clock)
- On MIDDLE frame: before appending, check elapsed time:
  `(_interface->get_current_tick() - msg->timerticks) >= threshold` → free the stale buffer
- The main loop also scans under lock as a safety net for the case where no further frames
  arrive at all (no MIDDLE frame to trigger the check)

This is safe because:
- Rx context exclusively owns the buffer list (all allocations happen there)
- When the main loop frees a buffer, it does so under lock (which disables interrupts),
  preventing the Rx context from touching the buffer during cleanup
- The timer tick never touches per-buffer fields — it only increments the global clock

## 2. Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/drivers/canbus/can_rx_message_handler.c` | Stamp `timerticks` via `_interface->get_current_tick()` on FIRST frame; check elapsed time on MIDDLE frame and free stale buffers |
| `src/openlcb/openlcb_buffer_list.c` | Add `OpenLcbBufferList_check_timeouts()` for main-loop safety-net scan |
| `src/openlcb/openlcb_buffer_list.h` | Declare `OpenLcbBufferList_check_timeouts()` |
| `src/drivers/canbus/can_main_statemachine.c` | Call `OpenLcbBufferList_check_timeouts()` from main loop under lock, passing tick as parameter |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

### Tests

| File | Change |
|------|--------|
| `src/openlcb/openlcb_buffer_list_Test.cxx` | Add tests for timeout scanning and cleanup |

## 3. Implementation Steps

### Step 1. Prerequisite: global clock refactor

This plan depends on the global 100ms tick counter from `plan_global_clock_refactor.md`.
That plan adds `volatile uint8_t _global_100ms_tick` to `openlcb_config.c`, exposes
`OpenLcb_get_global_100ms_tick()` for wiring code only, and wires the
`_interface->get_current_tick()` function pointer into the Rx handler interface. The global
clock refactor must be implemented first (or concurrently). The full 8 bits wrap at 255;
subtraction handles wrap correctly for durations up to 127 ticks (~12.7 seconds).

### Step 2. Define timeout constant

In `openlcb_buffer_list.c`:

```c
    /** @brief Multi-frame assembly timeout in 100ms ticks (3 seconds). */
#define BUFFER_LIST_INPROCESS_TIMEOUT_TICKS 30
```

### Step 3. Stamp timerticks on FIRST frame

In `can_rx_message_handler.c`, in `CanRxMessageHandler_first_frame()`, after
`OpenLcbUtilities_load_openlcb_message()` sets `timerticks = 0`, overwrite it with
the current tick snapshot obtained through the interface:

```c
    target_openlcb_msg->timerticks = _interface->get_current_tick();
    target_openlcb_msg->state.inprocess = true;
```

This stamps the buffer with the current clock value. The Rx context owns this buffer
exclusively at this point. Note that `can_rx_message_handler.c` never includes
`openlcb_config.h` — it accesses the tick only through its interface function pointer.

### Step 4. Check elapsed time on MIDDLE frame

In `can_rx_message_handler.c`, in `CanRxMessageHandler_middle_frame()`, after the
`OpenLcbBufferList_find()` call succeeds, check if the buffer has gone stale:

```c
void CanRxMessageHandler_middle_frame(can_msg_t* can_msg, uint8_t offset) {

    uint16_t dest_alias = CanUtilities_extract_dest_alias_from_can_message(can_msg);
    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    uint16_t mti = CanUtilities_convert_can_mti_to_openlcb_mti(can_msg);

    openlcb_msg_t* target_openlcb_msg = OpenLcbBufferList_find(source_alias, dest_alias, mti);

    if (!target_openlcb_msg) {

        _load_reject_message(dest_alias, source_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);

        return;

    }

    uint8_t elapsed = (uint8_t) (_interface->get_current_tick() - target_openlcb_msg->timerticks);

    if (elapsed >= CAN_RX_INPROCESS_TIMEOUT_TICKS) {

        // Stale assembly — free and reject
        OpenLcbBufferList_release(target_openlcb_msg);
        _interface->openlcb_buffer_store_free_buffer(target_openlcb_msg);

        _load_reject_message(dest_alias, source_alias, mti, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);

        return;

    }

    CanUtilities_append_can_payload_to_openlcb_payload(target_openlcb_msg, can_msg, offset);

}
```

**Note:** The `CAN_RX_INPROCESS_TIMEOUT_TICKS` constant needs to be accessible here.
Since this is a CAN-transport-level timeout, defining it locally in `can_rx_message_handler.c`
is appropriate — the buffer list module can use its own constant independently.

In `can_rx_message_handler.c`:

```c
    /** @brief Multi-frame assembly timeout in 100ms ticks (3 seconds). */
#define CAN_RX_INPROCESS_TIMEOUT_TICKS 30
```

### Step 5. Implement main-loop safety-net scan

For the case where no further frames arrive at all (sender crashed after FIRST frame, no
MIDDLE frame ever comes to trigger the Rx-context check), the main loop periodically scans
the buffer list under lock:

In `openlcb_buffer_list.c`:

```c
    /**
     * @brief Scan for stale in-progress buffers and free them (safety net).
     *
     * @details Algorithm:
     * -# Iterate all slots in the buffer list
     * -# For each non-NULL entry with state.inprocess == true:
     *    -# Compute elapsed = (current_tick - msg->timerticks)
     *    -# If elapsed >= threshold:
     *       -# Remove from buffer list (NULL the slot)
     *       -# Free the buffer back to the store
     *
     * @param current_tick  The current value of the global 100ms tick counter,
     *        passed as a parameter by the caller (not read from the global
     *        clock directly).
     *
     * @note The caller MUST hold the shared resource lock before calling this
     *       function. This prevents the Rx interrupt from touching a buffer
     *       that is being freed.
     */
void OpenLcbBufferList_check_timeouts(uint8_t current_tick) {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        openlcb_msg_t *msg = _openlcb_msg_buffer_list[i];

        if (msg && msg->state.inprocess) {

            uint8_t elapsed = (uint8_t) (current_tick - msg->timerticks);

            if (elapsed >= BUFFER_LIST_INPROCESS_TIMEOUT_TICKS) {

                _openlcb_msg_buffer_list[i] = NULL;
                OpenLcbBufferStore_free_buffer(msg);

            }

        }

    }

}
```

### Step 6. Declare in header

In `openlcb_buffer_list.h`:

```c
        /**
         * @brief Scans for stale in-progress buffers and frees them.
         *
         * @details Caller MUST hold the shared resource lock before calling.
         * Frees any in-progress message whose elapsed time (computed from the
         * global 100ms tick) has reached the timeout threshold.
         *
         * @param current_tick  Current value of the global 100ms tick counter,
         *        passed as a parameter from the main loop.
         */
    extern void OpenLcbBufferList_check_timeouts(uint8_t current_tick);
```

### Step 7. Wire the main-loop safety-net check

In `can_main_statemachine.c` (or the higher-level run loop), call under lock. The tick
is obtained through the main loop's parameter chain — the caller passes it down rather
than calling `OpenLcb_get_global_100ms_tick()` directly from this module:

```c
    _interface->lock_shared_resources();
    OpenLcbBufferList_check_timeouts(tick);
    _interface->unlock_shared_resources();
```

Where `tick` is the current tick value received as a parameter from the caller (ultimately
originating from the single read of `_global_100ms_tick` in `openlcb_config.c` at the top
of the main loop iteration, as described in `plan_global_clock_refactor.md`).

This should run every main loop iteration. The lock prevents the Rx interrupt from
allocating or appending to buffers while the scan is running.

### Step 8. Confirm timerticks is stamped on FIRST frame

In `can_rx_message_handler.c`, `CanRxMessageHandler_first_frame()` calls
`OpenLcbUtilities_load_openlcb_message()` which sets `timerticks = 0`. Then Step 3 above
overwrites it with `_interface->get_current_tick()`. Then `state.inprocess = true` is set.
Correct.

### Step 9. Confirm LAST frame clears inprocess

In `CanRxMessageHandler_last_frame()` (line 258): `target_openlcb_msg->state.inprocess = false`.
Then the buffer is released from the list and pushed to the FIFO. Once `state.inprocess` is
false, the safety-net scan skips it. Correct.

## 4. Test Changes

All changes are in `src/openlcb/openlcb_buffer_list_Test.cxx`.

### Test 1: Stale in-progress buffer reclaimed after timeout

```cpp
TEST(OpenLcbBufferList, timeout_reclaims_stale_inprocess)
{
    // Setup: allocate buffer, stamp timerticks = 0, state.inprocess = true, add to list
    // Call check_timeouts(current_tick = 30)
    // Verify buffer removed from list and freed
}
```

### Test 2: Non-in-process buffers not touched

```cpp
TEST(OpenLcbBufferList, timeout_ignores_non_inprocess)
{
    // Setup: allocate buffer, state.inprocess = false, add to list
    // Call check_timeouts(current_tick = 100)
    // Verify buffer still in list
}
```

### Test 3: No timeout before threshold

```cpp
TEST(OpenLcbBufferList, no_timeout_before_threshold)
{
    // Setup: in-progress buffer stamped at tick = 10
    // Call check_timeouts(current_tick = 39)  // elapsed = 29
    // Verify buffer still in list
}
```

### Test 4: Wrap-around arithmetic works

```cpp
TEST(OpenLcbBufferList, timeout_wraps_correctly)
{
    // Setup: in-progress buffer stamped at tick = 240
    // Call check_timeouts(current_tick = 15)  // elapsed = (15 - 240) = 31 unsigned
    // Verify buffer IS timed out (elapsed 31 >= 30)
}
```

### Test 5: LAST frame completes before timeout

```cpp
TEST(OpenLcbBufferList, completed_buffer_not_timed_out)
{
    // Setup: in-progress buffer stamped at tick = 0
    // Set state.inprocess = false (simulating LAST frame)
    // Call check_timeouts(current_tick = 100)
    // Verify buffer still in list (not freed — inprocess is false)
}
```

### Verification procedure

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

## 5. Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| Rx interrupt / main-loop race | NONE | Main loop holds lock during safety-net scan. Lock disables interrupts, preventing Rx handler from touching buffers during cleanup. |
| Rx appending to buffer being freed by main loop | NONE | Lock is held during free. Rx interrupt cannot fire. After unlock, the slot is NULL and `OpenLcbBufferList_find()` in MIDDLE/LAST frame handlers will return NULL, triggering an out-of-order rejection — correct behavior for a timed-out assembly. |
| Timer tick touching buffers | NONE | Timer tick only increments `_global_100ms_tick`. Never touches per-buffer fields. |
| timerticks wrap-around | NONE | `uint8_t` subtraction handles wrap correctly. Max timeout is 30 ticks, well within the 127-tick safe range for unsigned subtraction. |
| Freeing buffer still referenced | VERY LOW | Only in-progress buffers are candidates. Once LAST frame arrives, `state.inprocess` is cleared and the buffer is released from the list before the next scan. |
| Performance | LOW | Main-loop scan: linear scan of LEN_MESSAGE_BUFFER slots (typically small, 10-20). Rx check: single subtraction per MIDDLE frame. Negligible cost. |
| Dependency on global clock refactor | INFO | Requires the global clock from `plan_global_clock_refactor.md`. The global clock refactor should be implemented first as a prerequisite. |

## 6. Estimated Effort

| Task | Time |
|------|------|
| Define timeout constant | 2 min |
| Stamp timerticks on FIRST frame | 5 min |
| Add elapsed-time check on MIDDLE frame | 10 min |
| Implement check_timeouts in buffer_list.c | 10 min |
| Declare in header | 5 min |
| Wire check_timeouts into main loop | 10 min |
| Add 5 tests | 20 min |
| Build and run full test suite | 5 min |
| **Total** | **~67 min** |

## 7. Dependencies

- Requires the global 100ms tick counter from `plan_global_clock_refactor.md`. That plan
  adds `volatile uint8_t _global_100ms_tick` to `openlcb_config.c`, exposes
  `OpenLcb_get_global_100ms_tick()` for wiring code only, and wires the
  `_interface->get_current_tick()` function pointer into the Rx handler interface.
- If the global clock refactor has not been implemented yet, it should be done first
  as a prerequisite.
- `can_rx_message_handler.c` never includes `openlcb_config.h` — it accesses the tick
  only through `_interface->get_current_tick()`.
- `openlcb_buffer_list.c` never calls `OpenLcb_get_global_100ms_tick()` directly — it
  receives `current_tick` as a function parameter from the caller.

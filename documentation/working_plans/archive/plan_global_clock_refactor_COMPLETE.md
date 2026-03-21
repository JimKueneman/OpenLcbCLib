<!--
  ============================================================
  STATUS: IMPLEMENTED
  The global `_global_100ms_tick` counter lives in `openlcb_config.c`, `OpenLcb_100ms_timer_tick()` is a single increment, and `_run_periodic_services()` passes the tick to all module timer functions with the correct `uint8_t current_tick` signature.
  ============================================================
-->

# Plan: Global Clock Refactor — Move All Timer Work to the Main Loop

## 1. Summary

**Problem:** `OpenLcb_100ms_timer_tick()` runs from the timer interrupt and currently does
heavy work:

1. **`OpenLcbNode_100ms_timer_tick()`** — iterates ALL allocated nodes and increments
   `node[i].timerticks++` (a `uint16_t`). On 8-bit PICs, this is a two-byte read-modify-write
   — **NOT atomic**. The main-loop login statemachine reads `node->timerticks` concurrently,
   creating a torn-read risk.

2. **`OpenLcbApplicationTrain_100ms_timer_tick()`** — iterates train pool, decrements
   counters, and calls `_send_heartbeat_request()` which **sends a CAN message**. If the CAN
   transmit buffer is full, the send blocks — **stalling the timer interrupt**. Also calls
   `_interface->on_heartbeat_timeout()`, invoking application code from interrupt context.

3. **`OpenLcbApplicationBroadcastTime_100ms_time_tick()`** — iterates broadcast clocks,
   accumulates time, advances minutes, and calls `_interface->on_time_changed()` — application
   code running from interrupt context.

4. **`ProtocolDatagramHandler_100ms_timer_tick()`** — currently empty, but Item 2 plans to
   add datagram timeout tracking here.

**Fix:** Replace all per-item increment loops and work with a single global atomic clock.
The timer interrupt does ONE thing: increment `volatile uint8_t _global_100ms_tick`. All real
work moves to the main loop via `OpenLcb_run()`.

### Design Principles

```
Timer interrupt context:
    _global_100ms_tick++        ← single atomic byte, can never stall

Main loop context (OpenLcb_run):
    tick = _global_100ms_tick   ← read once
    pass tick down to each module as a parameter
    modules compute elapsed time via subtraction: (tick - snapshot)
    modules do all real work: send messages, fire callbacks, free buffers
```

**No module coupling:** `_global_100ms_tick` is private to `openlcb_config.c`. Modules
never call `OpenLcb_get_global_100ms_tick()` directly. Instead:
- Main-loop modules receive `uint8_t current_tick` as a **function parameter**
- The login statemachine receives the tick through `can_statemachine_info_t`
- The Rx handler (Item 7) receives the tick through a `get_current_tick` **interface
  function pointer**

Only the wiring code (`openlcb_config.c`, `can_config.c`) reads the global clock and
injects it into the modules. This follows the same dependency-injection pattern used
throughout the library.

**Platform safety:** A `volatile uint8_t` read/write is a single-instruction operation on
ALL target architectures:
- 8-bit PIC: single-cycle byte operation
- 16-bit PIC24/dsPIC: byte operation is atomic
- 32-bit ARM Cortex-M: LDRB/STRB single instruction
- 64-bit ARM Cortex-A: same

The `_global_100ms_tick++` increment is a read-modify-write, which is NOT atomic. However,
only the timer interrupt context performs the increment, and timer interrupts do not re-enter
themselves. Every other context only reads the counter.

**Wrap-around safety:** `uint8_t` arithmetic wraps at 256 (mod 256). Unsigned subtraction
`(current - snapshot)` yields the correct elapsed tick count for durations up to 255 ticks
(25.5 seconds). All timeouts in the library are well within this range:
- Login wait: 200ms (2 ticks)
- Datagram timeout: 3 seconds (30 ticks)
- Multi-frame assembly timeout: 3 seconds (30 ticks)
- Heartbeat: typically 3 seconds (30 ticks), max ~25 seconds safe range

## 2. Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/openlcb/openlcb_config.c` | Add `volatile uint8_t _global_100ms_tick`; reduce `OpenLcb_100ms_timer_tick()` to single increment; read tick once in `OpenLcb_run()` and pass to `_run_periodic_services(tick)` |
| `src/openlcb/openlcb_config.h` | Declare `OpenLcb_get_global_100ms_tick()` (used only by wiring code, not by modules) |
| `src/openlcb/openlcb_node.c` | Remove per-node `timerticks++` loop; change `OpenLcbNode_100ms_timer_tick(uint8_t current_tick)` signature |
| `src/openlcb/openlcb_node.h` | Update function declaration with `uint8_t current_tick` parameter |
| `src/drivers/canbus/can_types.h` | Add `uint8_t current_tick` to `can_statemachine_info_t` |
| `src/drivers/canbus/can_main_statemachine.c` | Set `current_tick` in `_can_statemachine_info` before login statemachine runs |
| `src/drivers/canbus/can_login_message_handler.c` | Use `can_statemachine_info->current_tick` for snapshot and elapsed check |
| `src/openlcb/openlcb_application_train.c` | Change to `OpenLcbApplicationTrain_100ms_timer_tick(uint8_t current_tick)` |
| `src/openlcb/openlcb_application_train.h` | Update function declaration |
| `src/openlcb/openlcb_application_broadcast_time.c` | Change to `OpenLcbApplicationBroadcastTime_100ms_time_tick(uint8_t current_tick)` |
| `src/openlcb/openlcb_application_broadcast_time.h` | Update function declaration |
| `src/openlcb/protocol_datagram_handler.c` | Change to `ProtocolDatagramHandler_100ms_timer_tick(uint8_t current_tick)` |
| `src/openlcb/protocol_datagram_handler.h` | Update function declaration |
| `src/drivers/canbus/can_rx_message_handler.h` | Add `uint8_t (*get_current_tick)(void)` to Rx handler interface |
| `src/drivers/canbus/can_rx_message_handler.c` | Use `_interface->get_current_tick()` to stamp FIRST frames (Item 7) |
| `src/drivers/canbus/can_config.c` | Wire `get_current_tick` to `OpenLcb_get_global_100ms_tick` |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

### Tests

| File | Change |
|------|--------|
| `src/openlcb/openlcb_node_Test.cxx` | Update timer tick tests for new signature |
| `src/drivers/canbus/can_login_message_handler_Test.cxx` | Update 200ms wait tests for `current_tick` in statemachine info |
| `src/openlcb/openlcb_application_train_Test.cxx` | Update heartbeat tests for new signature |

## 3. Implementation Steps

### Step 1. Add the global 100ms tick counter

In `openlcb_config.c`, add after the static declarations:

```c
    /**
     * @brief Global 100ms tick counter — the sole action of the timer interrupt.
     *
     * @details This counter is the library's timekeeping foundation. It is
     * incremented once per 100ms timer tick (which may be an interrupt or a
     * separate thread depending on the platform).
     *
     * ALL other modules compute elapsed time by snapshotting this counter at a
     * start event and later subtracting: elapsed = (current - snapshot). The
     * uint8_t wraps at 256, and unsigned subtraction handles the wrap correctly
     * for durations up to 255 ticks (25.5 seconds).
     *
     * Platform safety: volatile uint8_t reads and writes are single-instruction
     * operations on all target architectures (8-bit PIC through 64-bit ARM).
     * Only the timer interrupt performs the increment (read-modify-write); all
     * other contexts only read. Timer interrupts do not re-enter, so the
     * increment is safe without locking.
     *
     * Module isolation: no module calls this directly. The wiring code reads it
     * once per main-loop iteration and passes the value down as a parameter.
     * The Rx handler receives it via an interface function pointer. This
     * preserves the library's dependency-injection pattern.
     */
static volatile uint8_t _global_100ms_tick = 0;
```

Add a getter (used only by wiring code — `openlcb_config.c` and `can_config.c`):

```c
uint8_t OpenLcb_get_global_100ms_tick(void) {

    return _global_100ms_tick;

}
```

In `openlcb_config.h`, declare:

```c
    /**
     * @brief Returns the current value of the global 100ms tick counter.
     *
     * @details Used by wiring code (openlcb_config.c, can_config.c) to read
     * the clock and inject it into modules via parameters or interface
     * function pointers. Individual modules should NOT call this directly —
     * they receive the tick through their function parameters or interface.
     *
     * @return Current tick count (wraps at 255).
     */
extern uint8_t OpenLcb_get_global_100ms_tick(void);
```

### Step 2. Reduce the timer interrupt to one line

Replace the current `OpenLcb_100ms_timer_tick()`:

```c
void OpenLcb_100ms_timer_tick(void) {

    _global_100ms_tick++;

}
```

### Step 3. Move periodic service into the main loop

Add a new internal function and call it from `OpenLcb_run()`:

```c
    /**
     * @brief Runs all periodic service tasks from the main loop.
     *
     * @details Reads the global clock once and passes it to each module.
     * All work happens in the main loop context where it is safe to send
     * messages, free buffers, and call application callbacks.
     */
static void _run_periodic_services(void) {

    uint8_t tick = _global_100ms_tick;

    OpenLcbNode_100ms_timer_tick(tick);

#ifdef OPENLCB_COMPILE_DATAGRAMS
    ProtocolDatagramHandler_100ms_timer_tick(tick);
#endif

#ifdef OPENLCB_COMPILE_BROADCAST_TIME
    OpenLcbApplicationBroadcastTime_100ms_time_tick(tick);
#endif

#ifdef OPENLCB_COMPILE_TRAIN
    OpenLcbApplicationTrain_100ms_timer_tick(tick);
#endif

}
```

And modify `OpenLcb_run()`:

```c
void OpenLcb_run(void) {

    CanMainStateMachine_run();
    OpenLcbLoginMainStatemachine_run();
    OpenLcbMainStatemachine_run();

    _run_periodic_services();

}
```

### Step 4. Convert OpenLcbNode_100ms_timer_tick()

Change signature to accept the tick. Remove the per-node increment loop. Gate the app
callback so it fires at most once per real tick:

In `openlcb_node.h`:

```c
    extern void OpenLcbNode_100ms_timer_tick(uint8_t current_tick);
```

In `openlcb_node.c`:

```c
static uint8_t _last_app_callback_tick = 0;

void OpenLcbNode_100ms_timer_tick(uint8_t current_tick) {

    if ((uint8_t)(current_tick - _last_app_callback_tick) == 0) { return; }

    _last_app_callback_tick = current_tick;

    if (_interface && _interface->on_100ms_timer_tick) {

        _interface->on_100ms_timer_tick();

    }

}
```

### Step 5. Pass the tick to the login statemachine

Add `uint8_t current_tick` to `can_statemachine_info_t` (in `can_types.h` or wherever the
struct is defined):

```c
typedef struct {

    openlcb_node_t *openlcb_node;
    can_msg_t *login_outgoing_can_msg;
    bool login_outgoing_can_msg_valid;
    uint8_t current_tick;

} can_statemachine_info_t;
```

In `can_main_statemachine.c`, before calling the login statemachine, set the tick:

```c
    _can_statemachine_info.current_tick = OpenLcb_get_global_100ms_tick();
    _interface->login_statemachine_run(&_can_statemachine_info);
```

**Note:** `can_main_statemachine.c` is wiring-level code (it already includes openlcb_config
headers indirectly). Alternatively, the tick can be passed from `OpenLcb_run()` through the
`CanMainStateMachine_run()` interface. The exact wiring path depends on the existing include
structure — the goal is that `can_login_message_handler.c` (the leaf module) never includes
`openlcb_config.h`.

In `can_login_message_handler.c`:

**State 7 (load_cid04):** Snapshot the tick:

```c
void CanLoginMessageHandler_state_load_cid04(can_statemachine_info_t *can_statemachine_info) {

    // ... existing CID4 frame loading ...

    can_statemachine_info->openlcb_node->timerticks = can_statemachine_info->current_tick;
    can_statemachine_info->login_outgoing_can_msg_valid = true;

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_WAIT_200ms;

}
```

**State 8 (wait_200ms):** Check elapsed:

```c
void CanLoginMessageHandler_state_wait_200ms(can_statemachine_info_t *can_statemachine_info) {

    uint8_t elapsed = (uint8_t)(can_statemachine_info->current_tick
            - (uint8_t) can_statemachine_info->openlcb_node->timerticks);

    if (elapsed > 2) {

        can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_RESERVE_ID;

    }

}
```

No coupling — `can_login_message_handler.c` only reads from the struct it already receives.

### Step 6. Wire get_current_tick into the Rx handler interface (for Item 7)

In the Rx handler interface struct (in `can_rx_message_handler.h`), add:

```c
    /** @brief Returns the current global 100ms tick. Used to stamp incoming buffers. */
uint8_t (*get_current_tick)(void);
```

In `can_config.c`, wire it:

```c
    _rx_handler.get_current_tick = &OpenLcb_get_global_100ms_tick;
```

Then in `can_rx_message_handler.c`, FIRST frame stamping (Item 7) becomes:

```c
    target_openlcb_msg->timerticks = _interface->get_current_tick();
```

No coupling — the Rx handler calls through its injected interface, same as every other
dependency.

### Step 7. Convert the train heartbeat timer

Change signature in `openlcb_application_train.h`:

```c
extern void OpenLcbApplicationTrain_100ms_timer_tick(uint8_t current_tick);
```

In `openlcb_application_train.c`:

```c
static uint8_t _last_heartbeat_tick = 0;

void OpenLcbApplicationTrain_100ms_timer_tick(uint8_t current_tick) {

    uint8_t ticks_elapsed = (uint8_t)(current_tick - _last_heartbeat_tick);

    if (ticks_elapsed == 0) { return; }

    _last_heartbeat_tick = current_tick;

    for (uint8_t i = 0; i < _train_pool_count; i++) {

        train_state_t *state = &_train_pool[i];

        if (state->heartbeat_timeout_s == 0) {

            continue;

        }

        if (state->heartbeat_counter_100ms > ticks_elapsed) {

            state->heartbeat_counter_100ms -= ticks_elapsed;

        } else {

            state->heartbeat_counter_100ms = 0;

        }

        uint32_t halfway = (state->heartbeat_timeout_s * 10) / 2;

        if (state->heartbeat_counter_100ms <= halfway &&
            (state->heartbeat_counter_100ms + ticks_elapsed) > halfway) {

            _send_heartbeat_request(state);

        }

        if (state->heartbeat_counter_100ms == 0) {

            state->estop_active = true;

            bool reverse = OpenLcbFloat16_get_direction(state->set_speed);
            state->set_speed = reverse ? FLOAT16_NEGATIVE_ZERO : FLOAT16_POSITIVE_ZERO;

            if (_interface && _interface->on_heartbeat_timeout) {

                _interface->on_heartbeat_timeout(state->owner_node);

            }

        }

    }

}
```

### Step 8. Convert the broadcast time timer

Change signature in `openlcb_application_broadcast_time.h`:

```c
extern void OpenLcbApplicationBroadcastTime_100ms_time_tick(uint8_t current_tick);
```

In `openlcb_application_broadcast_time.c`:

```c
static uint8_t _last_bcast_tick = 0;

void OpenLcbApplicationBroadcastTime_100ms_time_tick(uint8_t current_tick) {

    uint8_t ticks_elapsed = (uint8_t)(current_tick - _last_bcast_tick);

    if (ticks_elapsed == 0) { return; }

    _last_bcast_tick = current_tick;

    for (int i = 0; i < BROADCAST_TIME_TOTAL_CLOCK_COUNT; i++) {

        broadcast_clock_t *clock = &_clocks[i];

        if (!clock->is_allocated || !clock->is_consumer || !clock->state.is_running) {

            continue;

        }

        int16_t rate = clock->state.rate.rate;

        if (rate == 0) {

            continue;

        }

        uint16_t abs_rate = (rate < 0) ? (uint16_t)(-rate) : (uint16_t)(rate);

        clock->state.ms_accumulator += (uint32_t)(100) * abs_rate * ticks_elapsed;

        while (clock->state.ms_accumulator >= BROADCAST_TIME_MS_PER_MINUTE_FIXED_POINT) {

            clock->state.ms_accumulator -= BROADCAST_TIME_MS_PER_MINUTE_FIXED_POINT;

            if (rate > 0) {

                _advance_minute_forward(&clock->state, NULL);

            } else {

                _advance_minute_backward(&clock->state, NULL);

            }

            if (_interface && _interface->on_time_changed) {

                _interface->on_time_changed(clock);

            }

        }

    }

}
```

### Step 9. Convert the datagram handler timer tick

Change signature in `protocol_datagram_handler.h`:

```c
extern void ProtocolDatagramHandler_100ms_timer_tick(uint8_t current_tick);
```

For now the function remains a no-op (Item 2 will add the timeout logic):

```c
void ProtocolDatagramHandler_100ms_timer_tick(uint8_t current_tick) {

    // Datagram timeout handling added by Item 2.

}
```

### Step 10. Remove timer calls from application drivers (informational)

Several application driver files call `OpenLcbNode_100ms_timer_tick()` and
`ProtocolDatagramHandler_100ms_timer_tick()` directly from their hardware timer ISR. After
this refactor, these calls must be replaced with `OpenLcb_100ms_timer_tick()` only.

**Affected application driver call sites:**
- `esp32_drivers.c:64-65` — direct calls to Node + Datagram timer ticks
- `osx_drivers.c:119-120` — direct calls to Node + Datagram timer ticks
- `ti_driverlib_drivers.c:124-125` — direct calls to Node + Datagram timer ticks
- `stm32_driverlib_drivers.c:130-131` — direct calls to Node + Datagram timer ticks
- `drivers.c:242-243` (dsPIC) — direct calls to Node + Datagram timer ticks

These should all be replaced with a single `OpenLcb_100ms_timer_tick()` call.

## 4. Test Changes

### Test updates for openlcb_node_Test.cxx

Existing timer tick tests that verify `node->timerticks` increments will need to be updated
or removed. Update call sites to pass `current_tick` parameter. Add a test verifying the
application callback fires only once per unique tick value.

### Test updates for can_login_message_handler_Test.cxx

Update the 200ms wait test. Set `can_statemachine_info.current_tick` directly in the test
instead of relying on the node timer tick.

### Test updates for openlcb_application_train_Test.cxx

Update heartbeat tests to pass `current_tick` to the timer tick function. Verify that
`_send_heartbeat_request` fires at the halfway point and estop fires at zero.

### Verification procedure

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

## 5. Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| **Timer interrupt stalling on CAN send** | **ELIMINATED** | Timer interrupt no longer sends messages. Heartbeat request and broadcast time callbacks now run from main loop where blocking on a full transmit buffer is safe. |
| **Non-atomic `uint16_t` increment on 8-bit PIC** | **ELIMINATED** | Timer interrupt no longer increments per-node `uint16_t timerticks`. Only increments a single `volatile uint8_t`. |
| **Application callbacks from interrupt context** | **ELIMINATED** | `on_heartbeat_timeout`, `on_time_changed`, and `on_100ms_timer_tick` all now fire from main loop context. No reentrancy risk. |
| **Module coupling to openlcb_config** | **ELIMINATED** | No module calls `OpenLcb_get_global_100ms_tick()` directly. The tick is passed as a function parameter or through interface function pointers. Only wiring code (openlcb_config.c, can_config.c) reads the global clock. |
| **Timer tick jitter / missed ticks** | LOW | If the main loop is slow, periodic services may not run for several 100ms periods. The `ticks_elapsed` pattern handles this correctly — it processes the accumulated elapsed time in a single pass. No ticks are lost, though timing precision degrades to main-loop granularity. |
| **Heartbeat halfway detection across multiple ticks** | LOW | If multiple ticks elapse between polls, the halfway point could be crossed without exact detection. The implementation checks whether the halfway point was within the elapsed interval. Alternatively, a `heartbeat_request_sent` flag simplifies this. |
| **Broadcast time accumulator overflow** | VERY LOW | If many ticks elapse at once, `100 * abs_rate * ticks_elapsed` could overflow `uint32_t` for very high rates and long gaps. In practice, `ticks_elapsed` is small (main loop runs in microseconds) and rates are typically 1-16x. |
| **`uint8_t` wrap-around** | NONE | Unsigned subtraction handles wrap correctly. Max measurable duration is 255 ticks (25.5 seconds). All library timeouts are well within range. |
| **`on_100ms_timer_tick` callback timing change** | LOW | Application callback previously fired exactly once per 100ms from interrupt context. Now fires from main loop, gated by tick check. Timing precision depends on main-loop speed — in practice much faster than 100ms. |
| **Application driver files need manual update** | MEDIUM | Five platform-specific driver files call individual timer tick functions directly from their ISR. These must be changed to `OpenLcb_100ms_timer_tick()` only. Failure to update will cause compile errors (function signatures changed), which is actually a good thing — it forces the update. |
| **Existing tests depend on per-node timerticks increment** | MEDIUM | Tests that call `OpenLcbNode_100ms_timer_tick()` and check `node->timerticks` will fail. Tests must be updated for new signature and behavior. |
| **node->timerticks field type mismatch** | VERY LOW | `node->timerticks` is `uint16_t` but global clock is `uint8_t`. The snapshot uses only 8 bits; the upper byte is unused. A future cleanup could shrink to `uint8_t`. |
| **Heartbeat timeout > 25.5 seconds** | NONE | The heartbeat still uses its own `heartbeat_counter_100ms` (uint32_t) decrementing counter. The global clock only gates WHEN the decrement runs. Long-duration countdown is in the counter itself, unaffected by the 8-bit clock. |
| **Function signature changes break compilation** | MEDIUM (intentional) | All `_100ms_timer_tick()` functions gain a `uint8_t current_tick` parameter. Any code that calls the old signature will fail to compile. This is desirable — it forces all call sites to be updated, preventing silent use of the old interrupt-context pattern. |

## 6. Dependency Graph

This refactor is a **prerequisite** for:
- **Item 2** (Datagram Timeout) — receives `current_tick` parameter
- **Item 7** (Multi-Frame Assembly Timeout) — uses `get_current_tick` interface pointer

Implementation order:
1. **This plan** (Global Clock Refactor)
2. Item 1 (Reply Pending Flag) — independent, can be done anytime
3. Item 2 + 6 (Datagram Timeout + Retry)
4. Item 7 (Multi-Frame Assembly Timeout)
5. Remaining items (9, 5, 8, 10, 3, etc.)

## 7. Estimated Effort

| Task | Time |
|------|------|
| Add global clock to openlcb_config | 5 min |
| Reduce timer interrupt to single increment | 5 min |
| Add `_run_periodic_services()` in `OpenLcb_run()` | 5 min |
| Convert `OpenLcbNode_100ms_timer_tick(tick)` | 10 min |
| Add `current_tick` to `can_statemachine_info_t` | 5 min |
| Convert login 200ms wait | 10 min |
| Wire `get_current_tick` into Rx handler interface | 5 min |
| Convert train heartbeat timer | 20 min |
| Convert broadcast time timer | 15 min |
| Convert datagram handler timer tick signature | 5 min |
| Update node tests | 15 min |
| Update login tests | 10 min |
| Update train heartbeat tests | 15 min |
| Build and run full test suite | 5 min |
| **Total** | **~130 min** |

## 8. Summary of What Changes

### Before (current)

```
Timer interrupt:
    for each node: node->timerticks++          ← non-atomic uint16_t on 8-bit PIC
    for each train: decrement counter, SEND HEARTBEAT MSG, fire callbacks
    for each clock: advance time, fire callbacks
    datagram handler: (empty)

Main loop:
    CanMainStateMachine_run()
    OpenLcbLoginMainStatemachine_run()
    OpenLcbMainStatemachine_run()
```

### After (refactored)

```
Timer interrupt:
    _global_100ms_tick++                       ← single atomic byte, can never stall

Main loop:
    CanMainStateMachine_run()                  ← login statemachine gets tick via struct
    OpenLcbLoginMainStatemachine_run()
    OpenLcbMainStatemachine_run()
    _run_periodic_services()                   ← reads tick once, passes to all modules
        OpenLcbNode_100ms_timer_tick(tick)       → app callback only, gated by tick
        ProtocolDatagramHandler_100ms_timer_tick(tick) → (Item 2 adds timeout check)
        OpenLcbApplicationBroadcastTime_100ms_time_tick(tick) → clock advancement
        OpenLcbApplicationTrain_100ms_timer_tick(tick) → heartbeat, safe to send msgs

Rx handler (interrupt context):
    _interface->get_current_tick()             ← reads tick via interface pointer
    stamps msg->timerticks on FIRST frame      ← (Item 7 adds stale-buffer check)
```

### Coupling diagram

```
openlcb_config.c          ← owns _global_100ms_tick, reads it, passes down
    │
    ├─ parameter ──→ OpenLcbNode_100ms_timer_tick(tick)
    ├─ parameter ──→ ProtocolDatagramHandler_100ms_timer_tick(tick)
    ├─ parameter ──→ OpenLcbApplicationBroadcastTime_100ms_time_tick(tick)
    ├─ parameter ──→ OpenLcbApplicationTrain_100ms_timer_tick(tick)
    │
can_config.c              ← wires getter into Rx interface
    │
    └─ interface ──→ can_rx_message_handler: _interface->get_current_tick()

can_main_statemachine.c   ← sets current_tick in can_statemachine_info_t
    │
    └─ struct ────→ can_login_message_handler: info->current_tick
```

No module includes `openlcb_config.h`. No module calls `OpenLcb_get_global_100ms_tick()`.

<!--
  ============================================================
  STATUS: IMPLEMENTED
  `verify_ticks` and `verify_pending` fields are in `listener_alias_entry_t`; `ListenerAliasTable_check_one_verification()` and `CanMainStatemachine_handle_listener_verification()` are wired into the CAN state machine loop.
  ============================================================
-->

# Periodic Listener Alias Verification — Implementation Plan

**Date:** 8 Mar 2026
**Status:** COMPLETED
**Depends on:** Listener alias table CAN integration (completed)

---

## Problem

If a listener node disappears without sending AMR (power loss, crash), the
listener alias table retains a stale alias.  Forwarded consist commands
silently go to a nonexistent node — fire-and-forget forwarding has no
acknowledgment.

---

## Solution Overview

Add a distributed prober to `alias_mapping_listener.c` that round-robins
through the listener alias table one entry at a time.  Each tick of the
prober:

1. Advances a static cursor to the next registered entry with a resolved alias
2. Checks if the entry's probe interval has elapsed
3. If due: queues a targeted AME for that entry's Node ID and marks it
   `verify_pending`
4. If already `verify_pending` and the verification timeout has elapsed:
   the listener is stale — clear its alias (force re-resolution on next AMD)

**One probe per call.**  The caller controls how often to call the prober
(default: every 100ms tick, configurable via
`USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL`).  This spreads AME traffic
across time — no burst of AMEs at once.

**AMD response handling** is already wired: `ListenerAliasTable_set_alias()`
is called from `CanRxMessageHandler_amd_frame()`.  We only need to clear
`verify_pending` when set_alias is called.

---

## Files Modified

| File | Change |
|------|--------|
| `can_types.h` | Add `verify_ticks`, `verify_pending` to `listener_alias_entry_t` |
| `alias_mapping_listener.h` | Declare `ListenerAliasTable_check_one_verification()` |
| `alias_mapping_listener.c` | Implement prober + static cursor; update `initialize`, `set_alias`, `clear_alias_by_alias`, `flush_aliases`, `unregister` to clear new fields |
| `templates/openlcb_user_config.h` | Add `USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL`, `USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS`, `USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS` |
| `can_main_statemachine.h` | Add `handle_listener_verification` to interface struct |
| `can_main_statemachine.c` | Add handler function with lock/unlock; call from `run()` at lowest priority |
| `openlcb_config.c` | Wire new handler in CAN main statemachine interface |
| `alias_mapping_listener_Test.cxx` | Verification tests |

---

## Step 1 — Expand `listener_alias_entry_t`

**File:** `can_types.h` (line ~209)

**WAS:**
```c
typedef struct listener_alias_entry_struct {
    node_id_t node_id; /**< @brief Listener Node ID. 0 = unused. */
    uint16_t alias;    /**< @brief Resolved CAN alias (0 = not yet resolved). */
} listener_alias_entry_t;
```

**IS:**
```c
typedef struct listener_alias_entry_struct {
    node_id_t node_id;         /**< @brief Listener Node ID. 0 = unused. */
    uint16_t alias;            /**< @brief Resolved CAN alias (0 = not yet resolved). */
    uint8_t verify_ticks;      /**< @brief Tick snapshot when last probed or confirmed. */
    uint8_t verify_pending : 1;/**< @brief AME sent, awaiting AMD reply. */
} listener_alias_entry_t;
```

**Rationale:** `verify_ticks` stores the tick snapshot when the entry was last
confirmed (AMD arrived) or probed (AME sent).  `verify_pending` distinguishes
"waiting for AMD reply" from "idle resolved."  Adding 2 bytes per entry is
negligible (default table = 24 entries = 48 extra bytes total).

---

## Step 2 — Add user-configurable timing constants

**File:** `templates/openlcb_user_config.h`

Add after the train protocol section:

```c
// =============================================================================
// Listener Alias Verification (requires OPENLCB_COMPILE_TRAIN)
// =============================================================================
// LISTENER_PROBE_TICK_INTERVAL  -- how many 100ms ticks between prober calls
//                                  (1 = every 100ms, 2 = every 200ms, etc.)
// LISTENER_PROBE_INTERVAL_TICKS -- 100ms ticks between probes of the SAME entry
//                                  (250 = 25 seconds)
// LISTENER_VERIFY_TIMEOUT_TICKS -- 100ms ticks to wait for AMD reply before
//                                  declaring stale (30 = 3 seconds)

#define USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL    1
#define USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS   250
#define USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS   30
```

**Explanation:**
- `PROBE_TICK_INTERVAL` = 1: the prober is eligible to run every 100ms
  tick.  Set to 5 for every 500ms, 10 for every 1s, etc.  This controls
  how often we send *any* AME — at most one per this interval.
- `PROBE_INTERVAL_TICKS` = 250: each individual entry is re-probed every
  25 seconds.  With 24 entries probed round-robin at 100ms intervals, one
  full cycle takes 2.4s — well under 25s, so every entry gets probed
  several times per interval.
- `VERIFY_TIMEOUT_TICKS` = 30: if no AMD arrives within 3 seconds of
  sending AME, clear the alias (entry goes back to "registered but
  unresolved").

---

## Step 3 — Implement the prober

**File:** `alias_mapping_listener.c`

### New static state (add after `_table` declaration):

```c
/** @brief Round-robin cursor for distributed verification. */
static uint16_t _verify_cursor = 0;

/** @brief Tick snapshot for rate-limiting prober calls. */
static uint8_t _verify_last_tick = 0;
```

### New function: `ListenerAliasTable_check_one_verification()`

```c
    /**
     * @brief Probes one listener entry for alias staleness (round-robin).
     *
     * @details Called periodically from the CAN main state machine.
     * Rate-limited to once per USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL
     * ticks.  Each call advances a static cursor to the next registered
     * entry with a resolved alias and checks:
     *
     * -# If verify_pending is set and timeout elapsed: alias is stale —
     *    clear alias and verify_pending (entry returns to "registered,
     *    unresolved" — next AMD will re-populate).
     * -# If verify_pending is NOT set and probe interval elapsed: return
     *    the node_id so the caller can send a targeted AME.
     * -# Otherwise: skip, advance cursor.
     *
     * At most one AME is queued per call (0 or 1).
     *
     * @verbatim
     * @param current_tick  Current value of the global 100ms tick counter.
     * @endverbatim
     *
     * @return The @ref node_id_t to probe (caller builds and queues targeted
     *         AME), or 0 if nothing to do this tick.
     */
node_id_t ListenerAliasTable_check_one_verification(uint8_t current_tick) {

    // Rate-limit: at most one probe per PROBE_TICK_INTERVAL
    uint8_t elapsed = (uint8_t)(current_tick - _verify_last_tick);

    if (elapsed < USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL) {

        return 0;

    }

    _verify_last_tick = current_tick;

    // Scan up to one full table rotation looking for work
    for (int scanned = 0; scanned < LISTENER_ALIAS_TABLE_DEPTH; scanned++) {

        _verify_cursor = (_verify_cursor + 1) % LISTENER_ALIAS_TABLE_DEPTH;
        listener_alias_entry_t *entry = &_table[_verify_cursor];

        if (entry->node_id == 0) {

            continue;  // empty slot

        }

        if (entry->alias == 0 && !entry->verify_pending) {

            continue;  // unresolved and not pending — nothing to check

        }

        if (entry->verify_pending) {

            uint8_t age = (uint8_t)(current_tick - entry->verify_ticks);

            if (age >= USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS) {

                // Stale — no AMD reply within timeout
                entry->alias = 0;
                entry->verify_pending = 0;

            }

            continue;  // either timed out or still waiting — move on

        }

        // Entry is resolved (alias != 0) and not pending — check probe interval
        uint8_t age = (uint8_t)(current_tick - entry->verify_ticks);

        if (age >= USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS) {

            entry->verify_pending = 1;
            entry->verify_ticks = current_tick;

            return entry->node_id;  // caller builds targeted AME

        }

        // Not due yet — try next entry

    }

    return 0;  // nothing to do this cycle

}
```

**Key design decisions:**
- **Return node_id, not build AME internally:** The listener table module
  has no dependency on CAN buffer allocation or FIFO.  The caller
  (`can_main_statemachine.c`) builds and queues the AME frame, just like
  the existing `_queue_targeted_ame()` pattern in `can_rx_message_handler.c`.
- **One probe per call:** Guarantees at most one AME per tick interval.
  Even with a full table of 24 entries, the bus sees at most 10 AMEs/second
  (at default 100ms interval).
- **Stale detection clears alias only:** `node_id` is preserved.  The entry
  remains registered.  If the listener re-appears and sends AMD, the alias
  is re-populated via the existing `set_alias()` path.

---

## Step 4 — Update existing functions to clear `verify_pending`

**File:** `alias_mapping_listener.c`

All changes below add one or two lines to existing functions.  The function
signatures are unchanged.

### `ListenerAliasTable_initialize()` — Startup

**WAS:**
```c
void ListenerAliasTable_initialize(void) {

    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        _table[i].node_id = 0;
        _table[i].alias = 0;

    }

}
```

**IS:**
```c
void ListenerAliasTable_initialize(void) {

    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        _table[i].node_id = 0;
        _table[i].alias = 0;
        _table[i].verify_ticks = 0;
        _table[i].verify_pending = 0;

    }

    _verify_cursor = 0;
    _verify_last_tick = 0;

}
```

### `ListenerAliasTable_register()` — Listener attached

**WAS** (inside first-empty-slot block):
```c
        if (_table[i].node_id == 0) {

            _table[i].node_id = node_id;
            _table[i].alias = 0;
            return &_table[i];

        }
```

**IS:**
```c
        if (_table[i].node_id == 0) {

            _table[i].node_id = node_id;
            _table[i].alias = 0;
            _table[i].verify_ticks = 0;
            _table[i].verify_pending = 0;
            return &_table[i];

        }
```

### `ListenerAliasTable_set_alias()` — AMD arrived, entry confirmed

**WAS** (inside matching-node_id block):
```c
        if (_table[i].node_id == node_id) {

            _table[i].alias = alias;
            return;

        }
```

**IS:**
```c
        if (_table[i].node_id == node_id) {

            _table[i].alias = alias;
            _table[i].verify_pending = 0;
            return;

        }
```

**Note:** `verify_ticks` is NOT reset here.  When `verify_pending` is cleared
the prober won't probe again until `PROBE_INTERVAL_TICKS` elapses from the
last AME send time (stored in `verify_ticks`).  This is correct — the AMD
reply confirms the alias is alive, and the reprobing clock starts from when
the AME was sent.  No signature change needed.

### `ListenerAliasTable_unregister()` — Listener detached

**WAS** (inside matching-node_id block):
```c
        if (_table[i].node_id == node_id) {

            _table[i].node_id = 0;
            _table[i].alias = 0;
            return;

        }
```

**IS:**
```c
        if (_table[i].node_id == node_id) {

            _table[i].node_id = 0;
            _table[i].alias = 0;
            _table[i].verify_ticks = 0;
            _table[i].verify_pending = 0;
            return;

        }
```

### `ListenerAliasTable_flush_aliases()` — Global AME received

**WAS:**
```c
void ListenerAliasTable_flush_aliases(void) {

    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        _table[i].alias = 0;

    }

}
```

**IS:**
```c
void ListenerAliasTable_flush_aliases(void) {

    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        _table[i].alias = 0;
        _table[i].verify_pending = 0;

    }

}
```

### `ListenerAliasTable_clear_alias_by_alias()` — AMR received

**WAS** (inside matching-alias block):
```c
        if (_table[i].alias == alias) {

            _table[i].alias = 0;
            return;

        }
```

**IS:**
```c
        if (_table[i].alias == alias) {

            _table[i].alias = 0;
            _table[i].verify_pending = 0;
            return;

        }
```

---

## Step 5 — Declaration in header

**File:** `alias_mapping_listener.h`

Add declaration after `ListenerAliasTable_clear_alias_by_alias`:

```c
        /**
         * @brief Probes one listener entry for alias staleness (round-robin).
         *
         * @details Rate-limited to once per USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL
         * ticks. At most one entry probed per call.  Returns the Node ID whose
         * alias needs verification, or 0 if nothing to do.
         *
         * State transitions:
         * - Resolved + probe interval elapsed → Verifying (AME sent)
         * - Verifying + timeout elapsed → Stale (alias cleared)
         * - Verifying + AMD arrives (via set_alias) → Resolved
         *
         * @param current_tick  Current value of the global 100ms tick counter.
         *
         * @return @ref node_id_t to probe (caller queues targeted AME), or 0.
         */
    extern node_id_t ListenerAliasTable_check_one_verification(uint8_t current_tick);
```

---

## Step 6 — CAN main statemachine integration

### `can_main_statemachine.h` — Add to interface struct

**WAS** (end of `interface_can_main_statemachine_t`):
```c
        /** @brief REQUIRED. Continue enumeration to the next node. Typical: CanMainStatemachine_handle_try_enumerate_next_node. */
        bool (*handle_try_enumerate_next_node)(void);

    } interface_can_main_statemachine_t;
```

**IS:**
```c
        /** @brief REQUIRED. Continue enumeration to the next node. Typical: CanMainStatemachine_handle_try_enumerate_next_node. */
        bool (*handle_try_enumerate_next_node)(void);

        /** @brief OPTIONAL. Probe one listener alias for staleness. NULL if unused. Typical: CanMainStatemachine_handle_listener_verification. */
        bool (*handle_listener_verification)(void);

    } interface_can_main_statemachine_t;
```

### `can_main_statemachine.c` — New handler function

Add new function (before `run()`), include `alias_mapping_listener.h` at top:

```c
    /**
     * @brief Probes one listener alias for staleness and queues an AME if due.
     *
     * @details Algorithm:
     * -# Call ListenerAliasTable_check_one_verification() with current tick
     * -# If non-zero node_id returned: get first node's alias, allocate a CAN
     *    buffer (with lock/unlock), build targeted AME, push to CAN FIFO
     *    (with lock/unlock)
     * -# Return true if an AME was queued, false otherwise
     *
     * @return true if a probe AME was queued, false if nothing to do.
     */
bool CanMainStatemachine_handle_listener_verification(void) {

    node_id_t probe_id =
            ListenerAliasTable_check_one_verification(_interface->get_current_tick());

    if (probe_id != 0) {

        // Use first node's alias as AME source
        openlcb_node_t *node = _interface->openlcb_node_get_first(CAN_STATEMACHINE_NODE_ENUMRATOR_KEY);

        if (node && node->alias != 0) {

            _interface->lock_shared_resources();
            can_msg_t *ame = CanBufferStore_allocate_buffer();
            _interface->unlock_shared_resources();

            if (ame) {

                ame->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AME | node->alias;
                CanUtilities_copy_node_id_to_payload(ame, probe_id, 0);

                _interface->lock_shared_resources();
                CanBufferFifo_push(ame);
                _interface->unlock_shared_resources();

                return true;

            }

        }

    }

    return false;

}
```

**Locking rationale:** `CanBufferStore_allocate_buffer()` and
`CanBufferFifo_push()` access shared pools that may be touched by the RX
interrupt context.  The existing `can_main_statemachine.c` code uses the
same lock/unlock pattern around `CanBufferFifo_pop()` (line 229-231) and
`CanBufferStore_free_buffer()` (line 239-241).

### `can_main_statemachine.c` — Add to `run()`

**WAS:**
```c
void CanMainStateMachine_run(void) {

    _interface->lock_shared_resources();
    OpenLcbBufferList_check_timeouts(_interface->get_current_tick());
    _interface->unlock_shared_resources();

    if (_interface->handle_duplicate_aliases()) {

        return;

    }

    if (_interface->handle_outgoing_can_message()) {

        return;

    }

    if (_interface->handle_login_outgoing_can_message()) {

        return;

    }

    if (_interface->handle_try_enumerate_first_node()) {

        return;

    }

    if (_interface->handle_try_enumerate_next_node()) {

        return;

    }

}
```

**IS:**
```c
void CanMainStateMachine_run(void) {

    _interface->lock_shared_resources();
    OpenLcbBufferList_check_timeouts(_interface->get_current_tick());
    _interface->unlock_shared_resources();

    // Unconditional — runs every call so rate-limiting and stale timeouts
    // advance reliably regardless of outgoing message traffic.
    if (_interface->handle_listener_verification) {

        _interface->handle_listener_verification();

    }

    if (_interface->handle_duplicate_aliases()) {

        return;

    }

    if (_interface->handle_outgoing_can_message()) {

        return;

    }

    if (_interface->handle_login_outgoing_can_message()) {

        return;

    }

    if (_interface->handle_try_enumerate_first_node()) {

        return;

    }

    if (_interface->handle_try_enumerate_next_node()) {

        return;

    }

}
```

**Note:** The verification handler runs unconditionally at the top (like
`check_timeouts`) rather than at the bottom of the priority chain.  If it
were at the bottom, the early-return pattern would starve it whenever
outgoing messages or enumeration are active — breaking the rate-limiting
and stale timeout logic.  It is NULL-checked because it is OPTIONAL
(non-train applications set it to NULL).

### `can_main_statemachine.h` — Declare for testing

Add declaration:

```c
    extern bool CanMainStatemachine_handle_listener_verification(void);
```

---

## Step 7 — Wire in `openlcb_config.c`

**File:** `openlcb_config.c`

In the CAN main statemachine interface struct initialization, add the new
field:

```c
#ifdef OPENLCB_COMPILE_TRAIN
    .handle_listener_verification = &CanMainStatemachine_handle_listener_verification,
#else
    .handle_listener_verification = NULL,
#endif
```

---

## Step 8 — Tests

**File:** `alias_mapping_listener_Test.cxx`

### Test 1: `verify_no_entries_returns_zero`
- Empty table → `check_one_verification()` returns 0
- Verify no crash, cursor wraps safely

### Test 2: `verify_unresolved_entry_skipped`
- Register node_id but alias = 0 (unresolved)
- `check_one_verification()` → returns 0 (nothing to probe)

### Test 3: `verify_resolved_entry_not_due`
- Register + set_alias; entry's verify_ticks = 0 (from register)
- `check_one_verification(1)` → returns 0 (interval not elapsed)

### Test 4: `verify_resolved_entry_due_returns_node_id`
- Register + set_alias
- Advance tick by `PROBE_INTERVAL_TICKS`
- `check_one_verification()` → returns the node_id
- Entry now has `verify_pending = 1`

### Test 5: `verify_pending_entry_amd_clears_pending`
- Put entry in verify_pending state
- Call `set_alias()` (simulates AMD arrival)
- Verify `verify_pending` is cleared

### Test 6: `verify_pending_timeout_clears_alias`
- Put entry in verify_pending state
- Advance tick by `VERIFY_TIMEOUT_TICKS`
- `check_one_verification()` → returns 0 (stale handled internally)
- Entry alias = 0, verify_pending = 0, node_id preserved

### Test 7: `verify_rate_limiting`
- Resolved entry due for probe
- Call `check_one_verification(tick)` → returns node_id (probe due)
- Call again with same tick → returns 0 (rate limited)
- Call again with `tick + PROBE_TICK_INTERVAL` → can advance again

### Test 8: `verify_round_robin_advances`
- Register 3 entries, all resolved, all due
- Call `check_one_verification()` three times (advancing tick by
  `PROBE_TICK_INTERVAL` each time)
- Verify returns 3 different node_ids (round-robin)

### Test 9: `verify_flush_clears_pending`
- Put entry in verify_pending state
- Call `flush_aliases()`
- Verify alias = 0, verify_pending = 0

### Test 10: `verify_unregister_clears_pending`
- Put entry in verify_pending state
- Call `unregister(node_id)`
- Verify entry fully cleared (node_id = 0, alias = 0, verify_pending = 0)

### Test 11: `verify_clear_alias_by_alias_clears_pending`
- Put entry in verify_pending state
- Call `clear_alias_by_alias(alias)`
- Verify alias = 0, verify_pending = 0

### Test 12: `verify_initialize_resets_cursor`
- Advance cursor to middle of table (call check_one_verification several times)
- Call `initialize()`
- Verify cursor = 0 and all entries cleared

---

## State Diagram

```
                    register()
  Empty ──────────────────────────► Registered
  (node_id == 0)                    (alias == 0, verify_pending == 0)
                                         │
                                    set_alias() (AMD arrives)
                                         │
                                         ▼
                                    Resolved
                                    (alias != 0, verify_pending == 0)
                                         │
                              probe interval elapsed
                              (check_one_verification)
                                         │
                                         ▼
                                    Verifying
                                    (alias != 0, verify_pending == 1)
                                       / │ \
                      AMD arrives     /  │  \   timeout
                      (set_alias)    /   │   \  (no AMD reply)
                                    /    │    \
                                   ▼     │     ▼
                              Resolved   │   Stale → Registered
                                         │   (alias = 0,
                                         │    verify_pending = 0,
                                         │    node_id preserved)
                                         │
                                  AMR received
                                  (clear_alias_by_alias)
                                         │
                                         ▼
                                    Registered
                                    (alias = 0, verify_pending = 0)
```

---

## Bus Traffic Analysis

**Worst case** with default settings (24 entries, 100ms interval):
- At most 1 AME per 100ms = 10 AME frames/second
- Each AME is a single CAN frame (6-byte payload)
- Full table scan: 24 × 100ms = 2.4 seconds
- Each entry reprobed every 25 seconds
- A stale entry is detected within 2.4s (scan time) + 3s (timeout) = ~5.4s

**Adjustable:** Set `PROBE_TICK_INTERVAL = 5` to reduce to 2 AMEs/second.
Set `PROBE_INTERVAL_TICKS = 600` for 60-second reprobe.  The user controls
the exact tradeoff between detection speed and bus utilization.

---

## Risks and Mitigations

| Risk | Mitigation |
|------|-----------|
| Bus flooding with AMEs | Rate-limited: at most 1 AME per PROBE_TICK_INTERVAL. User-configurable. |
| CAN buffer exhaustion for AME | `CanBufferStore_allocate_buffer()` returns NULL if full — AME simply skipped this tick. |
| Stale detection delay | Bounded: scan_time + timeout. Default ~5.4s. |
| False stale (AMD lost/delayed) | 3-second timeout is generous. Entry returns to Registered, not removed — next AMD re-resolves. |
| Impact on non-train builds | Handler is OPTIONAL (NULL-checked). Table depth is 0 when train not compiled. |

---

## Design Notes

- **Why not probe from `_run_periodic_services()`?**  Periodic services run
  in the OpenLCB layer.  AME is a CAN control frame — it belongs in the CAN
  main statemachine, which already has access to CAN buffer allocation and
  the CAN FIFO.

- **Why return node_id instead of building AME internally?** Keeps
  `alias_mapping_listener` decoupled from CAN buffer allocation and FIFO.
  The module only manages the table; the caller handles CAN framing.

- **Why lock/unlock around allocate and push?**  `CanBufferStore` and
  `CanBufferFifo` are shared between main-loop and RX-interrupt contexts.
  The existing `can_main_statemachine.c` code uses the same pattern:
  lock around `CanBufferFifo_pop()` (line 229-231) and
  `CanBufferStore_free_buffer()` (line 239-241).  The new handler follows
  this pattern exactly.

- **Why unconditional at the top of run()?**  The prober's rate-limiting
  and stale timeouts depend on being called at a steady rate.  If placed at
  the bottom of the early-return priority chain, active outgoing traffic or
  enumeration would starve it.  Running unconditionally (like
  `check_timeouts`) ensures the tick-based logic advances reliably.

- **Why use first node's alias as AME source?**  AME is a CAN-level control
  frame.  The source alias just needs to be a valid alias we own.  Using the
  first node's alias is simple and always available (at least one node must
  be logged in for listeners to exist).

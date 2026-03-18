# ARCHIVED — HISTORICAL / BRAINSTORMING

> **Status:** Superseded. All 7 failure scenarios documented here are resolved
> by the zero-FIFO sequential dispatch design in `plan_unified_sibling_dispatch.md`.
> The resolution summary is captured in the "Failure Scenario Resolution" table
> in that plan.
>
> This document was exploratory analysis that drove the design of the unified
> sibling dispatch plan. Retained for historical reference only.
>
> Archived: 2026-03-17

---

# Virtual Node Outgoing Path — Failure Scenarios

Date: 2026-03-17

## Overview

This document catalogs 7 identified failure modes in the current outgoing
transmit system when virtual nodes are present. Each scenario includes a flow
diagram showing exactly where and why the failure occurs.

These scenarios drive the design requirements for the proposed unified loopback
mechanism documented in `proposed_virtual_train_implementation.md`.

---

## Architecture Background

### The Run Loop (one step per call)

`OpenLcbMainStatemachine_run()` executes exactly one step per invocation:

```
1. handle_outgoing_openlcb_message()  → send pending msg, return if acted
2. handle_try_reenumerate()           → re-enter handler for next enum step, return if acted
3. handle_try_pop_next_incoming()     → grab from FIFO, dispatch to first node, return if acted
4. handle_try_enumerate_first_node()  → dispatch same msg to first node, return if acted
5. handle_try_enumerate_next_node()   → dispatch same msg to next node, return if acted
```

### Two Independent Send Paths

The codebase has two completely independent send paths, neither of which
provides loopback for the other:

```
    Path A: Protocol Handlers (statemachine)
    ─────────────────────────────────────────
    handler → sets outgoing_msg_info.valid = true
           → run loop drains via handle_outgoing_openlcb_message()
           → _loopback_to_siblings() ← LOOPBACK EXISTS (but buffer issue)
           → send_openlcb_msg() → transport layer

    Path B: Application Layer (direct)
    ───────────────────────────────────
    app callback / timer tick / user code
           → OpenLcbApplication_send_event_pc_report()
           → send_openlcb_msg() → transport layer ← NO LOOPBACK
```

Both paths ultimately call `send_openlcb_msg()` (a DI callback wired in
`openlcb_config.c`) which forwards to the transport layer to put frames on
the wire. But only Path A has loopback (with the buffer accumulation problem).
Path B has no loopback at all.

This means:
- Protocol-generated messages (P/C Identified, SNIP replies, datagram
  responses) CAN reach siblings (Path A, with caveats)
- Application-generated messages (PCERs, train commands, broadcast time
  events) CANNOT reach siblings (Path B)

---

## Scenario 1: Enumeration + Loopback Buffer Accumulation

**Root cause:** FIFO accumulation during enumerate cycle

**Trigger:** Any message that causes a multi-message enumerated response
(e.g., Identify Events Global to a node with many events)

Setup: 3 virtual nodes (A, B, C). Node B has 4 produced events. External node
sends Identify Events Global. The run loop dispatches to Node B.

```
                    Run Loop                    Outgoing     FIFO (head→tail)
                    ────────                    Slot         ────────────────
 1. pop incoming    Identify Events → Node B
    handler:        B produces P.Id #1          valid=true
                    sets enumerate=true
                    returns to run loop

 2. handle_outgoing send P.Id #1 → wire         valid=false
                    _loopback_to_siblings()                  [P.Id #1 copy]
                    allocate buffer, push copy

 3. reenumerate     re-enter handler
                    B produces P.Id #2          valid=true
                    enumerate still true

 4. handle_outgoing send P.Id #2 → wire         valid=false
                    _loopback_to_siblings()                  [P.Id #2 copy]
                                                             [P.Id #1 copy]

 5. reenumerate     re-enter handler
                    B produces P.Id #3          valid=true

 6. handle_outgoing send P.Id #3 → wire         valid=false
                    _loopback_to_siblings()                  [P.Id #3 copy]
                                                             [P.Id #2 copy]
                                                             [P.Id #1 copy]

 7. reenumerate     re-enter handler
                    B produces P.Id #4          valid=true
                    clears enumerate (last one)

 8. handle_outgoing send P.Id #4 → wire         valid=false
                    _loopback_to_siblings()                  [P.Id #4 copy]
                                                             [P.Id #3 copy]
                                                             [P.Id #2 copy]
                                                             [P.Id #1 copy]

    ── enumeration done, now the run loop finally reaches step 3 ──

 9. pop incoming    P.Id #4 copy → Node A
                    A's handler processes it     ...

10. pop incoming    P.Id #3 copy → Node A        ...

    ... (8 more pops: 4 copies × dispatch to A and C each)
    ... each dispatch may trigger A or C to respond
    ... those responses may themselves need loopback → more buffers
```

**4 events = 4 loopback buffers accumulated before any are consumed.**

With 90 events: 90 buffers. With 3 nodes each having 90 events receiving
Identify Events Global: the run loop processes Node B (90 buffers), then
Node C (90 more), then Node A (90 more) — all before draining any loopback
copies. **270 pool buffers consumed before a single loopback is processed.**

And when those loopback copies ARE finally processed, sibling handlers may
generate their own multi-message responses, each with their own loopback
copies. The buffer count compounds.

---

## Scenario 2: Application Callback Sends Multiple Messages — Reentrancy

**Root cause:** Callback sends via Path B (direct to transport) from within a
Path A dispatch — loopback from inside the call stack creates reentrancy

**Trigger:** Any application callback that sends messages using the
`while(!send())` pattern while the run loop is mid-dispatch

**Clarification:** Application Helpers (`OpenLcbApplication_send_event_pc_report`,
`OpenLcbApplicationTrain_send_set_speed`, etc.) call `_interface->send_openlcb_msg()`
which is wired directly to the transport layer — Path B. They do NOT use the
statemachine's outgoing slot (`outgoing_msg_info`). The `while(!send())` pattern
works fine today for a single node because each call blocks until the transport
layer transmits the frame. No slot contention, no message loss.

**The problem emerges with virtual nodes.** When vNode2's handler fires because
of a loopback message from vNode1, the application callback runs inside the
run loop's call stack:

```
    Call Stack                                      CAN Wire    FIFO
    ──────────                                      ────────    ────
 1. run()
 2.  └─ pop incoming (loopback from vNode1)
 3.      └─ process_main_statemachine()
 4.          └─ dispatch to vNode2's handler
 5.              └─ protocol handler processes request
 6.                  └─ application callback fires
 7.                      ├─ while(!send(msg1)) ;    → on wire
 8.                      ├─ while(!send(msg2)) ;    → on wire
 9.                      └─ while(!send(msg3)) ;    → on wire
10.     return to run loop
```

All 3 messages reach the wire. But none are looped back to siblings.

**If we add loopback to Path B (inside send_openlcb_msg):**

```
    Call Stack                                      CAN Wire    FIFO
    ──────────                                      ────────    ────
 1. run()
 2.  └─ pop incoming (loopback from vNode1)
 3.      └─ dispatch to vNode2's handler
 4.          └─ application callback fires
 5.              ├─ while(!send(msg1)) ;            → on wire
 6.              │   └─ loopback_to_siblings()                  [msg1 copy]
 7.              ├─ while(!send(msg2)) ;            → on wire
 8.              │   └─ loopback_to_siblings()                  [msg2 copy]
 9.              │                                              [msg1 copy]
10.              └─ while(!send(msg3)) ;            → on wire
11.                  └─ loopback_to_siblings()                  [msg3 copy]
12.                                                             [msg2 copy]
13.                                                             [msg1 copy]
14.     return to run loop
15.     (now the FIFO drains 3 loopback copies)
```

Loopback copies queue to the FIFO — but we're still inside run(). The FIFO
can't be drained until we return. This isn't a correctness failure IF we only
queue to the FIFO (deferred processing). But:

- 3 buffers allocated from the pool before any are consumed
- If the callback sends N messages (e.g., e-stop to 8 listeners), N buffers
- The caller never yields between sends — buffer accumulation is proportional
  to the number of messages the callback sends

**If loopback tries to dispatch inline instead of queueing:**

```
 5.              ├─ while(!send(msg1)) ;            → on wire
 6.              │   └─ inline dispatch to vNode1
 7.              │       └─ vNode1's handler fires
 8.              │           └─ sets outgoing_msg_info.valid
 9.              │               REENTRANCY: we're inside run()
10.              │               statemachine state CORRUPTED
```

The statemachine's `_statemachine_info` (incoming_msg_info, outgoing_msg_info,
current node pointer) is shared mutable state. Dispatching inline from within
a callback re-enters the statemachine while it's mid-dispatch. The outgoing
slot, the current node pointer, the enumerate flag — all are live. Writing to
any of them corrupts the in-progress dispatch.

**Summary:** The `while(!send())` pattern works today because there are no
virtual nodes to loopback to. Adding loopback to Path B creates a choice:
- **Queue to FIFO:** Works but accumulates N buffers per callback, never
  yields, and FIFO drains only after the entire callback chain completes
- **Dispatch inline:** Reentrancy — corrupts statemachine state

---

## Scenario 3: Combined — Callback + Enumerate + Loopback

**Root cause:** Both send paths active simultaneously during a single dispatch

**Trigger:** Loopback message triggers a handler that both enumerates (Path A)
and fires an application callback that sends via `while(!send())` (Path B)

The worst case combines Scenarios 1 and 2. vNode1 sends Init Complete (during
login). Loopback delivers it to vNode2. vNode2's handler wants to re-identify
its 90 events via the enumerate pattern. The application has also hooked the
callback and wants to send status messages via `while(!send())`.

```
    vNode1 Login          Loopback           vNode2 Handler        App Callback
    ──────────────        ────────           ──────────────        ────────────
    Init Complete ──→ wire
                     ──→ FIFO copy
                          │
                          ▼
                     pop & dispatch ──→ handler fires
                                        │
                                        ├─ protocol: enumerate 90 P.Id
                                        │  (sets enumerate, valid=true
                                        │   via Path A outgoing slot)
                                        │
                                        └─ app callback fires
                                             while(!send(msg_X)) ;
                                             (Path B: direct to transport)
                                             msg_X goes on wire ✓
                                             msg_X NOT looped back ✗
```

**Two paths, two problems:**

- **Path A (enumerate):** The 90 P.Id messages go through the outgoing slot.
  Each one gets looped back → 90 buffers accumulate in FIFO (Scenario 1).
- **Path B (callback sends):** Messages go directly to the transport layer, bypassing
  loopback entirely. Sibling nodes never see them (Scenario 4). If loopback
  were added to Path B, it would either accumulate more FIFO buffers or
  cause reentrancy (Scenario 2).

Both paths are active in the same dispatch cycle. The enumerate pattern
assumes it has exclusive use of the outgoing slot, while the callback is
independently pushing frames out through the transport layer. The two paths
don't interfere with each other today (they use different mechanisms), but
neither provides correct loopback for virtual nodes.

---

## Scenario 4: Tight-Loop Application Send Bypasses Loopback Entirely

**Root cause:** Application send path has no loopback infrastructure

**Trigger:** Any application-layer send to a sibling virtual node

**File:** `openlcb_application_train.c` — `_forward_estop_to_listeners()` (line 231)

When a heartbeat timeout fires, the train module forwards Set Speed 0 to every
registered listener in a tight loop:

```
    _forward_estop_to_listeners()
    ─────────────────────────────
    for (int i = 0; i < state->listener_count; i++) {

        // build msg on stack
        _interface->send_openlcb_msg(&msg);   // direct to transport layer

    }
```

The application layer's `send_openlcb_msg` is wired directly to the transport
layer — it goes straight to the transport hardware, **completely bypassing the
main statemachine's outgoing slot and loopback infrastructure**.

**What works:** Multiple messages can be sent because each call blocks until
the transport layer transmits the frame. No slot contention.

**What fails with virtual nodes:** Sibling virtual nodes never see these
messages. If vTrain A forwards e-stop to listener vTrain B (a local virtual
node), the message goes on the wire but never loops back through the incoming
FIFO. vTrain B's protocol handler never fires.

```
    vTrain A                  CAN Wire              vTrain B
    ────────                  ────────              ────────
    heartbeat timeout
    _forward_estop_to_listeners()
      send_openlcb_msg(→B) ──→ transport ──→ on wire ──→ external nodes see it
                                              vTrain B NEVER sees it
                                              (no loopback path exists
                                               for application sends via
                                               the transport layer directly)
```

**Impact:** This same pattern applies to ALL application-layer sends:
- `OpenLcbApplication_send_event_pc_report()` — PCER events
- `OpenLcbApplicationTrain_send_set_speed()` — speed commands
- `OpenLcbApplicationTrain_send_set_function()` — function commands
- `OpenLcbApplicationTrain_send_emergency_stop()` — e-stop
- All broadcast time report sends

Every message sent through the application layer bypasses loopback.

---

## Scenario 5: Broadcast Time Date Rollover — Triple Send in Sequence

**Root cause:** Multiple direct sends from timer tick context

**Trigger:** Clock rolls over midnight

**File:** `openlcb_application_broadcast_time.c` (lines 727-733)

When the clock rolls over midnight, three PCER events are sent back-to-back:

```
    if (prev_hour == 23 && clock->state.time.hour == 0) {

        send_date_rollover(node, clock_id);     // PCER #1
        send_report_year(node, clock_id, year);  // PCER #2
        send_report_date(node, clock_id, m, d);  // PCER #3

    }
```

Each calls `OpenLcbApplication_send_event_pc_report()` which calls
`send_openlcb_msg()` directly to the transport layer.

**What works today (single node):** All three go to the wire. The transport
layer blocks until each frame is sent.

**What fails with virtual nodes:** Same as Scenario 4 — no loopback. If a
sibling is a broadcast time consumer, it never sees the rollover events.

**Additional concern:** This is called from a 100ms timer tick handler, not
from the statemachine's dispatch path. The timer tick runs outside the run
loop's message processing cycle. Even if we added loopback to the application
send path, we'd be pushing to the FIFO from outside the statemachine —
potential reentrancy if the run loop is mid-dispatch.

**Priority: LOW.** There is no known use case for a broadcast time producer
and consumer being sibling virtual nodes on the same device. Broadcast time
producers are expected to be external. If the unified loopback solution
covers this scenario naturally, great — but it should not drive design
decisions.

---

## Scenario 6: Train Listener Forwarding via Enumerate — Buffer Accumulation

**Root cause:** Enumerate pattern + loopback = N buffers per consist

**Trigger:** Controller sends command to train with consist listeners

**File:** `protocol_train_handler.c` — `_forward_to_next_listener()` (line 451)

When a controller sends Set Speed to a train with listeners (consist), the
handler uses the enumerate pattern correctly — one listener per yield:

```
    run() → incoming Set Speed → dispatch to train handler
            handler sets enumerate=true, loads forwarded msg to listener #1
    run() → handle_outgoing → send to wire → loopback copy → clear valid
    run() → reenumerate → handler loads forwarded msg to listener #2
    run() → handle_outgoing → send to wire → loopback copy → clear valid
    ... repeat for each listener
```

**What works (single node):** The enumerate pattern correctly yields between
messages. One outgoing at a time.

**What fails with virtual nodes and loopback:** Same as Scenario 1. If the
train has 8 listeners, 8 loopback copies accumulate in the FIFO before any
are processed. Each loopback copy is then dispatched to all sibling nodes.
If sibling nodes react to the forwarded commands, more messages are generated.

**Real-world scale:** A consist of 8 locomotives, each a virtual train node.
Controller sends Set Speed → train forwards to 8 listeners → 8 loopback
copies → each dispatched to all siblings → potential cascade.

---

## Scenario 7: One-Level Loopback Guard Suppresses Legitimate Chains

**Root cause:** Single boolean `state.loopback` flag does double duty and
cannot distinguish originator identity across chain levels

**Trigger:** Multi-hop sibling interaction where a different node responds

The current code prevents re-loopback via the `state.loopback` flag:

```c
if (!_statemachine_info.incoming_msg_info.msg_ptr->state.loopback) {
    _loopback_to_siblings();
}
```

The `state.loopback` flag is doing two jobs with one bit:

1. **Self-skip:** "This is a loopback copy — the originator should not process
   its own message." (Handled separately via source_id comparison in
   `_should_node_process_message()`.)
2. **Cascade prevention:** "This response was triggered by a loopback — don't
   re-loopback the response." (Handled by the guard above.)

The problem: a single boolean cannot encode **who originated the chain**. It
only knows "this message came from a loopback." It cannot distinguish between
"Node A's loopback triggered this" and "Node B's loopback triggered this."

In a multi-level scenario, B is a **different originator** than A, and its
response is legitimately new information that siblings A and C need to see:

```
    Level 0: Node A sends Init Complete
               → loopback copy (source=A, loopback=true)
               → dispatched to B and C (A skips via source_id match)

    Level 1: Node B receives Init Complete from A
               → B responds with Identify Events (source=B)
               → should this be looped back to A and C?
               → guard checks: incoming.loopback == true → BLOCKED
               → A and C never see B's Identify Events

    Level 2: (never reached)
               Node A should receive B's Identify Events
               Node A should respond with Producer Identified
               → never happens because Level 1 was blocked
```

The guard treats ALL responses to loopback messages the same — regardless of
whether the responder is a different node with legitimately new information.
Node B's Identify Events is not a re-echo of A's Init Complete; it is B's
independent protocol reaction. But the flag cannot tell the difference.

**What would be needed:** The cascade prevention logic needs to know the
originator identity, not just "was this triggered by a loopback." Options:
- Track originator node_id in the loopback flag (not just a boolean)
- Use a chain depth counter instead of a boolean
- Accept bounded re-loopback (protocol interactions are finite — Init Complete
  → Identify Events → P/C Identified → done)

The chain depth is bounded (protocol interactions are finite), but it's not
bounded at 1. The one-level boolean guard suppresses legitimate multi-hop
sibling interactions where different nodes are the originators at each level.

---

## Summary Table

| # | Scenario | Root Cause | Path | Symptom |
|---|----------|------------|------|---------|
| 1 | Enumerate + loopback | FIFO accumulation | A | N buffers before any consumed |
| 2 | App callback multi-send | Reentrancy / no loopback | B in A | Loopback impossible without reentrancy or buffer accumulation |
| 3 | Combined enum + callback | Both paths active | A+B | Two loopback problems simultaneously |
| 4 | App layer tight-loop send | No loopback on Path B | B | Siblings never see the message |
| 5 | Broadcast time rollover | No loopback on Path B | B | Consumer siblings miss events (LOW priority) |
| 6 | Train listener forwarding | FIFO accumulation | A | N × siblings buffer explosion |
| 7 | One-level loopback guard | Blanket suppression | A | Multi-hop chains truncated |

---

## Fundamental Constraint

The outgoing transmit system was designed for a single node with cooperative
single-threaded message generation:

- **One outgoing slot** — only one message can be pending at a time
- **Enumerate pattern** — multi-message responses yield back to the run loop
  between each message
- **No message queue** — helpers write directly to the slot, not to a queue
- **No reentrancy** — the statemachine assumes exclusive access to its state
- **Two send paths** — protocol handlers use the outgoing slot; application
  helpers call `send_openlcb_msg()` directly to the transport layer

With virtual nodes, the system has N independent actors that all need to send
messages, potentially in response to each other. The single-slot cooperative
model cannot support:

1. Loopback-triggered responses that generate multiple messages
2. Application callbacks that want to send multiple messages
3. Multi-level sibling interaction chains
4. Application-layer sends that need sibling visibility
5. Timer-driven sends that occur outside the run loop

**The question is whether the outgoing transmit path needs to become a unified
queue with built-in loopback to support virtual nodes, or whether a different
dispatch model can work within the existing architecture.**

---

## Rejected Approaches

### Inline Dispatch (Zero-Copy Loopback)

**Idea:** Instead of allocating a buffer and copying to the FIFO, dispatch the
outgoing message directly to sibling handlers inline (same call stack) before
clearing the valid flag. Zero buffer allocation, zero FIFO involvement.

**Why it fails — reentrancy:** `_statemachine_info` is shared mutable state
(incoming_msg_info, outgoing_msg_info, current node pointer). Dispatching
inline re-enters the statemachine while it's mid-dispatch. The sibling's
handler may call an application callback, which calls a helper to send a
response — but we're still inside the original `handle_outgoing_openlcb_message()`
call. The outgoing slot is occupied, the current node pointer is live, the
enumerate flag is set. Writing to any of them corrupts the in-progress dispatch.

```
    handle_outgoing_openlcb_message()          ← outgoing slot occupied
      └─ inline dispatch to sibling
           └─ sibling handler fires
                └─ app callback fires
                     └─ while(!send(msg)) ;    ← Path B: goes to transport, ok
                     └─ OR: sets valid=true    ← Path A: OVERWRITES in-progress state
                          statemachine state CORRUPTED
```

The application helpers are designed around the contract that the outgoing slot
is free when they're called. Inline dispatch from within the send path violates
that contract. Even if the callback uses Path B (direct to transport) instead of
the outgoing slot, we're still re-entering handler dispatch logic from inside
handler dispatch logic — the `_statemachine_info` pointers (current node,
incoming message, enumerate flag) are all live and would be corrupted by the
sibling's dispatch.

### Single Fixed Loopback Buffer

**Idea:** Replace pool allocation with one static `openlcb_msg_t` dedicated to
loopback. Copy into it, set a pending flag. Next `_run()` cycle processes it
before pulling from the FIFO.

**Why it fails — enumeration:** The run loop's enumerate cycle (steps 1-2)
monopolizes `_run()` calls until enumeration completes. A loopback copy pushed
during step 2 must persist across multiple `_run()` calls until step 3 (pop
from FIFO) runs. But the next enumerate iteration overwrites the single buffer
before the previous copy is processed. With 90 events, 89 loopback copies
are silently lost.

---

## Related Documents

- `proposed_virtual_train_implementation.md` — proposed changes (Categories 1-5)
- `OpenLcbCLib_OpenMRN_message_comparison.md` — full comparison analysis
- `plan_virtual_node_verification_protocol.md` — draft conformance testing spec

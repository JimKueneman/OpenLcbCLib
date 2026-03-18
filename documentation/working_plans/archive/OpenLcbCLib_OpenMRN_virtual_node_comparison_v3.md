# OpenLcbCLib vs OpenMRN — Virtual Node Implementation Comparison v3

Date: 2026-03-17
Status: Current-state comparison against OpenMRN reference implementation

---

## Architectural Contrast

| Dimension | OpenMRN | OpenLcbCLib |
|-----------|---------|-------------|
| Dispatch model | Async StateFlow executor | Synchronous run-loop, one step per call |
| Virtual node isolation | Independent async state machines | Shared run loop, sequential dispatch |
| Buffer model | Dynamic pool allocation | Static slots only |
| Sibling message delivery | Reactive (registered handlers called by dispatcher) | Explicit (outgoing held in slot, shown to each sibling in turn) |
| Backpressure | Executor queue depth + allocation | Sequential model — one message in flight at a time |
| Determinism | Non-deterministic (scheduler-dependent) | Deterministic (strict priority order) |

---

## Category 1: CAN Alias Negotiation — Sibling Collision Prevention

### OpenMRN
- **File:** `src/openlcb/AliasAllocator.cxx` lines 141-160 (`add_allocated_alias`)
- Maintains a `local_aliases()` table on the IfCan interface. Newly generated
  aliases are checked against this table before use. The table persists
  across all virtual nodes on the same interface.

### OpenLcbCLib
- **File:** `src/drivers/canbus/can_login_message_handler.c:159`
- **Function:** `CanLoginMessageHandler_state_generate_alias()`
- Rejects alias `0x000` (invalid per spec) and any alias already held by a
  sibling virtual node before accepting:

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

### Verdict: EQUIVALENT
Both check the shared alias table before assigning. OpenLcbCLib retries
eagerly at generation time. OpenMRN checks lazily on registration.
Both prevent duplicate aliases across sibling virtual nodes.

---

## Category 2: CAN Control — Global AME / AMD

### OpenMRN
- **File:** `src/openlcb/IfCan.hxx` / IfCanImpl
- Global AME clears the local alias cache. Each virtual node's initialized
  state is checked; if initialized, its alias is repopulated and an AMD is sent.
  Handled implicitly within IfCan's message reception flow.

### OpenLcbCLib
- **File:** `src/drivers/canbus/can_rx_message_handler.c:579`
- **Function:** `CanRxMessageHandler_ame_frame()` — global AME branch
- Flushes listener cache, iterates `alias_mapping_info` to repopulate listener
  entries for all permitted local virtual nodes, and sends an AMD for each:

```c
if (_interface->listener_flush_aliases) {

    _interface->listener_flush_aliases();

}

for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

    if (alias_mapping_info->list[i].alias != 0x00 &&
            alias_mapping_info->list[i].is_permitted) {

        if (_interface->listener_set_alias) {

            _interface->listener_set_alias(
                    alias_mapping_info->list[i].node_id,
                    alias_mapping_info->list[i].alias);

        }

        // Send AMD for this alias...

    }

}
```

### Verdict: EQUIVALENT
Both flush on global AME, repopulate local virtual node entries immediately,
and send AMD for each. OpenLcbCLib is explicit and synchronous; OpenMRN is
implicit within its async flow. Functionally identical.

---

## Category 3: OpenLCB Login Messages — Sibling Dispatch

### OpenMRN
- **File:** `src/openlcb/NodeInitializeFlow.hxx`
- Each virtual node runs an independent `NodeInitializeFlow` state machine.
  Login messages pass through the IfCan dispatcher; other virtual nodes
  receive them via their registered handlers in the normal event chain.
  No explicit sibling dispatch — each node independently receives via the
  global dispatcher.

### OpenLcbCLib
- **File:** `src/openlcb/openlcb_login_statemachine.c`
- A dedicated `_sibling_statemachine_info` context (`openlcb_statemachine_info_t`)
  is used for sibling dispatch of login outgoing messages. Receiving siblings
  process login messages through the main protocol handlers, not login handlers:

```c
// src/openlcb/openlcb_login_statemachine.c:94
static openlcb_statemachine_info_t _sibling_statemachine_info;
```

- Uses `OPENLCB_LOGIN_SIBLING_DISPATCH_NODE_ENUMERATOR_INDEX` (independent
  from main statemachine's sibling dispatch key, defined in `openlcb_defines.h:1212`).
- Dispatch calls `process_main_statemachine()` on the sibling context.
- Run loop priority order (`OpenLcbLoginMainStatemachine_run()`):

```
Priority 1: Send pending login outgoing to wire (skip if sibling dispatch active)
Priority 2: Sibling dispatch
  2a. Send any pending sibling response to wire
  2b. Continue sibling handler if mid-enumerate
  2c. Dispatch outgoing to current sibling; advance to next
Priority 3: Re-enumerate login handler (multi-message)
Priority 4: Get first node
Priority 5: Advance to next node
```

### Verdict: DIFFERENT ARCHITECTURES, OPENLCBCLIB IS MORE DETERMINISTIC
OpenMRN uses async event dispatch — delivery order depends on executor
scheduling. OpenLcbCLib uses explicit sequential dispatch — every sibling
receives every login message in strict order, one per `_run()` call.
Functionally equivalent; OpenLcbCLib gives stronger ordering guarantees.

---

## Category 4: OpenLCB Global Messages — Sibling Dispatch

### OpenMRN
- Global messages (Identify Events Global, Verify Node ID Global, etc.) are
  handled by the dispatcher. Each virtual node registers an MTI-specific
  handler. The dispatcher calls each handler in registration order.
  Async — handlers may run at different executor ticks.

### OpenLcbCLib
- **File:** `src/openlcb/openlcb_main_statemachine.c`
- After any outgoing message is sent to the wire, `_sibling_dispatch_begin()`
  holds the outgoing slot and begins iterating siblings. Run loop priority
  order (`OpenLcbMainStatemachine_run()`):

```
Priority 1: Send pending main outgoing to wire (skip if sibling dispatch active)
Priority 2: Sibling dispatch
  2a. Send any pending sibling response to wire; push to response queue
  2b. Continue sibling handler if mid-enumerate
  2c. Dispatch outgoing to current sibling; advance to next
Priority 2.5: Pop sibling response queue or Path B pending; begin new dispatch
Priority 3: Re-enumerate main handler (multi-message)
Priority 4: Pop next incoming from wire FIFO
Priority 5: Enumerate first node for incoming
Priority 6: Enumerate next node
```

- Self-skip: `OpenLcbMainStatemachine_does_node_process_msg()` checks both
  `state.loopback` AND `source_id` to prevent the originating node from
  processing its own message (`openlcb_main_statemachine.c:456`).

### Verdict: OPENLCBCLIB IS MORE DETERMINISTIC
One message in flight. One sibling at a time. No scheduling variance.
OpenMRN's async model can vary in ordering. OpenLcbCLib guarantees every
sibling sees every message before the next message is generated.

---

## Category 5: OpenLCB Addressed Messages — Sibling Dispatch

### OpenMRN
- Addressed messages are routed by the dispatcher to the specific destination
  virtual node only (`dst_node` field check in `If.hxx`). Non-matching
  siblings do not receive them. Self-skip is implicit — the destination
  never matches the originator.

### OpenLcbCLib
- Addressed messages flow through the same sibling dispatch mechanism as
  global messages. `OpenLcbMainStatemachine_does_node_process_msg()` filters
  on `dest_alias` and `dest_id` — only the addressed sibling processes it;
  non-matching siblings skip silently.
- Self-skip uses a dual-layer guard: `state.loopback` flag AND `source_id`
  comparison. More robust than destination-only matching.

```c
// openlcb_main_statemachine.c:456
if (statemachine_info->incoming_msg_info.msg_ptr->state.loopback &&
        statemachine_info->openlcb_node->id ==
        statemachine_info->incoming_msg_info.msg_ptr->source_id) {

    return false;

}
```

### Verdict: OPENLCBCLIB SLIGHTLY BETTER
Both correctly route addressed messages to only the addressed sibling.
OpenLcbCLib's dual-layer self-skip prevents edge cases that
destination-only matching could miss.

---

## Category 6: Buffer / Memory Model

### OpenMRN
- Dynamic `Pool` allocation throughout. Message buffers are `Buffer<T>`
  objects allocated from pools shared across the entire stack.
  No static sibling dispatch queue — each StateFlow handler owns its queue.
  Bounded only by pool size (configurable at startup).

### OpenLcbCLib
- **Zero dynamic allocation** for sibling dispatch.
- Defined in `src/openlcb/openlcb_main_statemachine.c`:

```c
// Line 104
#define SIBLING_RESPONSE_QUEUE_DEPTH 5

// Line 107
static openlcb_stream_message_t _sibling_response_queue[SIBLING_RESPONSE_QUEUE_DEPTH];

// Line 113
static uint8_t _sibling_response_queue_high_water;

// Lines 120-122 — Path B pending slot
static openlcb_stream_message_t _path_b_pending_msg;
static openlcb_msg_t *_path_b_pending_ptr;
static bool _path_b_pending;
```

- High-water mark accessible at runtime via
  `OpenLcbMainStatemachine_get_sibling_response_queue_high_water()`.
- Total sibling dispatch budget: ~7 static message slots.
- Queue overflow results in a warned drop, not a crash.

### Verdict: DIFFERENT TRADE-OFFS
- OpenMRN: Flexible, scales with pool size. Allocation can fail.
- OpenLcbCLib: Deterministic, bounded, zero allocation failure possible.
  For embedded targets, static allocation is the correct choice.

---

## Category 7: Backpressure / Flow Control

### OpenMRN
- Executor queue depth provides natural backpressure. If a handler's queue
  fills, the sender blocks on allocation or is delayed by the executor.
  Each StateFlow yields after processing one message, preventing starvation.

### OpenLcbCLib
- The sequential run loop IS the backpressure. One message is in flight at
  any time. A sibling cannot receive the next message until the previous
  sibling dispatch cycle completes.
- Path B: the `while (!send())` blocking pattern in application code becomes
  the backpressure. Each send completes before the next one starts.
- Sibling response queue overflow: hard drop with `_sibling_response_queue_high_water`
  warning. Does not crash. Lost messages are a known bounded risk for
  chains deeper than `SIBLING_RESPONSE_QUEUE_DEPTH`.

### Verdict: OPENLCBCLIB IS MORE PREDICTABLE
OpenLcbCLib cannot exhaust an unbounded pool. Worst case is a dropped
deep-chain response, which is monitorable. OpenMRN's backpressure is
implicit and harder to reason about under load.

---

## Category 8: Protocol Coverage Between Siblings

| Protocol | OpenMRN | OpenLcbCLib | Status |
|----------|---------|-------------|--------|
| Datagram (addressed) | Per-node DatagramImpl handler | Main SM dispatch + `does_node_process_msg()` | Equivalent |
| Memory Config | MemoryConfig.hxx per-node registry | Address-space dispatch via datagram handler | Equivalent |
| Train commands | TractionCmdHandler per-node | Address-space dispatch, openlcb_application_train.c | Equivalent |
| Event P/C query | EventHandler registry per-node | Enumerate via main SM sibling dispatch | Equivalent |
| Stream | StreamSender per-node | Address-space dispatch, partial coverage | Partial |
| SNIP | Per-node SNIP handler | Main SM sibling dispatch | Equivalent |

**Stream:** The dispatch mechanism correctly routes addressed stream messages.
No multi-node stream loopback test coverage exists. Not a blocking gap for
current use cases.

---

## Remaining Gaps

### 1. Sibling Response Queue Depth
`SIBLING_RESPONSE_QUEUE_DEPTH = 5` is sufficient for all known protocol
chains (max observed depth 2). If a future protocol generates depth > 5
chains, the 6th response is dropped with a warning.

**Mitigation:** Monitor `_sibling_response_queue_high_water`. Increase to 16
if it reaches 4 in production.

### 2. No Multi-Node Stream Testing
Stream scenarios between co-located virtual nodes have no test coverage.
The dispatch mechanism supports it (addressed messages route correctly)
but behavior under load is unverified.

---

## Test Coverage

**OpenLCB multi-node e2e tests** (`src/openlcb/openlcb_multinode_e2e_Test.cxx`):
- `login_sibling_dispatch_reaches_real_main_handler`
- `sibling_response_queue_two_concurrent_responders`
- `sibling_response_queue_depth_3_sequential_chain`
- `login_and_main_statemachines_concurrent_no_interference`
- `datagram_external_to_sibling_b_bystanders_receive_nothing`
- `sibling_datagram_rejected_reply_routes_back_to_originator`
- `sibling_datagram_bystander_c_fully_isolated`

**CAN multi-node e2e tests** (`src/drivers/canbus/can_multinode_e2e_Test.cxx`):
- `two_nodes_login_with_distinct_aliases`
- `incoming_global_ame_repopulates_local_listener_aliases`
- `self_originated_global_ame_repopulates_listener_and_queues_wire_frames`

---

## Overall Verdict

| Area | Result |
|------|--------|
| CAN alias collision prevention | **Equivalent** |
| CAN control (AME/AMD) | **Equivalent** |
| Login message sibling dispatch | **Equivalent** |
| Global message sibling dispatch | **OpenLcbCLib better** (deterministic ordering) |
| Addressed message routing | **OpenLcbCLib slightly better** (dual self-skip) |
| Buffer model | **OpenLcbCLib better** for embedded (zero allocation) |
| Backpressure | **OpenLcbCLib better** (predictable, monitorable) |
| Protocol coverage | **Equivalent** (stream untested) |

OpenLcbCLib is **functionally equivalent to OpenMRN** for all core virtual
node scenarios. In several areas (determinism, bounded memory, predictable
backpressure) it is better suited to embedded targets. The async architectural
difference reflects a deliberate design choice — not a deficiency.

---

## Related Documents

- `documentation/working_plans/proposed_virtual_train_implementation.md` — CAN layer Categories 1-2
- `OpenLcbCLib_OpenMRN_message_comparison.md` — original pre-implementation comparison

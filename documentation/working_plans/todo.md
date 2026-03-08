# OpenLcbCLib — Remaining Work

Single source of truth. Everything completed is in `archive/`.

---

## COMPLETED

---

### Virtual Node Sibling Dispatch (8 Mar 2026)

**Problem:** When one virtual node sends a message, sibling virtual nodes within the same library instance never see it.

**Solution:** Loopback mechanism — after outgoing message is transmitted, copy it into a newly allocated buffer, mark as `loopback`, push to FIFO head. Self-skip prevents originating node from processing its own loopback. One-level cascade prevention: replies to loopback messages are NOT re-looped.

**Unconditional** — no compile flag. Sibling dispatch is a correctness fix, not an optional add-on. Push-to-head caps concurrent loopback buffers at 1 (2 during two-phase datagram enumerate).

**Files modified:**
- `openlcb_types.h` — `loopback` bit in `openlcb_msg_state_t`
- `openlcb_buffer_fifo.h/.c` — `OpenLcbBufferFifo_push_to_head()`
- `openlcb_main_statemachine.c` — `_loopback_to_siblings()`, self-skip in `does_node_process_msg()`, loopback call in `handle_outgoing_openlcb_message()`
- `openlcb_main_statemachine_Test.cxx` — swapped mock node enumeration to real `OpenLcbNode_get_first/get_next/is_last`, added e2e interface struct with real handle functions
- `openlcb_buffer_fifo_Test.cxx` — 6 push_to_head tests

**Tests:** 20 new tests total (6 FIFO + 11 component-level + 3 end-to-end via `run()`)

**Plan:** `documentation/plans/sibling_dispatch_plan.md`

---

### Periodic Listener Alias Verification (8 Mar 2026)

**Problem:** If a listener node disappears without sending AMR (power loss, crash), the alias table retains a stale alias. Forwarded consist commands silently go to a nonexistent node.

**Solution:** Distributed round-robin prober in `alias_mapping_listener.c`. One entry per call, rate-limited to `USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL` ticks (default 100ms). Returns node_id for caller to build targeted AME. Stale detection: if no AMD reply within `USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS` (default 3s), clear alias. Entry stays registered — next AMD re-populates.

**Files modified:**
- `can_types.h` — `verify_ticks`, `verify_pending` fields in `listener_alias_entry_t`
- `alias_mapping_listener.h/.c` — `ListenerAliasTable_check_one_verification()`, static cursor/tick, all existing functions updated to clear new fields
- `can_main_statemachine.h/.c` — `handle_listener_verification` function pointer and handler, unconditional call in `run()`
- `can_config.c` — wired handler
- `templates/openlcb_user_config.h` — 3 timing constants
- `test/openlcb_user_config.h` — same 3 constants
- `alias_mapping_listener_Test.cxx` — 12 new verification tests

**Tests:** 12 new tests (empty table, unresolved skip, not due, due returns node_id, AMD clears pending, timeout clears alias, rate-limiting, round-robin, flush/unregister/clear_alias clear pending, initialize resets cursor)

**Plan:** `documentation/plans/listener_alias_verification_plan.md`

---

## PLANNED — Future Work

---

### 1. Stream Transport Protocol Handler

**Issue:** All dispatch infrastructure exists (5 MTI cases in main statemachine, 5 function pointers, CAN frame type 7 routing, STREAM buffer type). The actual handler module is missing.

**Scope:**

New module `protocol_stream_handler.c/.h` with DI interface struct following `protocol_datagram_handler.h` pattern.

**Protocol summary (StreamTransportS):**
- 5 MTIs: Init Request (0x0CC8), Init Reply (0x0868), Data Send (0x1F88), Data Proceed (0x0888), Data Complete (0x08A8)
- SID/DID stream IDs (0-254, 0xFF reserved)
- Flow control: source proposes buffer size, destination may reduce, source sends up to buffer size bytes then pauses, destination sends Proceed to extend window
- CAN: Stream Data Send uses frame type 7, all other stream messages use normal addressed framing

**Implementation steps:**
1. Add `stream_connection_t` types to `openlcb_types.h` (state, SID, DID, aliases, buffer size, byte count, window)
2. New `protocol_stream_handler.h` with interface struct (lock/unlock, buffer alloc, 6 application callbacks)
3. Implement `protocol_stream_handler.c` — 5 MTI handlers + source-side send API
4. Fix CAN TX dispatch: move Init/Reply/Proceed/Complete to default (addressed framing), keep only `MTI_STREAM_SEND` in stream case
5. Implement CAN RX `stream_frame()` — frame type 7 to OpenLCB FIFO
6. Wire in `openlcb_config.c` under `OPENLCB_COMPILE_STREAM` flag
7. Add stream error codes to `openlcb_defines.h`
8. Tests: dest-side (init accept/reject, data receive, flow control, complete), source-side (open, accept/reject reply, proceed, send, close), edge cases (table full, unknown ID, duplicate init)
9. CMakeLists.txt updates

**Note:** Spec section 8.3 contradicts section 7.1 on which stream ID is in CAN data byte 0. TN section 2.4.1 confirms Destination Stream ID. Follow normative table (Section 7.1): byte 0 = DID.

---

### 2. OlcbChecker Train Control Tests (TR090-TR130)

**Issue:** OlcbChecker has 8 train control tests (TR010-TR080) but no coverage for controller assignment, reserve/release, heartbeat, or consist forwarding.

**Target directory:** `/Users/jimkueneman/Documents/OlcbChecker/`

#### TR090: Controller Configuration
- Query with no controller → empty reply
- Assign our node → accept (3 bytes)
- Query → our node ID returned
- Re-assign same → accept
- Assign different → accept or reject; if rejected, verify 9-byte reply with 6-byte Node ID
- Release → no reply
- Query → empty again

#### TR100: Reserve/Release
- Reserve → succeed
- Double-reserve → reject
- Release → no reply
- Reserve again → succeed

#### TR110: Heartbeat
- Assign controller, send NOOP, wait up to 60s for heartbeat request
- If received: respond with NOOP
- If timeout: informational skip (heartbeat period is implementation-defined)

#### TR120: Consist Speed Forwarding (source-skip test)
- Attach checker as listener with REVERSE flag
- Set speed → verify no forwarded message arrives (source-skip rule)
- Query speed → verify train acted on it
- Detach

#### TR130: Consist Function/EStop Forwarding (source-skip test)
- Attach with LINK_F0 + LINK_FN flags
- Set F0, send E-stop → verify source-skip, verify state via query
- Re-attach without LINK_F0 → verify flag filtering

**Limitation:** Full forwarding verification (P bit, direction reversal, chained consists) requires a second alias identity on the bus or multi-node test harness. Noted as future enhancement.

**Updates:** Add all 5 tests to `control_traincontrol.py` menu and `checkAll()`.

---

### 3. TCP/IP Transport Protocol Handler (Draft)

**Issue:** The OpenLCB specification defines a TCP/IP transport layer alongside CAN but it remains in draft status. The current library only supports CAN transport. Adding TCP/IP would enable direct Ethernet/WiFi connectivity without CAN hardware.

**Status:** Draft specification — implementation deferred until spec stabilizes.

**Scope:**

New transport driver directory `src/drivers/tcpip/` with DI interface structs following the `src/drivers/canbus/` pattern.

**Protocol summary (draft):**
- Binary TCP framing: 2-byte length prefix + OpenLCB common message format
- No alias negotiation — uses full 48-bit Node IDs on the wire
- One persistent TCP connection per link partner
- Gateway mode: CAN↔TCP bridge translates aliases to/from full Node IDs

**Implementation steps (preliminary — subject to spec changes):**
1. Define `tcpip_types.h` — connection state, framing structs
2. New `tcpip_rx_handler.c/.h` — TCP frame reassembly, push to OpenLCB FIFO
3. New `tcpip_tx_handler.c/.h` — OpenLCB buffer to TCP frame serialization
4. New `tcpip_main_statemachine.c/.h` — connection management, keepalive
5. New `tcpip_config.c/.h` — DI wiring (socket send/recv callbacks from application)
6. Wire in `openlcb_config.c` under `OPENLCB_COMPILE_TCPIP` flag
7. Tests
8. CMakeLists.txt updates

**Note:** Draft specification — review latest spec status before beginning implementation.

---

## Design Notes

- **Reply Pending bit (0x80):** ALWAYS set in Datagram Received OK. Write Reply ALWAYS sent. Intentional, spec-compliant. The `bool reply_pending` parameter was removed — do not re-add. See memory file `reply-pending.md`.
- **Event transport:** No send-side API. Applications use `OpenLcbApplication_send_event_pc_report()`.
- **Consumer event status:** Not auto-updated on incoming PCER — left to application callbacks by design.
- **Datagram content types:** Only 0x20 (Config Memory) handled; others rejected. Correct for current scope.
- **Lock/Reserve:** Advisory per spec; handlers do not enforce it.
- **DI pattern:** Optional modules use nullable function pointers. NULL = not linked in, call skipped.
- **Fire-and-forget forwarding:** Train commands to listeners have no acknowledgment. Periodic verification detects stale aliases and clears them so commands are dropped rather than sent to a dead node.
- **Periodic verification:** Distributed round-robin prober, one AME per configurable tick interval. Runs unconditionally at top of `CanMainStateMachine_run()` (like `check_timeouts`). User-configurable timing via 3 constants in `openlcb_user_config.h`.
- **Sibling dispatch:** Unconditional, no feature flag. Push-to-head caps concurrent loopback at 1-2 buffers. `assert()` on pool exhaustion. See `documentation/plans/sibling_dispatch_plan.md`.

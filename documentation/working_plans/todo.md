# OpenLcbCLib — Remaining Work

Single source of truth. Everything completed is in `archive/`.

---

## PLANNED — Future Work

---

### 1. Review Archive Plans Against Current Reality

**Issue:** The archive folder contains ~30 plan documents written during development. Some may describe designs that were later changed, abandoned, or superseded. Before relying on them as reference material, each should be checked against the current codebase.

**Scope:** For each file in `documentation/working_plans/archive/`:
- Read the plan and identify the key design decisions or implementation details it describes
- Check those details against the current source code
- If the implementation matches the plan: add a note at the top confirming it (e.g. `<!-- Verified against codebase 15 Mar 2026 — matches -->`)
- If the implementation diverged: document what actually exists and why the plan was changed or abandoned
- If the plan was never implemented: mark it clearly as abandoned with a reason

**Priority files to check first (known areas of change):**
- `plan_item_3_consist_forwarding.md` — no COMPLETE suffix, unclear status
- `plan_item_5_controller_assign_reject.md` — no COMPLETE suffix, unclear status
- `plan_listener_alias_table_integration.md` — no COMPLETE suffix, unclear status
- `plan_olcbchecker_train_control_tests.md` — TR090-110 done, TR120-130 deferred
- `plan_stream_transport.md` — stream spec not released, deferred
- `OLCBChecker_New_Tests.md` — check against ChecksToAdd.md for current status
- `compliance_test_node_implementation.md` — check if compliance node is in the repo

---

### 2. Stream Transport Protocol Handler

**Issue:** All dispatch infrastructure exists (5 MTI cases in main statemachine, 5 function pointers, CAN frame type 7 routing, STREAM buffer type). The actual handler module is missing.

**Status:** Deferred — Stream Transport specification has not been officially released by the OpenLCB standards group. Implementation will begin once the spec is stable.

**When spec is released, see:** `archive/plan_stream_transport.md` for the full implementation plan.

**Note:** Spec section 8.3 contradicts section 7.1 on which stream ID is in CAN data byte 0. TN section 2.4.1 confirms Destination Stream ID. Follow normative table (Section 7.1): byte 0 = DID.

---

### 3. TCP/IP Transport Protocol Handler

**Issue:** The OpenLCB specification defines a TCP/IP transport layer alongside CAN but it remains in draft status.

**Status:** Deferred — draft specification, implementation deferred until spec stabilizes.

**Scope when ready:** New transport driver directory `src/drivers/tcpip/` following the `src/drivers/canbus/` DI pattern.

---

### 4. Broadcast Time Helpers

**Issue:** App users should not have to hunt down what Event IDs they need to register for producers and consumers for common clocks.

**Scope:** Helper functions that register the correct event ID ranges for Broadcast Time producer and consumer roles, so the application only needs to call one function rather than manually registering individual event IDs.

---

### 5. OlcbChecker Consist Forwarding Tests (TR120, TR130)

**Issue:** TR120 (Consist Speed Forwarding source-skip) and TR130 (Consist Function/EStop Forwarding source-skip) require a second alias identity on the bus — a multi-node test harness not currently available.

**Status:** Deferred pending multi-node test harness capability.

**Detail:** See `archive/plan_olcbchecker_train_control_tests.md` and `OlcbChecker/ChecksToAdd.md`.

---

### 6. OlcbChecker — Additional Checks from ChecksToAdd.md

Outstanding items in `OlcbChecker/ChecksToAdd.md` not yet implemented:
- Multi-PIP and multi-SNIP message capacity cases
- PIP Simple bit vs Verify Node ID Simple bit cross-check
- Datagram error and overlap handling (4 cases)
- Memory Configuration error handling (section 4.3)
- ACDI user space writeback verification
- Train Control subsection 8–11 detailed plan and implementation
- Memory Configuration client-side checking (dummy server)
- Multi-alias AMD response checks
- Firmware Update check plan implementation
- Broadcast Time check plan completion (check_bt scripts exist but plan notes gaps)
- Three-PIP and three-SNIP message cases
- Frame section 3 message level checks
- FDI read-only address-space reply handling

---

### 7. Virtual Node CAN Login — Duplicate Alias Detection Gap

**Issue:** During CAN node login, virtual nodes do not get passed through the login sequence, so they cannot reply to a duplicate alias detection challenge. In practice this should never happen (virtual nodes share the physical node's alias space and the physical node handles alias negotiation), but there is no defensive check to catch it if it ever does.

**Proposed approach:** Rather than relying on the login message flow, directly scan the alias field in all node structs to detect a collision. This avoids the need to route CAN login frames through virtual nodes at all.

**Date noted:** 2026-03-16

---

## Design Notes

These are settled decisions. Do not revisit without good reason.

- **Reply Pending bit (0x80):** ALWAYS set in Datagram Received OK. Write Reply ALWAYS sent. Intentional, spec-compliant. The `bool reply_pending` parameter was removed — do not re-add. See `archive/reply-pending.md`.
- **Event transport:** No send-side API. Applications use `OpenLcbApplication_send_event_pc_report()`.
- **Consumer event status:** Not auto-updated on incoming PCER — left to application callbacks by design.
- **Datagram content types:** Only 0x20 (Config Memory) handled; others rejected. Correct for current scope.
- **Lock/Reserve:** Advisory per spec; handlers do not enforce it.
- **DI pattern:** Optional modules use nullable function pointers. NULL = not linked in, call skipped.
- **Fire-and-forget forwarding:** Train commands to listeners have no acknowledgment. Periodic verification detects stale aliases and clears them so commands are dropped rather than sent to a dead node.
- **Periodic verification:** Distributed round-robin prober, one AME per configurable tick interval. Runs unconditionally at top of `CanMainStateMachine_run()`. User-configurable timing via 3 constants in `openlcb_user_config.h`.
- **Sibling dispatch:** Unconditional, no feature flag. Push-to-head caps concurrent loopback at 1-2 buffers. `assert()` on pool exhaustion. See `archive/sibling_dispatch_plan.md`.
- **Arduino mode:** Set automatically by platform selection in the Node Wizard. There is no Arduino checkbox. ESP32 (TWAI and WiFi GridConnect) and RP2040 are Arduino; STM32, MSPM0, and None/Custom are non-Arduino.
- **Stream and TCP/IP:** Both deferred pending official spec release. Infrastructure stubs exist but handler modules are intentionally absent.

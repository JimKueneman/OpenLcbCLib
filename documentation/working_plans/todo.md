# OpenLcbCLib — Remaining Work

Single source of truth. Everything completed is in `archive/`.

---

## OPEN — Active / Future Work

---

### 1. Stream Transport Protocol Handler

**Issue:** All dispatch infrastructure exists (5 MTI cases in main statemachine, 5 function pointers, CAN frame type 7 routing, STREAM buffer type). The actual handler module is missing.

**Status:** Deferred — Stream Transport specification has not been officially released by the OpenLCB standards group. Implementation will begin once the spec is stable.

**When spec is released, see:** `archive/plan_stream_transport.md` for the full implementation plan.

**Note:** Spec section 8.3 contradicts section 7.1 on which stream ID is in CAN data byte 0. TN section 2.4.1 confirms Destination Stream ID. Follow normative table (Section 7.1): byte 0 = DID.

---

### 2. TCP/IP Transport Protocol Handler

**Issue:** The OpenLCB specification defines a TCP/IP transport layer alongside CAN but it remains in draft status.

**Status:** Deferred — draft specification, implementation deferred until spec stabilizes.

**Scope when ready:** New transport driver directory `src/drivers/tcpip/` following the `src/drivers/canbus/` DI pattern.

---

### 3. Broadcast Time Helpers

**Issue:** App users should not have to hunt down what Event IDs they need to register for producers and consumers for common clocks.

**Scope:** Helper functions that register the correct event ID ranges for Broadcast Time producer and consumer roles, so the application only needs to call one function rather than manually registering individual event IDs.

---

### 4. OlcbChecker Consist Forwarding Tests (TR120, TR130)

**Issue:** TR120 (Consist Speed Forwarding source-skip) and TR130 (Consist Function/EStop Forwarding source-skip) require a second alias identity on the bus — a multi-node test harness not currently available.

**Status:** Deferred pending multi-node test harness capability.

**Detail:** See `archive/plan_olcbchecker_train_control_tests.md` and `OlcbChecker/ChecksToAdd.md`.

---

### 5. OlcbChecker — Additional Checks from ChecksToAdd.md

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

### 7. Dead Code / Dead Type Audit

**Issue:** At least two dead structs found during bootloader work: `openlcb_statemachine_worker_t` (in `openlcb_types.h`) and `can_main_statemachine_t` (in `can_types.h`) — both removed. Likely more exist.

**Scope:** Systematically search the codebase for typedef'd structs, enums, and typedefs that are never instantiated or referenced in any `.c` file. Remove confirmed dead code.

---

### 8. CDI/FDI highest_address Should Use USER_DEFINED_CDI_LENGTH / USER_DEFINED_FDI_LENGTH

**Issue:** In all demo and template `openlcb_user_config.c` files, the `address_space_configuration_definition.highest_address` and `address_space_train_function_definition_info.highest_address` are hardcoded byte counts (e.g. `(1098 - 1)`) instead of referencing `USER_DEFINED_CDI_LENGTH` or `USER_DEFINED_FDI_LENGTH` from the `.h` file. This means the CDI/FDI length must be maintained in two places — the `#define` and the struct initializer — which drift apart.

**Scope:** Update all demo and template `openlcb_user_config.c` files to use `(USER_DEFINED_CDI_LENGTH - 1)` and `(USER_DEFINED_FDI_LENGTH - 1)` for the highest_address fields, or derive the value from the actual CDI byte array size.

---

### 9. Debug Broadcast Time OlcbChecker Failure

**Issue:** The OlcbChecker Broadcast Time tests are failing. Root cause not yet investigated.

**Scope:** Run the broadcast-time-consumer and broadcast-time-producer test suites, identify the failing checks, diagnose, and fix.

---

## COMPLETED

---

### 6. ~~Bootloader openlcb_user_config.h — Minimum Array Count Guards~~ DONE

**Resolved:** Added `#if DEFINE < 1 #error` guards in `openlcb_types.h` for all 9
affected defines. Added `#ifdef OPENLCB_COMPILE_STREAM` guard so `LEN_MESSAGE_BYTES_STREAM`
collapses to 1 when stream is not compiled in (avoids 256-byte waste; comment explains
why). Added trailing comment to `USER_DEFINED_STREAM_BUFFER_LEN` in all 13
`openlcb_user_config.h` files (template + 10 app demos + 2 test configs + compliance node).

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
- **Sibling dispatch:** Zero-FIFO sequential dispatch. After sending an outgoing message to the wire, the run loop shows it to each sibling one at a time — zero buffer allocation, zero FIFO copies. All 3 phases implemented: Phase 1 (main SM), Phase 2 (login SM), Phase 3 (CAN alias collision prevention + AME listener repopulate). See `plan_unified_sibling_dispatch.md`.
- **Sibling dispatch and CAN Rx buffer pressure:** Sequential dispatch occupies the `_run()` loop for N steps (one per sibling). This is pure CPU work (~10μs per step), so 50 siblings ≈ 500μs — less than one CAN frame time at 125kbps (~1ms). The CAN Rx ISR continues draining the hardware buffer into the software FIFO independently, so frames are not lost. Login messages (Init Complete, P/C Identified) are infrequent one-time events. The only stress case is a burst of external messages during a large sibling dispatch, absorbed by the software FIFO. Accepted as-is — no mitigation needed.
- **Arduino mode:** Set automatically by platform selection in the Node Wizard. There is no Arduino checkbox. ESP32 (TWAI and WiFi GridConnect) and RP2040 are Arduino; STM32, MSPM0, and None/Custom are non-Arduino.
- **Stream and TCP/IP:** Both deferred pending official spec release. Infrastructure stubs exist but handler modules are intentionally absent.

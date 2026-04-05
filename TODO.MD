# OpenLcbCLib — Remaining Work

Single source of truth. Everything completed is in `archive/`.

---

## OPEN — Active / Future Work

---

### 2. TCP/IP Transport Protocol Handler

**Issue:** The OpenLCB specification defines a TCP/IP transport layer alongside CAN but it remains in draft status.

**Status:** Deferred — draft specification, implementation deferred until spec stabilizes.

**Scope when ready:** New transport driver directory `src/drivers/tcpip/` following the `src/drivers/canbus/` DI pattern.

---

---

---

## COMPLETED

---

### 3. ~~Broadcast Time Helpers~~ DONE

**Resolved:** Added `OpenLcbApplicationBroadcastTime_make_clock_id()` helper that constructs
a properly formatted clock ID from a raw 48-bit unique identifier. Well-known clocks already
had `#define`d constants; `setup_consumer`/`setup_producer` already handled event range
registration. The missing piece was custom clock ID construction.

---

### 8. ~~Debug Broadcast Time OlcbChecker Failure~~ DONE

**Resolved:** Broadcast Time OlcbChecker tests now pass with latest changes.

---

### 7. ~~Dead Code / Dead Type Audit~~ DONE

**Resolved:** Dead structs `openlcb_statemachine_worker_t` and `can_main_statemachine_t` removed. Audit complete.

---

### 8. ~~CDI/FDI highest_address Should Use sizeof()~~ DONE

**Resolved:** Compliance node `openlcb_user_config.c` now uses `sizeof(_cdi_data) - 1`
and `sizeof(_fdi_data) - 1` for address space `highest_address` fields instead of
hardcoded byte counts. Values stay correct automatically when the XML changes.

---

### 6. ~~Bootloader openlcb_user_config.h — Minimum Array Count Guards~~ DONE

**Resolved:** Added `#if DEFINE < 1 #error` guards in `openlcb_types.h` for all 9
affected defines. Added `#ifdef OPENLCB_COMPILE_STREAM` guard so `LEN_MESSAGE_BYTES_STREAM`
collapses to 1 when stream is not compiled in (avoids 256-byte waste; comment explains
why). Added trailing comment to `USER_DEFINED_STREAM_BUFFER_LEN` in all 13
`openlcb_user_config.h` files (template + 10 app demos + 2 test configs + compliance node).

---

### 1. ~~Stream Transport Protocol Handler~~ DONE

**Resolved:** Full stream transport implemented: `protocol_stream_handler.c/.h` (Layer 1
transport with source and destination roles, flow control, content UIDs),
`protocol_config_mem_stream_handler.c/.h` (Layer 2 config-memory streaming reads/writes),
CAN stream frame routing in `can_rx_statemachine.c` and `can_tx_statemachine.c`, and full
unit test suites for both layers. Spec section 7.1 normative table followed: CAN data byte 0 = DID.
See `documentation/working_plans/archive/stream_plan.md`.

---

### 9. ~~Standalone C Bootloader~~ DONE

**Resolved:** Standalone datagram-only bootloader implemented in `bootloader/` with
triple CRC-16-IBM validation, platform demos for dsPIC, STM32, TI, and ESP32.
Polled operation, no OS dependency. See `documentation/working_plans/archive/standalone_bootloader_plan.md`.

---

### 11. ~~Config Memory Concurrent Streams~~ DONE

**Resolved:** Context pooling implemented in `protocol_config_mem_stream_handler.c`.
Uses `_context_pool[USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS]` with `_allocate_context()`
and `_find_context_by_stream()` pool lookup. See `documentation/working_plans/archive/config_mem_concurrent_streams_analysis.md`.

---

### 10. ~~DCC Detector Protocol~~ DONE

**Resolved:** DCC detector protocol implemented: `openlcb_application_dcc_detector.c/.h`
with full unit test suite (`openlcb_application_dcc_detector_Test.cxx`). Application
integration complete.

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
- **Periodic verification:** Distributed round-robin prober, one AME per configurable tick interval. Runs unconditionally at top of `CanMainStatemachine_run()`. User-configurable timing via 3 constants in `openlcb_user_config.h`.
- **Sibling dispatch:** Zero-FIFO sequential dispatch. After sending an outgoing message to the wire, the run loop shows it to each sibling one at a time — zero buffer allocation, zero FIFO copies. All 3 phases implemented: Phase 1 (main SM), Phase 2 (login SM), Phase 3 (CAN alias collision prevention + AME listener repopulate). See `plan_unified_sibling_dispatch.md`.
- **Sibling dispatch and CAN Rx buffer pressure:** Sequential dispatch occupies the `_run()` loop for N steps (one per sibling). This is pure CPU work (~10μs per step), so 50 siblings ≈ 500μs — less than one CAN frame time at 125kbps (~1ms). The CAN Rx ISR continues draining the hardware buffer into the software FIFO independently, so frames are not lost. Login messages (Init Complete, P/C Identified) are infrequent one-time events. The only stress case is a burst of external messages during a large sibling dispatch, absorbed by the software FIFO. Accepted as-is — no mitigation needed.
- **Arduino mode:** Set automatically by platform selection in the Node Wizard. There is no Arduino checkbox. ESP32 (TWAI and WiFi GridConnect) and RP2040 are Arduino; STM32, MSPM0, and None/Custom are non-Arduino.
- **Stream:** Fully implemented (transport layer + config-mem streaming). Single concurrent stream only; pooling for multiple concurrent streams deferred. See concurrent streams analysis.
- **TCP/IP:** Deferred pending official spec release. Infrastructure stubs exist but handler module is intentionally absent.
